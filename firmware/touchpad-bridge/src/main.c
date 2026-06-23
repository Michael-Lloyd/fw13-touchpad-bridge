/*
 * Entry point for the framework touchpad bridge. Brings up the I2C-HID
 * host link to the touchpad, then starts the USB device stack and
 * relays input reports across the bridge.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#include <pico/stdlib.h>
#include <tusb.h>

#include <status.h>
#include <touchpad.h>

int main(void) {
    stdio_init_all();
    status_init();

    status_set(STATUS_ERROR);
    while (touchpad_init() != 0) {
        status_task();
        sleep_ms(250);
    }

    status_set(STATUS_WAIT);
    tusb_init();

    while (true) {
        // service USB events
        tud_task(); 
        // forward pending touchpad input reports
        touchpad_task();    

        // Solid once the host enumerates us; slow blink while waiting.
        status_set(tud_mounted() ? STATUS_OK : STATUS_WAIT);
        status_task();
    }
}
