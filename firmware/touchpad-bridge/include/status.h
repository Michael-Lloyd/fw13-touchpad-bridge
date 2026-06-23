/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */
#ifndef _STATUS_H_
#define _STATUS_H_

enum status_state {
    STATUS_OK,      // enumerated and operating
    STATUS_WAIT,    // up, waiting for host
    STATUS_ERROR,   // touchpad bring-up failed
};

void status_init(void);
void status_set(enum status_state state);
void status_task(void);

#endif // _STATUS_H_
