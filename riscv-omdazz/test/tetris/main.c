#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "vga16color.h"

#define FIELD_WIDTH		12
#define FIELD_HEIGHT		22
#define BRICK_WIDTH		16
#define BRICK_HEIGHT		16
#define BRICK_GAP		2

#define NUM_SHAPES		8

#define FRAMES_BEFORE_DOWN	7

typedef struct
{
	int piece_x;
	int piece_y;
	int shape;
	int rotation;
	int piece_color;
	int down_counter;
} data_t;

data_t d;

unsigned char field[FIELD_HEIGHT][FIELD_WIDTH];

unsigned int brick[16][4] = {
	0xFFFFFFFF, 0x0FFFFFFF,
	0x0000000F, 0x0F000000,
	0x0000000F, 0x0F000000,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0xFFFFF00F, 0x0F0FFFFF,
	0x0000000F, 0x0F000000,
	0xFFFFFFFF, 0x0FFFFFFF,
	0x00000000, 0x00000000
};

const unsigned char shapes[4][8][4][5] = {
	{
		{
			"oo  ",
			"oo  ",
			"    ",
			"    "
		},
		{
			"o   ",
			"o   ",
			"o   ",
			"o   "
		},
		{
			" o  ",
			"ooo ",
			"    ",
			"    "
		},
		{
			"oo  ",
			"o   ",
			"o   ",
			"    "
		},
		{
			"oo  ",
			" o  ",
			" o  ",
			"    "
		},
		{
			"o   ",
			"oo  ",
			" o  ",
			"    "
		},
		{
			" o  ",
			"oo  ",
			"o   ",
			"    "
		},
		{
			"o o ",
			"ooo ",
			"    ",
			"    "
		}
	},
	{
		{
			"oo  ",
			"oo  ",
			"    ",
			"    "
		},
		{
			"oooo",
			"    ",
			"    ",
			"    "
		},
		{
			" o  ",
			" oo ",
			" o  ",
			"    "
		},
		{
			"ooo ",
			"  o ",
			"    ",
			"    "
		},
		{
			"  o ",
			"ooo ",
			"    ",
			"    "
		},
		{
			" oo ",
			"oo  ",
			"    ",
			"    "
		},
		{
			"oo  ",
			" oo ",
			"    ",
			"    "
		},
		{
			"oo  ",
			"o   ",
			"oo  ",
			"    "
		}
	},
	{
		{
			"oo  ",
			"oo  ",
			"    ",
			"    "
		},
		{
			"o   ",
			"o   ",
			"o   ",
			"o   "
		},
		{
			"    ",
			"ooo ",
			" o  ",
			"    "
		},
		{
			" o  ",
			" o  ",
			"oo  ",
			"    "
		},
		{
			"o   ",
			"o   ",
			"oo  ",
			"    "
		},
		{
			"o   ",
			"oo  ",
			" o  ",
			"    "
		},
		{
			" o  ",
			"oo  ",
			"o   ",
			"    "
		},
		{
			"ooo ",
			"o o ",
			"    ",
			"    "
		}
	},
	{
		{
			"oo  ",
			"oo  ",
			"    ",
			"    "
		},
		{
			"oooo",
			"    ",
			"    ",
			"    "
		},
		{
			" o  ",
			"oo  ",
			" o  ",
			"    "
		},
		{
			"o   ",
			"ooo ",
			"    ",
			"    "
		},
		{
			"ooo ",
			"o   ",
			"    ",
			"    "
		},
		{
			" oo ",
			"oo  ",
			"    ",
			"    "
		},
		{
			"oo  ",
			" oo ",
			"    ",
			"    "
		},
		{
			"oo  ",
			" o  ",
			"oo  ",
			"    "
		}
	}
};

void draw_field(int x, int y)
{
	int i, j;

	for (i = 0; i < FIELD_HEIGHT; i++)
	{
		for (j = 0; j < FIELD_WIDTH; j++)
		{
			bitblt_color(x + j * BRICK_WIDTH, y + i * BRICK_HEIGHT, BRICK_WIDTH, BRICK_HEIGHT, brick, field[i][j]);
		}
	}
}

