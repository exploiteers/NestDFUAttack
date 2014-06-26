/*
 *    Copyright (c) 2011 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file defines GPIO library functions shared across
 *      multiple Nest Labs Diamond files.
 */

#ifndef ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_GPIO_PRIVATE_H
#define ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_GPIO_PRIVATE_H

//#include <generated/autoconf.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

extern int __devinit diamond_gpio_input_request_and_export(unsigned gpio, const char *name, struct device *dev, const char *link);
extern int __devinit diamond_gpio_output_request_and_export(unsigned gpio, const char *name, int value, struct device *dev, const char *link);
extern void diamond_gpio_unexport_and_free(unsigned gpio);

#endif /* ARCH_ARM_MACH_OMAP2_NESTLABS_DIAMOND_GPIO_PRIVATE_H */
