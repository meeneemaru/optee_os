/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2022-2023, Telechips Inc.
 */

#ifndef TCC_OTPROM_H
#define TCC_OTPROM_H

#define OTPROM_MAX			U(0x2000)

/* Unique ID */
#define OTP_DATA_TEE_UID_OFFSET		U(0x0580)
#define OTP_DATA_TEE_UID_SIZE		U(0x10)

/* Product ID */
#define OTP_DATA_TEE_PID_OFFSET		U(0x0590)
#define OTP_DATA_TEE_PID_SIZE		U(0x10)

/* TA Root Encryption Key */
#define OTP_DATA_TA_ENC_KEY_OFFSET	U(0x05A0)
#define OTP_DATA_TA_ENC_KEY_SIZE	U(0x10)

#endif /* TCC_OTPROM_H */
