#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "sdcard.h"

#define CMD0	(0)		/* GO_IDLE_STATE */
#define CMD1	(1)		/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)		/* SEND_IF_COND */
#define CMD9	(9)		/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

#define CS	0x10000

static int sd2 = 0, block = 0;
static int sd_type = SD_NONE;

static unsigned int spi_speed = 0xff000000u;

static unsigned char xchg(unsigned char data)
{
	*spi_data = data;
	return *spi_data;
}

static int sd_wait(void)
{
	int i;

	for (i = 0; i < 100000; i++)
	{
		if (xchg(0xFF) == 0xFF)
			return 1;
	}

	return 0;
}

static void sd_deselect(void)
{
	*spi = spi_speed | 0xFF;
}

static int sd_select(void)
{
	*spi = spi_speed | CS | 0xFF;

	if (sd_wait())
		return 1;

	sd_deselect();

	return 0;
}

static int sd_cmd(int cmd, unsigned int arg)
{
	int res, crc, i;

	if (cmd & 0x80)
	{
		cmd &= 0x7f;
		res = sd_cmd(CMD55, 0);
		if (res > 1)
			return res;
	}

	if (cmd != CMD12)
	{
		sd_deselect();
		if (!sd_select())
			return 0xff;
	}

	xchg(0x40 | cmd);
	xchg((arg >> 24u) & 0xFF);
	xchg((arg >> 16u) & 0xFF);
	xchg((arg >> 8u) & 0xFF);
	xchg((arg >> 0u) & 0xFF);

	crc = 1;
	if (cmd == CMD0) crc = 0x95;
	if (cmd == CMD8) crc = 0x87;
	xchg(crc);

	if (cmd == CMD12)
		xchg(0xff);

	for (i = 0; i < 10; i++)
	{
		res = xchg(0xFF);
		if ((res & 0x80) == 0)
			break;
	}

	return res;
}

static unsigned int sd_read4(void)
{
	unsigned int res;

	res  = xchg(0xFF) << 24u;
	res |= xchg(0xFF) << 16u;
	res |= xchg(0xFF) << 8u;
	res |= xchg(0xFF) << 0u;

	return res;
}

int sd_init(void)
{
	int i, res, cmd;
	int type = SD_NONE;
	char s[80];

	spi_speed = 0xff000000u;

	*spi = 0xff0000ff;

	sd_deselect();

	for (i = 0; i < 10; i++)
		*spi = 0xff0000ff;

	if (sd_cmd(CMD0, 0) == 1)
	{
		if (sd_cmd(CMD8, 0x1AA) == 1)
		{
			if (sd_read4() == 0x1AA)
			{
				for (i = 0; i < 10000; i++)
				{
					if (sd_cmd(ACMD41, 1u << 30) == 0)
					{
						type = SD_V2;
						break;
					}
				}
				if ((type == SD_V2) && (sd_cmd(CMD58, 0) == 0))
				{
					if (sd_read4() & 0x40000000)
						type = SD_V2_BLOCK;
				}
			}
		}
		else
		{
			if (sd_cmd(ACMD41, 0) <= 1)
			{
				type = SD_V1;
				cmd = ACMD41;
			}
			else
			{
				type = SD_MMC;
				cmd = CMD1;
			}

			for (i = 0; i < 10000; i++)
			{
				if (sd_cmd(cmd, 0) == 0)
					break;
			}

			if (sd_cmd(CMD16, 512) != 0)
				type = SD_NONE;
		}
	}
	sd_deselect();

	spi_speed = (SDCARD_SPI_DIV & 0xFF) << 24u;

	return type;
}

int sd_read(void* buffer, unsigned int sector)
{
	int i, res = 0;
	unsigned int* buf = (unsigned int*)buffer;
	unsigned int v;

	//if (sd_type != SD_V2_BLOCK)
	//	sector <<= 9;

	if (sd_cmd(CMD17, sector) != 0)
	{
		sd_deselect();
		return 0;
	}

	for (i = 0; i < 10000; i++)
	{
		res = xchg(0xFF);
		if (res != 0xFF)
			break;
	}

	if (res != 0xFE)
		return 0;

	for (i = 0; i < 128; i++)
	{
		v  = xchg(0xFF) << 0u;
		v |= xchg(0xFF) << 8u;
		v |= xchg(0xFF) << 16u;
		v |= xchg(0xFF) << 24u;
		buf[i] = v;
	}

	xchg(0xFF);
	xchg(0xFF);

	sd_deselect();

	return 1;
}
