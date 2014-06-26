/*
 *    Copyright (c) 2011 Nest Labs, Inc.
 *
 *    Copyright (c) 2010 Analog Devices Inc.
 *
 *    (C) Copyright 2010
 *    Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 *      This file implements support for the public
 *      bootcount_{load,store} functions which back the booting
 *      failover feature.
 *
 *      With this feature, U-Boot uses the 'bootcount' and 'bootlimit'
 *      environment variables to try 'bootcmd' 'bootlimit' times
 *      before "failing over" and resorting to 'altbootcmd'.
 *
 *      The current boot count must be stored in some system location
 *      that is preserved across a warm reset.
 *
 *      The 1 KiB of TI OMAP3 SCM scratchpad memory provides just such
 *      a location. The lower 784 bytes, from 0x4800 2600 to 0x4800
 *      2910, of SCM scratch pad is reserved for OMAP-internal pin
 *      configuration suspend/resume save/restore state.
 *
 *      The remaining 240 bytes, from 0x4800 2910 to 0x4800 2A00, is
 *      available as user-defined scratchpad memory. The Linux kernel
 *      uses 172 of these bytes as follows:
 *
 *        - 24 bytes at 0x48002910 for generic suspend/resume state
 *        - 56 bytes at 0x4800293c for PRCM suspend/resume state
 *        - 88 bytes at 0x48002974 for SDRC suspend/resume state
 *        -  4 bytes at 0x480029cc for ARM core suspend/resume state
 *
 *      Additionally, the kernel also writes 4 bytes of user-defined
 *      scratchpad at 0x48002914 to communicate the kernel boot mode
 *      to U-Boot (or any other boot loader) as follows:
 *
 *         ('B' << 24) | ('M' << 16) | reboot_mode
 *
 *      For boot count support, we use the the top 4- or 8-bytes of
 *      the 240 bytes of user-defined of SCM scratchpad memory to
 *      allow Linux's scratchpad needs to grow and to generally stay
 *      out of the way of any other scratchpad use.
 */

#include <common.h>
#include <asm/io.h>

/* Preprocessor Definitions */

#define MAGIC_MASK 0xffff0000
#define COUNT_MASK 0x0000ffff

/*
 * SCM Scratchpad
 */
#define OMAP34XX_CONTROL_SAVE_RESTORE_MEM_OFF	0x600
#define OMAP34XX_CONTROL_SAVE_RESTORE_MEM_BASE	(OMAP34XX_CTRL_BASE + \
												 OMAP34XX_CONTROL_SAVE_RESTORE_MEM_OFF)
#define OMAP34XX_CONTROL_SAVE_RESTORE_MEM_SIZE	(1 << 10)

/*
 * User-defined SCM Scratchpad
 */
#define OMAP34XX_CONTROL_SCRATCHPAD_OFF			0x310
#define OMAP34XX_CONTROL_SCRATCHPAD_BASE		(OMAP34XX_CONTROL_SAVE_RESTORE_MEM_BASE + \
												 OMAP34XX_CONTROL_SCRATCHPAD_OFF)
#define OMAP34XX_CONTROL_SCRATCHPAD_SIZE		(OMAP34XX_CONTROL_SAVE_RESTORE_MEM_SIZE - \
												 OMAP34XX_CONTROL_SCRATCHPAD_OFF)

/*
 * If the user hasn't defined where they want the boot count stored,
 * then choose a default location.
 */
#if !defined(CONFIG_SYS_BOOTCOUNT_ADDR)
# if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
#  define CONFIG_SYS_BOOTCOUNT_ADDR_SIZE		sizeof(u32)
# else
#  define CONFIG_SYS_BOOTCOUNT_ADDR_SIZE		(2 * sizeof(u32))
# endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD) */
# define CONFIG_SYS_BOOTCOUNT_ADDR				(OMAP34XX_CONTROL_SCRATCHPAD_BASE + OMAP34XX_CONTROL_SCRATCHPAD_SIZE - CONFIG_SYS_BOOTCOUNT_ADDR_SIZE)
#endif /* !defined(CONFIG_SYS_BOOTCOUNT_ADDR) */

void bootcount_store(ulong count)
{
	u32 *reg = (u32 *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	writel(reg, (BOOTCOUNT_MAGIC & MAGIC_MASK) | (count & COUNT_MASK));
#else
	writel(reg, count);
	writel(reg + 1, BOOTCOUNT_MAGIC);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD) */
}

ulong bootcount_load(void)
{
	const u32 *reg = (const u32 *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	const u32 tmp = readl(reg);

	if ((tmp & MAGIC_MASK) == (BOOTCOUNT_MAGIC & MAGIC_MASK))
		return tmp & COUNT_MASK;
	else
		return 0;
#else
	if (readl(reg + 1) == BOOTCOUNT_MAGIC)
		return readl(reg);
	else
		return 0;
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD) */
}
