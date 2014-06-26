/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *      This file defines the x-loader configuration settings for the
 *      Nest Learning Thermostat board.
 *
 *      The file was originally derived from omap3evm.h.
 *
 *      SDRAM Controller (SDRC):
 *        - Samsung K4X51163PI-FCG6 64 MiB DDR SDRAM
 *
 *      General Purpose Memory Controller (GPMC):
 *        - Micron MT29F2G16ABDHC-ET 256 MiB SLC NAND Flash (Development)
 *        - Micron MT29F2G16ABBEAH4 256 MiB SLC NAND Flash (Non-development)
 *        - Samsung K9F4G08U0C 512 MiB SLC NAND Flash (Development)
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development)
 *
 *      Multimedia Card (MMC) Interface:
 *        - Molex 500998-0900 Secure Digital (SD) Connector
 *        - TI WL1270 Wireless LAN (WLAN) Module
 *
 *      Display Subsystem (DSS):
 *        - Samsung LMS350DF03 HVGA (320 x 480) LCM (Development and Prototype)
 *        - Tianma TM025ZDZ01 (320 x 320) LCM (Production)
 *
 *      Camera Image Signal Processor:
 *        - Unused
 *
 *      UART Controller:
 *        - Channel 1:
 *            * Head Unit Serial Console
 *
 *        - Channel 2:
 *            * ZigBee Wireless Module
 *
 *      Multichannel Buffered Serial Port (McBSP):
 *        - Channel 1:
 *        - Channel 2:
 *        - Channel 3:
 *        - Channel 4:
 *        - Channel 5:
 *
 *      Multichannel Serial Peripheral Interface (McSPI):
 *        - Channel 1:
 *        - Channel 2:
 *        - Channel 3:
 *
 *      GPIO Controller:
 *        - Backplate Detect
 *        - ZigBee
 */

#ifndef __DIAMOND_CONFIG_H
#define __DIAMOND_CONFIG_H

/*
 * ARM Processor Architecture, Family and Product Configuration
 */
#define CONFIG_ARMCORTEXA8		1
#define CONFIG_OMAP             1
#define CONFIG_OMAP34XX         1
#define CONFIG_OMAP3430         1

/*
 * With the architecture, family and product configuration defined,
 * include the appropriate CPU header file.
 */

#include <asm/arch/cpu.h>

/*
 * Assert the board type(s) we are building for.
 *
 */
#undef CONFIG_DIAMOND_BOARD_DEVELOPMENT
#undef CONFIG_DIAMOND_BOARD_PROTOTYPE
#undef CONFIG_DIAMOND_BOARD_EVT
#undef CONFIG_DIAMOND_BOARD_DVT
#undef CONFIG_DIAMOND_BOARD_PVT
#undef CONFIG_DIAMOND_BOARD_MP
#define CONFIG_DIAMOND_BOARD_MULTI			1

/*
 * Enable serial console printf facility (consumes about 3.5 KiB)
 */
#define CFG_PRINTF

/*
 * CONFIG_BOOTM_IMAGE - This can be used to specify whether X-Loader
 * supports booting from 'mkimage' u-boot image files rather than just
 * raw, u-boot binaries.
 *
 * Enabling this adds support for attempting to boot from one or more
 * images because it allows an image to be sanity-checked before
 * attempting to boot it.
 */
#define CONFIG_BOOTM_IMAGE

/*
 * Display image information with timestamps
 */
#ifdef CONFIG_BOOTM_IMAGE
#define	CONFIG_TIMESTAMP
#endif

#if defined(CFG_PRINTF)
/*
 * Use a printf buffer size of 256 B
 */
# define CFG_PBSIZE				256

/*
 * Indicate that we want to use CFG_NS16550_COM1 (see below) at a baud
 * rate of 115,200 bps for the serial console.
 */
# define CONFIG_CONS_INDEX      1
# define CONFIG_BAUDRATE        115200

/*
 * NS16550-compatible serial driver configuration
 */
# define CFG_NS16550
# define CFG_NS16550_SERIAL
# define CFG_NS16550_REG_SIZE   (-4)
# define CFG_NS16550_CLK        (48000000)
# define CFG_NS16550_COM1       OMAP34XX_UART1
#endif /* defined(CFG_PRINTF) */

/*
 * Disable timer-based delay (consumes about 250 B) since this is only
 * available at present on the ARM926EJS.
 */
#undef	CFG_UDELAY

