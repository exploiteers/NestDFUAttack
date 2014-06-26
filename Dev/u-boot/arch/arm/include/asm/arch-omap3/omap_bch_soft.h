/*
 * omap_bch_soft.h
 *
 * Header for support modules for BCH 4-bit/8-bit error correction.
 *
 * Copyright (C) {2011} Texas Instruments Incorporated - http://www.ti.com/
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __ASM_ARCH_OMAP_BCH_SOFT_H
#define __ASM_ARCH_OMAP_BCH_SOFT_H

#include <linux/mtd/nand.h>

struct nand_ecclayout *omap_get_ecc_layout_bch(int bus_width, 
                                               int correct_bits);
void omap_hwecc_init_bch(struct nand_chip *chip);
int omap_correct_data_bch4(struct mtd_info *mtd, uint8_t *dat,
				  uint8_t *read_ecc, uint8_t *calc_ecc);
int omap_correct_data_bch8(struct mtd_info *mtd, uint8_t *dat,
				  uint8_t *read_ecc, uint8_t *calc_ecc);
int omap_calculate_ecc_bch4(struct mtd_info *mtd, const uint8_t *dat,
				   uint8_t *ecc_code);
int omap_calculate_ecc_bch8(struct mtd_info *mtd, const uint8_t *dat,
				   uint8_t *ecc_code);
void omap_enable_hwecc_bch4(struct mtd_info *mtd, int32_t mode);
void omap_enable_hwecc_bch8(struct mtd_info *mtd, int32_t mode);

#endif
