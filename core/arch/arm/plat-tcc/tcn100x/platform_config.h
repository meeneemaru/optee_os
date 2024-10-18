/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2022-2023, Telechips Inc.
 */

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <mm/generic_ram_layout.h>
#include <util.h>

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		(64)

/* Peripherals */
#define TCC_IO_BASE		U(0x40000000)
#define TCC_IO_SIZE		U(0x10000000)

/* PMU */
#define PMU_BASE		U(0x4B200000)

/* GIC */
#define GICD_BASE		U(0x40800000)
#define GICC_BASE		U(0x40880000)

/* Console */
#define CONSOLE_UART_BASE	U(0x4A100000)
#define CONSOLE_UART_CLK_IN_HZ	(48000000)
#define CONSOLE_BAUDRATE	(115200)

/* RNG */
#define RNG_BASE		U(0x4B700000)

/* TZC Cfg */
#define TZC_OMC_BASE		U(0x45060000)
#define TZC_OMC_FILTERS		U(2)
#define TZC_OMC_FILTER_OFFS	U(0x20000)
#define TZC_OMC_INT_0		(U(324) + U(32))
#define TZC_OMC_INT_1		(U(323) + U(32))
#define TZC_TEEOS_REGION_NUM    U(1)

/* MailBox with HSM */
#define MBOX_HSM_BASE		U(0x44170000)
#define MBOX_FIFO_MAX		U(128)
#define IRQ_MBOX_HSM		(U(167) + U(32))

/* DRAM Info */
#define DRAM_BASE		U(0x100000000)
#define DRAM_SIZE		U(0xA0000000)	/* 2.5 GiB */

#endif /*PLATFORM_CONFIG_H*/
