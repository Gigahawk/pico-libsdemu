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

extern uint8_t* get_sd_block(uint32_t block_num, bool writable);

void spi_clear_read_buf(spi_inst_t *spi) {
    uint8_t i;
    while(spi_is_readable(spi)) {
        i = (uint8_t)spi_get_hw(spi)->dr;
    }
}

/* For large reads the TX buffer can get emptied, seems like the interrupt doesn't always fire?*/
void spi_read_large(spi_inst_t *spi, uint8_t* dst, size_t len, bool fill) {
    for(size_t i = 0; i < len; ++i) {
        while(!spi_is_readable(spi))
            tight_loop_contents();
        *dst++ = (uint8_t)(spi_get_hw(spi)->dr);
        // Hack: manually run irq handler to ensure TX buffer is always filled
        if(fill) {
            _spi_irq_handler(spi);
        }
    }
}

void spi_read(spi_inst_t *spi, uint8_t* dst, size_t len) {
    spi_read_large(spi, dst, len, false);
}

void spi_write(spi_inst_t *spi, uint8_t* src, size_t len) {
    for(size_t i = 0; i < len; ++i) {
        while(!spi_is_writable(spi))
            tight_loop_contents();
        spi_get_hw(spi)->dr = (uint32_t)src[i];
    }
}

void spi_write_const(spi_inst_t *spi, uint8_t val, size_t len) {
    for(size_t i = 0; i < len; ++i) {
        while(!spi_is_writable(spi))
            tight_loop_contents();
        spi_get_hw(spi)->dr = (uint32_t)val;
    }
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

inline uint32_t get_block_num(uint8_t* arg) {
    return (
        (uint32_t)arg[0] << 24 |
        (uint32_t)arg[1] << 16 |
        (uint32_t)arg[2] << 8 |
        (uint32_t)arg[3]
    );
}

void wait_for_cmd(spi_inst_t* spi, uint8_t* cmd_buf) {
    while(1) {
        spi_read(spi, cmd_buf, 1);
        // First two bits of transfer must be 0b01 to be a
        // valid command
        if((cmd_buf[0] & 0xC0) == 0x40)
            break;
    }
    // Read the rest of the command
    spi_read(spi, cmd_buf + 1, SD_CMD_LEN - 1);
    return;
}

void handle_cmd(spi_inst_t* spi, uint8_t* cmd_buf) {
    static bool app_mode = false;
    uint32_t block_num;
    uint8_t* arg = cmd_buf + 1;
    uint8_t cmd = cmd_buf[0] & 0b00111111;
    uint8_t resp[5] = {0};
    uint8_t* block;
    // TODO: check CRC?
    if(!app_mode) {
        switch(cmd) {
            case SD_CMD0:  // GO_IDLE_STATE
                // Reset always responds with R1=0x01 (in idle mode?)
                spi_write_const(spi, 0x01, 1);
                break;
            case SD_CMD8:  // SEND_IF_COND
                // Return an R7 response
                // R1 portion is always 0x01 (in idle mode?)
                resp[0] = 0x01;
                // Voltage?
                resp[3] = cmd_buf[3];
                // Echo back
                resp[4] = cmd_buf[4];
                spi_write(spi, resp, 5);
                break;
            case SD_CMD13:  // SEND_STATUS
                // Return R2 response
                spi_write_const(spi, 0x00, 2);
                break;
            case SD_CMD17:  // READ_BLOCK
                // Return an R1 response
                spi_write_const(spi, 0x00, 1);
                // We always emulate an SD card, the arg is always block number
                block_num = get_block_num(arg);
                // Request the block from the application, can be read only
                block = get_sd_block(block_num, false);
                // Disable IRQ, we don't want the buffer to accidentally be
                // filled with idle data
                _sd_enable_irq(spi, false);
                spi_write_const(spi, DATA_START_BLOCK, 1);
                // Respond with data (if available) + CRC
                // TODO: calculate CRC?
                if(block == NULL) {
                    spi_write_const(spi, 0x00, SD_SECTOR_SIZE + 2);
                } else {
                    spi_write(spi, block, SD_SECTOR_SIZE);
                    spi_write_const(spi, 0x00, 2);
                }
                // Ensure IRQ is reenabled and run
                _sd_enable_irq(spi, true);
                _spi_irq_handler(spi);
                break;
            case SD_CMD24:  // WRITE_BLOCK
                // We always emulate an SD card, the arg is always block number
                block_num = get_block_num(arg);
                // Request the block from the application, must be writable
                block = get_sd_block(block_num, true);
                // Ensure read buffer is cleared so that we can recieve the start block right away
                spi_clear_read_buf(spi);
                // Return an R1 response
                spi_write_const(spi_default, 0x00, 1);
                // Arduino lib expects the response to be sent on the transfer immediately after
                // the data/crc, it's too slow to manually send that so instead we start filling
                // the buffer with the response
                _set_spi_tx_idle_value(spi, DATA_RES_ACCEPTED);
                // Wait for data start block using the resp array
                do {
                    spi_read(spi, resp, 1);
                } while(resp[0] != DATA_START_BLOCK);
                // SPI interrupt doesn't seem to fire here if do a large read
                // Seems like if we use spi_read 0xFF still gets filled to the bus
                // until the transfer ends, and then we get a bunch of 0x00 instead,
                // DATA_RES_ACCEPTED never shows up on the bus
                spi_read_large(spi, block, SD_SECTOR_SIZE, true);
                // Read out CRC
                // TODO: do something with this?
                spi_read_large(spi, resp, 2, true);
                // Return idle value back to normal
                _set_spi_tx_idle_value(spi, 0xFF);
                break;
            case SD_CMD55:  // APP_CMD
                app_mode = true;
                // Respond with R1 response (idle mode?)
                spi_write_const(spi, 0x01, 1);
                break;
            case SD_CMD58:  // READ_OCR
                // Return an R3 response;
                // Always emulate an SDHC card
                resp[1] = 0xC0;
                spi_write(spi, resp, 5);
                break;
            default:
                // Unknown command
                break;
        }
    } else {
        switch(cmd) {
            case SD_ACMD41:  // SD_SEND_OP_COMD
                // Respond with R1?
                spi_write_const(spi, 0x00, 1);
                break;
            default:
                // Unknown app command
                break;
        }
        app_mode = false;
    }
}