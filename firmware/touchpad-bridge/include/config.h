/*
 * board and build configuration for the RP2350A touchpad bridge,
 * for use with the BC-TPI-A bridge board. Modify otherwise 
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <hardware/i2c.h>

// I2C host link to the Framework touchpad (PixArt PCT3854)
#define TP_I2C_INST         i2c0
#define TP_I2C_SDA_PIN      4
#define TP_I2C_SCL_PIN      5
#define TP_INT_PIN          6
#define TP_I2C_BAUD_HZ      400000
#define TP_I2C_ADDR         0x2c

// I2C HID descriptor register; the pointer at which the device exposes its HID 
// descriptor, found by probing the framework touchpad module. 
// Reverse-engineered by the touchpad-adventure project (MIT), re-derived
// here not copied; Framework's docs omit this register:
//   https://github.com/jeongm-in/touchpad-adventure
#define TP_HID_DESC_REG     0x0020

// status LED D2, active-high
#define TP_LED_PIN          11
// 0 = poll INT level, maybe use 1 = GPIO IRQ
#define TP_USE_INTERRUPT    0
// the PTP report descriptor is 687 bytes 
#define TP_REPORT_DESC_MAX  768         // 687-byte descriptor size from touchpad-adventure
// PTP input report is ~35 bytes 
#define TP_INPUT_REPORT_MAX 64          // ~35-byte report size from touchpad-adventure

#endif // _CONFIG_H_
