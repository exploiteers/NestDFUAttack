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
 *      Dynamic composite multi-function framework driver for USB
 *      gadgets. Based on the "Gadget Driver for Android".
 */

/* #define DEBUG */
/* #define VERBOSE_DEBUG */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>

#include <linux/usb/dynamic.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include "gadget_chips.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"
#include "composite.c"

/* Preprocessor Definitions */

#define DYNAMIC_DRIVER_NAME			"Dynamic Multi-function Composite USB Gadget Driver"
#define DYNAMIC_DRIVER_VERSION		"2011-02-28"

/* Function Prototypes */

static int __devinit dynamic_probe(struct platform_device *pdev);
static int __devexit dynamic_remove(struct platform_device *pdev);

/* Global Variables */

static struct platform_driver dynamic_gadget_driver = {
   .driver		= {
		.name	= "dynamic_usb",
   },
   .probe		= dynamic_probe,
   .remove		= __devexit_p(dynamic_remove)
};

static const char longname[] = DYNAMIC_DRIVER_NAME;

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID		0xFFFF
#define PRODUCT_ID		0xFFFF

struct dynamic_usb_dev {
	struct usb_composite_dev *cdev;
	struct usb_configuration *config;
	int num_products;
	const struct dynamic_usb_product *products;
	int num_functions;
	const char **functions;

	int vendor_id;
	int product_id;
	int version;
};

static struct dynamic_usb_dev *_dynamic_usb_dev;

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX			1
#define STRING_SERIAL_IDX			2

