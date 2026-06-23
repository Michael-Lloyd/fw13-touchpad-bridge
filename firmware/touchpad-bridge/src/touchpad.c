/*
 * Bring-up code for the framework 13 touchpad 
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <tusb.h>

#include <config.h>
#include <i2c_hid.h>
#include <touchpad.h>

static struct i2c_hid_dev tp_dev;
static uint8_t  report_desc[TP_REPORT_DESC_MAX];
static uint16_t report_desc_len;

// Configure the I2C block and INT GPIO for the touchpad.
static void touchpad_gpio_init(void) {
    i2c_init(TP_I2C_INST, TP_I2C_BAUD_HZ);
    gpio_set_function(TP_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(TP_I2C_SCL_PIN, GPIO_FUNC_I2C);
    // external pullups are should be provided on bridge board
    // but if using different hardware please enabvle internal pullups 

    gpio_init(TP_INT_PIN);
    gpio_set_dir(TP_INT_PIN, GPIO_IN);
    // INT is open drain and active low but should have an external pullup 
}

int touchpad_init(void) {

    touchpad_gpio_init();
    i2c_hid_dev_init(&tp_dev, TP_I2C_INST, TP_I2C_ADDR);

    if (i2c_hid_read_hid_desc(&tp_dev, TP_HID_DESC_REG) != 0) {
        return -1;
    }

    if (i2c_hid_read_report_desc(&tp_dev, report_desc, sizeof(report_desc),
                                    &report_desc_len) != 0) {
        return -1;
    }

    if (i2c_hid_set_power(&tp_dev, true) != 0) {
        return -1;
    }

    if (i2c_hid_reset(&tp_dev) != 0) {
        return -1;
    }

    // TODO: cache the certification blob (feature report id 5) here so
    // touchpad_get_report() can answer the host's request instantly.
    //
    return 0;
}

const uint8_t *touchpad_report_desc(uint16_t *len) {
    *len = report_desc_len;
    return report_desc;
}

void touchpad_task(void) {
    if (!tud_hid_ready()) {
        return;
    }

    // INT is active-low: nothing to read while it is high.
    if (gpio_get(TP_INT_PIN)) {
        return;
    }

    uint8_t report[TP_INPUT_REPORT_MAX];
    uint16_t len = 0;
    if (i2c_hid_get_input(&tp_dev, report, sizeof(report), &len) != 0 || len == 0) {
        return;
    }

    // The report's first byte is its id; TinyUSB takes id and payload apart.
    tud_hid_report(report[0], report + 1, (uint16_t)(len - 1));
}

int touchpad_get_report(uint8_t type, uint8_t report_id, uint8_t *buf, uint16_t max) {
    // TODO: serve the cached certification blob for the feature report,
    // otherwise translate to i2c_hid_get_report().
    uint16_t out_len = 0;
    if (i2c_hid_get_report(&tp_dev, type, report_id, buf, max, &out_len) != 0) {
        return -1;
    }
    return (int)out_len;
}

int touchpad_set_report(uint8_t type, uint8_t report_id, const uint8_t *buf, uint16_t len) {
    // Input Mode (feature report id 3) arrives here when the host
    // switches the device into Precision Touchpad mode; it must reach
    // the module for gestures to work.
    return i2c_hid_set_report(&tp_dev, type, report_id, buf, len);
}