/*
 * The Nest Learning Thermostat development board has an SD/MMC
 * connector, so enable support for it, which implicitly enables
 * rudimentary support for MS-DOS (MBR) partitions and FAT file
 * systems.
 */
#define CONFIG_MMC				1

#if defined(CONFIG_MMC)
# define CFG_CMD_MMC            1
# define CFG_CMD_FAT            1
#endif /* defined(CONFIG_MMC) */

/*
 * Clock Configuration
 *
 *  Choose PRCM Configuration 2 which subselects a L3 interconnect
 *  clock of 166 MHz for SDRAM.
 */

#undef CFG_PRCM_DEBUG

#define PRCM_CLK_CFG2_332MHZ     		1

#define	CONFIG_SYS_MPU_DPLL_300MHZ		1
#undef	CONFIG_SYS_MPU_DPLL_600MHZ

#define	CONFIG_SYS_CORE_DPLL_332MHZ		1
#undef	CONFIG_SYS_CORE_DPLL_200MHZ
#undef	CONFIG_SYS_CORE_DPLL_400MHZ

#define CFG_SDRAM_SIZE_MB				64
#undef CFG_SDRAM_DEBUG

/*
 * The Nest Learning Thermostat may have one of three SDRAMs
 * 1a)Samsung I-Die 64MB
 *    32Mbx16
 *    VDD=1.8V
 *    6ns cycle time
 *    CL=3
 * 
 * 1b)Samsung K-Die 64MB
 *    32Mbx16
 *    VDD=1.8V
 *    6ns cycle time
 *    CL=3
 *
 * 2) Nanya NT6DM 64MB
 *    32Mbx16
 *    VDD=1.8V
 *    6ns cycle time
 *    CL=3
 *
 */

#undef CFG_SDRAM_SMG_K4X
#undef CFG_SDRAM_NYA_NTD6
#define CFG_SDRAM_COMBINED
/*
 * These values are based on the Samsung K4X51163PK-FGC6 datasheet 
 * (applicable to the I-die and the K-die) and the TI spreadsheet 
 * at http://processors.wiki.ti.com/index.php?title=
 * Setting_up_AM37x_SDRC_registers.
 */
#if defined(CFG_SDRAM_SMG_K4X)
#define CFG_SDRC_ACTIM_CTRLA_TRFC		12
#define CFG_SDRC_ACTIM_CTRLA_TRC		10
#define CFG_SDRC_ACTIM_CTRLA_TRAS		 7
#define CFG_SDRC_ACTIM_CTRLA_TRP		 3
#define CFG_SDRC_ACTIM_CTRLA_TRCD		 3
#define CFG_SDRC_ACTIM_CTRLA_TRRD		 2
#define CFG_SDRC_ACTIM_CTRLA_TDPL		 2
#define CFG_SDRC_ACTIM_CTRLA_TDAL		 5

#define CFG_SDRC_ACTIM_CTRLB_TWTR		 1
#define CFG_SDRC_ACTIM_CTRLB_TCKE		 2
#define CFG_SDRC_ACTIM_CTRLB_TXP		 1
#define CFG_SDRC_ACTIM_CTRLB_TXSR		20
#endif

/*
 * These values are based on the Nanya NT6DM datasheet and the TI
 * spreadsheet at http://processors.wiki.ti.com/index.php?title=
 * Setting_up_AM37x_SDRC_registers.
 */
#if defined(CFG_SDRAM_NYA_NTD6)
#define CFG_SDRC_ACTIM_CTRLA_TRFC		12
#define CFG_SDRC_ACTIM_CTRLA_TRC		10
#define CFG_SDRC_ACTIM_CTRLA_TRAS		 7
#define CFG_SDRC_ACTIM_CTRLA_TRP		 3
#define CFG_SDRC_ACTIM_CTRLA_TRCD		 3
#define CFG_SDRC_ACTIM_CTRLA_TRRD		 2
#define CFG_SDRC_ACTIM_CTRLA_TDPL		 3
#define CFG_SDRC_ACTIM_CTRLA_TDAL	         6	

#define CFG_SDRC_ACTIM_CTRLB_TWTR		 1
#define CFG_SDRC_ACTIM_CTRLB_TCKE		 1
#define CFG_SDRC_ACTIM_CTRLB_TXP		 1
#define CFG_SDRC_ACTIM_CTRLB_TXSR		19
#endif


