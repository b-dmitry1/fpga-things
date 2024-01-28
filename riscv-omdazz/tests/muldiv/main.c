#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"

extern unsigned int asm_mulhu(unsigned int a, unsigned int b);

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

void print_hex32(unsigned int value)
{
	const char *hex = "0123456789ABCDEF";
	int i;

	for (i = 0; i < 8; i++, value <<= 4)
		putchar(hex[value >> 28]);
}

unsigned int c_mulhu(unsigned int a, unsigned int b)
{
	unsigned long long res = 0, mb = b;
	int i;

	for (i = 0; i < 32; i++)
	{
		if (a & 1)
			res += mb;
		a >>= 1;
		mb <<= 1;
	}

	for (i = 0; i < 32; i++)
		res >>= 1;

	return (unsigned int)res;
}

int main(void)
{
	char s[16];
	int i, j, test, tests = 0, errors = 0;
	unsigned int start_a = 0, start_b = 0, a, b;
	unsigned int c_res, asm_res;

	print("\n\n\n");

	print("MULHU test (please wait)\n");
	for (test = 0; test < 100; test++)
	{
		a = start_a;
		for (i = 0; i < 32; i++)
		{
			b = start_b;
			for (j = 0; j < 32; j++, tests++)
			{
				c_res = c_mulhu(a, b);
				asm_res = asm_mulhu(a, b);
				if (c_res != asm_res)
				{
					errors++;
					if (errors < 20)
					{
						print_hex32(a);
						print(" * ");
						print_hex32(b);
						print(" = ");
						print_hex32(asm_res);
						print("  /  ");
						print_hex32(c_res);
						print("\n");
					}
					
				}
					
				b += 0x10000000;
			}
			a += 0x10000000;
		}
		start_a += 31287;
		start_b += 18451;
	}

	int_to_str(tests, s);
	print("Tests: ");
	print(s);
	print("\n");

	int_to_str(errors, s);
	print("Errors: ");
	print(s);
	print("\n");

	for (;;);
}
