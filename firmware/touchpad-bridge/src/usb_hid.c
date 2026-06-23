/*
 * class callbacks for TinyUSB HID. The GET_REPORT and SET_REPORT 
 * are proxied to the touchpad so the certification blob and input mode 
 * negotiation flow through the real device unchanged.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#include <tusb.h>

#include <touchpad.h>

// host -> device GET_REPORT: 
// forward feature/input reads to the touchpad 
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    int n = touchpad_get_report((uint8_t)report_type, report_id, buffer, reqlen);
    if (n < 0) {
        return 0;   // unsupported report: no data, the host sees a stall
    }
    return (uint16_t)n;
}

// host -> device SET_REPORT: 
// forward feature/output writes to the touchpad 
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                           const uint8_t *buffer, uint16_t bufsize) {
    (void)instance;
    touchpad_set_report((uint8_t)report_type, report_id, buffer, bufsize);
}
