#include <stdio.h>
#include "libsdemu.h"
#include "spisd_program_init.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"

uint8_t read_buffer[SD_SECTOR_SIZE];
uint8_t write_buffer[SD_SECTOR_SIZE];
extern bool read_sd_block(uint32_t block_num, uint8_t* buff);
extern bool write_sd_block(uint32_t block_num, uint8_t* buff);

PIO pio = SD_DEFAULT_PIO;
uint sm_tx;
uint sm_rx;

void __time_critical_func(spi_clear_read_buf)() {
    while(!pio_sm_is_rx_fifo_empty(pio, sm_rx)) {
        pio_sm_get(pio, sm_rx);
    }
}


void __time_critical_func(spi_read)(uint8_t* dst, size_t len) {
    for(size_t i = 0; i < len; ++i) {
        dst[i] = (uint8_t)pio_sm_get_blocking(pio, sm_rx);
    }
}

void __time_critical_func(spi_write)(uint8_t* src, size_t len) {
    io_rw_8* tx_fifo = (io_rw_8*) &pio->txf[sm_tx];
    for(size_t i = 0; i < len; ++i) {
        //pio_sm_put_blocking(pio, sm_tx, ((uint32_t)src[i]) << 24);
        while(pio_sm_is_tx_fifo_full(pio, sm_tx)) 
            tight_loop_contents();
        *tx_fifo = src[i];
    }
}

void __time_critical_func(spi_write_const)(uint8_t val, size_t len) {
    io_rw_8* tx_fifo = (io_rw_8*) &pio->txf[sm_tx];
    for(size_t i = 0; i < len; ++i) {
        //pio_sm_put_blocking(pio, sm_tx, ((uint32_t)val) << 24);
        while(pio_sm_is_tx_fifo_full(pio, sm_tx)) 
            tight_loop_contents();
        *tx_fifo = val;
    }
}

void setup_sd_emu() {
    sm_tx = pio_claim_unused_sm(pio, true);
    sm_rx = pio_claim_unused_sm(pio, true);
    uint offset_tx = pio_add_program(pio, &spisd_tx_program);
    uint offset_rx = pio_add_program(pio, &spisd_rx_program);

    spisd_program_init(
        pio, sm_tx, offset_tx, sm_rx, offset_rx,
        SD_DEFAULT_SCK_PIN,
        SD_DEFAULT_TX_PIN,
        SD_DEFAULT_RX_PIN
    );

}

inline uint32_t get_block_num(uint8_t* arg) {
    return (
        (uint32_t)arg[0] << 24 |
        (uint32_t)arg[1] << 16 |
        (uint32_t)arg[2] << 8 |
        (uint32_t)arg[3]
    );
}

void wait_for_cmd(uint8_t* cmd_buf) {
    while(1) {
        spi_read(cmd_buf, 1);
        // First two bits of transfer must be 0b01 to be a
        // valid command
        if((cmd_buf[0] & 0xC0) == 0x40)
            break;
    }
    // Read the rest of the command
    spi_read(cmd_buf + 1, SD_CMD_LEN - 1);
    return;
}

void handle_cmd(uint8_t* cmd_buf) {
    static bool app_mode = false;
    uint32_t block_num;
    uint8_t* arg = cmd_buf + 1;
    uint8_t cmd = cmd_buf[0] & 0b00111111;
    uint8_t resp[5] = {0};
    bool access_ok;
    // TODO: check CRC?
    if(!app_mode) {
        switch(cmd) {
            case SD_CMD0:  // GO_IDLE_STATE
                // Reset always responds with R1=0x01 (in idle mode?)
                spi_write_const(0x01, 1);
                break;
            case SD_CMD8:  // SEND_IF_COND
                // Return an R7 response
                // R1 portion is always 0x01 (in idle mode?)
                resp[0] = 0x01;
                // Voltage?
                resp[3] = cmd_buf[3];
                // Echo back
                resp[4] = cmd_buf[4];
                spi_write(resp, 5);
                break;
            case SD_CMD13:  // SEND_STATUS
                // Return R2 response
                spi_write_const(0x00, 2);
                break;
            case SD_CMD17:  // READ_BLOCK
                // Return an R1 response
                spi_write_const(0x00, 1);
                // We always emulate an SD card, the arg is always block number
                block_num = get_block_num(arg);
                // Read the block from the application
                access_ok = read_sd_block(block_num, read_buffer);
                spi_write_const(DATA_START_BLOCK, 1);
                // Respond with data (if available) + CRC
                // TODO: calculate CRC?
                if(!access_ok) {
                    spi_write_const(0x00, SD_SECTOR_SIZE + 2);
                } else {
                    spi_write(read_buffer, SD_SECTOR_SIZE);
                    spi_write_const(0x00, 2);
                }
                break;
            case SD_CMD24:  // WRITE_BLOCK
                // We always emulate an SD card, the arg is always block number
                block_num = get_block_num(arg);
                // Ensure read buffer is cleared so that we can recieve the start block right away
                spi_clear_read_buf();
                // Return an R1 response
                spi_write_const(0x00, 1);
                spi_write_const(DATA_RES_ACCEPTED, 1);
                // Wait for data start block using the resp array
                do {
                    spi_read(resp, 1);
                } while(resp[0] != DATA_START_BLOCK);
                spi_read(write_buffer, SD_SECTOR_SIZE);
                // Read out CRC
                // TODO: do something with this?
                spi_read(resp, 2);
                write_sd_block(block_num, write_buffer);
                break;
            case SD_CMD55:  // APP_CMD
                app_mode = true;
                // Respond with R1 response (idle mode?)
                spi_write_const(0x01, 1);
                break;
            case SD_CMD58:  // READ_OCR
                // Return an R3 response;
                // Always emulate an SDHC card
                resp[1] = 0xC0;
                spi_write(resp, 5);
                break;
            default:
                // Unknown command
                break;
        }
    } else {
        switch(cmd) {
            case SD_ACMD41:  // SD_SEND_OP_COMD
                // Respond with R1?
                spi_write_const(0x00, 1);
                break;
            default:
                // Unknown app command
                break;
        }
        app_mode = false;
    }
}