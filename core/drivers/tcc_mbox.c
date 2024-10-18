// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2023, Telechips Inc.
 */

#include <kernel/delay.h>
#include <kernel/misc.h>
#include <initcall.h>
#include <stdint.h>
#include <string.h>
#include <platform_config.h>
#include <drivers/tcc_mbox.h>
#include <io.h>
#include <mm/core_memprot.h>

#define MBOX_CMD_TX_FIFO		U(0x00)
#define MBOX_CMD_RX_FIFO		U(0x20)
#define MBOX_CTRL			U(0x40)
#define MBOX_CMD_FIFO_STS		U(0x44)
#define MBOX_DAT_FIFO_TX_STS		U(0x50)
#define MBOX_DAT_FIFO_RX_STS		U(0x54)
#define MBOX_DAT_FIFO_TXD		U(0x60)
#define MBOX_DAT_FIFO_RXD		U(0x70)
#define MBOX_CTRL_SET			U(0x74)
#define MBOX_CTRL_CLR			U(0x78)
#define MBOX_TMN_STS			U(0x7C)

#define MBOX_CMD_RX_FIFO_COUNT_MASK	U(0xF)
#define MBOX_CMD_RX_FIFO_COUNT		U(20)
#define MBOX_CMD_RX_FIFO_FULL		U(17)
#define MBOX_CMD_RX_FIFO_EMPTY		U(16)
#define MBOX_CMD_TX_FIFO_COUNT_MASK	U(0xF)
#define MBOX_CMD_TX_FIFO_COUNT		U(4)
#define MBOX_CMD_TX_FIFO_FULL		U(1)
#define MBOX_CMD_TX_FIFO_EMPTY		U(0)
#define MBOX_CMD_FIFO_MAX_COUNT		U(8)

#define MBOX_DAT_TX_FIFO_COUNT_MASK	U(0xFFFF)
#define MBOX_DAT_TX_FIFO_COUNT		U(0)
#define MBOX_DAT_TX_FIFO_FULL		U(30)
#define MBOX_DAT_TX_FIFO_EMPTY		U(31)

#define MBOX_DAT_RX_FIFO_COUNT_MASK	U(0xFFFF)
#define MBOX_DAT_RX_FIFO_COUNT		U(0)
#define MBOX_DAT_RX_FIFO_FULL		U(30)
#define MBOX_DAT_RX_FIFO_EMPTY		U(31)

#define MBOX_DAT_FIFO_MAX_COUNT		U(128)

#define MBOX_CTRL_TEST			U(31)
#define MBOX_CTRL_ICLR_WRITE		U(21)
#define MBOX_CTRL_IEN_WRITE		U(20)
#define MBOX_CTRL_DF_FLUSH		U(7)
#define MBOX_CTRL_CF_FLUSH		U(6)
#define MBOX_CTRL_OEN			U(5)
#define MBOX_CTRL_IEN_READ		U(4)
#define MBOX_CTRL_ILEVEL		U(0)

#define MBOX_OPP_TMN_STS		U(16)
#define MBOX_OWN_TMN_STS		U(0)

#define MBOX_ILEVEL_NEMP		U(0x0)
#define MBOX_ILEVEL_GT2			U(0x1)
#define MBOX_ILEVEL_GT4			U(0x2)
#define MBOX_ILEVEL_FULL		U(0x3)

struct mbox_chan {
	vaddr_t reg_base;
	uint32_t data_max_num;
};

struct mbox_msg {
	uint32_t cmd[8];
	uint32_t *data_buf;
	uint32_t data_num;
};

static struct mbox_chan *s_chan;
static struct mutex tcc_mbox_mutex = MUTEX_INITIALIZER;

static uint32_t rx_data[MBOX_FIFO_MAX];

static inline uint32_t mbox_readl(const struct mbox_chan *chan, uint32_t offset)
{
	return io_read32(chan->reg_base + offset);
}

static inline void mbox_writel(const struct mbox_chan *chan, uint32_t val,
			       uint32_t offset)
{
	io_write32(chan->reg_base + offset, val);
}

