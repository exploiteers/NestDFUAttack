/*
 * (C) Copyright 2006
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _OMAP3_SYS_H_
#define _OMAP3_SYS_H_

#include <asm/arch/sizes.h>

/*
 * OMAP3 specific Section
 */

/* Stuff on L3 Interconnect */
#define SMX_APE_BASE			0x68000000

/* L3 Firewall */
#define A_REQINFOPERM0		(SMX_APE_BASE + 0x05048)
#define A_READPERM0		(SMX_APE_BASE + 0x05050)
#define A_WRITEPERM0		(SMX_APE_BASE + 0x05058)

/* GPMC */
#define OMAP34XX_GPMC_BASE		(0x6E000000)

/* SMS */
#define OMAP34XX_SMS_BASE		0x6C000000

/* SDRC */
#define OMAP34XX_SDRC_BASE		0x6D000000

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP34XX_CORE_L4_IO_BASE	0x48000000

#define OMAP34XX_WAKEUP_L4_IO_BASE	0x48300000

#define OMAP34XX_L4_PER			0x49000000

#define OMAP34XX_L4_IO_BASE		OMAP34XX_CORE_L4_IO_BASE

/* CONTROL */
#define OMAP34XX_CTRL_BASE		(OMAP34XX_L4_IO_BASE+0x2000)

/* TAP information  dont know for 3430*/
#define OMAP34XX_TAP_BASE		(0x49000000) /*giving some junk for virtio */

/* UART */
#define OMAP34XX_UART1			(OMAP34XX_L4_IO_BASE+0x6a000)
#define OMAP34XX_UART2			(OMAP34XX_L4_IO_BASE+0x6c000)
#define OMAP34XX_UART3			(OMAP34XX_L4_PER+0x20000)

/* General Purpose Timers */
#define OMAP34XX_GPT1			0x48318000
#define OMAP34XX_GPT2			0x49032000
#define OMAP34XX_GPT3			0x49034000
#define OMAP34XX_GPT4			0x49036000
#define OMAP34XX_GPT5			0x49038000
#define OMAP34XX_GPT6			0x4903A000
#define OMAP34XX_GPT7			0x4903C000
#define OMAP34XX_GPT8			0x4903E000
#define OMAP34XX_GPT9			0x49040000
#define OMAP34XX_GPT10			0x48086000
#define OMAP34XX_GPT11			0x48088000
#define OMAP34XX_GPT12			0x48304000

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE			(0x4830C000)
#define WD2_BASE			(0x48314000)
#define WD3_BASE			(0x49030000)

/* 32KTIMER */
#define SYNC_32KTIMER_BASE		(0x48320000)
#define S32K_CR				(SYNC_32KTIMER_BASE+0x10)

/*
 * SDP3430 specific Section
 */

/*
 *  The 343x's chip selects are programmable.  The mask ROM
 *  does configure CS0 to 0x08000000 before dispatch.  So, if
 *  you want your code to live below that address, you have to
 *  be prepared to jump though hoops, to reset the base address.
 *  Same as in SDP3430
 */
/* base address for indirect vectors (internal boot mode) */
#define SRAM_OFFSET0			0x40000000
#define SRAM_OFFSET1			0x00200000
#define SRAM_OFFSET2			0x0000F800
#define SRAM_VECT_CODE			(SRAM_OFFSET0|SRAM_OFFSET1|SRAM_OFFSET2)

#define LOW_LEVEL_SRAM_STACK		0x4020FFFC

/*
 * TODO: Are they required for AM3517?
 */
#if defined(CONFIG_3430SDP) || defined(CONFIG_OMAP3EVM)
/* FPGA on Debug board.*/
#define ETH_CONTROL_REG			(DEBUG_BASE+0x30b)
#define LAN_RESET_REGISTER		(DEBUG_BASE+0x1c)

#define DIP_SWITCH_INPUT_REG2		(DEBUG_BASE+0x60)
#define LED_REGISTER			(DEBUG_BASE+0x40)
#define FPGA_REV_REGISTER		(DEBUG_BASE+0x10)
#define EEPROM_MAIN_BRD			(DEBUG_BASE+0x10000+0x1800)
#define EEPROM_CONN_BRD			(DEBUG_BASE+0x10000+0x1900)
#define EEPROM_UI_BRD			(DEBUG_BASE+0x10000+0x1A00)
#define EEPROM_MCAM_BRD			(DEBUG_BASE+0x10000+0x1B00)
#define ENHANCED_UI_EE_NAME		"750-2075"
#endif

#endif  /* _OMAP3430_SYS_H_ */
