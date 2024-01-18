#ifndef SDCARD_H
#define SDCARD_H

#define SD_NONE		0
#define SD_MMC		1
#define SD_V1		2
#define SD_V2		3
#define SD_V2_BLOCK	4

#define SDCARD_SPI_DIV	2

int sd_init(void);
int sd_read(void* buffer, unsigned int sector);

#endif