#if defined(CFG_SDRAM_COMBINED)
#define CFG_SDRC_ACTIM_CTRLA_TRFC		12
#define CFG_SDRC_ACTIM_CTRLA_TRC		10
#define CFG_SDRC_ACTIM_CTRLA_TRAS		 7
#define CFG_SDRC_ACTIM_CTRLA_TRP		 3
#define CFG_SDRC_ACTIM_CTRLA_TRCD		 3
#define CFG_SDRC_ACTIM_CTRLA_TRRD		 2
#define CFG_SDRC_ACTIM_CTRLA_TDPL		 3
#define CFG_SDRC_ACTIM_CTRLA_TDAL	         6	

#define CFG_SDRC_ACTIM_CTRLB_TWTR		 1
#define CFG_SDRC_ACTIM_CTRLB_TCKE		 2
#define CFG_SDRC_ACTIM_CTRLB_TXP		 1
#define CFG_SDRC_ACTIM_CTRLB_TXSR	        20	
#endif



/*
 * NAND Configuration
 *
 * The Nest Learning Thermostat board may have two NAND devices:
 *
 *   1a) Micron MT29F2G16ABDHC-ET @ GPMC CS0
 *
 *      - 2 Gb x 16 (256 MiB)
 *      - 2 KiB page size
 *      - 128 KiB erase size
 *      - 1.8V
 *      - SLC
 *      - 1-bit ECC
 *
 *   1b) Micron MT29F2G16ABBEAH4 @ GPMC CS0
 *
 *      - 2 Gb x 16 (256 MiB)
 *      - 2 KiB page size
 *      - 128 KiB erase size
 *      - 1.8V
 *      - SLC
 *      - 4-bit ECC
 *
 *   2) Samsung K9F4G08U0C @ GPMC CS3
 *
 *      - 4 Gb x 8 (512 MiB)
 *      - 2 KiB page size
 *      - 128 KiB erase size
 *      - 3.3V
 *      - SLC
 */

#define CFG_GPMC_NAND
#define CFG_MMC_NAND
#undef	CFG_GPMC_ONENAND
#undef	CFG_MMC_ONENAND

#undef CFG_GPMC_DEBUG

#undef	CFG_NAND_K9F4G08U0C		// Samsung K9F4G08U0C
#define CFG_NAND_MT29F2G16		// Micron MT29F2G16

#undef  CFG_NAND_ECC_1BIT
#define CFG_NAND_ECC_4BIT	1
#undef  CFG_NAND_ECC_8BIT

#define CFG_GPMC_NAND_BASE	NAND_BASE
#define CFG_GPMC_NAND_SIZE	GPMC_SIZE_128M

#if defined(CFG_NAND_MT29F2G16)
#define CFG_GPMC_CONFIG1_0	M_NAND_GPMC_CONFIG1
#define CFG_GPMC_CONFIG2_0	M_NAND_GPMC_CONFIG2
#define CFG_GPMC_CONFIG3_0	M_NAND_GPMC_CONFIG3
#define CFG_GPMC_CONFIG4_0	M_NAND_GPMC_CONFIG4
#define CFG_GPMC_CONFIG5_0	M_NAND_GPMC_CONFIG5
#define CFG_GPMC_CONFIG6_0	M_NAND_GPMC_CONFIG6
#define CFG_GPMC_CONFIG7_0	\
	(GPMC_CONFIG7_MASKADDRESS_ENCODE(CFG_GPMC_NAND_SIZE) | \
	 GPMC_CONFIG7_BASEADDRESS_ENCODE(CFG_GPMC_NAND_BASE))
#endif /* defined(CFG_NAND_MT29F2G16) */

#if defined(CFG_NAND_K9F4G08U0C)
#define CFG_GPMC_CONFIG1_3	M_NAND_GPMC_CONFIG1 //SMNAND_GPMC_CONFIG1
#define CFG_GPMC_CONFIG2_3	M_NAND_GPMC_CONFIG2 //SMNAND_GPMC_CONFIG2
#define CFG_GPMC_CONFIG3_3	M_NAND_GPMC_CONFIG3 //SMNAND_GPMC_CONFIG3
#define CFG_GPMC_CONFIG4_3	M_NAND_GPMC_CONFIG4 //SMNAND_GPMC_CONFIG4
#define CFG_GPMC_CONFIG5_3	M_NAND_GPMC_CONFIG5 //SMNAND_GPMC_CONFIG5
#define CFG_GPMC_CONFIG6_3	M_NAND_GPMC_CONFIG6 //SMNAND_GPMC_CONFIG6
#define CFG_GPMC_CONFIG7_3	\
	(GPMC_CONFIG7_MASKADDRESS_ENCODE(CFG_GPMC_NAND_SIZE) | \
	 GPMC_CONFIG7_BASEADDRESS_ENCODE(CFG_GPMC_NAND_BASE))
