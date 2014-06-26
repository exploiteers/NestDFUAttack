/*
 * (C) Copyright 2004
 * Texas Instruments.
 * Jian Zhang <jzhang@ti.com>, Kshitij Gupta <kshitij@ti.com>
 * X-Loader Configuation settings for the TI OMAP H3 board.
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

/* exactly uncomment one */
#define CFG_BOOT_CS0	  /* ROM code -> signed X-Loader in NNAD */
//#define CFG_BOOT_CS3	  /* unsigned X-loader in NOR for development */

/* serial printf facility takes about 3.5K */
#define CFG_PRINTF 
//#undef CFG_PRINTF 

/* uncomment it if you need timer based udelay(). it takes about 250 bytes */
//#define CFG_UDELAY
 
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM926EJS	1	/* This is an arm926ejs CPU core  */
#define CONFIG_OMAP	1			/* in a TI OMAP core    */
#define CONFIG_OMAP1710	1		/* which is in a 1710  */
#define CONFIG_H3_OMAP1710	1	/*  a H3 Board  */

/* input clock of PLL */
/* the OMAP1710 H3 has 12MHz input clock */
#define CONFIG_SYS_CLK_FREQ	12000000


#ifdef CFG_PRINTF

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	(-4)
#define CFG_NS16550_CLK		(48000000)	/* can be 12M/32Khz or 48Mhz */
#define CFG_NS16550_COM1	0xfffb0000	/* uart1, bluetooth uart on helen */

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE	115200

#endif /* CFG_PRINTF */

#include <configs/omap1510.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_PBSIZE	256
#define CFG_LOADADDR 	0x11000000
  
#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
 
/* The 1710 has 6 timers, they can be driven by the RefClk (12Mhz) or by
 * DPLL1. This time is further subdivided by a local divisor.
 */
#define CFG_TIMERBASE	0xFFFEC500	/* use timer 1 */
#define CFG_PVT	7	/* 2^(pvt+1), divide by 256 */
#define CFG_HZ	((CONFIG_SYS_CLK_FREQ)/(2 << CFG_PVT))

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Board NAND Info.
 */
#define CFG_NAND_K9F5616    /* Samsung 16-bit 32MB chip */

#ifdef CFG_BOOT_CS0
#define NAND_ADDR 0x0c000000  /* physical address to access nand at CS3*/
#else
#define NAND_ADDR 0x0a000000  /* physical address to access nand at CS2B*/
#endif

/* H3 NAND is partitioned:
 * 0x0000000 - 0x0010000 	Booting Image
 * 0x0010000 - 0x0050000 	U-Boot Image
 * 0x0050000 - 0x0080000 	U-Boot Env Data (X-loader doesn't care)
 * 0x0080000 - 0x2000000 	depends on application
 */
#define NAND_UBOOT_START	0x0010000
#define NAND_UBOOT_END		0x0050000
#define NAND_BLOCK_SIZE		0x4000
  
#define WRITE_NAND_COMMAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr + 2) = (__u8)(d); } while(0)
#define WRITE_NAND_ADDRESS(d, adr) do{ *(volatile __u8 *)((unsigned long)adr + 4) = (__u8)(d); } while(0)
#define WRITE_NAND(d, adr) do{ *(volatile __u16 *)((unsigned long)adr) = (__u16)(d); } while(0)
#define READ_NAND(adr) ((volatile __u16)(*(volatile __u16 *)(unsigned long)adr))

#define GPIO1_DATAIN		0xfffbe42c
#define NAND_WAIT_READY()  	while(!((*(volatile __u16 *)(GPIO1_DATAIN) & 0x0400) == 0x0400));
 
#define NAND_CTL_CLRALE(adr)    
#define NAND_CTL_SETALE(adr)    
#define NAND_CTL_CLRCLE(adr)         
#define NAND_CTL_SETCLE(adr)         
#define NAND_DISABLE_CE()     
#define NAND_ENABLE_CE()       


#endif							/* __CONFIG_H */
