/*
 * linux/arch/arm/mach-omap2/usb-musb.c
 *
 * This file will contain the board specific details for the
 * MENTOR USB OTG controller on OMAP3430
 *
 * Copyright (C) 2007-2008 Texas Instruments
 * Copyright (C) 2008 Nokia Corporation
 * Author: Vikram Pandita
 *
 * Generalization by:
 * Felipe Balbi <felipe.balbi@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>

#include <linux/usb/musb.h>

#include <asm/sizes.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/am35xx.h>
#include <plat/usb.h>
#include "control.h"

#define OTG_SYSCONFIG	   0x404
#define OTG_SYSC_SOFTRESET BIT(1)
#define OTG_SYSSTATUS     0x408
#define OTG_SYSS_RESETDONE BIT(0)

static struct platform_device dummy_pdev = {
	.dev = {
		.bus = &platform_bus_type,
	},
};

static void __iomem *otg_base;
static struct clk *otg_clk;

static void __init usb_musb_pm_init(void)
{
	struct device *dev = &dummy_pdev.dev;

	if (!cpu_is_omap34xx() || cpu_is_omap3517() || cpu_is_omap3505())
		return;

	otg_base = ioremap(OMAP34XX_HSUSB_OTG_BASE, SZ_4K);
	if (WARN_ON(!otg_base))
		return;

	dev_set_name(dev, "musb-omap2430");
	otg_clk = clk_get(dev, "ick");

	if (otg_clk && clk_enable(otg_clk)) {
		printk(KERN_WARNING
			"%s: Unable to enable clocks for MUSB, "
			"cannot reset.\n",  __func__);
	} else {
		/* Reset OTG controller. After reset, it will be in
		 * force-idle, force-standby mode. */
		__raw_writel(OTG_SYSC_SOFTRESET, otg_base + OTG_SYSCONFIG);

		while (!(OTG_SYSS_RESETDONE &
					__raw_readl(otg_base + OTG_SYSSTATUS)))
			cpu_relax();
	}

	if (otg_clk)
		clk_disable(otg_clk);
}

void usb_musb_disable_autoidle(void)
{
	if (otg_clk) {
		unsigned long reg;

		clk_enable(otg_clk);
		reg = __raw_readl(otg_base + OTG_SYSCONFIG);
		__raw_writel(reg & ~1, otg_base + OTG_SYSCONFIG);
		clk_disable(otg_clk);
	}
}

#ifdef CONFIG_PM
void (*omap_musb_save)(void);
EXPORT_SYMBOL_GPL(omap_musb_save);
void (*omap_musb_restore)(void);
EXPORT_SYMBOL_GPL(omap_musb_restore);

void omap_musb_save_context(void)
{
	if (omap_musb_save)
		omap_musb_save();
}

void omap_musb_restore_context(void)
{
	if (omap_musb_restore)
		omap_musb_restore();
}
#endif

#if defined(CONFIG_USB_MUSB_OMAP2PLUS) || defined(CONFIG_USB_MUSB_AM35X) \
	|| defined(CONFIG_USB_MUSB_TI81XX)

static void ti81xx_musb_phy_power(u8 id, u8 on)
{
	u32 usbphycfg, ctrl_offs;

	ctrl_offs = id ? TI81XX_USBCTRL1 : TI81XX_USBCTRL0;
	usbphycfg = omap_ctrl_readl(ctrl_offs);

	if (on) {
		if (cpu_is_ti816x()) {
			usbphycfg |= (TI816X_USBPHY0_NORMAL_MODE
					| TI816X_USBPHY1_NORMAL_MODE);
			usbphycfg &= ~(TI816X_USBPHY_REFCLK_OSC);
		} else if (cpu_is_ti814x()) {
			usbphycfg &= ~(TI814X_USBPHY_CM_PWRDN
				| TI814X_USBPHY_OTG_PWRDN
				| TI814X_USBPHY_DMPULLUP
				| TI814X_USBPHY_DPPULLUP
				| TI814X_USBPHY_DPINPUT
				| TI814X_USBPHY_DMINPUT
				| TI814X_USBPHY_DATA_POLARITY);
			usbphycfg |= (TI814X_USBPHY_SRCONDM
				| TI814X_USBPHY_SINKONDP
				| TI814X_USBPHY_CHGISINK_EN
				| TI814X_USBPHY_CHGVSRC_EN
				| TI814X_USBPHY_CDET_EXTCTL
				| TI814X_USBPHY_DPOPBUFCTL
				| TI814X_USBPHY_DMOPBUFCTL
				| TI814X_USBPHY_DPGPIO_PD
				| TI814X_USBPHY_DMGPIO_PD
				| TI814X_USBPHY_OTGVDET_EN
				| TI814X_USBPHY_OTGSESSEND_EN);
		}

		omap_ctrl_writel(usbphycfg, ctrl_offs);
	} else {
		if (cpu_is_ti816x())
			usbphycfg &= ~(TI816X_USBPHY0_NORMAL_MODE
				| TI816X_USBPHY1_NORMAL_MODE
				| TI816X_USBPHY_REFCLK_OSC);
		else if (cpu_is_ti814x())
			usbphycfg |= TI814X_USBPHY_CM_PWRDN
				| TI814X_USBPHY_OTG_PWRDN;

		omap_ctrl_writel(usbphycfg, ctrl_offs);
	}
}

