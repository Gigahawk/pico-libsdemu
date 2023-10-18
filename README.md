# pico-libsdemu

Emulate a SPI mode SD card slave with an RP2040

## Examples

- [hello_ram_sd](examples/hello_ram_sd/main.c) is a toy example showing the basic functionality of the library using RAM backed storage.

## Resource usage
- 1 SPI bus (configured as a slave)
- Polling loop to wait for commands and respond
    - It is probably possible to have this be triggered by interrupt, but this has not been implemented or tested