static int mbox_check_tx_done(const struct mbox_chan *chan)
{
	uint32_t val;
	int ret = -1;

	if (chan) {
		val = mbox_readl(chan, MBOX_CMD_FIFO_STS);
		if (((val >> MBOX_CMD_TX_FIFO_EMPTY) & U(0x1)) != U(0)) {
			val = mbox_readl(chan, MBOX_DAT_FIFO_TX_STS);
			if (((val >> MBOX_DAT_TX_FIFO_EMPTY) & U(0x1)) != U(0))
				ret = 0;
		}
	}

	return ret;
}

static void mbox_tx_done(const struct mbox_chan *chan)
{
	uint32_t val;

	val = mbox_readl(chan, MBOX_CTRL);
	val &= ~(U(0x1) << MBOX_CTRL_OEN);
	val |= ((U(0x1) << MBOX_CTRL_CF_FLUSH) |
		(U(0x1) << MBOX_CTRL_DF_FLUSH) |
		(MBOX_ILEVEL_FULL << MBOX_CTRL_ILEVEL));
	mbox_writel(chan, val, MBOX_CTRL);
}

static uint32_t mbox_get_cmd_rx_fifo_cnt(const struct mbox_chan *chan)
{
	return (mbox_readl(chan, MBOX_CMD_FIFO_STS) >> MBOX_CMD_RX_FIFO_COUNT)
		& MBOX_CMD_RX_FIFO_COUNT_MASK;
}

static uint32_t mbox_get_data_rx_fifo_cnt(const struct mbox_chan *chan)
{
	return (mbox_readl(chan, MBOX_DAT_FIFO_RX_STS) >>
			MBOX_DAT_RX_FIFO_COUNT)
		& MBOX_DAT_RX_FIFO_COUNT_MASK;
}

static TEE_Result mbox_send_message(const struct mbox_chan *chan,
				    const struct mbox_msg *msg)
{
	TEE_Result res;
	uint32_t val;
	uint32_t i;

	if (!chan || !msg) {
		res = TEE_ERROR_BAD_PARAMETERS;
	} else if (msg->data_num > chan->data_max_num) {
		res = TEE_ERROR_BAD_PARAMETERS;
	} else if ((msg->data_num > U(0)) && !msg->data_buf) {
		res = TEE_ERROR_BAD_PARAMETERS;
	} else if (mbox_check_tx_done(chan) != 0) {
		res = TEE_ERROR_BAD_STATE;
	} else {
		/* write cmd to fifo */
		for (i = 0; i < MBOX_CMD_FIFO_MAX_COUNT; i++)
			mbox_writel(chan, msg->cmd[i],
				    (MBOX_CMD_TX_FIFO + (i * U(4))));

		/* write data to fifo */
		for (i = 0; i < msg->data_num; i++)
			mbox_writel(chan, msg->data_buf[i], MBOX_DAT_FIFO_TXD);

		/* set oen */
		val = mbox_readl(chan, MBOX_CTRL);
		val |= (U(0x1) << MBOX_CTRL_OEN);
		mbox_writel(chan, val, MBOX_CTRL);

		do {
		} while (mbox_check_tx_done(chan) != 0);

		mbox_tx_done(chan);

		res = TEE_SUCCESS;
	}

	return res;
}

static TEE_Result mbox_recv_message(const struct mbox_chan *chan,
				    struct mbox_msg *msg)
{
	TEE_Result res = TEE_ERROR_NOT_SUPPORTED;
	uint32_t fifo_cnt, cnt = U(100);
	uint32_t i;

	if (!chan || !msg) {
		res = TEE_ERROR_BAD_PARAMETERS;
	} else {
		/* read cmd */
		do {
			fifo_cnt = mbox_get_cmd_rx_fifo_cnt(chan);
			if (fifo_cnt == MBOX_CMD_FIFO_MAX_COUNT)
				break;

			mdelay(1);
			cnt--;
		} while (cnt != U(0));

		if (cnt == U(0))
			res = TEE_ERROR_BAD_STATE;
		else
			res = TEE_SUCCESS;
	}

