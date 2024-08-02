#ifndef _PICO_LIBSDEMU_H_
#define _PICO_LIBSDEMU_H_

#include "pico/stdlib.h"

#ifndef SD_DEFAULT_PIO
#define SD_DEFAULT_PIO pio1
#endif

#ifndef SD_DEFAULT_TX_PIN
#define SD_DEFAULT_TX_PIN 10
#endif
#ifndef SD_DEFAULT_RX_PIN
#define SD_DEFAULT_RX_PIN 11
#endif
#ifndef SD_DEFAULT_SCK_PIN
#define SD_DEFAULT_SCK_PIN 12
#endif

#if (SD_DEFAULT_RX_PIN + 1) != SD_DEFAULT_SCK_PIN
#error "SD_DEFAULT_RX_PIN must be 1 less than SD_DEFAULT_SCK_PIN"
#endif



#define SD_CMD_LEN 0x6

/** GO_IDLE_STATE - init card in spi mode if CS low */
#define SD_CMD0 0
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define SD_CMD8 8
/** SEND_STATUS - read the card status register */
#define SD_CMD13 13
/** READ_BLOCK - read a single data block from the card */
#define SD_CMD17 17
/** WRITE_BLOCK - write a single data block to the card */
#define SD_CMD24 24
/** APP_CMD - escape for application specific command */
#define SD_CMD55 55
/** READ_OCR - read the OCR register of a card */
#define SD_CMD58 58

/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define SD_ACMD41 41

/** start data token for read or write single block*/
#define DATA_START_BLOCK 0XFE
/** write data accepted token */
#define DATA_RES_ACCEPTED 0X05
/** size of a sector in bytes */
#define SD_SECTOR_SIZE 512



/*! \brief Setup a SPI peripheral for SD emulation
 *  \ingroup pico-libsdemu
 *
 *  Set a SPI peripheral up as a slave device, with an interrupt
 *  to keep the TX buffer full. This is required because SDs will transmit
 *  0xFF when they are idle/ready for commands, but the RP2040 will default
 *  to 0x00 if nothing is in the buffer
 */
void setup_sd_emu();

void wait_for_cmd(uint8_t* cmd_buf);

void handle_cmd(uint8_t* cmd_buf);


#endif