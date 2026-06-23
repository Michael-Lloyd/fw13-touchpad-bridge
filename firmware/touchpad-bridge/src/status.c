/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#include <hardware/gpio.h>
#include <pico/stdlib.h>

#include <config.h>
#include <status.h>

// LED toggles every half period while active
#define BLINK_ERROR_MS  100 // ~5 Hz: bring-up failed
#define BLINK_WAIT_MS   500 // ~1 Hz: waiting for the USB host

static enum status_state cur = STATUS_WAIT;
static bool led_on;
static absolute_time_t next_toggle;

// active-high: driving the pin high lights D2.
static void led_write(bool on) {
    led_on = on;
    gpio_put(TP_LED_PIN, on);
}

static uint32_t blink_half_ms(void) {
    if (cur == STATUS_ERROR) {
        return BLINK_ERROR_MS;
    }
    return BLINK_WAIT_MS;
}

void status_init(void) {
    gpio_init(TP_LED_PIN);
    gpio_set_dir(TP_LED_PIN, GPIO_OUT);
    led_write(false);
    next_toggle = make_timeout_time_ms(BLINK_WAIT_MS);
}

void status_set(enum status_state state) {
    if (state == cur) {
        return;
    }
    cur = state;

    // Show the new state at once: solid for OK, otherwise start the blink lit.
    led_write(true);
    if (state != STATUS_OK) {
        next_toggle = make_timeout_time_ms(blink_half_ms());
    }
}

void status_task(void) {
    if (cur == STATUS_OK || !time_reached(next_toggle)) {
        return;
    }
    led_write(!led_on);
    next_toggle = make_timeout_time_ms(blink_half_ms());
}
