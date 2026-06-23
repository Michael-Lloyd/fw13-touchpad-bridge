/*
 * HID over I2C host transport. See the Microsoft HID over I2C 
 * protocol spec for the register and command layout. 
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#include <string.h>
#include <pico/stdlib.h>

#include <i2c_hid.h>

// opcodes occupy the low half byte of comamnd word's second byte 
#define I2C_HID_OP_RESET        0x01
#define I2C_HID_OP_GET_REPORT   0x02
#define I2C_HID_OP_SET_REPORT   0x03
#define I2C_HID_OP_SET_POWER    0x08

#define I2C_HID_POWER_ON        0x00
#define I2C_HID_POWER_OFF       0x01

// Scratch buffer large enough for device input reports (PTP is 35 bytes),
// plus a timeout long enough for a reset to complete.
#define I2C_HID_INPUT_SCRATCH    64
#define I2C_HID_RESET_TIMEOUT_MS 500

// write a little endian 16-bit register pointer then read len number of bytes 
static int reg_read(struct i2c_hid_dev *dev, uint16_t reg, uint8_t *buf, size_t len) {

    uint8_t p[2] = { (uint8_t)(reg & 0xff), (uint8_t)(reg >> 8) };

    if (i2c_write_blocking(dev->i2c, dev->addr, p, 2, true) != 2) {
        return -1;
    }

    int n = i2c_read_blocking(dev->i2c, dev->addr, buf, len, false);

    return (n == (int)len) ? 0 : -1;
}

// Issue a command-register command: write [command_reg LE] then two command
// bytes: byte0 = (reportType << 4) | reportID, byte1 = opcode.
static int cmd_write(struct i2c_hid_dev *dev, uint8_t b0, uint8_t b1) {

    uint8_t c[4] = {
        (uint8_t)(dev->desc.command_reg & 0xff),
        (uint8_t)(dev->desc.command_reg >> 8),
        b0, b1,
    };

    if (i2c_write_blocking(dev->i2c, dev->addr, c, sizeof(c), false) != (int)sizeof(c)) {
        return -1;
    }
    return 0;
}

void i2c_hid_dev_init(struct i2c_hid_dev *dev, i2c_inst_t *i2c, uint8_t addr) {
    memset(dev, 0, sizeof(*dev));
    dev->i2c = i2c;
    dev->addr = addr;
}

int i2c_hid_read_hid_desc(struct i2c_hid_dev *dev, uint16_t desc_reg) {

    uint8_t raw[30];

    if (reg_read(dev, desc_reg, raw, sizeof(raw)) != 0) {
        return -1;
    }

    struct i2c_hid_desc *d = &dev->desc;
    // unpack the little endian 16-bit fields manually. The raw buffer 
    // is not guaranteed to be aligned for a struct cast ):
    d->hid_desc_len    = (uint16_t)(raw[0]  | raw[1]  << 8);
    d->bcd_version     = (uint16_t)(raw[2]  | raw[3]  << 8);
    d->report_desc_len = (uint16_t)(raw[4]  | raw[5]  << 8);
    d->report_desc_reg = (uint16_t)(raw[6]  | raw[7]  << 8);
    d->input_reg       = (uint16_t)(raw[8]  | raw[9]  << 8);
    d->max_input_len   = (uint16_t)(raw[10] | raw[11] << 8);
    d->output_reg      = (uint16_t)(raw[12] | raw[13] << 8);
    d->max_output_len  = (uint16_t)(raw[14] | raw[15] << 8);
    d->command_reg     = (uint16_t)(raw[16] | raw[17] << 8);
    d->data_reg        = (uint16_t)(raw[18] | raw[19] << 8);
    d->vendor_id       = (uint16_t)(raw[20] | raw[21] << 8);
    d->product_id      = (uint16_t)(raw[22] | raw[23] << 8);
    d->version_id      = (uint16_t)(raw[24] | raw[25] << 8);

    return 0;
}

int i2c_hid_read_report_desc(struct i2c_hid_dev *dev, uint8_t *buf, 
                             uint16_t max, uint16_t *out_len) {

    uint16_t len = dev->desc.report_desc_len;

    if (len == 0 || len > max) {
        return -1;
    }

    if (reg_read(dev, dev->desc.report_desc_reg, buf, len) != 0) {
        return -1;
    }

    *out_len = len;

    return 0;
}

// Reset the device and wait for it to report completion.
int i2c_hid_reset(struct i2c_hid_dev *dev) {
    if (cmd_write(dev, 0x00, I2C_HID_OP_RESET) != 0) {
        return -1;
    }

    // device signals completion by asserting INT and returning a 0-len input
    // report. poll the (default) input reg until that appears or we time out.
    absolute_time_t deadline = make_timeout_time_ms(I2C_HID_RESET_TIMEOUT_MS);
    do {
        uint8_t len[2];
        if (i2c_read_blocking(dev->i2c, dev->addr, len, sizeof(len), false) == 2
            && len[0] == 0 && len[1] == 0) {
            return 0;
        }
        sleep_ms(2);
    } while (!time_reached(deadline));

    return -1;
}

int i2c_hid_set_power(struct i2c_hid_dev *dev, bool on) {
    uint8_t state = I2C_HID_POWER_OFF;
    if (on) {
        state = I2C_HID_POWER_ON;
    }
    return cmd_write(dev, state, I2C_HID_OP_SET_POWER);
}

int i2c_hid_get_input(struct i2c_hid_dev *dev, uint8_t *buf,
                      uint16_t max, uint16_t *out_len) {

    // case 1: read returns input register [len lo][len hi][report ...] 
    // case 2: total len == 0 means no report pending (reset/idle condition)
    uint8_t raw[I2C_HID_INPUT_SCRATCH];
    uint16_t want = dev->desc.max_input_len;
    if (want < 2 || want > sizeof(raw)) {
        want = sizeof(raw);
    }

    int n = i2c_read_blocking(dev->i2c, dev->addr, raw, want, false);
    if (n < 2) {
        return -1;
    }

    uint16_t total = (uint16_t)(raw[0] | raw[1] << 8);
    if (total == 0) {
        *out_len = 0;
        return 0;
    }
    if (total < 2 || total > (uint16_t)n || (uint16_t)(total - 2) > max) {
        return -1;
    }

    *out_len = (uint16_t)(total - 2);
    memcpy(buf, raw + 2, *out_len);

    return 0;
}

int i2c_hid_get_report(struct i2c_hid_dev *dev, uint8_t type, uint8_t report_id,
                       uint8_t *buf, uint16_t max, uint16_t *out_len) {
    // TODO: command-register GET_REPORT sequence (type and report id),
    // then read the result from the data register.
    (void)dev;
    (void)type;
    (void)report_id;
    (void)buf;
    (void)max;
    (void)out_len;
    return -1;
}

int i2c_hid_set_report(struct i2c_hid_dev *dev, uint8_t type, uint8_t report_id,
                       const uint8_t *buf, uint16_t len) {
    // TODO: command-register SET_REPORT sequence writing into the data
    // register.
    (void)dev;
    (void)type;
    (void)report_id;
    (void)buf;
    (void)len;
    return -1;
}
