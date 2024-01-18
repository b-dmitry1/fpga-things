#ifndef FAT32_H
#define FAT32_H

#define FAT32_BUFFER_SIZE	512

typedef struct
{
	unsigned int part_start;
	unsigned int fat_start;
	unsigned int root_dir_start;
	unsigned char buffer[FAT32_BUFFER_SIZE];

	void *user;
	int (*read)(void *user, unsigned int number, void *buffer);
} fat32_t;

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
} fat32_entry_t;

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

int fat32_mount(fat32_t *fs, int (*read_sector)(void *user, unsigned int number, void *buffer),
	void *user, int partition_start);

#endif
