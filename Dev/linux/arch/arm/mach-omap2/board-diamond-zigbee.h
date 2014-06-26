/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
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
 *      This file defines the platform data structure for the 
 *      Diamond/J49 Zigbee driver.
 */

#ifndef ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_ZIGBEE_PRIVATE_H
#define ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_ZIGBEE_PRIVATE_H

struct diamond_zigbee_platform_data {
        unsigned int reset_gpio;
        unsigned int pwr_enable_gpio;
        unsigned int interrupt_gpio;
};

#endif /* ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_ZIGBEE_PRIVATE_H */
