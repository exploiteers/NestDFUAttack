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
 *      This file is the ancillary support driver for the ZigBee (IEEE
 *      802.15.4) GPIO functionality of the Nest Labs Diamond board.
 */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "board-diamond-gpio.h"
#include "board-diamond-zigbee.h"

#if !defined(CONFIG_GPIO_SYSFS)
#error "This driver requires CONFIG_GPIO_SYSFS sysfs interface for GPIOs"
#endif /* !defined(CONFIG_GPIO_SYSFS) */

/* Preprocessor Definitions */

#define DIAMOND_ZIGBEE_DRIVER_NAME			"Diamond ZigBee GPIO Driver"
#define DIAMOND_ZIGBEE_DRIVER_VERSION		"2011-03-21"

/* Function Prototypes */

static int __devinit diamond_zigbee_probe(struct platform_device *pdev);
static int __devexit diamond_zigbee_remove(struct platform_device *pdev);

/* Global Variables */

static struct platform_driver diamond_zigbee_driver = {
   .driver		= {
       .name	= "diamond-zigbee",
   },
   .probe		= diamond_zigbee_probe,
   .remove		= __devexit_p(diamond_zigbee_remove)
};

static int __devinit diamond_zigbee_probe(struct platform_device *pdev)
{
	int status = 0;
	unsigned gpio;
	int value;
	const char *link;
	const char *name;
	struct diamond_zigbee_platform_data *pdata = pdev->dev.platform_data;

	gpio = pdata->reset_gpio;
	name = "ZigBee Reset#";
	value = 1;
	link = "reset#";

	status = diamond_gpio_output_request_and_export(gpio, name, value, &pdev->dev, link);
	if (status)
		goto done;

	gpio = pdata->pwr_enable_gpio;
	if (gpio != 0) {
		name = "ZigBee Power";
		value = 0;
		link = "power_enable";

		status = diamond_gpio_output_request_and_export(gpio, name, value, &pdev->dev, link);
		if (status)
			goto unexport_and_free_reset;
 	}

	gpio = pdata->interrupt_gpio;
	if(gpio != 0) {
		name = "ZigBee Interrupt";
		link = "interrupt";
		status = diamond_gpio_input_request_and_export(gpio, name, &pdev->dev, link);
		if (status)
			goto unexport_and_power_enable;
	}

	goto done;

 unexport_and_power_enable:
	diamond_gpio_unexport_and_free(pdata->pwr_enable_gpio);	

 unexport_and_free_reset:
	diamond_gpio_unexport_and_free(pdata->reset_gpio);

 done:
	return status;
}

static int __devexit diamond_zigbee_remove(struct platform_device *pdev)
{
	struct diamond_zigbee_platform_data *pdata = pdev->dev.platform_data;

	diamond_gpio_unexport_and_free(pdata->reset_gpio);
        
	if (pdata->pwr_enable_gpio != 0)
		diamond_gpio_unexport_and_free(pdata->pwr_enable_gpio);

	return 0;
}

static int __init diamond_zigbee_init(void)
{
	pr_info("%s %s\n",
			DIAMOND_ZIGBEE_DRIVER_NAME,
			DIAMOND_ZIGBEE_DRIVER_VERSION);

	return platform_driver_register(&diamond_zigbee_driver);
}

static void __exit diamond_zigbee_exit(void)
{
	platform_driver_unregister(&diamond_zigbee_driver);
}

module_init(diamond_zigbee_init);
module_exit(diamond_zigbee_exit);

MODULE_AUTHOR("Grant Erickson <grant@nestlabs.com>");
MODULE_DESCRIPTION("Nest Labs " DIAMOND_ZIGBEE_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
MODULE_VERSION(DIAMOND_ZIGBEE_DRIVER_VERSION);
