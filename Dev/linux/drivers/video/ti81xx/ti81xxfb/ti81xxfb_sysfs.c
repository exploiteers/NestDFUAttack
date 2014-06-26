/*
 * linux/drivers/video/ti81xx/ti81xxfb/ti81xxfb_sysfs.c
 *
 * Copyright (C) 2009 Texas Instruments
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * Some codes and ideals are from TI OMAP2 by Tomi Valkeinen
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/ti81xxfb.h>

#include "fbpriv.h"

static ssize_t show_size(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);

	return snprintf(buf, PAGE_SIZE, "%lu\n", tfbi->mreg.size);
}


static ssize_t store_size(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf,
			  size_t count)
{
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	unsigned long size;
	int r;

	size = PAGE_ALIGN(simple_strtoul(buf, NULL, 0));
	ti81xxfb_lock(tfbi);

	/* FIX ME make sure that the FB is not actived,
		or we can not change it*/

	if (tfbi->gctrl->gstate.isstarted) {
		r = -EBUSY;
		goto out;
	}
	if (size != tfbi->mreg.size) {
		r = ti81xxfb_realloc_fbmem(fbi, size);
		if (r) {
			dev_err(dev, "realloc fbmem failed\n");
			goto out;
		}
	}

	r = count;
out:
	ti81xxfb_unlock(tfbi);

	return r;
}

static ssize_t show_phys(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);

	return snprintf(buf, PAGE_SIZE, "%0x\n", tfbi->mreg.paddr);
}

static ssize_t show_virt(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);

	return snprintf(buf, PAGE_SIZE, "%p\n", tfbi->mreg.vaddr);
}

static struct device_attribute ti81xxfb_attrs[] = {
	__ATTR(size, S_IRUGO | S_IWUSR, show_size, store_size),
	__ATTR(phy_addr, S_IRUGO, show_phys, NULL),
	__ATTR(virt_addr, S_IRUGO, show_virt, NULL),
};

int ti81xxfb_create_sysfs(struct ti81xxfb_device *fbdev)
{
	int i;
	int r;

	for (i = 0; i < fbdev->num_fbs; i++) {
		int t;
		for (t = 0; t < ARRAY_SIZE(ti81xxfb_attrs); t++) {
			r = device_create_file(fbdev->fbs[i]->dev,
					&ti81xxfb_attrs[t]);

			if (r) {
				dev_err(fbdev->dev,
					"failed to create sysfs file\n");
				return r;
			}
		}
	}

	return 0;
}

void ti81xxfb_remove_sysfs(struct ti81xxfb_device *fbdev)
{
	int i, t;

	for (i = 0; i < fbdev->num_fbs; i++) {
		for (t = 0; t < ARRAY_SIZE(ti81xxfb_attrs); t++)
			device_remove_file(fbdev->fbs[i]->dev,
					&ti81xxfb_attrs[t]);
	}
}


