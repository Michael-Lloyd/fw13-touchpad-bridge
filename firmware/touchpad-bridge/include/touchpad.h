/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#ifndef _TOUCHPAD_H_
#define _TOUCHPAD_H_

#include <stdint.h>

// Initialise I2C, enumerate the touchpad over I2C-HID, and cache its
// report descriptor. Returns 0 on success, -1 on failure.
int touchpad_init(void);

// Poll the touchpad for a new input report and forward it to the USB
// host. Called from the main loop; a no-op until USB is mounted.
void touchpad_task(void);

// The cached HID report descriptor read at bring-up. Returns the buffer
// and writes its length to len.
const uint8_t *touchpad_report_desc(uint16_t *len);

// Proxy a GET_REPORT from the USB host into the touchpad. Returns the
// number of bytes written to buf, or -1 on error.
int touchpad_get_report(uint8_t type, uint8_t report_id,
                        uint8_t *buf, uint16_t max);

// Proxy a SET_REPORT from the USB host into the touchpad. Returns 0 on
// success, -1 on error.
int touchpad_set_report(uint8_t type, uint8_t report_id,
                        const uint8_t *buf, uint16_t len);

#endif // _TOUCHPAD_H_
