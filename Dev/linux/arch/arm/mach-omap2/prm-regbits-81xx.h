/*
 * PRM Register bit macros.
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

#ifndef __ARCH_ARM_MACH_OMAP2_PRM_REGBITS_81XX_H
#define __ARCH_ARM_MACH_OMAP2_PRM_REGBITS_81XX_H

#include "prm.h"

/*
 * TODO: At present this file only contains few register values required to
 * manage local reset for some modules. Later this have to be generated from h/w
 * database.
 */

/* Used by RM_DEFAULT_RSTCTRL */
#define TI81XX_USB1_LRST_SHIFT				5
#define TI81XX_USB1_LRST_MASK				(1 << 5)

/* Used by RM_DEFAULT_RSTCTRL */
#define TI81XX_USB2_LRST_SHIFT				6
#define TI81XX_USB2_LRST_MASK				(1 << 6)

/* Used by RM_DEFAULT_RSTCTRL */
#define TI81XX_PCI_LRST_SHIFT				7
#define TI81XX_PCI_LRST_MASK				(1 << 7)

#endif
