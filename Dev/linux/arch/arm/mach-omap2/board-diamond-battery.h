/*
 *    Copyright (c) 2011 Nest, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file defines GPIO library functions shared across
 *      multiple Nest Diamond files.
 */

#ifndef ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_BATTERY_PRIVATE_H
#define ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_BATTERY_PRIVATE_H

#include <linux/types.h>

extern void diamond_battery_disconnect(bool disconnect);

#endif /* ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_BATTERY_PRIVATE_H */
