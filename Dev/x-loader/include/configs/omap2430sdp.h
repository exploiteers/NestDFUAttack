/*
 * (C) Copyright 2004 Texas Instruments.
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
#define CONFIG_OMAP2430H4        1    /* and on a H4 board */

#define CONFIG_OMAP243X 1

//#define PRCM_CONFIG_5A		 1
#define PRCM_CONFIG_2           1 /* 2430 ES2+330ARM+DDR-165-PISMO */

#define OMAP2430_SQUARE_CLOCK_INPUT 1

/* Memory type */
//#define CFG_SDRAM_DDR			1 /* not tested */
//#define CFG_SDRAM_COMBO		2 /* not tested */
#define CFG_2430SDRAM_DDR		3
//#define CFG_SDRAM_STACKED		4 /* not tested */

/* The actual register values are defined in u-boot- mem.h */
/* SDRAM Bank Allocation method */
//#define SDRC_B_R_C          1
//#define SDRC_B1_R_B0_C      1
#define SDRC_R_B_C            1

/* Boot type */
//#define CFG_NAND 1
#define CFG_ONENAND 1

# define NAND_BASE            0x0C000000  /* NAND flash */
# define ONENAND_BASE	      0x20000000  /* OneNand flash */

#ifdef CFG_NAND
#define NAND_LEGACY
#define OMAP24XX_GPMC_CS0_SIZE GPMC_SIZE_64M
#define OMAP24XX_GPMC_CS0_MAP NAND_BASE
#else
#define OMAP24XX_GPMC_CS0_SIZE GPMC_SIZE_128M
#define OMAP24XX_GPMC_CS0_MAP ONENAND_BASE
#define ONENAND_ADDR ONENAND_BASE  /* physical address to access OneNAND at CS0*/
#endif

/* Another dependency on u-boot */
#define sdelay delay
	 
#include <asm/arch/omap2430.h>        /* get chip and board defs */

#define V_SCLK                   13000000
/* input clock of PLL */
/* the OMAP2420 H4 has 12MHz, 13MHz, or 19.2Mhz crystal input */
#define CONFIG_SYS_CLK_FREQ      V_SCLK

#ifdef CFG_PRINTF

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	(-4)
#define CFG_NS16550_CLK		(48000000)	/* can be 12M/32Khz or 48Mhz */
#define CFG_NS16550_COM1         OMAP2430_UART1
 
/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1           1    /* UART1 on 2430SDP */
#define CONFIG_CONS_INDEX        1
#define CONFIG_BAUDRATE          115200
#define CFG_PBSIZE	256

#endif /* CFG_PRINTF */

/*
 * Miscellaneous configurable options
 */
#define CFG_LOADADDR 	0x80008000
  
#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
 
/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

#ifdef CFG_NAND

/*-----------------------------------------------------------------------
 * Board NAND Info.
 */
#define CFG_NAND_K9K1216    /* Samsung 16-bit 64MB chip */

/* NAND is partitioned:
 * 0x0000000 - 0x0010000 	Booting Image
 * 0x0010000 - 0x0050000 	U-Boot Image
 * 0x0050000 - 0x0080000 	U-Boot Env Data (X-loader doesn't care)
 * 0x0080000 - 0x00B0000 	Kernel Image
 * 0x00B0000 - 0x4000000 	depends on application
 */
#define NAND_UBOOT_START	0x0040000
#define NAND_UBOOT_END		0x0080000
#define NAND_BLOCK_SIZE		0x4000
  
#define GPMC_CONFIG		(OMAP24XX_GPMC_BASE+0x50)
#define GPMC_NAND_COMMAND_0	(OMAP24XX_GPMC_BASE+0x7C)
#define GPMC_NAND_ADDRESS_0	(OMAP24XX_GPMC_BASE+0x80)
#define GPMC_NAND_DATA_0	(OMAP24XX_GPMC_BASE+0x84)

#define WRITE_NAND_COMMAND(d, adr) do {*(volatile u16 *)GPMC_NAND_COMMAND_0 = d;} while(0)
#define WRITE_NAND_ADDRESS(d, adr) do {*(volatile u16 *)GPMC_NAND_ADDRESS_0 = d;} while(0)
#define WRITE_NAND(d, adr) do {*(volatile u16 *)GPMC_NAND_DATA_0 = d;} while(0)
#define READ_NAND(adr) (*(volatile u16 *)GPMC_NAND_DATA_0)
#define NAND_WAIT_READY()
#define NAND_WP_OFF()  do {*(volatile u32 *)(GPMC_CONFIG) |= 0x00000010;} while(0)
#define NAND_WP_ON()  do {*(volatile u32 *)(GPMC_CONFIG) &= ~0x00000010;} while(0)

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
#define CFG_SYNC_BURST_READ     1

/* OneNAND is partitioned:
 *   0x0000000 - 0x0080000        X-Loader 
 *   0x0080000 - 0x00c0000        U-boot Image
 *   0x00c0000 - 0x00e0000        U-Boot Env Data (X-loader doesn't care)
 *   0x00e0000 - 0x0120000        Kernel Image
 *   0x0120000 - 0x4000000        depends on application
 */

#define ONENAND_START_BLOCK             4
#define ONENAND_END_BLOCK             	6
#define ONENAND_PAGE_SIZE               2048            /* 2KB */
#define ONENAND_BLOCK_SIZE              0x20000         /* 128KB */

#endif // oneNAND
#endif							/* __CONFIG_H */
