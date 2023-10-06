#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/libsdemu.h"

int main() {

    stdio_init_all();
    while(!stdio_usb_connected());

    int val = foo();
    printf("Hello, world %d\n", val);
    return 0;
}