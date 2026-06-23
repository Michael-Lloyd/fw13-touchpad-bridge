/*
 * USB device configuration and string descriptors. The HID report
 * descriptor is sourced from the touchpad at runtime so the host sees 
 * a genuine "Precision Touchpad" rather than untrusted one.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#include <string.h>
#include <tusb.h>
#include <pico/unique_id.h>

#include <touchpad.h>

// NOTE: Unregistered! Change when appropriate to do so. 
#define USB_VID  0x1209
#define USB_PID  0x5450

enum {
    ITF_HID = 0,
    ITF_COUNT
};

#define EPNUM_HID 0x81
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

// offset of wReportLength within configuration descriptor
// config(9) + interface(9) + 7 bytes into the 9-byte HID descriptor
#define HID_REPORT_LEN_OFFSET  (TUD_CONFIG_DESC_LEN + 9 + 7)

static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&desc_device;
}

// HID report descriptor from the touchpad 
const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    uint16_t len;
    return touchpad_report_desc(&len);
}

// configuration descriptor. The HID report descriptor length is patched at 
// runtime from the descriptor read off the touchpad 
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {

    (void)index;
    static uint8_t cfg[CONFIG_TOTAL_LEN];

    uint16_t rlen;
    (void)touchpad_report_desc(&rlen);

    const uint8_t templ[] = {
        TUD_CONFIG_DESCRIPTOR(
                1, ITF_COUNT, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
        TUD_HID_DESCRIPTOR(
                ITF_HID, 0, HID_ITF_PROTOCOL_NONE, 0, EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5),
    };

    memcpy(cfg, templ, sizeof(templ));

    cfg[HID_REPORT_LEN_OFFSET] = (uint8_t)(rlen & 0xff);
    cfg[HID_REPORT_LEN_OFFSET + 1] = (uint8_t)(rlen >> 8);

    return cfg;
}

// USB Language ID for English (United States), returned as string 0.
#define LANGID_EN_US  0x0409

// The serial is the board's unique id, read from the QSPI flash at
// runtime so every unit reports a distinct value.
static char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

static const char *string_desc[] = {
    NULL,                           // 0: language id (handled specially)
    "micl",                         // 1: manufacturer
    "Touchpad Bridge",              // 2: product
    serial,                         // 3: serial (filled from the flash id)
};

// TinyUSB sends the returned descriptor after this call returns, so it
// must persist between calls; assemble it into this static buffer as
// [length+type byte][UTF-16LE code units].
static uint16_t desc_str[32];

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {

    (void)langid;
    uint8_t count;

    // The chip reports its own serial: fill once from the flash unique id.
    if (serial[0] == 0) {
        pico_get_unique_board_id_string(serial, sizeof(serial));
    }

    if (index == 0) {
        desc_str[1] = LANGID_EN_US;
        count = 1;
    } else {
        if (index >= TU_ARRAY_SIZE(string_desc)) {
            return NULL;
        }

        const char *str = string_desc[index];
        count = (uint8_t)strlen(str);

        if (count > 31) {
            count = 31;
        }

        for (uint8_t i = 0; i < count; i++) {
            desc_str[1 + i] = str[i];
        }
    }

    desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * count + 2));

    return desc_str;
}
