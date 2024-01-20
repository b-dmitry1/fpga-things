#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"

#define A		0x01
#define B		0x02
#define C		0x04
#define D		0x08
#define E		0x10
#define F		0x20
#define G		0x40
#define H		0x80

const unsigned int digits[16] =
{
	(A|B|C|D|E|F),
	(B|C),
	(A|B|D|E|G),
	(A|B|C|D|G),
	(B|C|F|G),
	(A|C|D|F|G),
	(A|C|D|E|F|G),
	(A|B|C),
	(A|B|C|D|E|F|G),
	(A|B|C|D|F|G),
	(A|B|C|E|F|G),
	(C|D|E|F|G),
	(A|D|E|F),
	(B|C|D|E|G),
	(A|D|E|F|G),
	(A|E|F|G)
};

void uint_to_hex(unsigned int value, char* s)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		s[i] = "0123456789ABCDEF"[value >> 28u];
		value <<= 4;
	}
	s[8] = 0;
}

int errors = 0;

void test(unsigned int start, unsigned int inc)
{
	unsigned int i, value, addr = 0;
	char s[80];
	int ok = 1;
	unsigned int *sdr = (unsigned int *)sdram;

	print("SDRAM test ");
	uint_to_hex(start, s);
	print(s);
	print(": ");

	value = start;
	for (i = 0; i < 2048 * 1024; i++, value += inc)
		sdr[i] = value;

	value = start;
	for (i = 0; i < 2048 * 1024; i++, value += inc)
	{
		if (sdr[i] != value)
		{
			ok = 0;
			addr = i * 4;
			errors++;
			break;
		}
	}

	if (ok)
	{
		print("OK\n");
	}
	else
	{
		print("Failed at ");
		uint_to_hex(addr, s);
		print(s);
		print("\n");
	}
}

void show_errors(void)
{
	int i, m;
	volatile unsigned char *seg8 = (volatile unsigned char *)seg;

	m = errors;
	for (i = 0; i < 4; i++)
	{
		seg8[i] = digits[m % 10];
		m /= 10;
	}
}

int main(void)
{
	for (;;)
	{
		test(0x00000000, 0x12345678);
		test(0x12345678, 0x01010101);
		test(0xAAAAAAAA, 0x21212121);
		test(0x46562874, 0x00000001);

		show_errors();
	}
}
