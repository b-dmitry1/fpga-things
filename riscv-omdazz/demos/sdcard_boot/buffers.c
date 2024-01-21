#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "sdcard.h"
#include "buffers.h"

static unsigned char buffer[512];
static unsigned int current_sector = 0xFFFFFFFFu;

unsigned char *get_buffer(unsigned int sector)
{
	if (current_sector != sector)
	{
		sd_read(buffer, sector);
		current_sector = sector;
	}

	return buffer;
}
