// Sample application emulating a SD card with a FAT16 filesystem from RAM
// Note that the RP2040 does not actually have enough RAM to store all of the data advertised,
// eventually this sample code will crash. Real implementations have to use some other form 
// of bulk storage
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "libsdemu.h"
#include "image.h"

void printbuf(uint8_t buf[], size_t len)
{
  char resp[128];
  uint32_t resp_count;
  uint32_t resp_idx = 0;
  for (size_t i = 0; i < len; ++i)
  {
    if (i % 16 == 15) {
      resp_count = sprintf(resp + resp_idx, "0x%02x,\r\n", buf[i]);
      resp_idx += resp_count;
      printf(resp);
      resp_idx = 0;
    }
    else
    {
      resp_count = sprintf(resp + resp_idx, "0x%02x, ", buf[i]);
      resp_idx += resp_count;
    }
  }
  printf("\n");
}

// Define a read_sd_block function in your application that copies an
// SD_SECTOR_SIZE sized block of memory from storage to `buff`
// Return true on success
bool read_sd_block(uint32_t block_num, uint8_t *buff) {
    printf("Reading block %d\n", block_num);
    if(block_num >= TOTAL_SECTORS) {
        printf("Block is not allocated, returning zeros...\n");
        return false;
    }
    printf("Copying block to buffer...\n");
    memcpy(buff, sectors[block_num], SD_SECTOR_SIZE);
    //printbuf(buff, SD_SECTOR_SIZE);
    printf("done\n");
    return true;
}


// Define a write_sd_block function in your application that copies an
// SD_SECTOR_SIZE sized block of memory from `buff` to your storage
bool write_sd_block(uint32_t block_num, uint8_t *buff) {
    printf("Writing block %d\n", block_num);
    if(block_num >= TOTAL_SECTORS) {
        printf("Block is not allocated, ignoring write...\n");
        return false;
    }
    printf("Copying block from buffer...\n");
    memcpy(sectors[block_num], buff, SD_SECTOR_SIZE);
    //printbuf(buff, SD_SECTOR_SIZE);
    printf("done\n");
    return true;
}

void init_sd() {
    printf("Initializing SD SPI\n");
    setup_sd_emu();
}

int main() {
    uint8_t cmd_buf[SD_CMD_LEN];

    stdio_init_all();

    printf("hello_ram_sd: init\n");

    init_sd();

    while(1) {
        wait_for_cmd(cmd_buf);
        handle_cmd(cmd_buf);
    }
    return 0;
}