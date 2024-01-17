#include "board.h"

unsigned char *sram              = (unsigned char *)SRAM_AREA;

volatile unsigned int  *gpio     = (volatile unsigned int  *)GPIO_AREA;
volatile unsigned int  *uart     = (volatile unsigned int  *)UART_AREA;
volatile unsigned int  *vdu_io   = (volatile unsigned int  *)VDU_IO_AREA;
volatile unsigned int  *timer    = (volatile unsigned int  *)TIMER_AREA;
volatile unsigned int  *seg      = (volatile unsigned int  *)SEG_AREA;
volatile unsigned int  *spi      = (volatile unsigned int  *)SPI_AREA;
volatile unsigned char *spi_data = (volatile unsigned char *)SPI_AREA;

unsigned char *sdram             = (unsigned char *)SDRAM_AREA;

unsigned char *vram              = (unsigned char *)VRAM_PAGE1;

int get_buttons(void)
{
	return (gpio[0] & 0x0F) ^ 0x0F;
}
