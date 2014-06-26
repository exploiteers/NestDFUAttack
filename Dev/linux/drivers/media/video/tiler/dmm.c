/*
 * dmm.c
 *
 * DMM driver support functions for TI OMAP processors.
 *
 * Authors: David Sin <davidsin@ti.com>
 *          Lajos Molnar <molnar@ti.com>
 *
 * Copyright (C) 2009-2010 Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h> /* platform_device() */
#include <linux/io.h>              /* ioremap() */
#include <linux/errno.h>
#include <linux/slab.h>

#include <mach/dmm.h>

#undef __DEBUG__

#define MASK(msb, lsb) (((1 << ((msb) + 1 - (lsb))) - 1) << (lsb))
#define SET_FLD(reg, msb, lsb, val) \
(((reg) & ~MASK((msb), (lsb))) | (((val) << (lsb)) & MASK((msb), (lsb))))

#ifdef __DEBUG__
#define DEBUG(x, y) printk(KERN_NOTICE "%s()::%d:%s=(0x%08x)\n", \
				__func__, __LINE__, x, (s32)y);
#else
#define DEBUG(x, y)
#endif

static struct platform_driver dmm_driver_ldm = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "dmm",
	},
	.probe = NULL,
	.shutdown = NULL,
	.remove = NULL,
};

s32 dmm_pat_refill(struct dmm *dmm, struct pat *pd, enum pat_mode mode)
{
	void __iomem *r;
	u32 v;

	/* Only manual refill supported */
	if (mode != MANUAL)
		return -EFAULT;

	/* Check that the DMM_PAT_STATUS register has not reported an error */
	r = dmm->base + DMM_PAT_STATUS__0;
	v = __raw_readl(r);
	if ((v & 0xFC00) != 0) {
		while (1)
			printk(KERN_ERR "dmm_pat_refill() error.\n");
	}

	/* Set "next" register to NULL */
	r = dmm->base + DMM_PAT_DESCR__0;
	v = __raw_readl(r);
	v = SET_FLD(v, 31, 4, (u32) NULL);
	__raw_writel(v, r);

	/* Set area to be refilled */
	r = dmm->base + DMM_PAT_AREA__0;
	v = __raw_readl(r);
	v = SET_FLD(v, 30, 24, pd->area.y1);
	v = SET_FLD(v, 23, 16, pd->area.x1);
	v = SET_FLD(v, 14, 8, pd->area.y0);
	v = SET_FLD(v, 7, 0, pd->area.x0);
	__raw_writel(v, r);
	wmb();

#ifdef __DEBUG__
	printk(KERN_NOTICE "\nx0=(%d),y0=(%d),x1=(%d),y1=(%d)\n",
						(char)pd->area.x0,
						(char)pd->area.y0,
						(char)pd->area.x1,
						(char)pd->area.y1);
#endif

	/* First, clear the DMM_PAT_IRQSTATUS register */
	r = dmm->base + DMM_PAT_IRQSTATUS;
	__raw_writel(0xFFFFFFFF, r);
	wmb();

	r = dmm->base + DMM_PAT_IRQSTATUS_RAW;
	do {
		v = __raw_readl(r);
		DEBUG("DMM_PAT_IRQSTATUS_RAW", v);
	} while (v != 0x0);

	/* Fill data register */
	r = dmm->base + DMM_PAT_DATA__0;
	v = __raw_readl(r);

	/* pd->data must be 16 aligned */
	BUG_ON(pd->data & 15);
	v = SET_FLD(v, 31, 4, pd->data >> 4);
	__raw_writel(v, r);
	wmb();

	/* Read back PAT_DATA__0 to see if write was successful */
	do {
		v = __raw_readl(r);
		DEBUG("DMM_PAT_DATA__0", v);
	} while (v != pd->data);

	r = dmm->base + DMM_PAT_CTRL__0;
	v = __raw_readl(r);
	v = SET_FLD(v, 31, 28, pd->ctrl.ini);
	v = SET_FLD(v, 16, 16, pd->ctrl.sync);
	v = SET_FLD(v, 9, 8, pd->ctrl.lut_id);
	v = SET_FLD(v, 6, 4, pd->ctrl.dir);
	v = SET_FLD(v, 0, 0, pd->ctrl.start);
	__raw_writel(v, r);
	wmb();

	/* Check if PAT_IRQSTATUS_RAW is set after the PAT has been refilled */
	r = dmm->base + DMM_PAT_IRQSTATUS_RAW;
	do {
		v = __raw_readl(r);
		DEBUG("DMM_PAT_IRQSTATUS_RAW", v);
	} while ((v & 0x3) != 0x3);

	/* Again, clear the DMM_PAT_IRQSTATUS register */
	r = dmm->base + DMM_PAT_IRQSTATUS;
	__raw_writel(0xFFFFFFFF, r);
	wmb();

	r = dmm->base + DMM_PAT_IRQSTATUS_RAW;
	do {
		v = __raw_readl(r);
		DEBUG("DMM_PAT_IRQSTATUS_RAW", v);
	} while (v != 0x0);

	/* Again, set "next" register to NULL to clear any PAT STATUS errors */
	r = dmm->base + DMM_PAT_DESCR__0;
	v = __raw_readl(r);
	v = SET_FLD(v, 31, 4, (u32) NULL);
	__raw_writel(v, r);

	/*
	 * Now, check that the DMM_PAT_STATUS register
	 * has not reported an error before exiting.
	*/
	r = dmm->base + DMM_PAT_STATUS__0;
	v = __raw_readl(r);
	if ((v & 0xFC00) != 0) {
		while (1)
			printk(KERN_ERR "dmm_pat_refill() error.\n");
	}

	return 0;
}
EXPORT_SYMBOL(dmm_pat_refill);

struct dmm *dmm_pat_init(u32 id)
{
	u32 base;
	struct dmm *dmm;
	switch (id) {
	case 0:
		/* only support id 0 for now */
		base = DMM_BASE;
		break;
	default:
		return NULL;
	}

	dmm = kmalloc(sizeof(*dmm), GFP_KERNEL);
	if (!dmm)
		return NULL;

	dmm->base = ioremap(base, DMM_SIZE);
	if (!dmm->base) {
		kfree(dmm);
		return NULL;
	}

	__raw_writel(0x88888888, dmm->base + DMM_PAT_VIEW__0);
	__raw_writel(0x88888888, dmm->base + DMM_PAT_VIEW__1);
	__raw_writel(0x80808080, dmm->base + DMM_PAT_VIEW_MAP__0);
	__raw_writel(0x80000000, dmm->base + DMM_PAT_VIEW_MAP_BASE);
	__raw_writel(0x88888888, dmm->base + DMM_TILER_OR__0);
	__raw_writel(0x88888888, dmm->base + DMM_TILER_OR__1);

	return dmm;
}
EXPORT_SYMBOL(dmm_pat_init);

/**
 * Clean up the physical address translator.
 * @param dmm    Device data
 * @return an error status.
 */
void dmm_pat_release(struct dmm *dmm)
{
	if (dmm) {
		iounmap(dmm->base);
		kfree(dmm);
	}
}
EXPORT_SYMBOL(dmm_pat_release);

static s32 __init dmm_init(void)
{
	return platform_driver_register(&dmm_driver_ldm);
}

static void __exit dmm_exit(void)
{
	platform_driver_unregister(&dmm_driver_ldm);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("davidsin@ti.com");
MODULE_AUTHOR("molnar@ti.com");
module_init(dmm_init);
module_exit(dmm_exit);