/* String Table */
static struct usb_string strings_dev[] = {
	/* These dummy values should be overridden by platform data */
	[STRING_MANUFACTURER_IDX].s = "Linux",
	[STRING_PRODUCT_IDX].s = "Linux",
	[STRING_SERIAL_IDX].s = "0123456789ABCDEF",
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= __constant_cpu_to_le16(USB_LANG_EN_US),
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(USB_BCD_REL20),
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

static struct list_head _functions = LIST_HEAD_INIT(_functions);
static bool _are_functions_bound;

static struct dynamic_usb_function *get_function(const char *name)
{
	struct dynamic_usb_function	*f;
	list_for_each_entry(f, &_functions, list) {
		if (!strcmp(name, f->name))
			return f;
	}
	return 0;
}

static bool are_functions_registered(const struct dynamic_usb_dev *dev)
{
	const char **functions = dev->functions;
	int i;

	/* Look only for functions required by the board config */
	for (i = 0; i < dev->num_functions; i++) {
		const char *name = *functions++;
		bool is_match = false;
		/* Could reuse get_function() here, but a reverse search
		 * should yield less comparisons overall */
		struct dynamic_usb_function *f;
		list_for_each_entry_reverse(f, &_functions, list) {
			if (!strcmp(name, f->name)) {
				is_match = true;
				break;
			}
		}
		if (is_match)
			continue;
		else
			return false;
	}

	return true;
}

static bool should_bind_functions(struct dynamic_usb_dev *dev)
{
	/* Don't waste time if the main driver hasn't bound */
	if (!dev->config)
		return false;

	/* Don't waste time if we've already bound the functions */
	if (_are_functions_bound)
		return false;

	/* This call is the most costly, so call it last */
	if (!are_functions_registered(dev))
		return false;

	return true;
}

static void bind_functions(const struct dynamic_usb_dev *dev)
{
	struct dynamic_usb_function	*f;
	const char **functions = dev->functions;
	int i;

	for (i = 0; i < dev->num_functions; i++) {
		const char *name = *functions++;
		f = get_function(name);
		if (f)
			f->bind_config(dev->config);
		else
			pr_err("function %s not found in %s\n", name, __func__);
	}

	_are_functions_bound = true;
}

static int dynamic_bind_config(struct usb_configuration *c)
{
	struct dynamic_usb_dev *dev = _dynamic_usb_dev;

	dev->config = c;

	if (should_bind_functions(dev))
		bind_functions(dev);

	return 0;
}

static int dynamic_setup_config(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl);

static struct usb_configuration dynamic_config_driver = {
	.label		= "dynamic",
	.setup		= dynamic_setup_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 500 / 2, /* 500ma */
};

static int dynamic_setup_config(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl)
{
	int i;
	int ret = -EOPNOTSUPP;

	for (i = 0; i < dynamic_config_driver.next_interface_id; i++) {
		if (dynamic_config_driver.interface[i]->setup) {
			ret = dynamic_config_driver.interface[i]->setup(
				dynamic_config_driver.interface[i], ctrl);
			if (ret >= 0)
				return ret;
		}
	}
	return ret;
}

static bool product_has_function(const struct dynamic_usb_product *p,
		const struct usb_function *f)
{
	const char **functions = p->functions;
	int count = p->num_functions;
	const char *name = f->name;
	int i;

	for (i = 0; i < count; i++) {
		/* For functions with multiple instances, usb_function.name
		 * will have an index appended to the core name (ex: acm0),
		 * while dynamic_usb_product.functions[i] will only have the
		 * core name (ex: acm). So, only compare up to the length of
		 * dynamic_usb_product.functions[i].
		 */
		if (!strncmp(name, functions[i], strlen(functions[i])))
			return true;
	}
	return false;
}

static bool product_matches_functions(const struct dynamic_usb_product *p)
{
	struct usb_function		*f;
	list_for_each_entry(f, &dynamic_config_driver.functions, list) {
		if (product_has_function(p, f) == !!f->disabled)
			return false;
	}
	return true;
}

static int get_vendor_id(const struct dynamic_usb_dev *dev)
{
	const struct dynamic_usb_product *p = dev->products;
	int count = dev->num_products;
	int i;

	if (p) {
		for (i = 0; i < count; i++, p++) {
			if (p->vendor_id && product_matches_functions(p))
				return p->vendor_id;
		}
	}
	/* use default vendor ID */
	return dev->vendor_id;
}

static int get_product_id(const struct dynamic_usb_dev *dev)
{
	const struct dynamic_usb_product *p = dev->products;
	int count = dev->num_products;
	int i;

	if (p) {
		for (i = 0; i < count; i++, p++) {
			if (product_matches_functions(p))
				return p->product_id;
		}
	}
	/* use default product ID */
	return dev->product_id;
}

static int dynamic_bind(struct usb_composite_dev *cdev)
{
	struct dynamic_usb_dev *dev = _dynamic_usb_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum, id, ret;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	/* register our configuration */
	ret = usb_add_config(cdev, &dynamic_config_driver, dynamic_bind_config);
	if (ret) {
		pr_err("usb_add_config failed\n");
		return ret;
	}

	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(USB_BCD_REL20 + gcnum);
	else {
		/* gadget zero is so simple (for now, no altsettings) that
		 * it SHOULD NOT have problems with bulk-capable hardware.
		 * so just warn about unrcognized controllers -- don't panic.
		 *
		 * things like configuration and altsetting numbering
		 * can need hardware-specific attention though.
		 */
		pr_warning("%s: controller '%s' not recognized\n",
			longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;
	device_desc.idVendor = __constant_cpu_to_le16(get_vendor_id(dev));
	device_desc.idProduct = __constant_cpu_to_le16(get_product_id(dev));
	cdev->desc.idVendor = device_desc.idVendor;
	cdev->desc.idProduct = device_desc.idProduct;

	return 0;
}

static struct usb_composite_driver dynamic_composite_driver = {
	.name		= "dynamic_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.enable_function = dynamic_enable_function,
};

void dynamic_register_function(struct dynamic_usb_function *f)
{
	struct dynamic_usb_dev *dev = _dynamic_usb_dev;

	pr_info("%s %s\n", __func__, f->name);

	list_add_tail(&f->list, &_functions);

	if (dev && should_bind_functions(dev))
		bind_functions(dev);
}

void update_dev_desc(struct dynamic_usb_dev *dev)
{
	struct usb_function *f;
	struct usb_function *last_enabled_f = NULL;
	int num_enabled = 0;
	int has_iad = 0;

	dev->cdev->desc.bDeviceClass = USB_CLASS_PER_INTERFACE;
	dev->cdev->desc.bDeviceSubClass = 0x00;
	dev->cdev->desc.bDeviceProtocol = 0x00;

	list_for_each_entry(f, &dynamic_config_driver.functions, list) {
		if (!f->disabled) {
			num_enabled++;
			last_enabled_f = f;
			if (f->descriptors[0]->bDescriptorType ==
					USB_DT_INTERFACE_ASSOCIATION)
				has_iad = 1;
		}
		if (num_enabled > 1 && has_iad) {
			dev->cdev->desc.bDeviceClass = USB_CLASS_MISC;
			dev->cdev->desc.bDeviceSubClass = 0x02;
			dev->cdev->desc.bDeviceProtocol = 0x01;
			break;
		}
	}
}

void dynamic_enable_function(struct usb_function *f, int enable)
{
	struct dynamic_usb_dev *dev = _dynamic_usb_dev;
	int disable = !enable;

	if (!!f->disabled != disable) {
		usb_function_set_enabled(f, !disable);


		update_dev_desc(dev);

		device_desc.idVendor = __constant_cpu_to_le16(get_vendor_id(dev));
		device_desc.idProduct = __constant_cpu_to_le16(get_product_id(dev));
		if (dev->cdev) {
			dev->cdev->desc.idVendor = device_desc.idVendor;
			dev->cdev->desc.idProduct = device_desc.idProduct;
		}
		usb_composite_force_reset(dev->cdev);
	}
}

static int __devinit dynamic_probe(struct platform_device *pdev)
{
	struct dynamic_usb_platform_data *pdata = pdev->dev.platform_data;
	struct dynamic_usb_dev *dev = _dynamic_usb_dev;

	if (pdata) {
		dev->products = pdata->products;
		dev->num_products = pdata->num_products;
		dev->functions = pdata->functions;
		dev->num_functions = pdata->num_functions;
		if (pdata->vendor_id) {
			dev->vendor_id = pdata->vendor_id;
			device_desc.idVendor =
				__constant_cpu_to_le16(pdata->vendor_id);
		}
		if (pdata->product_id) {
			dev->product_id = pdata->product_id;
			device_desc.idProduct =
				__constant_cpu_to_le16(pdata->product_id);
		}
		if (pdata->version)
			dev->version = pdata->version;

		if (pdata->product_name)
			strings_dev[STRING_PRODUCT_IDX].s = pdata->product_name;
		if (pdata->manufacturer_name)
			strings_dev[STRING_MANUFACTURER_IDX].s =
					pdata->manufacturer_name;
		if (pdata->serial_number)
			strings_dev[STRING_SERIAL_IDX].s = pdata->serial_number;
	}

	return usb_composite_probe(&dynamic_composite_driver, dynamic_bind);
}

static int __devexit dynamic_remove(struct platform_device *pdev)
{
	return 0;
}

static int __init dynamic_init(void)
{
	struct dynamic_usb_dev *dev;

	pr_info("%s %s\n",
			DYNAMIC_DRIVER_NAME,
			DYNAMIC_DRIVER_VERSION);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/* set default values, which should be overridden by platform data */
	dev->product_id = PRODUCT_ID;
	_dynamic_usb_dev = dev;

	return platform_driver_register(&dynamic_gadget_driver);
}

static void __exit dynamic_cleanup(void)
{
	usb_composite_unregister(&dynamic_composite_driver);
	platform_driver_unregister(&dynamic_gadget_driver);
	kfree(_dynamic_usb_dev);
	_dynamic_usb_dev = NULL;
}

module_init(dynamic_init);
module_exit(dynamic_cleanup);

MODULE_AUTHOR("Grant Erickson <grant@nestlabs.com>");
MODULE_DESCRIPTION(DYNAMIC_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
MODULE_VERSION(DYNAMIC_DRIVER_VERSION);
