/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2022-2023, Telechips Inc.
 */

#ifndef __DRIVERS_TCC_HSMOTP_H
#define __DRIVERS_TCC_HSMOTP_H

uint32_t otprom_read(uint32_t *rdata, uint32_t offset, uint32_t bsize);
uint32_t otprom_write(const uint32_t *wdata, uint32_t offset, uint32_t bsize);

#endif /* __DRIVERS_TCC_HSMOTP_H */
