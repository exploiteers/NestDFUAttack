/*
 * omap_bch_soft.c
 *
 * Support modules for BCH 4-bit/8-bit error correction.
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

#include <common.h>
#include <asm/arch/omap_bch_soft.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/mem.h>
#include <asm/arch/omap_gpmc.h>
#include <linux/mtd/nand_ecc.h>

#define GPMC_ECC_BCH_RESULT_0   0x240
extern uint8_t cs;

int decode_bch(int select_4_8, unsigned char *ecc, unsigned int *err_loc);

/* bus_width : 0 for x8, 1 for x16 */
static struct nand_ecclayout nand_bch_oob;
struct nand_ecclayout *omap_get_ecc_layout_bch(int bus_width, int correct_bits)
{
  int offset, ecc_romcode = 0;
  int i;

  offset = (bus_width == 0) ? 1 : 2;

  switch(correct_bits)
    {
    case 1: nand_bch_oob.eccbytes = 12; break;
    case 4: nand_bch_oob.eccbytes = 28; break;
    case 8: nand_bch_oob.eccbytes = 52; break;
    default: return 0;
    }

  if (!ecc_romcode)
    offset = (64 - nand_bch_oob.eccbytes);
  else
    offset = (bus_width == 0) ? 1 : 2;

  for (i = 0; i < nand_bch_oob.eccbytes; i++)
    nand_bch_oob.eccpos[i] = i + offset;

  offset = (bus_width == 0) ? 1 : 2;
  nand_bch_oob.oobfree->offset =
    (ecc_romcode ? (nand_bch_oob.eccbytes + offset) : offset);
  nand_bch_oob.oobfree->length = 64 - (nand_bch_oob.eccbytes + offset);

  return &nand_bch_oob;
};

/*
 * omap_hwecc_init - Initialize the Hardware ECC for NAND flash in
 *                   GPMC controller
 * @mtd:        MTD device structure
 *
 */
void omap_hwecc_init_bch(struct nand_chip *chip)
{
	/*
	 * Init ECC Control Register
	 * Clear all ECC | Enable Reg1
	 */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);
}

/* Implementation for 4b/8b BCH correction.  Pass either 4 or 8 into the 
   correct_bits parameter. */
static int omap_correct_data_bch(int correct_bits, struct mtd_info *mtd, 
                                 uint8_t *dat, uint8_t *read_ecc, 
                                 uint8_t *calc_ecc)
{
  int i=0, blockCnt=4, j, eccflag, count, corrected=0;
  int eccsize = (correct_bits == 8) ? 13 : 7;
  int mode = (correct_bits == 8) ? 1 : 0;
  unsigned int err_loc[8];

  if (correct_bits == 4)
    omap_calculate_ecc_bch4(mtd, dat, calc_ecc);
  else if (correct_bits == 8)
    omap_calculate_ecc_bch8(mtd, dat, calc_ecc);
  else
    return -1;  /* unsupported number of correction bits */

  for (i = 0; i < blockCnt; i++) {
    /* check if any ecc error */
    eccflag = 0;
    for (j = 0; (j < eccsize) && (eccflag == 0); j++)
      if (calc_ecc[j] != 0)
	eccflag = 1;

    if (eccflag == 1) {
      eccflag = 0;
      for (j = 0; (j < eccsize) && (eccflag == 0); j++)
	if (read_ecc[j] != 0xFF)
	  eccflag = 1;
    }

    if (eccflag == 1) {
      count = 0;
      /*printk(KERN_INFO "...bch correct(%d 512 byte)\n", i+1);*/
      count = decode_bch(mode, calc_ecc, err_loc);
      
      /*
       * If uncorrectable error detected in sub-page,
       * keep uncorrectable flag to upper layer
       */
      if(count == -1)
        corrected = -1;
      /*
       * If no uncorrectable error then increment
       * corrected count
       */
      if(corrected != -1)
        corrected += count;
			
      for (j = 0; j < count; j++) {
	/*printk(KERN_INFO "err_loc=%d\n", err_loc[j]);*/
        printf("err_loc=%d\n", err_loc[j]);
	if (err_loc[j] < 4096)
	  dat[err_loc[j] >> 3] ^= 1 << (err_loc[j] & 7);
	/* else, not interested to correct ecc */
      }

    }

    calc_ecc = calc_ecc + eccsize;
    read_ecc = read_ecc + eccsize;
    dat += 512;
  }

  return corrected;
}

