/*
 *
 * Framebuffer device registration for TI TI816x platforms
 *
 * Copyright (C) 2009 Texas Instruments Inc.
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * Some code and ideas taken from TI OMAP2 Platforms
 * by Tomi Valkeinen.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/io.h>
#include <linux/ti81xxfb.h>

#include <mach/hardware.h>
#include <asm/mach/map.h>


#if (defined(CONFIG_FB_TI81XX) || defined(CONFIG_FB_TI81XX_MODULE) || \
	defined(CONFIG_ARCH_TI81XX))

static u64 ti81xx_fb_dma_mask = ~(u32)0;
static struct ti81xxfb_platform_data ti81xxfb_config;

static struct platform_device ti81xx_fb_device = {
	.name		= "ti81xxfb",
	.id		= -1,
	.dev = {
		.dma_mask		= &ti81xx_fb_dma_mask,
		.coherent_dma_mask	= ~(u32)0,
		.platform_data		= &ti81xxfb_config,
	},
	.num_resources = 0,
};

void ti81xxfb_set_platform_data(struct ti81xxfb_platform_data *data)
{
	ti81xxfb_config = *data;
}

static inline int ti81xx_init_fb(void)
{
	printk(KERN_INFO "Registered ti81xx_fb device\n");
	return platform_device_register(&ti81xx_fb_device);
}

arch_initcall(ti81xx_init_fb);

#endif
