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
 *      This file implements GPIO library functions shared across
 *      multiple Nest Labs Diamond files.
 */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "board-diamond-gpio.h"

int __devinit diamond_gpio_input_request_and_export(unsigned gpio, const char *name, struct device *dev, const char *link)
{
	int status = 0;
	const bool direction_may_change = true;

	status = gpio_request(gpio, name);
	if (status) {
		dev_err(dev, "Could not request GPIO %u: %d\n", gpio, status);
		goto done;
	}

	status = gpio_direction_input(gpio);
	if (status) {
		dev_err(dev, "Could not set GPIO %u as input: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export(gpio, !direction_may_change);
	if (status) {
		dev_err(dev, "Could not export GPIO %u: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export_link(dev, link, gpio);
	if (status) {
		dev_err(dev, "Could not export GPIO %u as link %s: %d\n", gpio, link, status);
		goto unexport_gpio;
	}

	goto done;

 unexport_gpio:
	gpio_unexport(gpio);

 free_gpio:
	gpio_free(gpio);

 done:
	return status;
}

int __devinit diamond_gpio_output_request_and_export(unsigned gpio, const char *name, int value, struct device *dev, const char *link)
{
	int status = 0;
	const bool direction_may_change = true;

	status = gpio_request(gpio, name);
	if (status) {
		dev_err(dev, "Could not request GPIO %u: %d\n", gpio, status);
		goto done;
	}

	status = gpio_direction_output(gpio, value);
	if (status) {
		dev_err(dev, "Could not set GPIO %u as output to value %d: %d\n", gpio, value, status);
		goto free_gpio;
	}

	status = gpio_export(gpio, !direction_may_change);
	if (status) {
		dev_err(dev, "Could not export GPIO %u: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export_link(dev, link, gpio);
	if (status) {
		dev_err(dev, "Could not export GPIO %u as link %s: %d\n", gpio, link, status);
		goto unexport_gpio;
	}

	goto done;

 unexport_gpio:
	gpio_unexport(gpio);

 free_gpio:
	gpio_free(gpio);

 done:
	return status;
}

void diamond_gpio_unexport_and_free(unsigned gpio)
{
	gpio_unexport(gpio);
	gpio_free(gpio);
}

EXPORT_SYMBOL(diamond_gpio_input_request_and_export);
EXPORT_SYMBOL(diamond_gpio_output_request_and_export);
EXPORT_SYMBOL(diamond_gpio_unexport_and_free);
