#ifndef VGA_H
#define VGA_H

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

#define BLACK           0
#define BLUE            1
#define GREEN           2
#define CYAN            3
#define RED             4
#define MAGENTA         5
#define YELLOW          6
#define WHITE           7

void set_pixel(int x, int y, int color);
void hline(int x, int y, int l, int color);
void fill_rect(int x, int y, int w, int h, int color);
void bitblt(int x, int y, int w, int h, const void *bitmap);
void bitblt_color(int x, int y, int w, int h, const void *bitmap, unsigned int color);
void draw_char(int x, int y, unsigned char ch, unsigned char color);
void draw_text(int x, int y, char *text, unsigned char color);
void clear_screen(int color);
void wait_vsync(void);
void flip(void);

#endif
