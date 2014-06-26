/*
 * Copyright (C) 2004 - 2005 Texas Instruments.
 * 
 * X-Loader Configuation settings for the TI OMAP H4 board.
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

/* uncomment it if you need timer based udelay(). it takes about 250 bytes */
//#define CFG_UDELAY
 
/*
 * High Level Configuration Options
 */
#define CONFIG_ARM1136           1    /* This is an arm1136 CPU core */
#define CONFIG_OMAP              1    /* in a TI OMAP core */
#define CONFIG_OMAP2420	         1    /* which is in a 2420 */
#define CONFIG_OMAP2420H4        1    /* and on a H4 board */

#define CONFIG_OMAP242X

#define PRCM_CONFIG_II           1    
//#define PRCM_CONFIG_III           1    
#define CONFIG_PARTIAL_SRAM      1

//#define CFG_SDRAM_DDR			1
#define CFG_SDRAM_COMBO		2
//#define CFG_SDRAM_SDR			3
//#define CFG_SDRAM_STACKED		4
	 
/* Chipselect and NAND information :
   Since we share the mem.h from u-boot, we define few macros here
   so as to pick the right gpmc values from there for the macros in mem.h
*/
/* NAND fixed at CS5 */
#define OMAP24XX_GPMC_CS0 SMNAND
#define OMAP24XX_GPMC_CS0_SIZE GPMC_SIZE_64M
#define OMAP24XX_GPMC_CS0_MAP CFG_FLASH_BASE
#define CFG_NAND_BOOT
#define NAND_LEGACY

#include <asm/arch/omap2420.h>        /* get chip and board defs */

#define V_SCLK                   12000000
/* input clock of PLL */
/* the OMAP2420 H4 has 12MHz, 13MHz, or 19.2Mhz crystal input */
#define CONFIG_SYS_CLK_FREQ      V_SCLK

#ifdef CFG_PRINTF

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	(-4)
#define CFG_NS16550_CLK		(48000000)	/* can be 12M/32Khz or 48Mhz */
#define CFG_NS16550_COM1         OMAP2420_UART1
 
/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1           1    /* UART1 on H4 */

#define CONFIG_CONS_INDEX        1
#define CONFIG_BAUDRATE          115200
#define CFG_PBSIZE	256

#endif /* CFG_PRINTF */

/*
 * Miscellaneous configurable options
 */
#define CFG_LOADADDR 	0x80000000
  
#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
 
/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Board NAND Info.
 */
#define CFG_NAND_K9K1216    /* Samsung 16-bit 64MB chip */

#define NAND_ADDR 0x04000000  /* physical address to access nand at CS0*/
 
/* H4 NAND is partitioned:
 * 0x0000000 - 0x0010000 	Booting Image
 * 0x0010000 - 0x0050000 	U-Boot Image
 * 0x0050000 - 0x0080000 	U-Boot Env Data (X-loader doesn't care)
 * 0x0080000 - 0x4000000 	depends on application
 */
#define NAND_UBOOT_START	0x0040000
#define NAND_UBOOT_END		0x0080000
#define NAND_BLOCK_SIZE		0x4000
  
#define WRITE_NAND_COMMAND(d, adr) do {*(volatile u16 *)0x6800A07C = d;} while(0)
#define WRITE_NAND_ADDRESS(d, adr) do {*(volatile u16 *)0x6800A080 = d;} while(0)
#define WRITE_NAND(d, adr) do {*(volatile u16 *)0x6800A084 = d;} while(0)
#define READ_NAND(adr) (*(volatile u16 *)0x6800A084)

#define NAND_WAIT_READY()
  
#define NAND_WP_OFF()  do {*(volatile u32 *)(0x6800A050) |= 0x00000010;} while(0)
#define NAND_WP_ON()  do {*(volatile u32 *)(0x6800A050) &= ~0x00000010;} while(0)
  
#define NAND_CTL_CLRALE(adr)    
#define NAND_CTL_SETALE(adr)    
#define NAND_CTL_CLRCLE(adr)         
#define NAND_CTL_SETCLE(adr)         
#define NAND_DISABLE_CE()     
#define NAND_ENABLE_CE()       


#endif							/* __CONFIG_H */
