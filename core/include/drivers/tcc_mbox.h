/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2018-2022, Telechips Inc.
 */

#ifndef __DRIVERS_TCC_MBOX_H
#define __DRIVERS_TCC_MBOX_H

#define TCC_MBOX_MAX_MSG	U(512)

enum mbox_type {
	MBOX_TYPE_ALLOC = 0,
	MBOX_TYPE_DMA,
};

struct tcc_mbox_msg {
	uint32_t	cmd;
	uint32_t	msg_len;
	enum mbox_type	type;
	uint32_t	dma_addr;
	uint32_t	*message;
};

TEE_Result send_mbox(const struct tcc_mbox_msg *msg,
		     struct tcc_mbox_msg *rx_msg);

#endif /* __DRIVERS_TCC_MBOX_H */
