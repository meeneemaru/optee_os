// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2017-2022, Telechips Inc.
 */

#include <console.h>
#include <io.h>
#include <kernel/boot.h>
#include <kernel/tee_common_otp.h>
#include <drivers/gic.h>
#include <drivers/pl011.h>
#include <drivers/tcc_hsm.h>
#include <crypto/crypto.h>
#include <platform_config.h>
#include <otprom.h>

register_phys_mem(MEM_AREA_IO_SEC, TCC_IO_BASE, TCC_IO_SIZE);
#if defined(TZC_BASE)
register_phys_mem(MEM_AREA_IO_SEC, TZC_BASE, TZC_SIZE);
#endif

void boot_primary_init_intc(void)
{
	gic_init(GICC_BASE, GICD_BASE);
}

void boot_secondary_init_intc(void)
{
	gic_init_per_cpu();
}

void plat_console_init(void)
{
#if defined(CFG_PL011)
	static struct pl011_data console_data;

	pl011_init(&console_data, CONSOLE_UART_BASE, CONSOLE_UART_CLK_IN_HZ,
		   CONSOLE_BAUDRATE);
	register_serial_console(&console_data.chip);
#endif
}

TEE_Result tee_otp_get_hw_unique_key(struct tee_hw_unique_key *hwkey)
{
	static uint32_t plat_huk[OTP_DATA_TEE_UID_SIZE / 4U] = { 0 };
	static bool is_first = true;

	if (is_first) {
		/* init h/w unique key */
		uint32_t cnt, res = 0;

		res = otprom_read(plat_huk, OTP_DATA_TEE_UID_OFFSET,
				  OTP_DATA_TEE_UID_SIZE);
		if (res != OTP_DATA_TEE_UID_SIZE)
			panic();

		for (cnt = 0; cnt < (OTP_DATA_TEE_UID_SIZE / 4U); cnt++) {
			if (plat_huk[cnt] != 0U)
				break;
		}

		/* Write HUK into OTP if OTP is empty */
		if (cnt == (OTP_DATA_TEE_UID_SIZE / 4U)) {
			/* generate huk and write it into OTP */
			(void)crypto_rng_read(plat_huk, OTP_DATA_TEE_UID_SIZE);
			res = otprom_write((const uint32_t *)plat_huk,
					   OTP_DATA_TEE_UID_OFFSET,
					   OTP_DATA_TEE_UID_SIZE);

			/* read huk from OTP */
			res = otprom_read(plat_huk, OTP_DATA_TEE_UID_OFFSET,
					  OTP_DATA_TEE_UID_SIZE);
			if (res != OTP_DATA_TEE_UID_SIZE)
				panic();
		}
	}
	is_first = false;

	(void)memset(&hwkey->data[0], 0x0, sizeof(hwkey->data));
	(void)memcpy(&hwkey->data[0], &plat_huk[0], OTP_DATA_TEE_UID_SIZE);

	return TEE_SUCCESS;
}

