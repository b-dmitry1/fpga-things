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
	// DOS 2.0
	unsigned char jmp[3];
	unsigned char name[8];
	unsigned char bytes_per_sector[2];
	unsigned char sectors_per_cluster;
	unsigned char reserved_sectors[2];
	unsigned char num_fats;
	unsigned char root_dir_entries[2];
	unsigned char total_sectors_w[2];
	unsigned char media_descriptor;
	unsigned char sectors_per_fat_w[2];
	// DOS 3.31
	unsigned char sectors_per_track[2];
	unsigned char num_heads[2];
	unsigned char hidden_sectors[4];
	unsigned char total_sectors[4];
	// DOS 7.1
	unsigned char sectors_per_fat[4];
	unsigned char mirroring_flags[2];
	unsigned char version[2];
	unsigned char root_dir_cluster[4];
	unsigned char fsinfo_sector[2];
	unsigned char backup_location[2];
	unsigned char boot_name[2];
	unsigned char drive_number;
	unsigned char flags;
	unsigned char ext_boot_sign;
	unsigned char serial_number[4];
	unsigned char volume_label[11];
	unsigned char fstype[8];
} __attribute__((__packed__)) fat_boot_t;

static int readsect(fat_t *fs, unsigned int number)
{
	return fs->read(fs->user, number, fs->buffer);
}

int fat_mount(fat_t *fs, int (*read_sector)(void *user, unsigned int number, void *buffer), void *user, int partition_start)
{
	fat_boot_t *boot;

	memset(fs, 0, sizeof(*fs));
	fs->read = read_sector;
	fs->user = user;
	fs->part_start = partition_start;

	if (!readsect(fs, fs->part_start))
		return 0;

	boot = (fat_boot_t *)fs->buffer;

	fs->fat_start = fs->part_start + LE16(boot->reserved_sectors);

	fs->root_dir_start = fs->fat_start +
		LE32(boot->sectors_per_fat) * boot->num_fats;

	return 1;
}
