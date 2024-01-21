#ifndef FAT_H
#define FAT_H

#define SECTOR_SIZE	512

#define FAT_BUFFER_SIZE	512

typedef struct
{
	unsigned int part_start;
	unsigned int fat_start;
	unsigned int root_dir_start;
	unsigned char buffer[FAT_BUFFER_SIZE];
	unsigned int sectors_per_cluster;
	unsigned int root_dir_cluster_;

	void *user;
	int (*read)(void *user, unsigned int number, void *buffer);
} fat_t;

typedef struct
{
	char name[16];
	unsigned int attr;
	unsigned int size;
	unsigned int first_sector;
	unsigned int dir_sector;
	unsigned int dir_pos;
} file_entry_t;

typedef struct
{
	file_entry_t entry;
	unsigned int pos;
	unsigned int sector;
} file_t;

typedef struct
{
	char name[11];
	unsigned char attr;
	unsigned char nt_res;
	unsigned char c_time_tenth;
	unsigned char c_time[2];
	unsigned char c_date[2];
	unsigned char a_date[2];
	unsigned char cluster_hi[2];
	unsigned char w_time[2];
	unsigned char w_date[2];
	unsigned char cluster_lo[2];
	unsigned char size[4];
} fat_entry_t;

typedef struct
{
	unsigned char order;
	char name1[10];
	unsigned char attr;
	unsigned char type;
	unsigned char checksum;
	char name2[12];
	unsigned char cluster_lo[2];
	char name3[4];
} fat32_lfn_entry_t;

int fat_mount(fat_t *fs, int (*read_sector)(void *user, unsigned int number, void *buffer),
	void *user, int partition_start);
int fat_find_init(fat_t *fs, file_entry_t *entry);
int fat_find_next(fat_t *fs, file_entry_t *entry);
int fat_open(fat_t *fs, file_t *file, const char *name);
int fat_read_next_sector(fat_t *fs, file_t *file, unsigned char *buf);

#endif