#endif /* defined(CFG_NAND_K9F4G08U0C) */

/*
 * Architecture, processor and/or board-specific NAND access macros
 * and driver parameters.
 */

#if defined(CFG_NAND_MT29F2G16)
# define NAND_16BIT
# define NAND_BLOCK_SIZE	(128 << 10)
#elif defined(CFG_NAND_K9F4G08U0C)
# undef	NAND_16BIT
# define NAND_BLOCK_SIZE	(128 << 10)
#endif /* defined(CFG_NAND_MT29F2G16) */

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
        do {*(volatile u32 *)(GPMC_CONFIG) |= GPMC_WRITEPROTECT_HIGH;} while(0)
#define NAND_WP_ON()  \
        do {*(volatile u32 *)(GPMC_CONFIG) &= ~GPMC_WRITEPROTECT_HIGH;} while(0)

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
        do {*(volatile u32 *)(GPMC_CONFIG) |= GPMC_WRITEPROTECT_HIGH;} while(0)
#define NAND_WP_ON()  \
        do {*(volatile u32 *)(GPMC_CONFIG) &= ~GPMC_WRITEPROTECT_HIGH;} while(0)
#endif /* NAND_16BIT */

#define NAND_CTL_CLRALE(adr)
#define NAND_CTL_SETALE(adr)
#define NAND_CTL_CLRCLE(adr)
#define NAND_CTL_SETCLE(adr)
#define NAND_DISABLE_CE()
#define NAND_ENABLE_CE()

/*
 * Location where we expect to find the second-stage boot loader in
 * the boot NAND device.
 *
 *   ------------------------------------------------------------------------
 *                                                          Offsets
 *                                                ---------------------------
 *   Alias Description            Size    Blocks  Block     Address     Size
 *   ========================================================================
 *   ipl	 X-Loader Image	      256 KiB      2      0	  0x00000000    0 B
 *   ------------------------------------------------------------------------
 *   spl0	 Primary U-Boot Image 1.5 MiB     12      2	  0x00040000  256 KiB
 *   ------------------------------------------------------------------------
 *   spl1	 Reserved for Future  1.5 MiB     12     14	  0x001C0000 1.75 MiB
 *           Secondary U-Boot
 *           Image
 *   ========================================================================
 *
 */

#define CFG_NAND_BOOTEXTENT0_START		0x00040000
#define CFG_NAND_BOOTEXTENT1_START		0x001C0000
#define CFG_NAND_BOOTEXTENT_SIZE		0x180000

#ifdef CONFIG_BOOTM_IMAGE
#define CONFIG_NAND_BOOTEXTENT_LIST		{ { CFG_NAND_BOOTEXTENT0_START, \
											CFG_NAND_BOOTEXTENT_SIZE },	\
										  { CFG_NAND_BOOTEXTENT1_START, \
											CFG_NAND_BOOTEXTENT_SIZE } }
#else
#define CONFIG_NAND_BOOTEXTENT			{ CFG_NAND_BOOTEXTENT0_START, \
										  CFG_NAND_BOOTEXTENT_SIZE }
#endif

/*
 * Miscellaneous configurable options
 */

/*
 * CONFIG_BOOTFILE - This can be used to set a default value for the
 * second-stage boot file loaded from SD card if the system has been
 * configured to boot as such.
 *
 * CONFIG_BOOTFILE_LIST - This can be used to a list of default values
 * for the second-stage boot file loaded from SD card if the system has been
 * configured to boot as such.
 *
 * Try to boot an 'mkimage' u-boot image first; otherwise, try to boot a
 * raw u-boot binary if it exists as a fallback.
 */
#ifdef CONFIG_BOOTM_IMAGE
#define CONFIG_BOOTFILE_LIST 			{ "u-boot.img", "u-boot.bin" }
#else
#define CONFIG_BOOTFILE					"u-boot.img"
#endif

/*
 * Location in SDRAM where the second-stage boot loader will be loaded
 * to and executed from.
 */
#define CFG_LOADADDR             		0x80008000

#endif /* __DIAMOND_CONFIG_H */

