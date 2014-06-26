/*
 * linux/arch/arm/mach-omap2/ti-psp-version.c
 *
 * Create a proc entry for showing PSP version.
 *
 * Copyright (C) 2011 Texas Instruments Incorporated.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __ARCH_ARM_PLAT_PSP_VERSION_H
#define __ARCH_ARM_PLAT_PSP_VERSION_H

#define TI_PSP_ENTRY		"ti-psp-version"
#define TI_PSP_VERSION		"04.02.00.07"

#if defined (CONFIG_MACH_OMAP3EVM)
#define TI_PSP_PLATFORM		"OMAP3EVM"
#elif defined(CONFIG_MACH_OMAP3517EVM)
#define TI_PSP_PLATFORM		"AM3517EVM"
#elif defined(CONFIG_MACH_OMAP3_BEAGLE)
#define TI_PSP_PLATFORM		"OMAP3BEAGLE"
#else
#define TI_PSP_DEVICE		"DEVICE"
#define TI_PSP_PLATFORM		"PLATFORM"
#endif

#endif

