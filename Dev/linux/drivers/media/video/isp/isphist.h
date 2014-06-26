/*
 * isphist.h
 *
 * TI OMAP3 ISP - Histogram module
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * Contacts: David Cohen <david.cohen@nokia.com>
 *	     Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *	     Sakari Ailus <sakari.ailus@maxwell.research.nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef OMAP3_ISP_HIST_H
#define OMAP3_ISP_HIST_H

#include <linux/omap3isp.h>

#define ISPHIST_IN_BIT_WIDTH_CCDC	10

struct isp_device;

int isp_hist_init(struct isp_device *isp);
void isp_hist_cleanup(struct isp_device *isp);

#endif /* OMAP3_ISP_HIST */