/* Wrapper function for 4 bit BCH correction */
int omap_correct_data_bch4(struct mtd_info *mtd, uint8_t *dat,
				  uint8_t *read_ecc, uint8_t *calc_ecc)
{
  return omap_correct_data_bch(4, mtd, dat, read_ecc, calc_ecc);
}

/* Wrapper function for 8 bit BCH correction */
int omap_correct_data_bch8(struct mtd_info *mtd, uint8_t *dat,
				  uint8_t *read_ecc, uint8_t *calc_ecc)
{
  return omap_correct_data_bch(8, mtd, dat, read_ecc, calc_ecc);
}

/*
 *  omap_calculate_ecc_bch8 - Version for 8BIT BCH correction.
 *
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
int omap_calculate_ecc_bch8(struct mtd_info *mtd, const uint8_t *dat,
				   uint8_t *ecc_code)
{
  struct nand_chip *nand = mtd->priv;
  unsigned long reg, val1 = 0x0, val2 = 0x0;
  unsigned long val3 = 0x0, val4 = 0x0;
  int i;

  for (i = 0; i < nand->ecc.size/512; i++) {
    /* Reading HW ECC_BCH_Results
     * 0x240-0x24C, 0x250-0x25C, 0x260-0x26C, 0x270-0x27C
     */
    reg = (unsigned long)(GPMC_BASE +
			  GPMC_ECC_BCH_RESULT_0 + (0x10 * i));
    val1 = __raw_readl(reg);
    val2 = __raw_readl(reg + 4);
    val3 = __raw_readl(reg + 8);
    val4 = __raw_readl(reg + 12);

    *ecc_code++ = (val4 & 0xFF);
    *ecc_code++ = ((val3 >> 24) & 0xFF);
    *ecc_code++ = ((val3 >> 16) & 0xFF);
    *ecc_code++ = ((val3 >> 8) & 0xFF);
    *ecc_code++ = (val3 & 0xFF);
    *ecc_code++ = ((val2 >> 24) & 0xFF);

    *ecc_code++ = ((val2 >> 16) & 0xFF);
    *ecc_code++ = ((val2 >> 8) & 0xFF);
    *ecc_code++ = (val2 & 0xFF);
    *ecc_code++ = ((val1 >> 24) & 0xFF);
    *ecc_code++ = ((val1 >> 16) & 0xFF);
    *ecc_code++ = ((val1 >> 8) & 0xFF);
    *ecc_code++ = (val1 & 0xFF);
  }
  return 0;
}

/*
 *  omap_calculate_ecc_bch4 - Version for 4BIT BCH correction.
 *
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
int omap_calculate_ecc_bch4(struct mtd_info *mtd, const uint8_t *dat,
				   uint8_t *ecc_code)
{
  struct nand_chip *nand = mtd->priv;
  unsigned long reg, val1 = 0x0, val2 = 0x0;	
  int i;

  for (i = 0; i < nand->ecc.size/512; i++) {
    /* Reading HW ECC_BCH_Results
     * 0x240-0x24C, 0x250-0x25C, 0x260-0x26C, 0x270-0x27C
     */
    reg = (unsigned long)(GPMC_BASE +
			  GPMC_ECC_BCH_RESULT_0 + (0x10 * i));
    val1 = __raw_readl(reg);
    val2 = __raw_readl(reg + 4);

    *ecc_code++ = ((val2 >> 16) & 0xFF);
    *ecc_code++ = ((val2 >> 8) & 0xFF);
    *ecc_code++ = (val2 & 0xFF);
    *ecc_code++ = ((val1 >> 24) & 0xFF);
    *ecc_code++ = ((val1 >> 16) & 0xFF);
    *ecc_code++ = ((val1 >> 8) & 0xFF);
    *ecc_code++ = (val1 & 0xFF);
  }
  return 0;
}

/*
 * omap_enable_ecc - This function enables the hardware ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 */
