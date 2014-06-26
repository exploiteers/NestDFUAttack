/*
 * Platform data for ti816x PCIe Root Complex module.
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

#ifndef __ARCH_ARM_MACH_OMAP2_PCIE_TI816X_H
#define __ARCH_ARM_MACH_OMAP2_PCIE_TI816X_H

struct ti816x_pcie_data {
	int msi_irq_base;
	int msi_irq_num;
};

#endif
