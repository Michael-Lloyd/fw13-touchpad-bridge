# Touchpad bridge/adapter firmware

This contains the firmware for the framework 13 touchpad bridge/adapter. The touchpad uses a PixArt PCT3854 HID-over-I2C IC, the report descriptors and feature reports can be passed transparently (including the Microsoft certification blob) unchanged. 

To use this firmware you will need access to the BC-TPI-* board and an appropriately selected 0.3mm pitch ribbon FPC cable to use with it. 

## Building

Requires the pico SDK, an appropriate compile (i.e `arm-none-eabi-gcc`), `cmake`, `ninja` and `picotool` for interacting with the microcontroller. 

To build the binary: 

```bash
# Make sure PICO_SDK_PATH is set 
cmake -B build -G Ninja 
cmake --build build
``` 
