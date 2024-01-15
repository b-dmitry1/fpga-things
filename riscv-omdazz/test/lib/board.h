#ifndef BOARD_H
#define BOARD_H

#define SRAM_AREA       0x00000000u
#define GPIO_AREA       0x10000000u
#define UART_AREA       0x11000000u
#define VDU_IO_AREA     0x12000000u
#define TIMER_AREA      0x13000000u
#define SEG_AREA        0x14000000u

#define SDRAM_AREA      0x80000000u

#define VRAM_PAGE1      0x807A0000u
#define VRAM_PAGE2      0x807D0000u

extern unsigned char *sram;
extern volatile unsigned int  *gpio;
extern volatile unsigned int  *uart;
extern volatile unsigned int  *vdu_io;
extern volatile unsigned int  *timer;
extern volatile unsigned int  *seg;

extern unsigned char *sdram;

extern unsigned char *vram;

int get_buttons(void);
void print(const char* s);

#endif
