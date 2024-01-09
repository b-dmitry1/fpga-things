#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define UART_TX_EMPTY	(1 << 9)

#define STACK_SIZE	256

__attribute__ ((section(".stack")))
static unsigned char stack[STACK_SIZE];

unsigned int* uart = (unsigned int*)0x20000000;

void sendchar(int ch)
{
	while (!(*uart & UART_TX_EMPTY));
	*uart = ch;
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

void print(const char* s)
{
	while (*s)
		putchar(*s++);
}

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
	for (i = -100; ; i++)
	{
		print("i = ");
		int_to_str(i, s);
		print(s);
		print("\n");
		wait();
	}
}
