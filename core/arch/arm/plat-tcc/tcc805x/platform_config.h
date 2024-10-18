/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2020-2023, Telechips Inc.
 */

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <mm/generic_ram_layout.h>
#include <util.h>

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		(64)

/* Peripherals */
#define TCC_IO_BASE		U(0x10000000)
#define TCC_IO_SIZE		U(0x10000000)

/* GIC */
#define GICD_BASE		U(0x17301000)
#define GICC_BASE		U(0x17302000)

/* Console */
#define CONSOLE_UART_BASE	U(0x16600000)
#define CONSOLE_UART_CLK_IN_HZ	(48000000)
#define CONSOLE_BAUDRATE	(115200)

/* TZC Cfg */
#define TZC_BASE		U(0xE8300000)
#define TZC_SIZE		U(0x00100000)
#define TZC_OMC_BASE		(TZC_BASE + U(0x80000))
#define TZC_OMC_FILTERS		U(4)
#define TZC_OMC_FILTER_OFFS	U(0x10000)
#define TZC_OMC_INT_0		(U(215) + U(32))
#define TZC_OMC_INT_1		(U(220) + U(32))
#define TZC_OMC_INT_2		(U(225) + U(32))
#define TZC_OMC_INT_3		(U(230) + U(32))
#define TZC_TEEOS_REGION_NUM    U(1)

/* MailBox with HSM */
#define MBOX_HSM_BASE		U(0x1E140000)
#define MBOX_FIFO_MAX		U(128)
#define IRQ_MBOX_HSM		(U(311) + U(32))

/* DRAM Info */
#define DRAM_BASE		U(0x20000000)
#define DRAM_SIZE		U(0xA0000000)	/* 2.5 GiB */
#define DRAM_BASE_EXT		U(0x1A0000000)

#endif /*PLATFORM_CONFIG_H*/
