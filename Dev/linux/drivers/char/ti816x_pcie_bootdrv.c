/*
 * TI816X PCIe EP boot driver.
 *
 * Copyright (C) 2010 Texas Instruments, Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <asm/irq.h>

#include "ti816x_pcie_bootdrv.h"

/* PCIe application registers virtual address (mapped to PCI window) */
#define PCI_REGV(reg)           (reg_virt + reg)

struct pci_dev *ti816x_pci_dev;

/*
 * TODO: Check for the possibility of avoiding the following static variables
 * and make it part of driver/device data or at least group in one structure.
 */

/*
 * TI816X access ranges. Note that all the addresses below translate to PCIe
 * access.
 */
static unsigned int reg_phys, reg_virt, reg_len;
static unsigned int ocmc1_phys, ocmc1_virt, ocmc1_len;
static unsigned int ddr_phys, ddr_virt, ddr_len;

static int ti816x_pci_major;
static struct cdev ti816x_pci_cdev;
static struct class *ti816x_pci_class;
static dev_t ti816x_dev_id;

/**
 * ti816x_ep_find_device() - Look-up for first available TI816X Endpoint
 *
 * Since we could even be running on another TI816X device acting as RC, we need
 * to skip it - this is done based on checking device class to be set as "PCI
 * Bridge" for RC as the RC driver does this setting during enumeration.
 *
 * Note: This checking needs to be updated if RC driver is changed to set (or
 * not to set) class differently.
 */
static int ti816x_ep_find_device(void)
{
	struct pci_dev *dev = pci_get_device(TI816X_PCI_VENDOR_ID,
						TI816X_PCI_DEVICE_ID, NULL);

	while (NULL != dev) {
		pr_info(TI816X_PCIE_MODFILE ": Found TI816x PCIe EP @0x%08x\n",
				(int)dev);

		if ((dev->class >> 8) == PCI_CLASS_BRIDGE_PCI) {
			pr_warning(TI816X_PCIE_MODFILE
					": skipping TI816x PCIe RC...\n");
			dev = pci_get_device(TI816X_PCI_VENDOR_ID,
						TI816X_PCI_DEVICE_ID, dev);
			continue;
		}

		ti816x_pci_dev = dev;
		return 0;
	}

	pr_err(TI816X_PCIE_MODFILE ": No TI816x PCIe EP found\n");
	return -1;
}

/**
 * ti816x_ep_setup_bar() - Setup specified BAR on TI816X for inbound
 * @bar_num: BAR to set inbound access to
 * @addr: Address for inbound translation to TI816X internal region
 *
 * On TI816X, BAR0 is hrdwired and hence skipped for inbound configuration and a
 * maximum of 4 inbound translations are allowed.
 *
 * TODO: Add 64-bit addressing support.
 * FIXME: Might need to disable BAR before changing IB values.
 */
static int ti816x_ep_setup_bar(u32 bar_num, u32 addr)
{
	u32 bar_val, ib_num;

	if ((bar_num == 0) || (bar_num > 4))
		return -1;

	ib_num = bar_num - 1;

	bar_val = pci_resource_start(ti816x_pci_dev, bar_num);

	__raw_writel(0, PCI_REGV(IB_BAR(ib_num)));
	__raw_writel(bar_val, PCI_REGV(IB_START_LO(ib_num)));
	__raw_writel(0, PCI_REGV(IB_START_HI(ib_num)));
	__raw_writel(addr, PCI_REGV(IB_OFFSET(ib_num)));
	__raw_writel(bar_num, PCI_REGV(IB_BAR(ib_num)));

	return 0;
}

/**
 * ti816x_pci_get_resources() - Read BARs as set by Host and reserve resources
 *
 * Only reads first 3 BARs. Expects the BAR sizes are already set by the boot
 * ROM on TI816X EP as follows:
 * - BAR0 = 4KB
 * - BAR1 = 256KB
 * - BAR2 = 8MB
 *
 * FIXME: Presently this function maps all 3 BARs but in reality we only need to
 * map BAR0 and rest can be relied upon the application (mmap).
 */
