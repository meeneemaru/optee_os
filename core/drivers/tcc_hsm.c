// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022, Telechips Inc.
 */

#include <mm/core_memprot.h>
#include <io.h>
#include <stdint.h>
#include <string.h>
#include <drivers/tcc_hsm.h>
#include <trace.h>
#include <drivers/tcc_mbox.h>
#include <initcall.h>
#include <tee/tee_cryp_utl.h>
#include <kernel/panic.h>

#define REQ_HSM_TEE_WRITE_OTP   U(0x30010000)
#define REQ_HSM_TEE_READ_OTP    U(0x30020000)
#define REQ_HSM_GET_RNG			U(0x10610000)

#define HSM_OTP_READ_TX_LEN	U(8)
#define HSM_DATA_LEN		(TCC_MBOX_MAX_MSG - HSM_OTP_READ_TX_LEN)

#define RNG_DWORD_LEN       (8)
#define RNG_DATA_SIZE       (RNG_DWORD_LEN*4)

uint32_t otprom_read(uint32_t *rdata, uint32_t offset, uint32_t bsize)
{
	struct tcc_mbox_msg mbox_tx, mbox_rx;
	uint32_t tx_data[HSM_OTP_READ_TX_LEN];
	uint32_t cnt = 0;
	TEE_Result res;

	if (bsize > HSM_DATA_LEN) {
		EMSG("Bad parameter: size:0x%x", bsize);
	} else {
		tx_data[0] = offset;
		tx_data[1] = bsize;

		mbox_tx.cmd = REQ_HSM_TEE_READ_OTP;
		mbox_tx.msg_len = HSM_OTP_READ_TX_LEN;
		mbox_tx.message = &tx_data[0];

		(void)memset(&mbox_rx, 0x0, sizeof(struct tcc_mbox_msg));
		res = send_mbox((const struct tcc_mbox_msg *)&mbox_tx,
				&mbox_rx);
		if (res == TEE_ERROR_NO_DATA) {
			EMSG("Cannot receive ACK");
		} else if (res != TEE_SUCCESS) {
			EMSG("%s offset:0x%x, size:0x%x, res:0x%x",
			     "Failed to read OTP",
			     offset, bsize, res);
		} else {
			if (mbox_rx.msg_len < HSM_DATA_LEN) {
				if (mbox_rx.message[0] == 0U) {
					(void)memcpy(rdata, &mbox_rx.message[2],
						     bsize);
					cnt = bsize;
				}
			}

			if (mbox_rx.type == MBOX_TYPE_ALLOC)
				free(mbox_rx.message);
		}
	}

	return cnt;
}

uint32_t otprom_write(const uint32_t *wdata, uint32_t offset, uint32_t bsize)
{
	struct tcc_mbox_msg mbox_tx, mbox_rx;
	uint32_t *tx_data = NULL;
	TEE_Result res;

	if (bsize > (TCC_MBOX_MAX_MSG - U(8))) {
		EMSG("Bad parameter: size:0x%x", bsize);
		bsize = 0;
	} else {
		tx_data = (uint32_t *)malloc((size_t)bsize + U(8));
		if (!tx_data) {
			EMSG("Failed to alloc memory");
			bsize = 0;
		}
	}

	if (bsize > U(0) && tx_data) {
		tx_data[0] = offset;
		tx_data[1] = bsize;
		(void)memcpy(&tx_data[2], wdata, bsize);

		mbox_tx.cmd = REQ_HSM_TEE_WRITE_OTP;
		mbox_tx.msg_len = bsize + U(8);
		mbox_tx.message = tx_data;
		(void)memset(&mbox_rx, 0x0, sizeof(struct tcc_mbox_msg));
		res = send_mbox((const struct tcc_mbox_msg *)&mbox_tx,
				&mbox_rx);
		if (res == TEE_ERROR_NO_DATA) {
			EMSG("Cannot receive ACK");
		} else if (res != TEE_SUCCESS) {
			EMSG("%s offset:0x%x, size:0x%x, res:0x%x",
			     "Failed to write OTP",
			     offset, bsize, res);
		} else {
			// Do Nothing
		}

		if (mbox_rx.msg_len != U(4) || !mbox_rx.message) {
			EMSG("Received data is bad format");
			bsize = 0;
		} else if (mbox_rx.message[0] != U(0)) {
			EMSG("%s offset:0x%x, size:0x%x, ret:0x%x",
			     "Failed to write OTP",
			     offset, bsize, mbox_rx.message[0]);
			bsize = 0;
		} else {
			DMSG("OTP Write Success");
		}

		if (mbox_rx.type == MBOX_TYPE_ALLOC) {
			if (mbox_rx.message)
				free(mbox_rx.message);
		}
	}
	free(tx_data);

	return bsize;
}

static inline TEE_Result rng_get_seed(uint32_t *rng)
{
	struct tcc_mbox_msg mbox_tx, mbox_rx;
	TEE_Result res;

	(void)memset(&mbox_tx, 0x0, sizeof(struct tcc_mbox_msg));
	(void)memset(&mbox_rx, 0x0, sizeof(struct tcc_mbox_msg));

	mbox_tx.cmd = REQ_HSM_GET_RNG;
	mbox_tx.msg_len = 8U;
	mbox_tx.message = rng;

	res = send_mbox((const struct tcc_mbox_msg *)&mbox_tx, &mbox_rx);
	if (res == TEE_SUCCESS) {
		if (mbox_rx.msg_len > 8U) {
			if ((mbox_rx.message[0] == U(0)) &&
				(mbox_rx.message[1] == (mbox_rx.msg_len - U(8)))) {
				(void)memcpy(rng, &mbox_rx.message[2], mbox_rx.message[1]);
			} else {
				EMSG("Failed to get RNG. res:0x%x, size:%d",
					mbox_rx.message[0], mbox_rx.message[1]);
				res = TEE_ERROR_BAD_STATE;
			}
		} else {
			res = TEE_ERROR_BAD_STATE;
		}

		if (mbox_rx.type == MBOX_TYPE_ALLOC) {
			free(mbox_rx.message);
		}
	} else {
		EMSG("Failed to get RNG. mbox res:0x%x", res);
	}

	return res;
}

void plat_rng_init(void)
{
	uint32_t rng[RNG_DWORD_LEN];

	rng[0] = 0U; /* 0: Alloc, 1: DMA */
	rng[1] = RNG_DATA_SIZE;
	if (rng_get_seed(&rng[0]) != TEE_SUCCESS) {
		panic("HSM TRNG not supported");
	}
	if (crypto_rng_init(&rng[0], RNG_DATA_SIZE) != TEE_SUCCESS) {
		panic();
	}
}
