#include <string.h>
#include "vga16color.h"

static unsigned char* vram = (unsigned char*)VRAM_PAGE1;

static unsigned int *vdu_io = (unsigned int *)VDU_IO;

#include "font.c"

void set_pixel(int x, int y, int color)
{
	unsigned int *v = (unsigned int *)vram;
	unsigned int c;
	if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
		return;

	if ((x & 1) == 0)
		vram[y * SCREEN_WIDTH / 2 + x / 2] = (vram[y * SCREEN_WIDTH / 2 + x / 2] & 0xF0) | (color & 0x0F);
	else
		vram[y * SCREEN_WIDTH / 2 + x / 2] = (vram[y * SCREEN_WIDTH / 2 + x / 2] & 0x0F) | ((color << 4) & 0xF0);
}

void hline(int x, int y, int l, int color)
{
	int i;

	for (i = 0; i < l; i++)
		set_pixel(x + i, y, color);
}

void fill_rect(int x, int y, int w, int h, int color)
{
	int i;
	unsigned char *v;
	unsigned char colorx2;

	if (x < 0)
	{
		w += x;
		x = 0;
	}

	if (y < 0)
	{
		h += y;
		y = 0;
	}

	if (x + w >= SCREEN_WIDTH)
		w = SCREEN_WIDTH - x;

	if (y + h >= SCREEN_HEIGHT)
		h = SCREEN_HEIGHT - y;

	if (x % 2)
	{
		for (i = 0; i < h; i++)
			set_pixel(x, y + i, color);

		x++;
		w--;
	}

	if (w > 1)
	{
		v = &vram[y * SCREEN_WIDTH / 2 + x / 2];
		colorx2 = color & 0x0F;
		colorx2 |= colorx2 << 4;
		for (i = 0; i < h; i++)
		{
			memset(v, colorx2, w / 2);
			v += SCREEN_WIDTH / 2;
		}
	}

	if (w % 2)
	{
		for (i = 0; i < h; i++)
			set_pixel(x + w - 1, y + i, color);
	}
}

void draw_char(int x, int y, unsigned char ch, unsigned char color)
{
	unsigned char i, j, w, h, d;

	const unsigned char *k;

	w = font8x8[0];
	h = font8x8[1];

	if (ch < 32)
		k = &font8x8[(('.' - 32) * h) + 2];
	else
		k = &font8x8[((ch - 32) * h) + 2];

	for (i = 0; i < h; i++)
	{
		d = *k;
		for (j = 0; j < w; j++)
		{
			if (d & 0x80)
				set_pixel(x + j, y + i, color);
			d <<= 1;
		}
		k++;
	}
}

void draw_text(int x, int y, char *text, unsigned char color)
{
	char *t;
	unsigned char ch;
	int tx, ty;

	t = text;
	tx = x;
	ty = y;

	while (1)
	{
		ch = *t;
		switch(ch) {
			case 0:
				return;
			case '\n':
				tx = x;
				ty += 8;
				break;
			case '\t':
				tx = tx & 0xFFC0;
				tx += 64;
				break;
			default:
				draw_char(tx, ty, ch, color);
				tx += 8;
				break;
		}
		t++;
	}
}

void clear_screen(int color)
{
	color &= 0x0F;
	color |= color << 4;
	memset(vram, color, SCREEN_WIDTH * SCREEN_HEIGHT / 2);
}

void wait_vsync(void)
{
	while (!(*vdu_io & 1));
}

void flip(void)
{
	wait_vsync();

	if (vram == (unsigned char *)VRAM_PAGE1)
	{
		*vdu_io = VRAM_PAGE1;
		vram = (unsigned char *)VRAM_PAGE2;
	}
	else
	{
		*vdu_io = VRAM_PAGE2;
		vram = (unsigned char *)VRAM_PAGE1;
	}
}
