/*
 *    Copyright (c) 2011 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    Copyright (C) 2008 Google, Inc.
 *
 *      Author: Mike Lockwood <lockwood@android.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      Platform-specific data for the dynamic composite multi-
 *      function framework driver for USB gadgets. Based on the
 *      "Gadget Driver for Android".
 */

#ifndef	__LINUX_USB_DYNAMIC_H
#define	__LINUX_USB_DYNAMIC_H

#include <linux/list.h>
#include <linux/usb/composite.h>

struct dynamic_usb_function {
	struct list_head	list;
	char			*name;
	int 			(*bind_config)(struct usb_configuration *c);
};

struct dynamic_usb_product {
	/* Vendor ID for this set of functions.
	 * Default vendor_id in platform data will be used if this is zero.
	 */
	__u16 vendor_id;

	/* Product ID for this set of functions. */
	__u16 product_id;

	/* List of function names associated with this product.
	 * This is used to compute the USB product ID dynamically
	 * based on which functions are enabled.
	 */
	int num_functions;
	const char **functions;
};

struct dynamic_usb_platform_data {
	/* USB device descriptor fields */
	__u16 vendor_id;

	/* Default product ID. */
	__u16 product_id;

	__u16 version;

	const char *product_name;
	const char *manufacturer_name;
	const char *serial_number;

	/* List of available USB products.
	 * This is used to compute the USB product ID dynamically
	 * based on which functions are enabled.
	 * if num_products is zero or no match can be found,
	 * we use the default product ID
	 */
	int num_products;
	const struct dynamic_usb_product *products;

	/* List of all supported USB functions.
	 * This list is used to define the order in which
	 * the functions appear in the configuration's list of USB interfaces.
	 * This is necessary to avoid depending upon the order in which
	 * the individual function drivers are initialized.
	 */
	int num_functions;
	const char **functions;
};

extern void dynamic_register_function(struct dynamic_usb_function *f);
extern void dynamic_enable_function(struct usb_function *f, int enable);

#endif /* __LINUX_USB_DYNAMIC_H */