static void am35x_musb_reset(void)
{
	u32	regval;

	/* Reset the musb interface */
	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);

	regval |= AM35XX_USBOTGSS_SW_RST;
	omap_ctrl_writel(regval, AM35XX_CONTROL_IP_SW_RESET);

	regval &= ~AM35XX_USBOTGSS_SW_RST;
	omap_ctrl_writel(regval, AM35XX_CONTROL_IP_SW_RESET);

	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);
}

static void am35x_musb_phy_power(u8 id, u8 on)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(100);
	u32 devconf2;

	if (on) {
		/*
		 * Start the on-chip PHY and its PLL.
		 */
		devconf2 = omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2);

		devconf2 &= ~(CONF2_RESET | CONF2_PHYPWRDN | CONF2_OTGPWRDN);
		devconf2 |= CONF2_PHY_PLLON;

		omap_ctrl_writel(devconf2, AM35XX_CONTROL_DEVCONF2);

		pr_info(KERN_INFO "Waiting for PHY clock good...\n");
		while (!(omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2)
				& CONF2_PHYCLKGD)) {
			cpu_relax();

			if (time_after(jiffies, timeout)) {
				pr_err(KERN_ERR "musb PHY clock good timed out\n");
				break;
			}
		}
	} else {
		/*
		 * Power down the on-chip PHY.
		 */
		devconf2 = omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2);

		devconf2 &= ~CONF2_PHY_PLLON;
		devconf2 |=  CONF2_PHYPWRDN | CONF2_OTGPWRDN;
		omap_ctrl_writel(devconf2, AM35XX_CONTROL_DEVCONF2);
	}
}

static void am35x_musb_clear_irq(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval |= AM35XX_USBOTGSS_INT_CLR;
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void am35x_musb_set_mode(u8 musb_mode)
{
	u32 devconf2 = omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2);

	devconf2 &= ~CONF2_OTGMODE;
	switch (musb_mode) {
#ifdef	CONFIG_USB_MUSB_HDRC_HCD
	case MUSB_HOST:		/* Force VBUS valid, ID = 0 */
		devconf2 |= CONF2_FORCE_HOST;
		break;
#endif
#ifdef	CONFIG_USB_GADGET_MUSB_HDRC
	case MUSB_PERIPHERAL:	/* Force VBUS valid, ID = 1 */
		devconf2 |= CONF2_FORCE_DEVICE;
		break;
#endif
#ifdef	CONFIG_USB_MUSB_OTG
	case MUSB_OTG:		/* Don't override the VBUS/ID comparators */
		devconf2 |= CONF2_NO_OVERRIDE;
		break;
#endif
	default:
		pr_info(KERN_INFO "Unsupported mode %u\n", musb_mode);
	}

	omap_ctrl_writel(devconf2, AM35XX_CONTROL_DEVCONF2);
}

static struct resource musb_resources[] = {
	[0] = { /* start and end set dynamically */
		.flags	= IORESOURCE_MEM,
	},
	[1] = {	/* general IRQ */
		.start	= INT_243X_HS_USB_MC,
		.flags	= IORESOURCE_IRQ,
		.name	= "mc",
	},
	[2] = {	/* DMA IRQ */
		.start	= INT_243X_HS_USB_DMA,
		.flags	= IORESOURCE_IRQ,
		.name   = "dma",
	},
	[3] = { /* MEM for TI81x's second musb */
		.flags  = IORESOURCE_MEM,
		.start	= TI81XX_USB1_BASE,
		.end	= TI81XX_USB1_BASE + SZ_2K - 1,
	},
	[4] = {	/* IRQ for TI81x's second musb */
		.start	= TI81XX_IRQ_USB1,
		.flags	= IORESOURCE_IRQ,
		.name	= "mc",
	},
};

