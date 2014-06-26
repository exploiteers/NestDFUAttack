/*
 * Definitions required by PCI code. Should gnerally get included from io.h
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef __ARCH_ARM_OMAP2_PCI_H
#define __ARCH_ARM_OMAP2_PCI_H

#include <plat/ti81xx.h>

/* Enumeration flag: Checked during PCI Enumeration of Bridges */
#define pcibios_assign_all_busses()     1

/*
 * PCI Resource allocation.
 *
 * IMPORTANT: Ensure that the values used below match with the ones passed
 * though PCIe RC platform data (from SoC/board file).
 */
#define PCIBIOS_MIN_IO          	(TI816X_PCIE_IO_BASE)
#define PCIBIOS_MIN_MEM         	(TI816X_PCIE_MEM_BASE)

#endif
