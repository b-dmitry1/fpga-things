#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "vga16color.h"

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
	int x = 2, y = 100;
	int w = 48, h = 48;
	int dx = 1, dy = 1;
	char s[16];
	int frame = 0;

	for (;;)
	{
		clear_screen(0);
		fill_rect(0, 0, SCREEN_WIDTH, 2, GREEN);
		fill_rect(0, 0, 2, SCREEN_HEIGHT, GREEN);
		fill_rect(SCREEN_WIDTH - 2, 0, 2, SCREEN_HEIGHT, GREEN);
		fill_rect(0, SCREEN_HEIGHT - 2, SCREEN_WIDTH, 2, GREEN);

		draw_text(16, 16, "Blue", BLUE);
		draw_text(16, 32, "Green", GREEN);
		draw_text(16, 48, "Cyan", CYAN);
		draw_text(16, 64, "Red", RED);
		draw_text(16, 80, "Magenta", MAGENTA);
		draw_text(16, 96, "Yellow", YELLOW);
		draw_text(16, 112, "White", WHITE);

		int_to_str(frame, s);
		draw_text(16, 128, "Frame: ", GREEN);
		draw_text(16 + 7 * 8, 128, s, CYAN);

		x += dx;
		y += dy;
		dy++;

		fill_rect(x, y, w, h, YELLOW);

		if (x <= 0)
			dx = 1;
		if (x + w >= SCREEN_WIDTH - 1)
			dx = -(1 + rand() % 3);

		if (y + h >= SCREEN_HEIGHT - 1)
		{
			dy = -dy * 2 / 3;
			if (rand() % 10 == 0)
				dy = -(20 + rand() % 10);
			y = SCREEN_HEIGHT - h - 2;
		}

		frame++;

		// Don't forget to flip video page or you'll see nothing!
		flip();
	}
}
