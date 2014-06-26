/*
 * TI816X clock function prototypes and macros.
 *
 * Copyright (C) 2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ARM_MACH_OMAP2_CLOCK81XX_H
#define __ARCH_ARM_MACH_OMAP2_CLOCK81XX_H

int ti816x_clk_init(void);
int ti814x_clk_init(void);

extern const struct clkops clkops_ti81xx_dflt_wait;
extern const struct clkops clkops_ti81xx_pcie;
extern const struct clkops clkops_ti81xx_usb;

#endif
