/*
 * linux/drivers/char/ti81xx_hdmi/ti81xx_hdmi.c
 *
 * Copyright (C) 2009 Texas Instruments
 *
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

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>           /*     everything... */
#include <linux/errno.h>        /*     error codes     */
#include <linux/types.h>        /*     size_t */
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ti81xxhdmi.h>

#include "ti81xx_hdmi_cfg.h"

#define TI81XX_HDMI_DRIVER_NAME     "TI81XX_HDMI"

struct ti81xx_hdmi_params
{
	/* Handle to library */
	void* hdmi_lib_handle;
	/* Other parameters */
	u32 wp_v_addr;
	u32 core_v_addr;
	u32 phy_v_addr;
	u32 prcm_v_addr;
	u32 venc_v_addr;
#ifndef CONFIG_ARCH_TI816X
	u32 hdmi_pll_v_addr;
#endif
	int i;
};

/* Global var */
struct ti81xx_hdmi_params hdmi_obj;
static struct ti81xx_hdmi_init_params initParams;


/* Module param */
static int  hdmi_mode = -1;

static int ti81xx_hdmi_major;
static struct cdev ti81xx_hdmi_cdev;
static dev_t ti81xx_hdmi_dev_id;
static struct device *ti81xx_hdmi_device;
static struct class *ti81xx_hdmi_class = NULL;

static void ti81xx_hdmi_platform_release(struct device *device);
static int ti81xx_hdmi_probe(struct device *device);
static int ti81xx_hdmi_remove(struct device *device);
static int ti81xx_hdmi_open(struct inode *inode, struct file *filp);
static int ti81xx_hdmi_release(struct inode *inode, struct file *filp);
static long ti81xx_hdmi_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg);

static struct file_operations ti81xx_hdmi_fops = {
	.owner = THIS_MODULE,
	.open = ti81xx_hdmi_open,
	.release = ti81xx_hdmi_release,
	.unlocked_ioctl = ti81xx_hdmi_ioctl,
};

static struct device_driver ti81xx_hdmi_driver = {
	.name = TI81XX_HDMI_DRIVER_NAME,
	.bus = &platform_bus_type,
	.probe = ti81xx_hdmi_probe,
	.remove = ti81xx_hdmi_remove,
};

static struct platform_device ti81xx_hdmi_plat_device = {
	.name = TI81XX_HDMI_DRIVER_NAME,
	.id = 2,
	.dev = {
		.release = ti81xx_hdmi_platform_release,
	}
};

/*
 * ti81xx_hdmi_open: This function opens hdmi driver.
 */
static int ti81xx_hdmi_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	/* Call library to open HDMI */
	hdmi_obj.hdmi_lib_handle = ti81xx_hdmi_lib_open(0, &ret, 0x0);
	if ((ret == 0x0) && (hdmi_obj.hdmi_lib_handle != NULL)) {

		THDBG("TI81xx_hdmi: Opend\n");
		filp->private_data = &hdmi_obj;

	}
	else{
		printk("TI81xx_hdmi: Could not open %d %p\n",
				ret, hdmi_obj.hdmi_lib_handle);
	}

	return ret;
}

/*
 * ti81xx_hdmi_release: This function releases hdmi driver.
 */
static int ti81xx_hdmi_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct ti81xx_hdmi_params *params =
		(struct ti81xx_hdmi_params *)filp->private_data;

	THDBG("TI81xx_hdmi: Release\n");

	/* Call close of the library */
	if (!ret) {
		if (params) {
			/* kfree(params);*/
			filp->private_data = NULL;
		}
	}

	return ret;
}

/*
 * ti81xx_hdmi_ioctl: This function will process IOCTL commands sent by
 * the application.
 */
static long ti81xx_hdmi_ioctl(struct file *file, unsigned int cmd,
			unsigned long arg)
{
	struct ti81xx_hdmi_params *params =
		(struct ti81xx_hdmi_params *)file->private_data;
	void * handle = params->hdmi_lib_handle;
	THDBG("TI81xx_hdmi: Ioctl\n");
	return (long)(ti81xx_hdmi_lib_control(handle, cmd, (void *)arg, NULL));
}

