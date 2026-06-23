/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#ifndef _I2C_HID_H_
#define _I2C_HID_H_

#include <stdint.h>
#include <stdbool.h>
#include <hardware/i2c.h>

// parsed I2C HID descriptor, the 30 byte block at the descriptor register 
struct i2c_hid_desc {
    uint16_t hid_desc_len;
    uint16_t bcd_version;
    uint16_t report_desc_len;
    uint16_t report_desc_reg;
    uint16_t input_reg;
    uint16_t max_input_len;
    uint16_t output_reg;
    uint16_t max_output_len;
    uint16_t command_reg;
    uint16_t data_reg;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t version_id;
};

// bound I2C HID device 
struct i2c_hid_dev {
    i2c_inst_t          *i2c;
    uint8_t              addr;
    struct i2c_hid_desc  desc;
};

// HID report types (the wType field of GET_REPORT or SET_REPORT), per HID spec
#define I2C_HID_REPORT_INPUT    0x01
#define I2C_HID_REPORT_OUTPUT   0x02
#define I2C_HID_REPORT_FEATURE  0x03

void i2c_hid_dev_init(struct i2c_hid_dev *dev, i2c_inst_t *i2c, uint8_t addr);
int i2c_hid_read_hid_desc(struct i2c_hid_dev *dev, uint16_t desc_reg);
int i2c_hid_read_report_desc(struct i2c_hid_dev *dev, uint8_t *buf, uint16_t max, 
                             uint16_t *out_len);
int i2c_hid_reset(struct i2c_hid_dev *dev);
int i2c_hid_set_power(struct i2c_hid_dev *dev, bool on);
int i2c_hid_get_input(struct i2c_hid_dev *dev, uint8_t *buf, uint16_t max, uint16_t *out_len);
int i2c_hid_get_report(struct i2c_hid_dev *dev, uint8_t type, uint8_t report_id, 
                       uint8_t *buf, uint16_t max, uint16_t *out_len);
int i2c_hid_set_report(struct i2c_hid_dev *dev, uint8_t type, uint8_t report_id,
                       const uint8_t *buf, uint16_t len);

#endif // _I2C_HID_H_
