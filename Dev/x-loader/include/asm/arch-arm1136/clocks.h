/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _OMAP24XX_CLOCKS_H_
#define _OMAP24XX_CLOCKS_H_

#define COMMIT_DIVIDERS  0x1
#define MODE_BYPASS_FAST 0x2
#define APLL_LOCK        0xc
#define DPLL_LOCK        0x3   /* DPLL lock */
#define LDELAY           12000000

#if defined(CONFIG_OMAP242X)
#include <asm/arch/clocks242x.h>
#elif defined(CONFIG_OMAP243X)
#include <asm/arch/clocks243x.h>
#endif

#define S12M		12000000
#define S13M		13000000
#define S19_2M		19200000
#define S24M		24000000
#define S26M		26000000
#define S38_4M		38400000

#endif








