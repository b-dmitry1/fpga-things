#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define STACK_SIZE	2048

__attribute__ ((section(".stack")))
static unsigned char stack[STACK_SIZE];

unsigned int* port = (unsigned int*)0x10000000;

void sendchar(char ch)
{
	*port = ch;
}

int putchar(int ch)
{
	if (ch == '\n')
		sendchar('\r');
	sendchar(ch);
	return ch;
}

int recvchar(void)
{
	return -1;
}

int main(void)
{
	int i;

	for (i = 0; ; i++)
	{
		printf("i = %d, 0x%x\n", i, i);
	}
}