	if (res == TEE_SUCCESS) {
		for (i = 0; i < fifo_cnt; i++)
			msg->cmd[i] =
				mbox_readl(chan, MBOX_CMD_RX_FIFO + (i * U(4)));

		msg->data_num = msg->cmd[1] & U(0x3FF);

		fifo_cnt = mbox_get_data_rx_fifo_cnt(chan);
		if (fifo_cnt > MBOX_DAT_FIFO_MAX_COUNT) {
			res = TEE_ERROR_BAD_STATE;
		} else if (fifo_cnt != U(0)) {
			/* read data */
			for (i = 0; i < fifo_cnt; i++) {
				if (!msg->data_buf ||
				    (msg->data_num < ((i * U(4)) + U(1))))
					(void)mbox_readl(chan,
							 MBOX_DAT_FIFO_RXD);
				else
					msg->data_buf[i] =
						mbox_readl(chan,
							   MBOX_DAT_FIFO_RXD);
			}
		}
	}

	return res;
}

TEE_Result send_mbox(const struct tcc_mbox_msg *msg,
		     struct tcc_mbox_msg *rx_msg)
{
	TEE_Result res = TEE_ERROR_BAD_STATE;
	const struct mbox_chan *chan = (const struct mbox_chan *)s_chan;
	struct mbox_msg mbox_message;

	if (!chan)
		res = TEE_ERROR_BAD_STATE;
	else if (!msg || !rx_msg)
		res = TEE_ERROR_BAD_PARAMETERS;
	else if (msg->msg_len > TCC_MBOX_MAX_MSG)
		res = TEE_ERROR_BAD_PARAMETERS;
	else
		res = TEE_SUCCESS;

	if (res == TEE_SUCCESS) {
		(void)memset(&mbox_message, 0x0, sizeof(struct mbox_msg));
		(void)memset(rx_msg, 0x0, sizeof(struct tcc_mbox_msg));
		mbox_message.cmd[0] = U(0);
		mbox_message.cmd[1] = msg->cmd | msg->msg_len;
		if (msg->message && msg->msg_len > U(0)) {
			mbox_message.cmd[1] |= U(0x400);
			mbox_message.data_buf = msg->message;
			mbox_message.data_num = (msg->msg_len + U(3)) / U(4);
		} else {
			mbox_message.data_buf = NULL;
			mbox_message.data_num = 0;
		}
	}

	mutex_lock(&tcc_mbox_mutex);
	if (res == TEE_SUCCESS)
		res = mbox_send_message(chan, &mbox_message);

	if (res == TEE_SUCCESS) {
		mbox_message.data_buf = rx_data;
		res = mbox_recv_message(chan, &mbox_message);
	}
	if (res == TEE_SUCCESS) {
		rx_msg->cmd = mbox_message.cmd[1] & U(0xFFFF0000);
		rx_msg->msg_len = mbox_message.cmd[1] & U(0x3FF);
		if (rx_msg->msg_len != U(0))
			rx_msg->message = (uint32_t *)malloc(rx_msg->msg_len);

		if (((mbox_message.cmd[1] & U(0x400)) == U(0)) &&
		    rx_msg->msg_len <= U(0x18))
			(void)memcpy(rx_msg->message, &mbox_message.cmd[2],
				     rx_msg->msg_len);
		else
			(void)memcpy(rx_msg->message, rx_data, rx_msg->msg_len);

		res = TEE_SUCCESS;
	}
	mutex_unlock(&tcc_mbox_mutex);

	return res;
}

static TEE_Result mbox_init(void)
{
	static struct mbox_chan chan;
	TEE_Result res = TEE_SUCCESS;
	uint32_t val;

	(void)memset(&chan, 0x0, sizeof(struct mbox_chan));

	chan.reg_base =
		(vaddr_t)phys_to_virt_io(MBOX_HSM_BASE, U(0x1000));
	chan.data_max_num = MBOX_FIFO_MAX;

	val = mbox_readl(&chan, MBOX_CTRL);
	val |= ((U(0x1) << MBOX_CTRL_CF_FLUSH) |
		(U(0x1) << MBOX_CTRL_DF_FLUSH));
	val &= ~((U(0x1) << MBOX_CTRL_OEN) |
		(U(0x1) << MBOX_CTRL_IEN_READ) |
		(MBOX_ILEVEL_FULL << MBOX_CTRL_ILEVEL));

	mbox_writel(&chan, val, MBOX_CTRL);
	s_chan = &chan;

	return res;
}
service_init(mbox_init);
