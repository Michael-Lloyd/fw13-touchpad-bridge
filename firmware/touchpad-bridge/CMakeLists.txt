# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Michael Lloyd

cmake_minimum_required(VERSION 3.13)

# Target the RP2350A
set(PICO_BOARD pico2 CACHE STRING "Board type")
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Pico platform")

include(pico_sdk_import.cmake)

project(touchpad_bridge C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(touchpad_bridge
    src/main.c
    src/status.c
    src/i2c_hid.c
    src/touchpad.c
    src/usb_hid.c
    src/usb_descriptors.c
)

# include/ for headers 
target_include_directories(touchpad_bridge PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(touchpad_bridge PRIVATE
    pico_stdlib
    hardware_i2c
    pico_unique_id
    tinyusb_device
    tinyusb_board
)

# Debug stream on UART0 and USB for touchpad interface
pico_enable_stdio_uart(touchpad_bridge 1)
pico_enable_stdio_usb(touchpad_bridge 0)

pico_add_extra_outputs(touchpad_bridge)
