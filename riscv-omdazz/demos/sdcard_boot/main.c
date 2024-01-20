#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "board.h"
#include "sdcard.h"
#include "fat.h"

typedef struct
{
	unsigned char active;
	unsigned char start_head;
	unsigned char start_sector_cyl;
	unsigned char start_cyl;
	unsigned char type;
	unsigned char end_head;
	unsigned char end_sector_cyl;
	unsigned char end_cyl;
	unsigned char lba_start[4];
	unsigned char num_sectors[4];
} __attribute__((__packed__)) mbr_part_t;

typedef struct
{
	unsigned char bootcode[446];
	mbr_part_t part[4];
	unsigned char sign[2];
} __attribute__((__packed__)) mbr_t;

unsigned int part_start = 0;
unsigned int part_size = 0;
unsigned int part_type = 0;

fat_t fat;

int read_sector(void *user, unsigned int number, void *buffer)
{
	return sd_read(buffer, number);
}

void wait(void)
{
	volatile int w;
	for (w = 0; w < 100000; w++);
}

void int_to_str(int value, char* str)
{
	char buf[12];
	int i;

	if (value < 0)
	{
		value = -value;
		*str++ = '-';
	}

	for (i = 0; i < 10; i++)
	{
		buf[9 - i] = '0' + value % 10;
		value /= 10;
	}

	for (i = 0; i < 9 && buf[i] == '0'; i++);

	for (; i < 10; i++)
		*str++ = buf[i];

	*str = 0;
}

void print_int(const char *prefix, int value, const char *suffix)
{
	char s[16];
	int_to_str(value, s);
	print(prefix);
	print(s);
	print(suffix);
}

unsigned char buf[512];
mbr_t *mbr = (mbr_t *)buf;

int main(void)
{
	char s[16];
	int i, j;
	unsigned int start, size;
	fat_entry_t *dir = (fat_entry_t *)buf;
	fat32_lfn_entry_t *lfn = (fat32_lfn_entry_t *)buf;

	*seg = 0x7C5C5C78;

	print("\n\n\n\n\n");

init_loop:
	wait();

	print("sd_init: ");

	switch (sd_init())
	{
		case SD_NONE:     print("not found\n"); goto init_loop;
		case SD_MMC:      print("MMC\n"); break;
		case SD_V1:       print("SD v1\n"); break;
		case SD_V2:       print("SD v2\n"); break;
		case SD_V2_BLOCK: print("SD v2 (block)\n"); break;
	}

	sd_read(buf, 0);

	print("MBR:\n");
	for (i = 0; i < 4; i++)
	{
		start = LE32(mbr->part[i].lba_start);
		size = LE32(mbr->part[i].num_sectors);

		print_int("Partition ", i, ": ");
		print_int("type ", mbr->part[i].type, ", ");
		print_int("start ", start, ", ");
		print_int("size ", size, ", ");
		print("\n");

		if (part_type == 0)
		{
			if (mbr->part[i].type == 0x0C)
			{
				part_type = mbr->part[i].type;
				part_start = start;
				part_size = size;
				// break;
			}
		}
	}

	if (part_type == 0)
	{
		print("Can\'t find any known partition type\n");
		for (;;);
	}

	print_int("\nReading sector: ", part_start, "\n");
	sd_read(buf, part_start);

	if (!fat_mount(&fat, read_sector, 0, part_start))
	{
		print("mount failed\n");
		for (;;);
	}

	print_int("mount ok\nFAT start: ", fat.fat_start, "\n");
	print_int("Root dir start: ", fat.root_dir_start, "\n");

	sd_read(buf, fat.root_dir_start);

	for (i = 0; i < 512 / 32; i++)
	{
		if (dir[i].name[0] == 0)
			break;

		if (dir[i].name[0] != 0xe5 && dir[i].name[0] != 0x05)
		{
			if (dir[i].attr == 0x0f)
			{
				for (j = 0; j < 10; j+=2)
				{
					if (lfn[i].name1[j] == 0xFF) goto lfn_done;
					putchar(lfn[i].name1[j]);
				}
				for (j = 0; j < 12; j+=2)
				{
					if (lfn[i].name2[j] == 0xFF) goto lfn_done;
					putchar(lfn[i].name2[j]);
				}
				for (j = 0; j < 4; j+=2)
				{
					if (lfn[i].name3[j] == 0xFF) goto lfn_done;
					putchar(lfn[i].name3[j]);
				}
			}
			else
			{
				for (j = 0; j < 11; j++)
					putchar(dir[i].name[j]);
			}

lfn_done:
			print("\n");
		}
	}

	for (;;);
}
