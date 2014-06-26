/*
 * (C) Copyright 2009
 * Texas Instruments, <www.ti.com>
 * Manikandan Pillai<mani.pillai@ti.com>
 * X-Loader Configuation settings for the AM3517EVM board.
 *
 * Derived from /include/configs/omap3evm.h
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

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMCORTEXA8       1    /* This is an ARM V7 CPU core */
#define CONFIG_OMAP              1    /* in a TI OMAP core */

#define CONFIG_AM3517EVM       1    /* working with AM3517 EVM */
//#define CONFIG_AM3517TEB       1    /* working with AM3517 TEB */

/* Enable the below macro if MMC boot support is required */
#define CONFIG_MMC               1
#if defined(CONFIG_MMC)
	#define CFG_CMD_MMC              1
	#define CFG_CMD_FAT              1
#endif

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

//#define PRCM_CLK_CFG2_266MHZ   1    /* VDD2=1.15v - 133MHz DDR */
#define PRCM_CLK_CFG2_332MHZ     1    /* VDD2=1.15v - 166MHz DDR */
#define PRCM_PCLK_OPP2           1    /* ARM=381MHz - VDD1=1.20v */

# define NAND_BASE_ADR           NAND_BASE  /* NAND flash */
# define ONENAND_BASE            ONENAND_MAP  /* OneNand flash */

#define OMAP34XX_GPMC_CS0_SIZE GPMC_SIZE_128M
#define ONENAND_ADDR ONENAND_BASE  /* physical address of OneNAND at CS0*/

#ifdef CFG_PRINTF

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE     (-4)
#define CFG_NS16550_CLK          (48000000)
#define CFG_NS16550_COM3         OMAP34XX_UART3

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL3           3    /* UART1 on OMAP3EVM */
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

/*-----------------------------------------------------------------------
 * Board NAND Info.
 */

#define CFG_NAND_K9F1G08R0A    /* Samsung 8-bit 128MB chip large page NAND chip*/
#define NAND_16BIT

/* NAND is partitioned:
 * 0x00000000 - 0x0007FFFF  Booting Image
 * 0x00080000 - 0x0023FFFF  U-Boot Image
 * 0x00240000 - 0x0027FFFF  U-Boot Env Data (X-loader doesn't care)
 * 0x00280000 - 0x0077FFFF  Kernel Image
 * 0x00780000 - 0x08000000  depends on application
 */
#define NAND_UBOOT_START         0x0080000 /* Leaving first 4 blocks for x-load */
#define NAND_UBOOT_END           0x0240000 /* Giving a space of 2 blocks = 256KB */
#define NAND_BLOCK_SIZE          0x20000

#define GPMC_CONFIG              (OMAP34XX_GPMC_BASE+0x50)

#if defined (CONFIG_AM3517EVM)
#define GPMC_NAND_COMMAND_0      (OMAP34XX_GPMC_BASE+0x7C)
#define GPMC_NAND_ADDRESS_0      (OMAP34XX_GPMC_BASE+0x80)
#define GPMC_NAND_DATA_0         (OMAP34XX_GPMC_BASE+0x84)
#elif defined (CONFIG_AM3517TEB)
#define GPMC_NAND_COMMAND_0      (OMAP34XX_GPMC_BASE+0xDC)
#define GPMC_NAND_ADDRESS_0      (OMAP34XX_GPMC_BASE+0xE0)
#define GPMC_NAND_DATA_0         (OMAP34XX_GPMC_BASE+0xE4)
#endif

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

/*-----------------------------------------------------------------------
 * Board oneNAND Info.
 */
#define CFG_SYNC_BURST_READ	1

/* OneNAND is partitioned:
 *   0x0000000 - 0x0080000  X-Loader
 *   0x0080000 - 0x0240000   U-boot Image
 *   0x0240000 - 0x0280000   U-Boot Env Data (X-loader doesn't care)
 *   0x0280000 - 0x0780000   Kernel Image
 *   0x0780000 - 0x8000000   depends on application
 */

#define ONENAND_START_BLOCK      4
#define ONENAND_END_BLOCK        18
#define ONENAND_PAGE_SIZE        2048     /* 2KB */
#define ONENAND_BLOCK_SIZE       0x20000  /* 128KB */

#endif /* __CONFIG_H */