static int ti816x_pci_get_resources(void)
{
	int index;
	u32  bar_start[3];
	u32  bar_len[3];
	u32  bar_flags[3];

	dev_info(&ti816x_pci_dev->dev, "BAR Configuration - \n\t   "
			"Start\t|\tLength\t|\tFlags\n");
	for (index = 0; index < 3; index++) {
		bar_start[index] = pci_resource_start(ti816x_pci_dev, index);
		bar_len[index] = pci_resource_len(ti816x_pci_dev, index);
		bar_flags[index] = pci_resource_flags(ti816x_pci_dev, index);

		if (bar_flags[index] & IORESOURCE_IO) {
			dev_err(&ti816x_pci_dev->dev,
				"This driver does not support PCI IO.\n");
			return -1;
		}

		dev_info(&ti816x_pci_dev->dev, "\t0x%08x\t|\t%d\t|\t0x%08x\n",
				(int)bar_start[index], (int)bar_len[index],
				(int)bar_flags[index]);
	}

	reg_phys = bar_start[0];
	reg_len = bar_len[0];

	if (NULL == request_mem_region(reg_phys, reg_len, "ti816x_reg")) {
		dev_err(&ti816x_pci_dev->dev, "Failed reserve reg resource\n");
		return -1;
	}

	reg_virt = (unsigned int)ioremap_nocache(reg_phys, reg_len);
	if (!reg_virt) {
		dev_err(&ti816x_pci_dev->dev, "Failed remapping registers\n");
		goto err_regremap;
	}

	dev_info(&ti816x_pci_dev->dev, "TI816X registers mapped to 0x%08x\n",
			(int)reg_virt);

	ocmc1_phys = bar_start[1];
	ocmc1_len = bar_len[1];

	if (NULL == request_mem_region(ocmc1_phys, ocmc1_len, "ti816x_ocmc1")) {
		dev_err(&ti816x_pci_dev->dev, "Failed reserve ocmc resource\n");
		goto err_ocmc1res;
	}

	ocmc1_virt = (unsigned int)ioremap_nocache(ocmc1_phys, ocmc1_len);
	if (!ocmc1_virt) {
		dev_err(&ti816x_pci_dev->dev, "Failed remapping OCMC1\n");
		goto err_ocmc1remap;
	}

	dev_info(&ti816x_pci_dev->dev, "TI816X OCMC1 mapped to 0x%08x\n",
			(int)ocmc1_virt);

	ddr_phys = bar_start[2];
	ddr_len = bar_len[2];

	if (NULL == request_mem_region(ddr_phys, ddr_len, "ti816x_ram")) {
		dev_err(&ti816x_pci_dev->dev, "Failed remapping RAM\n");
		goto err_ramres;
	}

	ddr_virt = (unsigned int)ioremap_nocache(ddr_phys, ddr_len);
	if (!ddr_virt) {
		dev_err(&ti816x_pci_dev->dev, "Failed remapping RAM\n");
		goto err_ramremap;
	}

	dev_info(&ti816x_pci_dev->dev, "TI816X DDR mapped to 0x%08x\n",
			(int)ddr_virt);

	return 0;
err_ramremap:
	release_mem_region(ddr_phys, ddr_len);
err_ramres:
	iounmap((void *)ocmc1_virt);
err_ocmc1remap:
	release_mem_region(ocmc1_phys, ocmc1_len);
err_ocmc1res:
	iounmap((void *)reg_virt);
err_regremap:
	release_mem_region(reg_phys, reg_len);

	return -1;
}

/**
 * ti816x_pci_set_master() - Set ti816x EP to be master.
 *
 * Also sets latency timer value.
 *
 * _NOTE_: As of now, it is not mandatory to set ti816x device as master for
 * boot operation and this may be skipped. It is advisable to retain some of the
 * other configurations this function does though.
 */
static void ti816x_pci_set_master(void)
{
	s32   ret_val ;
	u16  cmd_val ;
	struct pci_dev *dev = ti816x_pci_dev;

	pci_set_master(dev);
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0x80);

	/* Add support memory write invalidate */
	ret_val = pci_set_mwi(dev);

	pci_read_config_word(dev, PCI_COMMAND, (u16 *) &cmd_val);

	/* and set the master bit in command register. */
	cmd_val |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_SERR);

	pci_write_config_word(dev, PCI_COMMAND, cmd_val);
}

