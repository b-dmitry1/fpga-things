#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"

void wait(void)
{
	volatile int w;
	for (w = 0; w < 100000; w++);
}

void print_hex(unsigned int value)
{
	const char *hex = "0123456789ABCDEF";
	int i;

	for (i = 0; i < 8; i++)
	{
		putchar(hex[value >> 28u]);
		value <<= 4;
	}
}

int main(void)
{
	char s[64];
	int i, errors = 0;
	unsigned int *ptr;
	unsigned short *ptru;
	signed short *ptrs;
	unsigned int value32 = 0x34333231;
	unsigned short value16u = 0x3231;
	signed int value16s = 0xFFFF8281;

	print("\n\n\n");
	print("\nMisaligned 32 bit reads/writes test:\n");
	for (i = 4; i < sizeof(s) - 9; i++)
	{
		memset(s, 0x5f, sizeof(s) - 1);
		s[sizeof(s) - 1] = 0;

		ptr = (unsigned int *)&s[i];
		*ptr = value32;
		
		print(s);
		print("\r");

		if (*ptr != value32 || ptr[-1] != 0x5f5f5f5f || ptr[1] != 0x5f5f5f5f)
		{
			print("\nError!\nValue 0x");
			print_hex(*ptr);
			print(" != ");
			print_hex(value32);
			print("\n");
			errors |= 1 << (i & 3);
		}

		wait();
	}
	print("\n");

	if (errors & (1 << 0)) print("Aligned access error (should never happen)\n");
	if (errors & (1 << 1)) print("Unaligned +1 access error\n");
	if (errors & (1 << 2)) print("Unaligned +2 access error\n");
	if (errors & (1 << 3)) print("Unaligned +3 access error\n");

	if (errors == 0) print("Test ok\n");

	errors = 0;
	print("\nMisaligned unsigned 16 bit reads/writes test:\n");
	for (i = 2; i < sizeof(s) - 5; i++)
	{
		memset(s, 0x5f, sizeof(s) - 1);
		s[sizeof(s) - 1] = 0;

		ptru = (unsigned short *)&s[i];
		*ptru = value16u;

		print(s);
		print("\r");

		if (*ptru != value16u || ptru[-1] != 0x5f5f || ptru[1] != 0x5f5f)
		{
			print("\nError!\nValue 0x");
			print_hex(*ptru);
			print(" != ");
			print_hex(value16u);
			print("\n");
			errors |= 1 << (i & 3);
		}

		wait();
	}
	print("\n");

	if (errors & (1 << 0)) print("Aligned access error (should never happen)\n");
	if (errors & (1 << 1)) print("Unaligned +1 access error\n");
	if (errors & (1 << 2)) print("Unaligned +2 access error\n");
	if (errors & (1 << 3)) print("Unaligned +3 access error\n");

	if (errors == 0) print("Test ok\n");

	errors = 0;
	print("\nMisaligned signed 16 bit reads/writes test:\n");
	for (i = 2; i < sizeof(s) - 5; i++)
	{
		memset(s, 0x5f, sizeof(s) - 1);
		s[sizeof(s) - 1] = 0;

		ptrs = (signed short *)&s[i];
		*ptrs = value16s;

		print(s);
		print("\r");

		if (*ptrs != value16s || ptrs[-1] != 0x5f5f || ptrs[1] != 0x5f5f)
		{
			print("\nError!\nValue 0x");
			print_hex(*ptrs);
			print(" != ");
			print_hex(value16s);
			print("\n");
			errors |= 1 << (i & 3);
		}

		wait();
	}
	print("\n");

	if (errors & (1 << 0)) print("Aligned access error (should never happen)\n");
	if (errors & (1 << 1)) print("Unaligned +1 access error\n");
	if (errors & (1 << 2)) print("Unaligned +2 access error\n");
	if (errors & (1 << 3)) print("Unaligned +3 access error\n");

	if (errors == 0) print("Test ok\n");

	for (;;);
}
