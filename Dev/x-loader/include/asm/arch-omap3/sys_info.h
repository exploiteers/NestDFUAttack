/*
 * (C) Copyright 2006
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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

#ifndef _OMAP34XX_SYS_INFO_H_
#define _OMAP34XX_SYS_INFO_H_

#define XDR_POP		5      /* package on package part */
#define SDR_DISCRETE	4      /* 128M memory SDR module*/
#define DDR_STACKED	3      /* stacked part on 2422 */
#define DDR_COMBO	2      /* combo part on cpu daughter card (menalaeus) */
#define DDR_DISCRETE	1      /* 2x16 parts on daughter card */

#define DDR_100		100    /* type found on most mem d-boards */
#define DDR_111		111    /* some combo parts */
#define DDR_133		133    /* most combo, some mem d-boards */
#define DDR_165		165    /* future parts */

#define CPU_3430	0x3430

/* 343x real hardware:
 *  ES1     = rev 0
 */

/* 343x code defines:
 * ES1     = 0+1 = 1
 * ES1     = 1+1 = 1
 */
#define CPU_3430_ES1		1
#define CPU_3430_ES2		2

/*
 * Hawkeye values
 */
#define HAWKEYE_OMAP34XX        0xb7ae
#define HAWKEYE_AM35XX          0xb868
#define HAWKEYE_OMAP36XX        0xb891

#define HAWKEYE_SHIFT           12

/*
 * Define CPU families
 */
#define CPU_OMAP34XX            0x3400  /* OMAP34xx/OMAP35 devices */
#define CPU_AM35XX              0x3500  /* AM35xx devices          */
#define CPU_OMAP36XX            0x3600  /* OMAP36xx devices        */

/* Currently Virtio models this one */
#define CPU_3430_CHIPID		0x0B68A000

#define GPMC_MUXED		1
#define GPMC_NONMUXED		0

#define TYPE_NAND		0x800	/* bit pos for nand in gpmc reg */
#define TYPE_NOR		0x000
#define TYPE_ONENAND		0x800

#define WIDTH_8BIT		0x0000
#define WIDTH_16BIT		0x1000	/* bit pos for 16 bit in gpmc */

#define I2C_MENELAUS		0x72	/* i2c id for companion chip */
#define I2C_TRITON2		0x4B	/* addres of power group */

#define BOOT_FAST_XIP		0x1f

/* SDP definitions according to FPGA Rev. Is this OK?? */
#define SDP_3430_V1		0x1
#define SDP_3430_V2		0x2

#define BOARD_3430_LABRADOR	0x80
#define BOARD_3430_LABRADOR_V1	0x1

#endif
