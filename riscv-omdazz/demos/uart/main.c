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
	int i;

	// Please don't use "printf" or other large functions!

	for (i = -100; ; i++)
	{
		print("i = ");
		int_to_str(i, s);
		print(s);
		print("\n");

		wait();
	}
}