/**
 * ti816x_pcie_ioctl() - Application interface for booting TI816X EP
 *
 * Provides the interface to the application code to transfer data to PCI mapped
 * memory spaces. It will be used to initially transfer the U-Boot binary and
 * file. This code largely depends on the handshaking done using the Boot
 * Complete flag written/read in TI816X OCMC1 RAM by this driver as well as
 * TI816X boot ROM and U-Boot.
 */
long ti816x_pcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0 ;

	switch (cmd) {
	case TI816X_PCI_SET_DWNLD_DONE:
	{

		/* Set the download complete flag */
		__raw_writel(0x1, (ocmc1_virt + TI816X_EP_BOOTFLAG_OFFSET));

		if (arg) {
			unsigned int msec_loops = (unsigned int)arg * 1000;

			/* Wait until it is cleared  */
			while (__raw_readl((ocmc1_virt +
						TI816X_EP_BOOTFLAG_OFFSET))) {
				udelay(1000);

				if (!--msec_loops)
					break;
			}

			if (!msec_loops) {
				dev_warn(&ti816x_pci_dev->dev,
						"Bootflag clear timed out\n");
				ret = -1;
			} else {
				dev_info(&ti816x_pci_dev->dev,
						"Bootflag cleared\n");
			}
		}

	}

	break;

	case TI816X_PCI_SET_BAR_WINDOW:
	{
		struct ti816x_bar_info *bar
			= (struct ti816x_bar_info *) arg;

		if (ti816x_ep_setup_bar(bar->num, bar->addr)) {
			dev_err(&ti816x_pci_dev->dev,
					"Setting inbound for BAR%d failed\n",
					(int)bar->num);
			ret = -1;
		}
	}

	break;

	case TI816X_PCI_GET_BAR_INFO:
	{
		struct ti816x_bar_info *bar
			= (struct ti816x_bar_info *) arg;

		bar->addr = pci_resource_start(ti816x_pci_dev, bar->num);
		bar->size = pci_resource_len(ti816x_pci_dev, bar->num);
	}

	break;

	default:
		ret = -1;
	}

	return ret;
}

/* Check if requested address range lies within those supported */
#define is_in_ocmc1(a, l)	\
	((a >= ocmc1_phys) && ((a + l) <= (ocmc1_phys + ocmc1_len)))

#define is_in_ddr(a, l)		\
	((a >= ddr_phys) && ((a + l) <= (ddr_phys + ddr_len)))

/**
 * ti816x_ep_pcie_mmap() - Provide userspace mapping for specified kernel memory
 * @filp: File private data - ignored
 * @vma: User virtual memory area to map to
 *
 * At present, only allows mapping BAR1 & BAR2 spaces. It is assumed that these
 * BARs are internally translated to access ti816x OCMC1 and DDR RAM
 * respectively (application can ensure this using TI816X_PCI_SET_BAR_WINDOW
 * ioctl to setup proper translation on ti816x EP).
 *
 * Note that the application has to get the physical BAR address as assigned by
 * the host code. One way to achieve this is to use ioctl
 * TI816X_PCI_GET_BAR_INFO.
 */
int ti816x_ep_pcie_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = -EINVAL;
	unsigned long sz = vma->vm_end - vma->vm_start;
	unsigned int addr = (unsigned int)vma->vm_pgoff << PAGE_SHIFT;

	dev_info(&ti816x_pci_dev->dev, "Mapping %#lx bytes from address %#x\n",
			sz, addr);

	if (is_in_ocmc1(addr, (unsigned int)sz)
		|| is_in_ddr(addr, (unsigned int)sz)) {
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

		ret = remap_pfn_range(vma, vma->vm_start,
				vma->vm_pgoff,
				sz, vma->vm_page_prot);
	}

	return ret;
}

/**
 * ti816x_pci_fops - Declares supported file access functions
 */
static const struct file_operations ti816x_pci_fops = {
	.owner		= THIS_MODULE,
	.mmap		= ti816x_ep_pcie_mmap,
	.unlocked_ioctl = ti816x_pcie_ioctl,
};

/**
 * ti816x_ep_pci_init() - Initialize TI816X PCIe EP
 *
 * Checks if TI816X Endpoint is detected in the system. Initialize the PCI
 * configuration for the device to enable access for downloading the images and
 * booting.
 *
 * Note: Currently supports only booting the first TI816X EP detected.
 */
