import sys
import textwrap

SECTOR_SIZE = 512
TOTAL_SECTORS = 500

header_name = "image.h"
row_len = 16

with open(sys.argv[1], "rb") as f:
    data = f.read()

sectors_text = ""
for idx in range(TOTAL_SECTORS):
    start = idx*SECTOR_SIZE
    end = start + SECTOR_SIZE

    sector = data[start:end]
    sector_text = "\n".join([
        " ".join(f"0x{b:02x}," for b in sector[i:i+16]) for i in range(0, len(sector), 16)
    ])
    sectors_text += "{\n" +  textwrap.indent(sector_text, "    ") + "\n},\n"

out = f"""
#include <stdio.h>
#include "libsdemu.h"

#define TOTAL_SECTORS {TOTAL_SECTORS}

uint8_t sectors[TOTAL_SECTORS][SD_SECTOR_SIZE] = {{
{textwrap.indent(sectors_text, "    ")}
}};
"""

with open(header_name, "w") as f:
    f.write(out)

