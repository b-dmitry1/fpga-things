#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"

#define TEST_VALUE	0x12345678u
#define TEST_SVALUE	((signed int)0x12345678)

unsigned int sll_correct[32] =
{
	(TEST_VALUE << 0),
	(TEST_VALUE << 1),
	(TEST_VALUE << 2),
	(TEST_VALUE << 3),
	(TEST_VALUE << 4),
	(TEST_VALUE << 5),
	(TEST_VALUE << 6),
	(TEST_VALUE << 7),
	(TEST_VALUE << 8),
	(TEST_VALUE << 9),
	(TEST_VALUE << 10),
	(TEST_VALUE << 11),
	(TEST_VALUE << 12),
	(TEST_VALUE << 13),
	(TEST_VALUE << 14),
	(TEST_VALUE << 15),
	(TEST_VALUE << 16),
	(TEST_VALUE << 17),
	(TEST_VALUE << 18),
	(TEST_VALUE << 19),
	(TEST_VALUE << 20),
	(TEST_VALUE << 21),
	(TEST_VALUE << 22),
	(TEST_VALUE << 23),
	(TEST_VALUE << 24),
	(TEST_VALUE << 25),
	(TEST_VALUE << 26),
	(TEST_VALUE << 27),
	(TEST_VALUE << 28),
	(TEST_VALUE << 29),
	(TEST_VALUE << 30),
	(TEST_VALUE << 31)
};

unsigned int srl_correct[32] =
{
	(TEST_VALUE >> 0),
	(TEST_VALUE >> 1),
	(TEST_VALUE >> 2),
	(TEST_VALUE >> 3),
	(TEST_VALUE >> 4),
	(TEST_VALUE >> 5),
	(TEST_VALUE >> 6),
	(TEST_VALUE >> 7),
	(TEST_VALUE >> 8),
	(TEST_VALUE >> 9),
	(TEST_VALUE >> 10),
	(TEST_VALUE >> 11),
	(TEST_VALUE >> 12),
	(TEST_VALUE >> 13),
	(TEST_VALUE >> 14),
	(TEST_VALUE >> 15),
	(TEST_VALUE >> 16),
	(TEST_VALUE >> 17),
	(TEST_VALUE >> 18),
	(TEST_VALUE >> 19),
	(TEST_VALUE >> 20),
	(TEST_VALUE >> 21),
	(TEST_VALUE >> 22),
	(TEST_VALUE >> 23),
	(TEST_VALUE >> 24),
	(TEST_VALUE >> 25),
	(TEST_VALUE >> 26),
	(TEST_VALUE >> 27),
	(TEST_VALUE >> 28),
	(TEST_VALUE >> 29),
	(TEST_VALUE >> 30),
	(TEST_VALUE >> 31)
};

signed int sra_correct[32] =
{
	(TEST_SVALUE >> 0),
	(TEST_SVALUE >> 1),
	(TEST_SVALUE >> 2),
	(TEST_SVALUE >> 3),
	(TEST_SVALUE >> 4),
	(TEST_SVALUE >> 5),
	(TEST_SVALUE >> 6),
	(TEST_SVALUE >> 7),
	(TEST_SVALUE >> 8),
	(TEST_SVALUE >> 9),
	(TEST_SVALUE >> 10),
	(TEST_SVALUE >> 11),
	(TEST_SVALUE >> 12),
	(TEST_SVALUE >> 13),
	(TEST_SVALUE >> 14),
	(TEST_SVALUE >> 15),
	(TEST_SVALUE >> 16),
	(TEST_SVALUE >> 17),
	(TEST_SVALUE >> 18),
	(TEST_SVALUE >> 19),
	(TEST_SVALUE >> 20),
	(TEST_SVALUE >> 21),
	(TEST_SVALUE >> 22),
	(TEST_SVALUE >> 23),
	(TEST_SVALUE >> 24),
	(TEST_SVALUE >> 25),
	(TEST_SVALUE >> 26),
	(TEST_SVALUE >> 27),
	(TEST_SVALUE >> 28),
	(TEST_SVALUE >> 29),
	(TEST_SVALUE >> 30),
	(TEST_SVALUE >> 31)
};


unsigned int asm_sll(unsigned int value, unsigned int shamt);
unsigned int asm_slli1(unsigned int value);
unsigned int asm_slli2(unsigned int value);
unsigned int asm_slli4(unsigned int value);
unsigned int asm_slli5(unsigned int value);
unsigned int asm_slli8(unsigned int value);
unsigned int asm_slli13(unsigned int value);
unsigned int asm_slli16(unsigned int value);
unsigned int asm_slli31(unsigned int value);

unsigned int asm_srl(unsigned int value, unsigned int shamt);
signed int asm_sra(signed int value, unsigned int shamt);

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

int main(void)
{
	char s[16];
	int i, errors;
	unsigned int value, res;
	signed int svalue, sres;

	print("\n\n\n");

	print("Testing SLL\n");
	for (errors = 0, value = TEST_VALUE, i = 0; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i++)
	{
		res = asm_sll(value, i);
		if (res != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 1\n");
	for (errors = 0, value = TEST_VALUE, i = 1; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i++)
	{
		value = asm_slli1(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 2\n");
	for (errors = 0, value = TEST_VALUE, i = 2; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 2)
	{
		value = asm_slli2(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 4\n");
	for (errors = 0, value = TEST_VALUE, i = 4; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 4)
	{
		value = asm_slli4(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 5\n");
	for (errors = 0, value = TEST_VALUE, i = 5; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 5)
	{
		value = asm_slli5(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 8\n");
	for (errors = 0, value = TEST_VALUE, i = 8; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 8)
	{
		value = asm_slli8(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 13\n");
	for (errors = 0, value = TEST_VALUE, i = 13; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 13)
	{
		value = asm_slli13(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 16\n");
	for (errors = 0, value = TEST_VALUE, i = 16; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 16)
	{
		value = asm_slli16(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SLLI 31\n");
	for (errors = 0, value = TEST_VALUE, i = 31; i < sizeof(sll_correct) / sizeof(sll_correct[0]); i += 31)
	{
		value = asm_slli31(value);
		if (value != sll_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SRL\n");
	for (errors = 0, value = TEST_VALUE, i = 0; i < sizeof(srl_correct) / sizeof(srl_correct[0]); i++)
	{
		res = asm_srl(value, i);
		if (res != srl_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	print("Testing SRA\n");
	for (errors = 0, svalue = TEST_SVALUE, i = 0; i < sizeof(sra_correct) / sizeof(sra_correct[0]); i++)
	{
		sres = asm_sra(svalue, i);
		if (sres != sra_correct[i])
		{
			print("Wrong at i = ");
			int_to_str(i, s);
			print(s);
			print("\n");
			errors++;
		}
	}

	if (errors > 0)
	{
		print("Errors: ");
		int_to_str(i, s);
		print(s);
		print("\n");
	}

	if (errors == 0)
		print("Test ok\n");

	for (;;);
}