void fill_rect_field(int x, int y, int w, int h, int color)
{
	int i, j;
	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++)
			field[y + i][x + j] = color;
}

void draw_piece(int x, int y, int number, int rotation, int color)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (shapes[rotation][number][i][j] == 'o')
				field[y + i][x + j] = color;
}

void new_piece(void)
{
	d.piece_x = FIELD_WIDTH / 2 - 1;
	d.piece_y = 0;
	d.shape = rand() % NUM_SHAPES;
	d.rotation = rand() % 4;
	d.piece_color = rand() % 4 + 3;
	d.down_counter = 0;

	draw_piece(d.piece_x, d.piece_y, d.shape, d.rotation, d.piece_color);
}

void new_game(void)
{
	fill_rect_field(0, 0, FIELD_WIDTH, FIELD_HEIGHT, WHITE);
	fill_rect_field(1, 0, FIELD_WIDTH - 2, FIELD_HEIGHT - 1, BLACK);

	new_piece();
}

int can_place(int x, int y, int shape, int rotation)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (shapes[rotation][shape][i][j] == 'o')
				if (field[y + i][x + j] != BLACK)
					return 0;
	return 1;
}

void wait(int count)
{
	volatile int w;
	for (w = 0; w < count; w++);
}

int rotate(int drot)
{
	int res = d.rotation;
	res += drot;
	if (res < 0)
		res = 3;
	if (res > 3)
		res = 0;
	return res;
}

int move_piece(int dx, int dy, int drot)
{
	draw_piece(d.piece_x, d.piece_y, d.shape, d.rotation, BLACK);
	if (can_place(d.piece_x + dx, d.piece_y + dy, d.shape, rotate(drot)))
	{
		d.piece_x += dx;
		d.piece_y += dy;
		d.rotation = rotate(drot);
		draw_piece(d.piece_x, d.piece_y, d.shape, d.rotation, d.piece_color);
		return 1;
	}
	draw_piece(d.piece_x, d.piece_y, d.shape, d.rotation, d.piece_color);
	return 0;
}

void process_user_input(void)
{
	int buttons = get_buttons();
	static int prev_buttons = 0;

	int left         = buttons & 0x01;
	int right        = buttons & 0x02;
	int rotate_left  = 0;
	int rotate_right = buttons & 0x04;
	int down         = buttons & 0x08;

	int prev_rotate_left  = 0;
	int prev_rotate_right = prev_buttons & 0x04;

	if (left)
	{
		if (!move_piece(-1, 0, 0))
			;
	}

	if (right)
	{
		if (!move_piece(1, 0, 0))
			;
	}

	if (rotate_left && !prev_rotate_left)
	{
		if (!move_piece(0, 0, -1))
			;
	}

	if (rotate_right && !prev_rotate_right)
	{
		if (!move_piece(0, 0, 1))
			;
	}

	if (down)
	{
		if (!move_piece(0, 1, 0))
			;
	}

	prev_buttons = buttons;
}

void drop_row(int row)
{
	int i, j;

	for (i = row; i > 0; i--)
		for (j = 1; j < FIELD_WIDTH - 1; j++)
			field[i][j] = field[i - 1][j];

	for (j = 1; j < FIELD_WIDTH - 1; j++)
		field[0][j] = BLACK;
}

void remove_full_rows(void)
{
	int i, j, gap;

	for (i = 0; i < FIELD_HEIGHT - 1; i++)
	{
		gap = 0;

		for (j = 1; j < FIELD_WIDTH - 1; j++)
		{
			if (field[i][j] == BLACK)
			{
				gap = 1;
				break;
			}
		}

		if (!gap)
		{
			drop_row(i);
		}
	}
}

int main(void)
{
	new_game();

	for (;;)
	{
		d.down_counter++;
		if (d.down_counter >= FRAMES_BEFORE_DOWN)
		{
			d.down_counter = 0;
			if (!move_piece(0, 1, 0))
			{
				remove_full_rows();
				new_piece();
			}
		}

		process_user_input();

		// Update image
		clear_screen(0);

		draw_field(48, 48);

		flip();

		// wait(5000);
	}
}