static void ti81xx_hdmi_platform_release(struct device *device)
{
	/* this is called when the reference count goes to zero */
}
static int ti81xx_hdmi_probe(struct device *device)
{
	THDBG("TI81xx_hdmi: probe\n");
	return 0;
}
static int ti81xx_hdmi_remove(struct device *device)
{
	THDBG("TI81xx_hdmi: remove\n");
	return 0;
}

/**
 * ti81xx_hdmi_init() - Initialize TI81XX HDMI Driver
 */
int __init ti81xx_hdmi_init(void)
{
	int result;

	/* Get the major number for this module */
	result = alloc_chrdev_region(&ti81xx_hdmi_dev_id, 0, 1, TI81XX_HDMI_DRIVER_NAME);
	if (result) {
		printk("TI81xx_HDMI: Cound not register region\n");
		goto err_exit;
	}

	ti81xx_hdmi_major = MAJOR(ti81xx_hdmi_dev_id);
	/*     printk("Major Number %d MinorNumber %d\n",  MAJOR(ti81xx_hdmi_dev_id),  MINOR(ti81xx_hdmi_dev_id));*/

	/* initialize character device */
	cdev_init(&ti81xx_hdmi_cdev, &ti81xx_hdmi_fops);
	ti81xx_hdmi_cdev.owner = THIS_MODULE;
	ti81xx_hdmi_cdev.ops = &ti81xx_hdmi_fops;

	/* add char driver */
	result = cdev_add(&ti81xx_hdmi_cdev, ti81xx_hdmi_dev_id, 1);
	if (result) {
		printk("TI81xx_hdmi: Could not add hdmi char driver\n");
		goto err_remove_region;
	}

	/* register driver as a platform driver */
	result = driver_register(&ti81xx_hdmi_driver);
	if (result) {
		printk("TI81xx_hdmi: Cound register driver\n");
		goto err_remove_cdev;
	}

	/* register the drive as a platform device */
	result = platform_device_register(&ti81xx_hdmi_plat_device);
	if (result) {
		printk("TI81xx_hdmi: Cound register as platform device\n");
		goto err_driver_unregister;
	}

	ti81xx_hdmi_class = class_create(THIS_MODULE, TI81XX_HDMI_DRIVER_NAME);
	if (IS_ERR(ti81xx_hdmi_class)) {
		result = -EIO;
		printk("TI81xx_hdmi: Could not create class\n");
		goto err_remove_platform_device;
	}

	ti81xx_hdmi_device = device_create(ti81xx_hdmi_class, NULL,
			ti81xx_hdmi_dev_id, NULL,
			TI81XX_HDMI_DRIVER_NAME);
	if(IS_ERR(ti81xx_hdmi_device)) {
		result = -EIO;
		printk("TI81xx_hdmi: Cound not create device file\n");
		goto err_remove_class;
	}

	hdmi_obj.prcm_v_addr = (int) ioremap(PRCM_0_REGS, 0x500);
	if (hdmi_obj.prcm_v_addr == 0x0){
		printk("TI81xx_hdmi: Could not ioremap for PRCM\n");
		goto err_remove_class;
	} else {
		THDBG("PRCM at address %x\n", hdmi_obj.prcm_v_addr);
	}
	/* Initialize the global strucutres... */
	hdmi_obj.hdmi_lib_handle = NULL;
	hdmi_obj.wp_v_addr = (int) ioremap(HDMI_WP_0_REGS, 512);
	if (hdmi_obj.wp_v_addr == 0x0){
		printk("TI81xx_hdmi: Could not ioremap for WP\n");
		goto err_remove_class;
	} else {
		THDBG("Wrapper at address %x\n", hdmi_obj.wp_v_addr);
	}
	hdmi_obj.core_v_addr = (int) ioremap(HDMI_CORE_0_REGS, 2560);
	if (hdmi_obj.core_v_addr == 0x0){
		printk("TI81xx_hdmi: Could not ioremap for Core\n");
		goto err_remove_class;
	} else {
		THDBG("Core at address %x\n", hdmi_obj.core_v_addr);
	}
#if 0
	hdmi_obj.phy_v_addr = kmalloc(512, GFP_KERNEL);
	if (!hdmi_obj.phy_v_addr){
		printk("TI81xx_hdmi: Could not ioremap for PHY\n");
		goto err_remove_class;
	} else {
		THDBG("PHY at address %x\n", hdmi_obj.phy_v_addr);
	}
#else
	hdmi_obj.phy_v_addr = (int) ioremap(HDMI_PHY_0_REGS, 64);
	if (hdmi_obj.phy_v_addr == 0x0){
		printk("TI81xx_hdmi: Could not ioremap for PHY\n");
		goto err_remove_class;
	} else {
		THDBG("PHY at address %x\n", hdmi_obj.phy_v_addr);
	}
#endif
	hdmi_obj.venc_v_addr = (volatile u32) ioremap(0x48106000, 0x80);
	if (hdmi_obj.venc_v_addr == 0x0){
		printk("TI81xx_hdmi: Could not ioremap for Venc\n");
		goto err_remove_class;
	} else {
		THDBG("PHY at address %x\n", hdmi_obj.venc_v_addr);
	}
#ifndef CONFIG_ARCH_TI816X
	hdmi_obj.hdmi_pll_v_addr = (volatile u32) ioremap(0x481c5200, 0x80);
	if (hdmi_obj.hdmi_pll_v_addr == 0x0){
		printk("TI81xx_hdmi: Could not ioremap for HDMI PLL\n");
		goto err_remove_class;
	} else {
		THDBG("HDMI PLL at address %x\n", hdmi_obj.hdmi_pll_v_addr);
	}
#endif
	/* Initialize the HDMI library */
	initParams.wp_base_addr       =   (u32) hdmi_obj.wp_v_addr;
	initParams.core_base_addr     =   (u32) hdmi_obj.core_v_addr;
	initParams.phy_base_addr      =   (u32) hdmi_obj.phy_v_addr;
	initParams.prcm_base_addr     =   (u32) hdmi_obj.prcm_v_addr;
	initParams.venc_base_addr     =   (u32) hdmi_obj.venc_v_addr;
#ifndef CONFIG_ARCH_TI816X
	initParams.hdmi_pll_base_addr =   (u32) hdmi_obj.hdmi_pll_v_addr;
#endif

	/* Set the HDMI user to proper value if not set correctly */
	if (hdmi_mode != -1 && (hdmi_mode < 0 || hdmi_mode >= hdmi_max_mode))
	{
		hdmi_mode = hdmi_1080P_60_mode;
	}
	if (ti81xx_hdmi_lib_init(&initParams, hdmi_mode) != 0x0){
		printk("TI81xx_hdmi: Init failed\n");
		goto err_remove_class;
	}
	THDBG("TI81xx_hdmi: Initialized \n");
	return 0;

err_remove_class:
	class_destroy(ti81xx_hdmi_class);
err_remove_platform_device:
	platform_device_unregister(&ti81xx_hdmi_plat_device);
err_driver_unregister:
	driver_unregister(&ti81xx_hdmi_driver);
err_remove_cdev:
	cdev_del(&ti81xx_hdmi_cdev);
err_remove_region:
	unregister_chrdev_region(ti81xx_hdmi_dev_id, 1);
err_exit:
	return result;
}

