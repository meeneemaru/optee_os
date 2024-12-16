/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2022-2023, Telechips Inc.
 */

#ifndef TCC_OTPROM_H
#define TCC_OTPROM_H

#define OTPROM_MAX			U(0x2000)
#define OTPROM_128_START		U(0x0A00)
#define OTPROM_128_LIMIT		U(0x0B00)

/* HUK */
#define OTP_DATA_TEE_HUK_OFFSET		U(0x0AD0)
#define OTP_DATA_TEE_HUK_SIZE		U(0x10)

#endif /* TCC_OTPROM_H */
