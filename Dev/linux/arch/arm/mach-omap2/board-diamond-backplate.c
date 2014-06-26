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
 *      This file is the ancillary support driver for the backplate
 *      GPIO functionality of the Nest Labs Diamond board.
 */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <mach/board-diamond-gpio.h>

#include "board-diamond-gpio.h"
#include "board-diamond-backplate.h"

#if !defined(CONFIG_GPIO_SYSFS)
#error "This driver requires CONFIG_GPIO_SYSFS sysfs interface for GPIOs"
#endif /* !defined(CONFIG_GPIO_SYSFS) */

/* Preprocessor Definitions */

#define DIAMOND_BACKPLATE_DRIVER_NAME		"Diamond Backplate GPIO Driver"
#define DIAMOND_BACKPLATE_DRIVER_VERSION	"2011-03-21"

/* Function Prototypes */

static int __devinit diamond_backplate_probe(struct platform_device *pdev);
static int __devexit diamond_backplate_remove(struct platform_device *pdev);

/* Global Variables */

static struct platform_driver diamond_backplate_driver = {
   .driver		= {
       .name	= "diamond-backplate",
   },
   .probe		= diamond_backplate_probe,
   .remove		= __devexit_p(diamond_backplate_remove)
};

static bool probed = false;

void diamond_backplate_reset(void)
{
	if (probed)
            gpio_set_value(DIAMOND_GPIO_BACKPLATE_RESET, 1);
}

EXPORT_SYMBOL(diamond_backplate_reset);


static int __devinit diamond_backplate_probe(struct platform_device *pdev)
{
	int status = 0;
	unsigned gpio;
	int value;
	const char *link;
	const char *name;

	gpio = DIAMOND_GPIO_BACKPLATE_DETECT;
	name = "Backplate Detect";
	link = "detect#";

	status = diamond_gpio_input_request_and_export(gpio, name, &pdev->dev, link);
	if (status)
		goto done;

	// If platform data is identically zero, then we are on an older
	// development board where the GPIO is mapped to the 3.3v
	// enable. Otherwise, we are on a prototype board where the GPIO
	// is mapped to reset.

	if (pdev->dev.platform_data == 0) {
		gpio = DIAMOND_GPIO_BACKPLATE_3V3_ENABLE;
		name = "Backplate Power";
		value = 1;
		link = "power_enable";
	} else {
		gpio = DIAMOND_GPIO_BACKPLATE_RESET;
		name = "Backplate Reset";
		value = 0;
		link = "reset";
	}

	status = diamond_gpio_output_request_and_export(gpio, name, value, &pdev->dev, link);

	if (status)
		goto unexport_and_free_detect;

        probed = true;
	goto done;

 unexport_and_free_detect:
	diamond_gpio_unexport_and_free(DIAMOND_GPIO_BACKPLATE_DETECT);

 done:
	return status;
}

static int __devexit diamond_backplate_remove(struct platform_device *pdev)
{
	if (pdev->dev.platform_data == 0) {
		diamond_gpio_unexport_and_free(DIAMOND_GPIO_BACKPLATE_3V3_ENABLE);
	} else {
		diamond_gpio_unexport_and_free(DIAMOND_GPIO_BACKPLATE_RESET);
	}

	diamond_gpio_unexport_and_free(DIAMOND_GPIO_BACKPLATE_DETECT);

	return 0;
}

static int __init diamond_backplate_init(void)
{
	pr_info("%s %s\n",
			DIAMOND_BACKPLATE_DRIVER_NAME,
			DIAMOND_BACKPLATE_DRIVER_VERSION);

   return platform_driver_register(&diamond_backplate_driver);
}

static void __exit diamond_backplate_exit(void)
{
   platform_driver_unregister(&diamond_backplate_driver);
}

module_init(diamond_backplate_init);
module_exit(diamond_backplate_exit);

MODULE_AUTHOR("Grant Erickson <grant@nestlabs.com>");
MODULE_DESCRIPTION("Nest Labs " DIAMOND_BACKPLATE_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
MODULE_VERSION(DIAMOND_BACKPLATE_DRIVER_VERSION);