static struct musb_hdrc_config musb_config = {
	.fifo_mode	= 4,
	.multipoint	= 1,
	.dyn_fifo	= 1,
	.num_eps	= 16,
	.ram_bits	= 12,
};

static struct musb_hdrc_platform_data musb_plat[] = {
	{
		.config         = &musb_config,
		.clock          = "ick",
	},
	{
		.config         = &musb_config,
		.clock          = "ick",
	},
};

static u64 musb_dmamask = DMA_BIT_MASK(32);

static struct platform_device musb_device[] = {
	{
		.name		= "musb-omap2430",
		.id		= 0,
		.dev = {
			.dma_mask		= &musb_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &musb_plat[0],
		},
		.num_resources	= 3,
		.resource	= &musb_resources[0],
	},
	{
		.name		= "musb-omap2430",
		.id		= 1,
		.dev = {
			.dma_mask		= &musb_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &musb_plat[1],
		},
		.num_resources	= 3,
		.resource	= &musb_resources[3],
	},

};

void __init usb_musb_init(struct omap_musb_board_data *board_data)
{
	int i;

	if (cpu_is_omap243x()) {
		musb_resources[0].start = OMAP243X_HS_BASE;
	} else if (cpu_is_omap3517() || cpu_is_omap3505()) {
		musb_device[0].name = "musb-am35x";
		musb_resources[0].start = AM35XX_IPSS_USBOTGSS_BASE;
		musb_resources[1].start = INT_35XX_USBOTG_IRQ;
		board_data->set_phy_power = am35x_musb_phy_power;
		board_data->clear_irq = am35x_musb_clear_irq;
		board_data->set_mode = am35x_musb_set_mode;
		board_data->reset = am35x_musb_reset;
	} else if (cpu_is_omap34xx()) {
		musb_resources[0].start = OMAP34XX_HSUSB_OTG_BASE;
	} else if (cpu_is_omap44xx()) {
		musb_resources[0].start = OMAP44XX_HSUSB_OTG_BASE;
		musb_resources[1].start = OMAP44XX_IRQ_HS_USB_MC_N;
		musb_resources[2].start = OMAP44XX_IRQ_HS_USB_DMA_N;
	} else if (cpu_is_ti81xx()) {

		/* disable musb multipoint support for ti8168 */
		if (cpu_is_ti816x())
			musb_config.multipoint = 0;

		/* only usb0 port enabled in peripheral mode*/
		if (board_data->mode == MUSB_PERIPHERAL)
			board_data->instances = 0;

		musb_resources[0].start = TI81XX_USB0_BASE;
		musb_resources[1].start = TI81XX_IRQ_USB0;
		musb_resources[0].end = musb_resources[0].start + SZ_2K - 1;

		for (i = 0; i <= board_data->instances; i++) {
			musb_device[i].name = "musb-ti81xx";
			musb_device[i].num_resources = 0;
		}

		musb_config.fifo_mode = 4;
		board_data->set_phy_power = ti81xx_musb_phy_power;
	}

	if (cpu_is_omap3517() || cpu_is_omap3505())
		musb_resources[0].end = musb_resources[0].start + SZ_32K - 1;
	else if (!cpu_is_ti81xx())
		musb_resources[0].end = musb_resources[0].start + SZ_4K - 1;

	/*
	 * OMAP3630/AM35x platform has MUSB RTL-1.8 which has the fix for the
	 * issue restricting active endpoints to use first 8K of FIFO space.
	 * This issue restricts OMAP35x platform to use fifo_mode '5'.
	 */
	if (cpu_is_omap3430())
		musb_config.fifo_mode = 5;

	for (i = 0; i <= board_data->instances; i++) {
		if (cpu_is_ti816x())
			musb_plat[i].clock = "usbotg_ick";
		else if (cpu_is_ti814x())
			musb_plat[i].clock = "usb_ick";

		musb_plat[i].board_data = board_data;
		musb_plat[i].power = board_data->power >> 1;
		musb_plat[i].mode = board_data->mode;
		musb_plat[i].extvbus = board_data->extvbus;

		if (platform_device_register(&musb_device[i]) < 0)
			printk(KERN_ERR "Unable to register HS-USB (MUSB) device\n");
	}

	usb_musb_pm_init();
}

#else
void __init usb_musb_init(struct omap_musb_board_data *board_data)
{
	usb_musb_pm_init();
}
#endif /* CONFIG_USB_MUSB_SOC */
