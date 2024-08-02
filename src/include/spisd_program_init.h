#include "hardware/pio.h"
#include "spisd.pio.h"

static inline void spisd_program_init(
        PIO pio,
        uint sm_tx, uint offset_tx,
        uint sm_rx, uint offset_rx,
        uint pin_sck, uint pin_tx, uint pin_rx
) {

    pio_sm_config c_tx = spisd_tx_program_get_default_config(offset_tx);
    pio_sm_config c_rx = spisd_rx_program_get_default_config(offset_rx);

    sm_config_set_out_pins(&c_tx, pin_tx, 1);
    sm_config_set_in_pins(&c_tx, pin_sck);
    sm_config_set_in_pins(&c_rx, pin_rx);

    sm_config_set_out_shift(&c_tx, false, false, 8);
    sm_config_set_clkdiv(&c_tx, 1.0);
    sm_config_set_in_shift(&c_rx, false, true, 8);
    sm_config_set_clkdiv(&c_rx, 1.0);

    pio_sm_set_pins_with_mask(pio, sm_tx, (1u << pin_tx), (1u << pin_tx));
    pio_sm_set_pindirs_with_mask(
        pio, sm_tx, (1u << pin_tx), (1u << pin_tx) | (1u << pin_sck));
    pio_sm_set_pindirs_with_mask(
        pio, sm_rx, 0, (1u << pin_rx) | (1u << pin_sck));

    pio_gpio_init(pio, pin_sck);
    pio_gpio_init(pio, pin_tx);
    pio_gpio_init(pio, pin_rx);

    pio_sm_init(pio, sm_tx, offset_tx, &c_tx);
    pio_sm_init(pio, sm_rx, offset_rx, &c_rx);
    pio_sm_set_enabled(pio, sm_tx, true);
    pio_sm_set_enabled(pio, sm_rx, true);
}