static int __init ti816x_ep_pci_init(void)
{
	int ret, val;

	if (ti816x_ep_find_device())
		return -1;

	dev_warn(&ti816x_pci_dev->dev, "This driver supports booting the "
			"first TI816x target found on the bus\n");

	ret = alloc_chrdev_region(&ti816x_dev_id, 0, 1, TI816X_PCIE_MODFILE);
	if (ret) {
		dev_err(&ti816x_pci_dev->dev,
				"could not allocate the character driver");
		goto err_post_pci;
	}

	ti816x_pci_major = MAJOR(ti816x_dev_id);

	cdev_init(&ti816x_pci_cdev, &ti816x_pci_fops);
	ti816x_pci_cdev.owner = THIS_MODULE;
	ti816x_pci_cdev.ops = &ti816x_pci_fops;

	ret = cdev_add(&ti816x_pci_cdev, ti816x_dev_id, TI816X_DEV_COUNT);
	if (ret) {
		dev_err(&ti816x_pci_dev->dev,
				"Failed creation of node for PCI boot\n");
		unregister_chrdev_region(ti816x_dev_id, TI816X_DEV_COUNT);
		goto err_post_pci;
	}

	dev_info(&ti816x_pci_dev->dev, "Major number %d assigned\n",
			ti816x_pci_major);

	ti816x_pci_class = class_create(THIS_MODULE, TI816X_PCIE_MODFILE);
	if (!ti816x_pci_class) {
		cdev_del(&ti816x_pci_cdev);
		unregister_chrdev_region(ti816x_dev_id, TI816X_DEV_COUNT);
		dev_err(&ti816x_pci_dev->dev,
				"Failed to add device to sys fs\n");
		ret = -1;
		goto err_post_pci;
	}

	device_create(ti816x_pci_class, NULL, ti816x_dev_id,
				NULL, TI816X_PCIE_MODFILE);
	dev_info(&ti816x_pci_dev->dev, "Added device to the sys file system\n");

	ret = pci_enable_device(ti816x_pci_dev);
	if (ret) {
		dev_err(&ti816x_pci_dev->dev, "Failed to enable device.\n");
		goto err_post_cdev;
	}

	ti816x_pci_set_master() ;

	ret = ti816x_pci_get_resources();
	if (ret == -1) {
		dev_err(&ti816x_pci_dev->dev, "could not get resources\n");
		goto err_post_cdev;
	}

	/* Set up default inbound access windows */
	ti816x_ep_setup_bar(1, TI816X_EP_UBOOT_IB_OFFSET);
	ti816x_ep_setup_bar(2, TI816X_EP_KERNEL_IB_OFFSET);

	/* Enable inbound translation */
	val = __raw_readl(PCI_REGV(CMD_STATUS));
	val |= IB_XLAT_EN_VAL;
	__raw_writel(val, PCI_REGV(CMD_STATUS));

	return 0 ;

err_post_cdev:
	device_destroy(ti816x_pci_class, ti816x_dev_id);
	class_destroy(ti816x_pci_class);
	cdev_del(&ti816x_pci_cdev);
	unregister_chrdev_region(ti816x_dev_id, TI816X_DEV_COUNT);

err_post_pci:
	pci_dev_put(ti816x_pci_dev);

	return ret;
}
module_init(ti816x_ep_pci_init);

/**
 * ti816x_ep_pcie_cleanup() - Perform cleanups before module unload
 */
static void __exit ti816x_ep_pcie_cleanup(void)
{
	device_destroy(ti816x_pci_class, ti816x_dev_id);
	class_destroy(ti816x_pci_class);
	cdev_del(&ti816x_pci_cdev);
	unregister_chrdev_region(ti816x_dev_id, TI816X_DEV_COUNT);

	iounmap((void *)ocmc1_virt);
	release_mem_region(ocmc1_phys, ocmc1_len);

	iounmap((void *)ddr_virt);
	release_mem_region(ddr_phys, ddr_len);

	iounmap((void *)reg_virt);
	release_mem_region(reg_phys, reg_len);

	pci_disable_device(ti816x_pci_dev);

	pci_dev_put(ti816x_pci_dev);
}
module_exit(ti816x_ep_pcie_cleanup);
MODULE_LICENSE("GPL");
