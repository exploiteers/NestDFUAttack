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
 *      Platform-specific data for the USB mass storage class (MSC)
 *      gadget.
 */

#ifndef	__LINUX_USB_MSC_H
#define	__LINUX_USB_MSC_H

struct usb_msc_platform_data {
	const char *vendor;
	const char *product;
	int release;
	int nluns;
};

#endif /* __LINUX_USB_MSC_H */