void omap_enable_hwecc_bch4(struct mtd_info *mtd, int32_t mode)
{
  struct nand_chip *chip = mtd->priv;
  uint32_t bch_mod=0;
  uint32_t dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;
  unsigned int eccsize1, eccsize0;
  unsigned int ecc_conf_val = 0, ecc_size_conf_val = 0;

  switch (mode) {
  case NAND_ECC_READ    :
    eccsize1 = 0xD; eccsize0 = 0x48;
    /* ECCSIZE1=26 | ECCSIZE0=12 */
    ecc_size_conf_val = (eccsize1 << 22) | (eccsize0 << 12);

    /* ECCALGORITHM | ECCBCHT8 | ECCWRAPMODE | ECC16B |
     * ECCTOPSECTOR | ECCCS | ECC Enable
     */
    ecc_conf_val = ((0x01 << 16) | (bch_mod << 12) | (0x09 << 8) |
		    (dev_width << 7) | (0x03 << 4) |
		    (cs << 1) | (0x1));
    break;
  case NAND_ECC_WRITE   :
    eccsize1 = 0x20; eccsize0 = 0x00;

    /* ECCSIZE1=32 | ECCSIZE0=00 */
    ecc_size_conf_val = (eccsize1 << 22) | (eccsize0 << 12);

    /* ECCALGORITHM | ECCBCHT8 | ECCWRAPMODE | ECC16B |
     * ECCTOPSECTOR | ECCCS | ECC Enable
     */
    ecc_conf_val = ((0x01 << 16) | (bch_mod << 12) | (0x06 << 8) |
		    (dev_width << 7) | (0x03 << 4) |
		    (cs << 1) | (0x1));
    break;
  default:
    printf("Error: Unrecognized Mode[%d]!\n", mode);
    break;
  }

  writel(0x1, &gpmc_cfg->ecc_control);
  writel(ecc_size_conf_val, &gpmc_cfg->ecc_size_config);
  writel(ecc_conf_val, &gpmc_cfg->ecc_config);
  writel(0x101, &gpmc_cfg->ecc_control);
}

/*
 * omap_enable_ecc - This function enables the hardware ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 */
void omap_enable_hwecc_bch8(struct mtd_info *mtd, int32_t mode)
{
  struct nand_chip *chip = mtd->priv;
  uint32_t bch_mod=1;
  uint32_t dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;
  unsigned int eccsize1, eccsize0;
  unsigned int ecc_conf_val = 0, ecc_size_conf_val = 0;

  switch (mode) {
  case NAND_ECC_READ    :
    eccsize1 = 0x1A; eccsize0 = 0x18;
    /* ECCSIZE1=26 | ECCSIZE0=12 */
    ecc_size_conf_val = (eccsize1 << 22) | (eccsize0 << 12);

    /* ECCALGORITHM | ECCBCHT8 | ECCWRAPMODE | ECC16B |
     * ECCTOPSECTOR | ECCCS | ECC Enable
     */
    ecc_conf_val = ((0x01 << 16) | (bch_mod << 12) | (0x04 << 8) |
		    (dev_width << 7) | (0x03 << 4) |
		    (cs << 1) | (0x1));
    break;
  case NAND_ECC_WRITE   :
    eccsize1 = 0x20; eccsize0 = 0x00;

    /* ECCSIZE1=32 | ECCSIZE0=00 */
    ecc_size_conf_val = (eccsize1 << 22) | (eccsize0 << 12);

    /* ECCALGORITHM | ECCBCHT8 | ECCWRAPMODE | ECC16B |
     * ECCTOPSECTOR | ECCCS | ECC Enable
     */
    ecc_conf_val = ((0x01 << 16) | (bch_mod << 12) | (0x06 << 8) |
		    (dev_width << 7) | (0x03 << 4) |
		    (cs << 1) | (0x1));
    break;
  default:
    printf("Error: Unrecognized Mode[%d]!\n", mode);
    break;
  }

  writel(0x1, &gpmc_cfg->ecc_control);
  writel(ecc_size_conf_val, &gpmc_cfg->ecc_size_config);
  writel(ecc_conf_val, &gpmc_cfg->ecc_config);
  writel(0x101, &gpmc_cfg->ecc_control);
}

