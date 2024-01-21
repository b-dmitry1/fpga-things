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

#define NUM_KEYS		5

#define KEY_LEFT		0
#define KEY_RIGHT		1
#define KEY_DOWN		2
#define KEY_ROTATE_LEFT		3
#define KEY_ROTATE_RIGHT	4

const unsigned int key_masks[NUM_KEYS] = { 0x01, 0x02, 0x04, 0, 0x08 };

extern const unsigned char *font8x8;

typedef struct
{
	int piece_x;
	int piece_y;
	int shape;
	int rotation;
	int piece_color;
	int down_counter;
	int score;
} data_t;

data_t d;

typedef struct
{
	int prev_buttons;

	int keys[NUM_KEYS];
	int trig[NUM_KEYS];
	int prev_trig[NUM_KEYS];

	int trig_any;
} input_t;

input_t input;

typedef enum
{
	s_intro, s_game, s_gameover
} state_t;

state_t state = s_intro;
int state_timeout = 0;

unsigned char field[FIELD_HEIGHT][FIELD_WIDTH];

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
	(A|B|C|D|E|F),
	(B|C),
	(A|B|D|E|G),
	(A|B|C|D|G),
	(B|C|F|G),
	(A|C|D|F|G),
	(A|C|D|E|F|G),
	(A|B|C),
	(A|B|C|D|E|F|G),
	(A|B|C|D|F|G),
	(A|B|C|E|F|G),
	(C|D|E|F|G),
	(A|D|E|F),
	(B|C|D|E|G),
	(A|D|E|F|G),
	(A|E|F|G)
};

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

int new_piece(void)
{
	d.piece_x = FIELD_WIDTH / 2 - 1;
	d.piece_y = 0;
	d.shape = rand() % NUM_SHAPES;
	d.rotation = rand() % 4;
	d.piece_color = rand() % 4 + 3;
	d.down_counter = 0;

	if (!can_place(d.piece_x, d.piece_y, d.shape, d.rotation))
		state = s_gameover;

	draw_piece(d.piece_x, d.piece_y, d.shape, d.rotation, d.piece_color);
}

void new_game(void)
{
	srand(*timer);

	d.score = 0;

	fill_rect_field(0, 0, FIELD_WIDTH, FIELD_HEIGHT, WHITE);
	fill_rect_field(1, 0, FIELD_WIDTH - 2, FIELD_HEIGHT - 1, BLACK);

	new_piece();
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

void get_user_input(void)
{
	int buttons = get_buttons();
	int i;

	input.trig_any = 0;
	for (i = 0; i < NUM_KEYS; i++)
	{
		input.keys[i] = buttons & key_masks[i] ? 1 : 0;

		input.prev_trig[i] = input.trig[i];
		input.trig[i] = input.keys[i] && !(input.prev_buttons & key_masks[i]) ? 1 : 0;

		if (input.trig[i])
			input.trig_any = 1;

		if (input.keys[i] && input.prev_trig[i])
			input.keys[i] = 0;
	}

	input.prev_buttons = buttons;
}

void process_user_input(void)
{
	if (input.keys[KEY_LEFT])
	{
		if (!move_piece(-1, 0, 0))
			;
	}

	if (input.keys[KEY_RIGHT])
	{
		if (!move_piece(1, 0, 0))
			;
	}

	if (input.trig[KEY_ROTATE_LEFT])
	{
		if (!move_piece(0, 0, -1))
			;
	}

	if (input.trig[KEY_ROTATE_RIGHT])
	{
		if (!move_piece(0, 0, 1))
			;
	}

	if (input.keys[KEY_DOWN])
	{
		if (!move_piece(0, 1, 0))
			;
	}
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
	int i, j, gap, n = 0;
	const int scores[] = {1, 2, 5, 10};

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
			n++;
		}
	}

	if (n > 0)
		d.score += scores[n - 1];
}

void draw_big_char(int x, int y, unsigned char ch, unsigned char color)
{
	unsigned char i, j, w, h, d;

	const unsigned char *k;
	const unsigned char *font = font8x8;

	if (font == NULL)
		return;

	w = font[0];
	h = font[1];

	if (ch < 32)
		k = &font[(('.' - 32) * h) + 2];
	else
		k = &font[((ch - 32) * h) + 2];

	for (i = 0; i < h; i++)
	{
		d = *k;
		for (j = 0; j < w; j++)
		{
			if (d & 0x80)
				bitblt_color(x + j * BRICK_WIDTH, y + i * BRICK_HEIGHT, BRICK_WIDTH, BRICK_HEIGHT, brick, color);
			d <<= 1;
		}
		k++;
	}
}

void intro(void)
{
	static int y[6] = { 0 };
	int i;
	const char *msg = "TETRIS";
	static int last_buttons = 0;

	draw_text(240, 100, "Finger killer", 7);

	for (i = 0; i < 6; i++)
	{
		y[i] += rand() % 3 - 1;
		if (y[i] < -4)
			y[i] = -4;
		if (y[i] > 4)
			y[i] = 4;
		draw_big_char(i * BRICK_WIDTH * 13 / 2, SCREEN_HEIGHT / 2 - 100 + y[i], msg[i], 2 + i);
	}

	draw_text(40, 320, "With this game the OMDAZZ board will destroy your fingers in no time!", 7);

	draw_text(20, 380, "https://github.com/b-dmitry1/fpga-things", 6);                       

	draw_text(20, 400, "Press any key to start", 7);                       

	wait(10000);

	if (input.trig_any)
	{
		new_game();
		state = s_game;
	}
}

void game(void)
{
	char s[16];

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

	draw_field(48, 48);

	draw_text(300, 48, "Score: ", 7);

	int_to_str(d.score, s);

	draw_text(300 + 7 * 8, 48, s, 3);

	wait(20000);
}

void gameover(void)
{
	draw_field(48, 48);

	draw_text(300, SCREEN_HEIGHT / 2 - 4, "GAME OVER!", 7);

	wait(10000);

	if (input.trig_any)
		state = s_intro;
}

void update_leds(void)
{
	unsigned int res = 0;

	res  = digits[d.score / 1000 % 10];
	res <<= 8;
	res |= digits[d.score / 100 % 10];
	res <<= 8;
	res |= digits[d.score / 10 % 10];
	res <<= 8;
	res |= digits[d.score % 10];

	if (d.score < 1000) res &= 0xFFFFFFu;
	if (d.score < 100) res &= 0xFFFFu;
	if (d.score < 10) res &= 0xFFu;

	if (state != s_game && (*timer % 1000000 < 500000))
		*seg = 0;
	else
		*seg = res;
}

int main(void)
{
	// Hide LEDs
	*gpio = 0xFFFF;

	select_bit_font(font8x8);

	new_game();

	for (;;)
	{
		get_user_input();

		clear_screen(0);

		switch (state)
		{
			case s_intro:
				intro();
				break;
			case s_game:
				game();
				break;
			case s_gameover:
				gameover();
				break;
		}

		// Don't forget to flip video!
		flip();

		update_leds();
	}
}
