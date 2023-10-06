#include "pico/libsdemu.h"

#include "pico/stdlib.h"
//#include "hardware/spi.h"

uint8_t _spi0_tx_idle_value = 0xFF;
uint8_t _spi1_tx_idle_value = 0xFF;
void _set_spi_tx_idle_value(spi_inst_t* spi, uint8_t value) {
    if (spi == spi0) {
        _spi0_tx_idle_value = value;
    } else if (spi == spi1) {
        _spi1_tx_idle_value = value;
    } else {
        // TODO: Log failure, spin forever?
        while(1);
    }
}

inline static uint8_t _get_spi_tx_idle_value(spi_inst_t* spi) {
    if (spi == spi0) {
        return _spi0_tx_idle_value;
    } else if (spi == spi1) {
        return _spi1_tx_idle_value;
    } else {
        // TODO: Log failure, spin forever?
        while(1);
    }
}

inline static void _spi_irq_handler(spi_inst_t* spi) {
    while(spi_get_hw(spi)->ris & 0b1000) {
        spi_get_hw(spi)->dr = _get_spi_tx_idle_value(spi);
    }
}
void _spi0_irq_handler() {
    _spi_irq_handler(spi0);
}
void _spi1_irq_handler() {
    _spi_irq_handler(spi1);
}

void _sd_enable_irq(spi_inst_t* spi, bool enable) {
    uint8_t irq_num;
    if (spi == spi0) {
        irq_num = SPI0_IRQ;
        irq_set_exclusive_handler(irq_num, &_spi0_irq_handler);
    } else if (spi == spi1) {
        irq_num = SPI1_IRQ;
        irq_set_exclusive_handler(irq_num, &_spi1_irq_handler);
    } else {
        while(1);
    }
    irq_set_enabled(irq_num, enable);
    spi_get_hw(spi)->imsc = SPI_SSPIMSC_TXIM_BITS;
}

void setup_sd_emu(spi_inst_t* spi) {
    // TODO: does the speed matter here?
    spi_init(spi, 1000*1000);

    // From the SPI section of the datasheet, the SPI controller defaults to using the 
    // Motorla SPI format, which only allows continuous transfers with chip select held
    // low across multiple frames if the clock phase is set to 1.
    // We must therefore also set the clock polarity to 1 to properly support SD transfers
    // (where both are 0)
    spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_set_slave(spi, true);
    _sd_enable_irq(spi, true);
}