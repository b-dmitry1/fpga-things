#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "board.h"
#include "sdcard.h"
#include "fat.h"
#include "buffers.h"

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

	fs->sectors_per_cluster = boot->sectors_per_cluster;

	fs->fat_start = fs->part_start + LE16(boot->reserved_sectors);

	fs->root_dir_start = fs->fat_start +
		LE32(boot->sectors_per_fat) * boot->num_fats;

	fs->root_dir_cluster_ = LE32(boot->root_dir_cluster);

	return 1;
}

unsigned int next_sector(fat_t *fs, unsigned int sector)
{
	if ((sector + 1) % fs->sectors_per_cluster != 0)
	{
		return sector + 1;
	}

	return 0;
}

int fat_find_next(fat_t *fs, file_entry_t *entry)
{
	unsigned char *buf;
	fat_entry_t *e;
	int i, n;

	for (;;)
	{
		if (entry->dir_pos >= SECTOR_SIZE)
		{
			entry->dir_pos = 0;
			entry->dir_sector = next_sector(fs, entry->dir_sector);
			if (entry->dir_sector == 0)
				return 0;
		}

		buf = get_buffer(entry->dir_sector);
		if (buf == NULL)
			return 0;

		e = (fat_entry_t *)buf;

		for (e += entry->dir_pos / sizeof(*e); entry->dir_pos < SECTOR_SIZE; entry->dir_pos += 32, e++)
		{
			if (e->attr == 0x0f)
				continue;

			if (e->name[0] == 0)
				return 0;

			if (e->name[0] == 0x05 || e->name[0] == 0xe5)
				continue;

			entry->dir_pos += 32;

			for (i = 0, n = 0; i < 8; i++)
				if (e->name[i] == ' ')
					break;
				else
					entry->name[n++] = e->name[i];

			if (e->name[8] != ' ')
				entry->name[n++] = '.';

			for (i = 8; i < 11; i++)
				if (e->name[i] == ' ')
					break;
				else
					entry->name[n++] = e->name[i];
			entry->name[n] = 0;

			entry->attr = e->attr;
			entry->first_sector = LE16(e->cluster_lo) + (LE16(e->cluster_hi) << 16u);
			entry->first_sector *= fs->sectors_per_cluster;
			entry->first_sector += fs->root_dir_start - fs->root_dir_cluster_ * fs->sectors_per_cluster;
			entry->size = LE32(e->size);

			return 1;
		}
	}

	return 1;
}

int fat_find_init(fat_t *fs, file_entry_t *entry)
{
	memset(entry, 0, sizeof(*entry));

	entry->dir_sector = fs->root_dir_start;

	return 1;
}

int fat_open(fat_t *fs, file_t *file, const char *name)
{
	memset(file, 0, sizeof(*file));

	if (!fat_find_init(fs, &file->entry))
		return 0;

	while (fat_find_next(fs, &file->entry))
	{
		if (strcmp(file->entry.name, name))
			continue;

		file->pos = 0;
		file->sector = file->entry.first_sector;
		return 1;
	}

	return 0;
}

int fat_read_next_sector(fat_t *fs, file_t *file, unsigned char *buf)
{
	unsigned char *b;

	b = get_buffer(file->sector);
	if (b == NULL)
		return 0;

	memcpy(buf, b, SECTOR_SIZE);

	file->sector = next_sector(fs, file->sector);

	return 1;
}
