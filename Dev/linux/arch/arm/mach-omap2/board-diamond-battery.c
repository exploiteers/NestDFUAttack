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
 *      This file is the ancillary support driver for the battery
 *      GPIO functionality of the Nest Diamond board.
 */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <mach/board-diamond-gpio.h>

#include "board-diamond-gpio.h"
#include "board-diamond-battery.h"

#if !defined(CONFIG_GPIO_SYSFS)
#error "This driver requires CONFIG_GPIO_SYSFS sysfs interface for GPIOs"
#endif /* !defined(CONFIG_GPIO_SYSFS) */

/* Preprocessor Definitions */

#define DIAMOND_BATTERY_DRIVER_NAME			"Diamond Battery GPIO Driver"
#define DIAMOND_BATTERY_DRIVER_VERSION		"2011-05-11"

/* Function Prototypes */

static int __devinit diamond_battery_probe(struct platform_device *pdev);
static int __devexit diamond_battery_remove(struct platform_device *pdev);

/* Global Variables */

static struct platform_driver diamond_battery_driver = {
   .driver		= {
       .name	= "diamond-battery",
   },
   .probe		= diamond_battery_probe,
   .remove		= __devexit_p(diamond_battery_remove)
};

static bool probed = false;

void diamond_battery_disconnect(bool disconnect)
{
	if (probed)
		gpio_set_value(DIAMOND_GPIO_BATT_DISCONNECT, disconnect);
}

EXPORT_SYMBOL(diamond_battery_disconnect);

static int __devinit diamond_battery_probe(struct platform_device *pdev)
{
	int status = 0;
	unsigned gpio;
	int value;
	const char *link;
	const char *name;

	gpio = DIAMOND_GPIO_BATT_DISCONNECT;
	name = "Battery Disconnect";
	value = 0;
	link = "disconnect";

	status = diamond_gpio_output_request_and_export(gpio, name, value, &pdev->dev, link);

	if (status < 0)
		return status;

	probed = true;

	return 0;
}

static int __devexit diamond_battery_remove(struct platform_device *pdev)
{
	diamond_gpio_unexport_and_free(DIAMOND_GPIO_BATT_DISCONNECT);

	probed = false;

	return 0;
}

static int __init diamond_battery_init(void)
{
	pr_info("%s %s\n",
			DIAMOND_BATTERY_DRIVER_NAME,
			DIAMOND_BATTERY_DRIVER_VERSION);

	return platform_driver_register(&diamond_battery_driver);
}

static void __exit diamond_battery_exit(void)
{
	platform_driver_unregister(&diamond_battery_driver);
}

module_init(diamond_battery_init);
module_exit(diamond_battery_exit);

MODULE_AUTHOR("Grant Erickson <grant@nestlabs.com>");
MODULE_DESCRIPTION("Nest " DIAMOND_BATTERY_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
MODULE_VERSION(DIAMOND_BATTERY_DRIVER_VERSION);
