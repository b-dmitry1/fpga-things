/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2008 David Gibson, IBM Corporation
 * Copyright (C) 2012 Regents of the University of California
 * Copyright (C) 2017 SiFive
 */

#include <linux/console.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/types.h>

#include <asm/sbi.h>

#include "hvc_console.h"

static void uart_console_putchar(int ch)
{
	volatile unsigned int *uart = (volatile unsigned int *)0x10000000u;

	while ((uart[1] & 0x6000) == 0);
	uart[0] = ch;
}

static int uart_console_getchar(void)
{
	volatile unsigned int *uart = (volatile unsigned int *)0x10000000u;

	if ((uart[1] & 0x100) == 0)
		return -1;

	return uart[0] & 0xFF;
}

static int hvc_sbi_tty_put(uint32_t vtermno, const char *buf, int count)
{
	int i;

	for (i = 0; i < count; i++)
		uart_console_putchar(buf[i]);

	return i;
}

static int hvc_sbi_tty_get(uint32_t vtermno, char *buf, int count)
{
	int i, c;

	for (i = 0; i < count; i++) {
		c = uart_console_getchar();
		if (c < 0)
			break;
		buf[i] = c;
	}

	return i;
}

static const struct hv_ops hvc_sbi_ops = {
	.get_chars = hvc_sbi_tty_get,
	.put_chars = hvc_sbi_tty_put,
};

static int __init hvc_sbi_init(void)
{
	return PTR_ERR_OR_ZERO(hvc_alloc(0, 0, &hvc_sbi_ops, 16));
}
device_initcall(hvc_sbi_init);

static int __init hvc_sbi_console_init(void)
{
	hvc_instantiate(0, 0, &hvc_sbi_ops);

	return 0;
}
console_initcall(hvc_sbi_console_init);
