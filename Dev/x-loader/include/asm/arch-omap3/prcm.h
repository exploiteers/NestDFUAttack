/*
 *    Copyright (c) 2010 Nest Labs, Inc.
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
 *      This file defines register and register field constants and
 *      macros for the Texas Instruments Open Multimedia Application
 *      Platform (OMAP) 3 Power, Reset and Clock Management (PRCM)
 *      module.
 */

#ifndef _OMAP3_PRCM_H
#define _OMAP3_PRCM_H

#include <asm/arch/bits.h>

#define PRCM_BASE						0x48000000

#define PRCM_CM_BASE						(PRCM_BASE + 0x00004000)

#define PRCM_PRM_BASE						(PRCM_BASE + 0x00306000)

#define PRCM_PRM_CCR_BASE					(PRCM_PRM_BASE + 0x00000D00)

#define PRCM_PRM_CCR_CLKSEL						(PRCM_PRM_CCR_BASE + 0x00000040)
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_START		0
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_BITS		3
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_MASK		ARM_REG_VAL(0, ARM_REG_MASK(PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_BITS))
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_ENCODE(v)	ARM_REG_VAL_ENCODE(0, PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_MASK, v)
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_DECODE(v)	ARM_REG_VAL_DECODE(0, PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_MASK, v)
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_12_0_MHZ		0x0
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_13_0_MHZ		0x1
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_19_2_MHZ		0x2
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_26_0_MHZ		0x3
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_38_4_MHZ		0x4
#define PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_16_8_MHZ		0x5

#define PRCM_PRM_CCR_CLKOUT_CTRL				(PRCM_PRM_CCR_BASE + 0x00000070)

#define PRCM_PRM_GR_BASE					(PRCM_PRM_BASE + 0x00001200)

#define	PRCM_PRM_GR_CLKSRC_CTRL					(PRCM_PRM_GR_BASE + 0x00000070)
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_START		6
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BITS		2
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_MASK		ARM_REG_VAL(0, ARM_REG_MASK(PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BITS))
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_ENCODE(v)	ARM_REG_VAL_ENCODE(0, PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_MASK, v)
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_DECODE(v)	ARM_REG_VAL_DECODE(0, PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_MASK, v)
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BY_1			1
#define PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BY_2			2

#define PRCM_SR_BASE					(PRCM_BASE + 0x000C9000)

#define PRM_CLKSEL           PRCM_PRM_CCR_CLKSEL
#define PRM_RSTCTRL          0x48307250
#define PRM_CLKSRC_CTRL      PRCM_PRM_GR_CLKSRC_CTRL

#endif /* _OMAP3_PRCM_H */
