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

int main(void)
{
	int i, j, k, m;

	for (k = 0; ; k++)
	{
		m = k;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 1000; j++)
				*gpio = ((0x100 << i) | digits[m % 10]) ^ 0xF00;
			m /= 10;
		}
	}
}
