/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2020-2023, Telechips Inc.
 */

#ifndef TCC_OTPROM_H
#define TCC_OTPROM_H

#define OTPROM_MAX			U(0x4000)

/* Unique ID */
#define OTP_DATA_TEE_UID_OFFSET		U(0x1ED0)
#define OTP_DATA_TEE_UID_SIZE		U(0x10)

/* Product ID */
#define OTP_DATA_TEE_PID_OFFSET		U(0x1EE0)
#define OTP_DATA_TEE_PID_SIZE		U(0x10)

/* TA Root Encryption Key */
#define OTP_DATA_TA_ENC_KEY_OFFSET	U(0x1EF0)
#define OTP_DATA_TA_ENC_KEY_SIZE	U(0x10)

#endif /* TCC_OTPROM_H */
