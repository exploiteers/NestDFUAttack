/*
 * (C) Copyright 2006 
 * Texas Instruments <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * X-Loader Configuation settings for the TI OMAP SDP3430 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* serial printf facility takes about 3.5K */
#define CFG_PRINTF 
//#undef CFG_PRINTF 

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMCORTEXA8       1    /* This is an ARM V7 CPU core */
#define CONFIG_OMAP              1    /* in a TI OMAP core */
#define CONFIG_OMAP34XX          1    /* which is a 34XX */
#define CONFIG_OMAP3430          1    /* which is in a 3430 */
#define CONFIG_3430LABRADOR      1    /* working on Labrador */
//#define CONFIG_3430_AS_3410    1    /* true for 3430 in 3410 mode */

#include <asm/arch/cpu.h>        /* get chip and board defs */

/* uncomment it if you need timer based udelay(). it takes about 250 bytes */
//#define CFG_UDELAY
 
/* Clock Defines */
#define V_OSCK                   26000000  /* Clock output from T2 */

#if (V_OSCK > 19200000)          
#define V_SCLK                   (V_OSCK >> 1)
#else
#define V_SCLK                   V_OSCK
#endif

#define PRCM_CLK_CFG2_266MHZ     1    /* VDD2=1.15v - 133MHz DDR */
//#define PRCM_CLK_CFG2_332MHZ   1    /* VDD2=1.15v - 166MHz DDR */
#define PRCM_PCLK_OPP2           1    /* ARM=500MHz - VDD1=1.20v */

/* Memory type */
#define CFG_3430SDRAM_DDR        1

/* The actual register values are defined in u-boot- mem.h */
/* SDRAM Bank Allocation method */
//#define SDRC_B_R_C             1
//#define SDRC_B1_R_B0_C         1
#define SDRC_R_B_C               1

/* Boot type */
#define CFG_NAND 1
//#define CFG_ONENAND 1

# define NAND_BASE_ADR           NAND_BASE  /* NAND flash */
# define ONENAND_BASE            ONENAND_MAP  /* OneNand flash */

#ifdef CFG_NAND
#define OMAP34XX_GPMC_CS0_SIZE GPMC_SIZE_128M  /* u = ofdon't need so much for nand port */
#define OMAP34XX_GPMC_CS0_MAP NAND_BASE_ADR
#else
#define OMAP34XX_GPMC_CS0_SIZE GPMC_SIZE_128M
#define OMAP34XX_GPMC_CS0_MAP ONENAND_BASE
#define ONENAND_ADDR ONENAND_BASE  /* physical address of OneNAND at CS0*/
#endif


#ifdef CFG_PRINTF

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE     (-4)
#define CFG_NS16550_CLK          (48000000)
#define CFG_NS16550_COM3         OMAP34XX_UART3
 
/*
 * select serial console configuration
 */
#define CONFIG_SERIAL3           3    /* UART3 on board */
#define CONFIG_CONS_INDEX        3

#define CONFIG_BAUDRATE          115200
#define CFG_PBSIZE               256

#endif /* CFG_PRINTF */

/*
 * Miscellaneous configurable options
 */
#define CFG_LOADADDR             0x80008000
  
#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
 
/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE         (128*1024) /* regular stack */

#ifdef CFG_NAND

/*-----------------------------------------------------------------------
 * Board NAND Info.
 */


#define CFG_NAND_K9F1G08R0A    /* Micron 16-bit 256MB chip large page NAND chip*/
#define NAND_16BIT

/* NAND is partitioned:
 * 0x00000000 - 0x0007FFFF  Booting Image
 * 0x00080000 - 0x000BFFFF  U-Boot Image
 * 0x000C0000 - 0x000FFFFF  U-Boot Env Data (X-loader doesn't care)
 * 0x00100000 - 0x002FFFFF  Kernel Image
 * 0x00300000 - 0x08000000  depends on application
 */
#define NAND_UBOOT_START         0x0080000 /* Leaving first 4 blocks for x-load */
#define NAND_UBOOT_END           0x00C0000 /* Giving a space of 2 blocks = 256KB */
#define NAND_BLOCK_SIZE          0x20000
  
#define GPMC_CONFIG              (OMAP34XX_GPMC_BASE+0x50)
#define GPMC_NAND_COMMAND_0      (OMAP34XX_GPMC_BASE+0x7C)
#define GPMC_NAND_ADDRESS_0      (OMAP34XX_GPMC_BASE+0x80)
#define GPMC_NAND_DATA_0         (OMAP34XX_GPMC_BASE+0x84)

#ifdef NAND_16BIT
#define WRITE_NAND_COMMAND(d, adr) \
        do {*(volatile u16 *)GPMC_NAND_COMMAND_0 = d;} while(0)
#define WRITE_NAND_ADDRESS(d, adr) \
        do {*(volatile u16 *)GPMC_NAND_ADDRESS_0 = d;} while(0)
#define WRITE_NAND(d, adr) \
        do {*(volatile u16 *)GPMC_NAND_DATA_0 = d;} while(0)
#define READ_NAND(adr) \
        (*(volatile u16 *)GPMC_NAND_DATA_0)
#define NAND_WAIT_READY()
#define NAND_WP_OFF()  \
        do {*(volatile u32 *)(GPMC_CONFIG) |= 0x00000010;} while(0)
#define NAND_WP_ON()  \
        do {*(volatile u32 *)(GPMC_CONFIG) &= ~0x00000010;} while(0)

#else /* to support 8-bit NAND devices */
#define WRITE_NAND_COMMAND(d, adr) \
        do {*(volatile u8 *)GPMC_NAND_COMMAND_0 = d;} while(0)
#define WRITE_NAND_ADDRESS(d, adr) \
        do {*(volatile u8 *)GPMC_NAND_ADDRESS_0 = d;} while(0)
#define WRITE_NAND(d, adr) \
        do {*(volatile u8 *)GPMC_NAND_DATA_0 = d;} while(0)
#define READ_NAND(adr) \
        (*(volatile u8 *)GPMC_NAND_DATA_0);
#define NAND_WAIT_READY()
#define NAND_WP_OFF()  \
        do {*(volatile u32 *)(GPMC_CONFIG) |= 0x00000010;} while(0)
#define NAND_WP_ON()  \
        do {*(volatile u32 *)(GPMC_CONFIG) &= ~0x00000010;} while(0)

#endif

#define NAND_CTL_CLRALE(adr) 
#define NAND_CTL_SETALE(adr) 
#define NAND_CTL_CLRCLE(adr) 
#define NAND_CTL_SETCLE(adr) 
#define NAND_DISABLE_CE() 
#define NAND_ENABLE_CE() 

#else
/*-----------------------------------------------------------------------
 * Board oneNAND Info.
 */
#define CFG_SYNC_BURST_READ      1

/* OneNAND is partitioned:
 *   0x0000000 - 0x0080000  X-Loader 
 *   0x0080000 - 0x00c0000   U-boot Image
 *   0x00c0000 - 0x00e0000   U-Boot Env Data (X-loader doesn't care)
 *   0x00e0000 - 0x0120000   Kernel Image
 *   0x0120000 - 0x4000000   depends on application
 */

#define ONENAND_START_BLOCK      4
#define ONENAND_END_BLOCK        6
#define ONENAND_PAGE_SIZE        2048     /* 2KB */
#define ONENAND_BLOCK_SIZE       0x20000  /* 128KB */

#endif  /* oneNAND */
#endif /* __CONFIG_H */

