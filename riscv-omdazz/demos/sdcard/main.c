#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "sdcard.h"

void wait(void)
{
	volatile int w;
	for (w = 0; w < 1000000; w++);
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

unsigned char buf[512];

int main(void)
{
	char s[16];
	int i, j;
	int found;

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

	for (i = 0; ; i++)
	{
		sd_read(buf, i);

		for (j = 0, found = 0; j < 512; j++)
		{
			if (buf[j] >= '0' && buf[j] <= 'z')
			{
				found = 1;
				break;
			}
		}

		if (!found)
			continue;

		print("Sector ");
		int_to_str(i, s);
		print(s);
		print(":\n");

		for (j = 0; j < 512; j++)
			if ((buf[j] >= ' ' && buf[j] <= 0x7f) || buf[j] == '\n')
				putchar(buf[j]);

		wait();
	}
}
