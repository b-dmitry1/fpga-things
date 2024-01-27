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

int main(void)
{
	char s[64];
	int i;
	unsigned int *ptr;

	for (i = 0; i < sizeof(s) - 5; i++)
	{
		memset(s, '_', sizeof(s) - 1);
		s[sizeof(s) - 1] = 0;

		ptr = (unsigned int *)&s[i];
		*ptr = 0x31313131;
		
		print(s);
		print("\n");

		wait();
	}

	for (;;);
}
