#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define A		0x01
#define B		0x02
#define C		0x04
#define D		0x08
#define E		0x10
#define F		0x20
#define G		0x40
#define H		0x80

unsigned int* port = (unsigned int*)0x10000000;
unsigned int* sdram = (unsigned int*)0x80000000;

const unsigned int digits[16] =
{
	0xFF ^ (A|B|C|D|E|F),
	0xFF ^ (B|C),
	0xFF ^ (A|B|D|E|G),
	0xFF ^ (A|B|C|D|G),
	0xFF ^ (B|C|F|G),
	0xFF ^ (A|C|D|F|G),
	0xFF ^ (A|C|D|E|F|G),
	0xFF ^ (A|B|C),
	0xFF ^ (A|B|C|D|E|F|G),
	0xFF ^ (A|B|C|D|F|G),
	0xFF ^ (A|B|C|E|F|G),
	0xFF ^ (C|D|E|F|G),
	0xFF ^ (A|D|E|F),
	0xFF ^ (B|C|D|E|G),
	0xFF ^ (A|D|E|F|G),
	0xFF ^ (A|E|F|G)
};

void print(const char* s);

void wait(void)
{
	volatile int w;
	for (w = 0; w < 1000000; w++);
}

void int_to_str(int value, char* s)
{
	int n = 0, i, temp;

	if (value == 0)
	{
		s[0] = '0';
		s[1] = 0;
		return;
	}

	if (value < 0)
	{
		*s++ = '-';
		value = -value;
	}

	while (value > 0)
	{
		s[n++] = '0' + value % 10;
		value /= 10;
	}

	for (i = 0; i < n / 2; i++)
	{
		temp = s[i];
		s[i] = s[n - i - 1];
		s[n - i - 1] = temp;
	}

	s[n] = 0;
}

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

	print("SDRAM test ");
	uint_to_hex(start, s);
	print(s);
	print(": ");

	value = start;
	for (i = 0; i < 2048 * 1024 / 8; i++, value += inc)
		sdram[i] = value;

	value = start;
	for (i = 0; i < 2048 * 1024 / 8; i++, value += inc)
	{
		if (sdram[i] != value)
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
	int i, j, k, m;
	char s[16];

	print("Errors: ");
	int_to_str(errors, s);
	print(s);
	print("\n");

	for (k = 0; k < 100; k++)
	{
		m = errors;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 200; j++)
				*port = ((0x100 << i) | digits[m % 10]) ^ 0xF00;
			m /= 10;
		}
	}
}

int main(void)
{
	for (;;)
	{
		test(0, 0x12345678);
		test(0x12345678, 0x01010101);
		test(0xAAAAAAAA, 0x21212121);

		show_errors();
	}
}
