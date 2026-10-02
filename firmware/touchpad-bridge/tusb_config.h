/*
 * TinyUSB stack configuration
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Michael Lloyd
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

// CFG_TUSB_MCU and CFG_TUSB_OS are normally supplied by the pico-sdk TinyUSB
// integration but we'll define them anyway for later porting work. 
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU OPT_MCU_RP2040
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS  OPT_OS_PICO
#endif

#define CFG_TUD_ENABLED         1
#define CFG_TUD_ENDPOINT0_SIZE  64

// root-hub port 0 runs the device stack at full speed 
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#define CFG_TUD_HID 1
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0

// Sizes the HID input buffer and the control buffer for GET/SET_REPORT, 
// so it must hold the largest feature report. 
// Windows reads that blob and the device capabilities before it starts 
// the Precision Touchpad stack.
#define CFG_TUD_HID_EP_BUFSIZE  260

#endif // _TUSB_CONFIG_H_
