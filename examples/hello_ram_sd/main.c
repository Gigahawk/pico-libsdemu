#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/libsdemu.h"

void init_sd() {
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    setup_sd_emu(spi0);
}

int main() {

    stdio_init_all();
    while(!stdio_usb_connected());

    init_sd();

    printf("Hello, world\n");
    return 0;
}