/**
 * ti81xx_hdmi_exit() - Perform clean before unload
 */
void __exit ti81xx_hdmi_exit(void)
{
	ti81xx_hdmi_lib_deinit(NULL);
	device_destroy(ti81xx_hdmi_class, ti81xx_hdmi_dev_id);
	class_destroy(ti81xx_hdmi_class);
	platform_device_unregister(&ti81xx_hdmi_plat_device);
	driver_unregister(&ti81xx_hdmi_driver);
	cdev_del(&ti81xx_hdmi_cdev);
	unregister_chrdev_region(ti81xx_hdmi_dev_id, 1);
	iounmap((int *)hdmi_obj.wp_v_addr);
	iounmap((int *)hdmi_obj.core_v_addr);
	iounmap((int *)hdmi_obj.phy_v_addr);
	iounmap((int *)hdmi_obj.prcm_v_addr);
	iounmap((int *)hdmi_obj.venc_v_addr);
#ifndef CONFIG_ARCH_TI816X
	iounmap((int *)hdmi_obj.hdmi_pll_v_addr);
#endif
}
module_param_named(hdmi_mode, hdmi_mode, int, 0664);

module_init(ti81xx_hdmi_init);
module_exit(ti81xx_hdmi_exit);
MODULE_LICENSE("GPL");

