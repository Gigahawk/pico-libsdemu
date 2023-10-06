#ifndef _PICO_LIBSDEMU_H_
#define _PICO_LIBSDEMU_H_

#include "hardware/spi.h"

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

/*! \brief Setup a SPI peripheral for SD emulation
 *  \ingroup pico-libsdemu
 *
 *  Set a SPI peripheral up as a slave device, with an interrupt
 *  to keep the TX buffer full. This is required because SDs will transmit
 *  0xFF when they are idle/ready for commands, but the RP2040 will default
 *  to 0x00 if nothing is in the buffer
 */
void setup_sd_emu(spi_inst_t* spi);

#endif