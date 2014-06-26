/*
 * linux/arch/arm/mach-omap2/devices.c
 *
 * OMAP2 platform device setup/initialization
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/davinci_emac.h>
#include <linux/cpsw.h>
#include <linux/ahci_platform.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <asm/pmu.h>
#include <asm/hardware/edma.h>

#include <plat/tc.h>
#include <plat/board.h>
#include <plat/mcbsp.h>
#include <mach/gpio.h>
#include <plat/mmc.h>
#include <plat/dma.h>
#include <plat/omap_hwmod.h>
#include <plat/omap_device.h>
#include <plat/asp.h>

#include "mux.h"
#include "control.h"
#include "pcie-ti816x.h"

#include "devices.h"

#if defined(CONFIG_VIDEO_OMAP2) || defined(CONFIG_VIDEO_OMAP2_MODULE)

static struct resource omap2cam_resources[] = {
	{
		.start		= OMAP24XX_CAMERA_BASE,
		.end		= OMAP24XX_CAMERA_BASE + 0xfff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_CAM_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};

static struct platform_device omap2cam_device = {
	.name		= "omap24xxcam",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(omap2cam_resources),
	.resource	= omap2cam_resources,
};
#endif

static struct resource omap3isp_resources[] = {
	{
		.start		= OMAP3430_ISP_BASE,
		.end		= OMAP3430_ISP_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CCP2_BASE,
		.end		= OMAP3430_ISP_CCP2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CCDC_BASE,
		.end		= OMAP3430_ISP_CCDC_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_HIST_BASE,
		.end		= OMAP3430_ISP_HIST_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_H3A_BASE,
		.end		= OMAP3430_ISP_H3A_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_PREV_BASE,
		.end		= OMAP3430_ISP_PREV_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_RESZ_BASE,
		.end		= OMAP3430_ISP_RESZ_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_SBL_BASE,
		.end		= OMAP3430_ISP_SBL_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CSI2A_REGS1_BASE,
		.end		= OMAP3430_ISP_CSI2A_REGS1_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CSIPHY2_BASE,
		.end		= OMAP3430_ISP_CSIPHY2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSI2A_REGS2_BASE,
		.end		= OMAP3630_ISP_CSI2A_REGS2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSI2C_REGS1_BASE,
		.end		= OMAP3630_ISP_CSI2C_REGS1_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSIPHY1_BASE,
		.end		= OMAP3630_ISP_CSIPHY1_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSI2C_REGS2_BASE,
		.end		= OMAP3630_ISP_CSI2C_REGS2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_34XX_CAM_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};

static struct platform_device omap3isp_device = {
	.name		= "omap3isp",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(omap3isp_resources),
	.resource	= omap3isp_resources,
};

int omap3_init_camera(void *pdata)
{
	omap3isp_device.dev.platform_data = pdata;
	return platform_device_register(&omap3isp_device);
}
EXPORT_SYMBOL_GPL(omap3_init_camera);

static inline void omap_init_camera(void)
{
#if defined(CONFIG_VIDEO_OMAP2) || defined(CONFIG_VIDEO_OMAP2_MODULE)
	if (cpu_is_omap24xx())
		platform_device_register(&omap2cam_device);
#endif
}

#if defined(CONFIG_OMAP_MBOX_FWK) || defined(CONFIG_OMAP_MBOX_FWK_MODULE)

#define MBOX_REG_SIZE   0x120

#ifdef CONFIG_ARCH_OMAP2
static struct resource omap2_mbox_resources[] = {
	{
		.start		= OMAP24XX_MAILBOX_BASE,
		.end		= OMAP24XX_MAILBOX_BASE + MBOX_REG_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_MAIL_U0_MPU,
		.flags		= IORESOURCE_IRQ,
		.name		= "dsp",
	},
	{
		.start		= INT_24XX_MAIL_U3_MPU,
		.flags		= IORESOURCE_IRQ,
		.name		= "iva",
	},
};
static int omap2_mbox_resources_sz = ARRAY_SIZE(omap2_mbox_resources);
#else
#define omap2_mbox_resources		NULL
#define omap2_mbox_resources_sz		0
#endif

#ifdef CONFIG_ARCH_OMAP3
static struct resource omap3_mbox_resources[] = {
	{
		.start		= OMAP34XX_MAILBOX_BASE,
		.end		= OMAP34XX_MAILBOX_BASE + MBOX_REG_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_MAIL_U0_MPU,
		.flags		= IORESOURCE_IRQ,
		.name		= "dsp",
	},
};
static int omap3_mbox_resources_sz = ARRAY_SIZE(omap3_mbox_resources);
#else
#define omap3_mbox_resources		NULL
#define omap3_mbox_resources_sz		0
#endif

#ifdef CONFIG_ARCH_OMAP4

#define OMAP4_MBOX_REG_SIZE	0x130
static struct resource omap4_mbox_resources[] = {
	{
		.start          = OMAP44XX_MAILBOX_BASE,
		.end            = OMAP44XX_MAILBOX_BASE +
					OMAP4_MBOX_REG_SIZE - 1,
		.flags          = IORESOURCE_MEM,
	},
	{
		.start          = OMAP44XX_IRQ_MAIL_U0,
		.flags          = IORESOURCE_IRQ,
		.name		= "mbox",
	},
};
static int omap4_mbox_resources_sz = ARRAY_SIZE(omap4_mbox_resources);
#else
#define omap4_mbox_resources		NULL
#define omap4_mbox_resources_sz		0
#endif

static struct platform_device mbox_device = {
	.name		= "omap-mailbox",
	.id		= -1,
};

static inline void omap_init_mbox(void)
{
	if (cpu_is_omap24xx()) {
		mbox_device.resource = omap2_mbox_resources;
		mbox_device.num_resources = omap2_mbox_resources_sz;
	} else if (cpu_is_omap34xx()) {
		mbox_device.resource = omap3_mbox_resources;
		mbox_device.num_resources = omap3_mbox_resources_sz;
	} else if (cpu_is_omap44xx()) {
		mbox_device.resource = omap4_mbox_resources;
		mbox_device.num_resources = omap4_mbox_resources_sz;
	} else {
		pr_err("%s: platform not supported\n", __func__);
		return;
	}
	platform_device_register(&mbox_device);
}
#else
static inline void omap_init_mbox(void) { }
#endif /* CONFIG_OMAP_MBOX_FWK */

static inline void omap_init_sti(void) {}

#if defined(CONFIG_SND_SOC) || defined(CONFIG_SND_SOC_MODULE)

#if defined(CONFIG_ARCH_TI81XX)
struct platform_device ti81xx_pcm_device = {
	.name		= "davinci-pcm-audio",
	.id		= -1,
};

static void ti81xx_init_pcm(void)
{
	platform_device_register(&ti81xx_pcm_device);
}
#else
static struct platform_device omap_pcm = {
	.name	= "omap-pcm-audio",
	.id	= -1,
};

/*
 * OMAP2420 has 2 McBSP ports
 * OMAP2430 has 5 McBSP ports
 * OMAP3 has 5 McBSP ports
 * OMAP4 has 4 McBSP ports
 */
OMAP_MCBSP_PLATFORM_DEVICE(1);
OMAP_MCBSP_PLATFORM_DEVICE(2);
OMAP_MCBSP_PLATFORM_DEVICE(3);
OMAP_MCBSP_PLATFORM_DEVICE(4);
OMAP_MCBSP_PLATFORM_DEVICE(5);

static void omap_init_audio(void)
{
	platform_device_register(&omap_mcbsp1);
	platform_device_register(&omap_mcbsp2);
	if (cpu_is_omap243x() || cpu_is_omap34xx() || cpu_is_omap44xx()) {
		platform_device_register(&omap_mcbsp3);
		platform_device_register(&omap_mcbsp4);
	}
	if (cpu_is_omap243x() || cpu_is_omap34xx())
		platform_device_register(&omap_mcbsp5);

	platform_device_register(&omap_pcm);
}

#endif /* defined(CONFIG_ARCH_TI81XX) */

#else

#if defined(CONFIG_ARCH_TI81XX)
static inline void ti81xx_init_pcm(void) {}
#else
static inline void omap_init_audio(void) {}
#endif

#endif /* defined(CONFIG_SND_SOC) || defined(CONFIG_SND_SOC_MODULE) */

#if defined(CONFIG_SPI_OMAP24XX) || defined(CONFIG_SPI_OMAP24XX_MODULE)

#include <plat/mcspi.h>

#define OMAP2_MCSPI1_BASE		0x48098000
#define OMAP2_MCSPI2_BASE		0x4809a000
#define OMAP2_MCSPI3_BASE		0x480b8000
#define OMAP2_MCSPI4_BASE		0x480ba000

#define OMAP4_MCSPI1_BASE		0x48098100
#define OMAP4_MCSPI2_BASE		0x4809a100
#define OMAP4_MCSPI3_BASE		0x480b8100
#define OMAP4_MCSPI4_BASE		0x480ba100

#define TI81XX_MCSPI1_BASE		0x48030100
#define TI814X_MCSPI2_BASE		0x481A0100
#define TI814X_MCSPI3_BASE		0x481A2100
#define TI814X_MCSPI4_BASE		0x481A4100

static struct omap2_mcspi_platform_config omap2_mcspi1_config = {
	.num_cs		= 4,
};

static struct resource omap2_mcspi1_resources[] = {
	{
		.start		= OMAP2_MCSPI1_BASE,
		.end		= OMAP2_MCSPI1_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device omap2_mcspi1 = {
	.name		= "omap2_mcspi",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(omap2_mcspi1_resources),
	.resource	= omap2_mcspi1_resources,
	.dev		= {
		.platform_data = &omap2_mcspi1_config,
	},
};

static struct omap2_mcspi_platform_config omap2_mcspi2_config = {
	.num_cs		= 2,
};

static struct resource omap2_mcspi2_resources[] = {
	{
		.start		= OMAP2_MCSPI2_BASE,
		.end		= OMAP2_MCSPI2_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device omap2_mcspi2 = {
	.name		= "omap2_mcspi",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(omap2_mcspi2_resources),
	.resource	= omap2_mcspi2_resources,
	.dev		= {
		.platform_data = &omap2_mcspi2_config,
	},
};

#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3) || \
	defined(CONFIG_ARCH_OMAP4) || defined(CONFIG_ARCH_TI814X)
static struct omap2_mcspi_platform_config omap2_mcspi3_config = {
	.num_cs		= 2,
};

static struct resource omap2_mcspi3_resources[] = {
	{
	.start		= OMAP2_MCSPI3_BASE,
	.end		= OMAP2_MCSPI3_BASE + 0xff,
	.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device omap2_mcspi3 = {
	.name		= "omap2_mcspi",
	.id		= 3,
	.num_resources	= ARRAY_SIZE(omap2_mcspi3_resources),
	.resource	= omap2_mcspi3_resources,
	.dev		= {
		.platform_data = &omap2_mcspi3_config,
	},
};
#endif

#if defined(CONFIG_ARCH_OMAP3) || defined(CONFIG_ARCH_OMAP4) || \
	defined(CONFIG_ARCH_TI814X)
static struct omap2_mcspi_platform_config omap2_mcspi4_config = {
	.num_cs		= 1,
};

static struct resource omap2_mcspi4_resources[] = {
	{
		.start		= OMAP2_MCSPI4_BASE,
		.end		= OMAP2_MCSPI4_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device omap2_mcspi4 = {
	.name		= "omap2_mcspi",
	.id		= 4,
	.num_resources	= ARRAY_SIZE(omap2_mcspi4_resources),
	.resource	= omap2_mcspi4_resources,
	.dev		= {
		.platform_data = &omap2_mcspi4_config,
	},
};
#endif

#ifdef CONFIG_ARCH_TI81XX
static inline void ti81xx_mcspi_fixup(void)
{
	omap2_mcspi1_resources[0].start	= TI81XX_MCSPI1_BASE;
	omap2_mcspi1_resources[0].end	= TI81XX_MCSPI1_BASE + 0xff;

	if (cpu_is_ti814x()) {
		omap2_mcspi2_resources[0].start	= TI814X_MCSPI2_BASE;
		omap2_mcspi2_resources[0].end	= TI814X_MCSPI2_BASE + 0xff;
#ifdef CONFIG_ARCH_TI814X
		omap2_mcspi3_resources[0].start	= TI814X_MCSPI3_BASE;
		omap2_mcspi3_resources[0].end	= TI814X_MCSPI3_BASE + 0xff;
		omap2_mcspi4_resources[0].start	= TI814X_MCSPI4_BASE;
		omap2_mcspi4_resources[0].end	= TI814X_MCSPI4_BASE + 0xff;
#endif
	}
}
#else
static inline void ti81xx_mcspi_fixup(void)
{
}
#endif



#ifdef CONFIG_ARCH_OMAP4
static inline void omap4_mcspi_fixup(void)
{
	omap2_mcspi1_resources[0].start	= OMAP4_MCSPI1_BASE;
	omap2_mcspi1_resources[0].end	= OMAP4_MCSPI1_BASE + 0xff;
	omap2_mcspi2_resources[0].start	= OMAP4_MCSPI2_BASE;
	omap2_mcspi2_resources[0].end	= OMAP4_MCSPI2_BASE + 0xff;
	omap2_mcspi3_resources[0].start	= OMAP4_MCSPI3_BASE;
	omap2_mcspi3_resources[0].end	= OMAP4_MCSPI3_BASE + 0xff;
	omap2_mcspi4_resources[0].start	= OMAP4_MCSPI4_BASE;
	omap2_mcspi4_resources[0].end	= OMAP4_MCSPI4_BASE + 0xff;
}
#else
static inline void omap4_mcspi_fixup(void)
{
}
#endif

static inline void omap2_mcspi2_init(void)
{
	platform_device_register(&omap2_mcspi2);
}

#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3) || \
	defined(CONFIG_ARCH_OMAP4) || defined(CONFIG_ARCH_TI814X)
static inline void omap2_mcspi3_init(void)
{
	platform_device_register(&omap2_mcspi3);
}
#else
static inline void omap2_mcspi3_init(void)
{
}
#endif

#if defined(CONFIG_ARCH_OMAP3) || defined(CONFIG_ARCH_OMAP4) || \
	defined(CONFIG_ARCH_TI814X)
static inline void omap2_mcspi4_init(void)
{
	platform_device_register(&omap2_mcspi4);
}
#else
static inline void omap2_mcspi4_init(void)
{
}
#endif

static void omap_init_mcspi(void)
{
	if (cpu_is_omap44xx())
		omap4_mcspi_fixup();

	if (cpu_is_ti81xx())
		ti81xx_mcspi_fixup();

	platform_device_register(&omap2_mcspi1);

	if (!cpu_is_ti816x())
		omap2_mcspi2_init();

	if (cpu_is_omap2430() || cpu_is_omap343x() || cpu_is_omap44xx() ||
		cpu_is_ti814x())
		omap2_mcspi3_init();

	if (cpu_is_omap343x() || cpu_is_omap44xx() || cpu_is_ti814x())
		omap2_mcspi4_init();
}

#else
static inline void omap_init_mcspi(void) {}
#endif

static struct resource omap2_pmu_resource = {
	.start	= 3,
	.end	= 3,
	.flags	= IORESOURCE_IRQ,
};

static struct resource omap3_pmu_resource = {
	.start	= INT_34XX_BENCH_MPU_EMUL,
	.end	= INT_34XX_BENCH_MPU_EMUL,
	.flags	= IORESOURCE_IRQ,
};

static struct platform_device omap_pmu_device = {
	.name		= "arm-pmu",
	.id		= ARM_PMU_DEVICE_CPU,
	.num_resources	= 1,
};

static void omap_init_pmu(void)
{
	if (cpu_is_omap24xx())
		omap_pmu_device.resource = &omap2_pmu_resource;
	else if (cpu_is_omap34xx())
		omap_pmu_device.resource = &omap3_pmu_resource;
	else
		return;

	platform_device_register(&omap_pmu_device);
}


#if defined(CONFIG_CRYPTO_DEV_OMAP_SHAM) || defined(CONFIG_CRYPTO_DEV_OMAP_SHAM_MODULE)

#ifdef CONFIG_ARCH_OMAP2
static struct resource omap2_sham_resources[] = {
	{
		.start	= OMAP24XX_SEC_SHA1MD5_BASE,
		.end	= OMAP24XX_SEC_SHA1MD5_BASE + 0x64,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_24XX_SHA1MD5,
		.flags	= IORESOURCE_IRQ,
	}
};
static int omap2_sham_resources_sz = ARRAY_SIZE(omap2_sham_resources);
#else
#define omap2_sham_resources		NULL
#define omap2_sham_resources_sz		0
#endif

#ifdef CONFIG_ARCH_OMAP3
static struct resource omap3_sham_resources[] = {
	{
		.start	= OMAP34XX_SEC_SHA1MD5_BASE,
		.end	= OMAP34XX_SEC_SHA1MD5_BASE + 0x64,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_34XX_SHA1MD52_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= OMAP34XX_DMA_SHA1MD5_RX,
		.flags	= IORESOURCE_DMA,
	}
};
static int omap3_sham_resources_sz = ARRAY_SIZE(omap3_sham_resources);
#else
#define omap3_sham_resources		NULL
#define omap3_sham_resources_sz		0
#endif

static struct platform_device sham_device = {
	.name		= "omap-sham",
	.id		= -1,
};

static void omap_init_sham(void)
{
	if (cpu_is_omap24xx()) {
		sham_device.resource = omap2_sham_resources;
		sham_device.num_resources = omap2_sham_resources_sz;
	} else if (cpu_is_omap34xx()) {
		sham_device.resource = omap3_sham_resources;
		sham_device.num_resources = omap3_sham_resources_sz;
	} else {
		pr_err("%s: platform not supported\n", __func__);
		return;
	}
	platform_device_register(&sham_device);
}
#else
static inline void omap_init_sham(void) { }
#endif

#if defined(CONFIG_CRYPTO_DEV_OMAP_AES) || defined(CONFIG_CRYPTO_DEV_OMAP_AES_MODULE)

#ifdef CONFIG_ARCH_OMAP2
static struct resource omap2_aes_resources[] = {
	{
		.start	= OMAP24XX_SEC_AES_BASE,
		.end	= OMAP24XX_SEC_AES_BASE + 0x4C,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= OMAP24XX_DMA_AES_TX,
		.flags	= IORESOURCE_DMA,
	},
	{
		.start	= OMAP24XX_DMA_AES_RX,
		.flags	= IORESOURCE_DMA,
	}
};
static int omap2_aes_resources_sz = ARRAY_SIZE(omap2_aes_resources);
#else
#define omap2_aes_resources		NULL
#define omap2_aes_resources_sz		0
#endif

#ifdef CONFIG_ARCH_OMAP3
static struct resource omap3_aes_resources[] = {
	{
		.start	= OMAP34XX_SEC_AES_BASE,
		.end	= OMAP34XX_SEC_AES_BASE + 0x4C,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= OMAP34XX_DMA_AES2_TX,
		.flags	= IORESOURCE_DMA,
	},
	{
		.start	= OMAP34XX_DMA_AES2_RX,
		.flags	= IORESOURCE_DMA,
	}
};
static int omap3_aes_resources_sz = ARRAY_SIZE(omap3_aes_resources);
#else
#define omap3_aes_resources		NULL
#define omap3_aes_resources_sz		0
#endif

static struct platform_device aes_device = {
	.name		= "omap-aes",
	.id		= -1,
};

static void omap_init_aes(void)
{
	if (cpu_is_omap24xx()) {
		aes_device.resource = omap2_aes_resources;
		aes_device.num_resources = omap2_aes_resources_sz;
	} else if (cpu_is_omap34xx()) {
		aes_device.resource = omap3_aes_resources;
		aes_device.num_resources = omap3_aes_resources_sz;
	} else {
		pr_err("%s: platform not supported\n", __func__);
		return;
	}
	platform_device_register(&aes_device);
}

#else
static inline void omap_init_aes(void) { }
#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_ARCH_OMAP3) || defined(CONFIG_ARCH_OMAP4)

#define MMCHS_SYSCONFIG			0x0010
#define MMCHS_SYSCONFIG_SWRESET		(1 << 1)
#define MMCHS_SYSSTATUS			0x0014
#define MMCHS_SYSSTATUS_RESETDONE	(1 << 0)

static struct platform_device dummy_pdev = {
	.dev = {
		.bus = &platform_bus_type,
	},
};

/**
 * omap_hsmmc_reset() - Full reset of each HS-MMC controller
 *
 * Ensure that each MMC controller is fully reset.  Controllers
 * left in an unknown state (by bootloader) may prevent retention
 * or OFF-mode.  This is especially important in cases where the
 * MMC driver is not enabled, _or_ built as a module.
 *
 * In order for reset to work, interface, functional and debounce
 * clocks must be enabled.  The debounce clock comes from func_32k_clk
 * and is not under SW control, so we only enable i- and f-clocks.
 **/
static void __init omap_hsmmc_reset(void)
{
	u32 i, nr_controllers;
	struct clk *iclk, *fclk;

	if (cpu_is_omap242x() || cpu_is_ti81xx())
		return;

	nr_controllers = cpu_is_omap44xx() ? OMAP44XX_NR_MMC :
		(cpu_is_omap34xx() ? OMAP34XX_NR_MMC : OMAP24XX_NR_MMC);

	for (i = 0; i < nr_controllers; i++) {
		u32 v, base = 0;
		struct device *dev = &dummy_pdev.dev;

		switch (i) {
		case 0:
			base = OMAP2_MMC1_BASE;
			break;
		case 1:
			base = OMAP2_MMC2_BASE;
			break;
		case 2:
			base = OMAP3_MMC3_BASE;
			break;
		case 3:
			if (!cpu_is_omap44xx())
				return;
			base = OMAP4_MMC4_BASE;
			break;
		case 4:
			if (!cpu_is_omap44xx())
				return;
			base = OMAP4_MMC5_BASE;
			break;
		}

		if (cpu_is_omap44xx())
			base += OMAP4_MMC_REG_OFFSET;

		dummy_pdev.id = i;
		dev_set_name(&dummy_pdev.dev, "mmci-omap-hs.%d", i);
		iclk = clk_get(dev, "ick");
		if (IS_ERR(iclk))
			goto err1;
		if (clk_enable(iclk))
			goto err2;

		fclk = clk_get(dev, "fck");
		if (IS_ERR(fclk))
			goto err3;
		if (clk_enable(fclk))
			goto err4;

		omap_writel(MMCHS_SYSCONFIG_SWRESET, base + MMCHS_SYSCONFIG);
		v = omap_readl(base + MMCHS_SYSSTATUS);
		while (!(omap_readl(base + MMCHS_SYSSTATUS) &
			 MMCHS_SYSSTATUS_RESETDONE))
			cpu_relax();

		clk_disable(fclk);
		clk_put(fclk);
		clk_disable(iclk);
		clk_put(iclk);
	}
	return;

err4:
	clk_put(fclk);
err3:
	clk_disable(iclk);
err2:
	clk_put(iclk);
err1:
	printk(KERN_WARNING "%s: Unable to enable clocks for MMC%d, "
			    "cannot reset.\n",  __func__, i);
}
#else
static inline void omap_hsmmc_reset(void) {}
#endif

#if defined(CONFIG_MMC_OMAP) || defined(CONFIG_MMC_OMAP_MODULE) || \
	defined(CONFIG_MMC_OMAP_HS) || defined(CONFIG_MMC_OMAP_HS_MODULE)

static inline void omap2_mmc_mux(struct omap_mmc_platform_data *mmc_controller,
			int controller_nr)
{
	if ((mmc_controller->slots[0].switch_pin > 0) && \
		(mmc_controller->slots[0].switch_pin < OMAP_MAX_GPIO_LINES))
		omap_mux_init_gpio(mmc_controller->slots[0].switch_pin,
					OMAP_PIN_INPUT_PULLUP);
	if ((mmc_controller->slots[0].gpio_wp > 0) && \
		(mmc_controller->slots[0].gpio_wp < OMAP_MAX_GPIO_LINES))
		omap_mux_init_gpio(mmc_controller->slots[0].gpio_wp,
					OMAP_PIN_INPUT_PULLUP);

	if (cpu_is_omap2420() && controller_nr == 0) {
		omap_mux_init_signal("sdmmc_cmd", 0);
		omap_mux_init_signal("sdmmc_clki", 0);
		omap_mux_init_signal("sdmmc_clko", 0);
		omap_mux_init_signal("sdmmc_dat0", 0);
		omap_mux_init_signal("sdmmc_dat_dir0", 0);
		omap_mux_init_signal("sdmmc_cmd_dir", 0);
		if (mmc_controller->slots[0].caps & MMC_CAP_4_BIT_DATA) {
			omap_mux_init_signal("sdmmc_dat1", 0);
			omap_mux_init_signal("sdmmc_dat2", 0);
			omap_mux_init_signal("sdmmc_dat3", 0);
			omap_mux_init_signal("sdmmc_dat_dir1", 0);
			omap_mux_init_signal("sdmmc_dat_dir2", 0);
			omap_mux_init_signal("sdmmc_dat_dir3", 0);
		}

		/*
		 * Use internal loop-back in MMC/SDIO Module Input Clock
		 * selection
		 */
		if (mmc_controller->slots[0].internal_clock) {
			u32 v = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
			v |= (1 << 24);
			omap_ctrl_writel(v, OMAP2_CONTROL_DEVCONF0);
		}
	}

	if (cpu_is_omap34xx()) {
		if (controller_nr == 0) {
			omap_mux_init_signal("sdmmc1_clk",
				OMAP_PIN_INPUT_PULLUP);
			omap_mux_init_signal("sdmmc1_cmd",
				OMAP_PIN_INPUT_PULLUP);
			omap_mux_init_signal("sdmmc1_dat0",
				OMAP_PIN_INPUT_PULLUP);
			if (mmc_controller->slots[0].caps &
				(MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA)) {
				omap_mux_init_signal("sdmmc1_dat1",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc1_dat2",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc1_dat3",
					OMAP_PIN_INPUT_PULLUP);
			}
			if (mmc_controller->slots[0].caps &
						MMC_CAP_8_BIT_DATA) {
				omap_mux_init_signal("sdmmc1_dat4",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc1_dat5",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc1_dat6",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc1_dat7",
					OMAP_PIN_INPUT_PULLUP);
			}
		}
		if (controller_nr == 1) {
			/* MMC2 */
			omap_mux_init_signal("sdmmc2_clk",
				OMAP_PIN_INPUT_PULLUP);
			omap_mux_init_signal("sdmmc2_cmd",
				OMAP_PIN_INPUT_PULLUP);
			omap_mux_init_signal("sdmmc2_dat0",
				OMAP_PIN_INPUT_PULLUP);

			/*
			 * For 8 wire configurations, Lines DAT4, 5, 6 and 7 need to be muxed
			 * in the board-*.c files
			 */
			if (mmc_controller->slots[0].caps &
				(MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA)) {
				omap_mux_init_signal("sdmmc2_dat1",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc2_dat2",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc2_dat3",
					OMAP_PIN_INPUT_PULLUP);
			}
			if (mmc_controller->slots[0].caps &
							MMC_CAP_8_BIT_DATA) {
				omap_mux_init_signal("sdmmc2_dat4.sdmmc2_dat4",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc2_dat5.sdmmc2_dat5",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc2_dat6.sdmmc2_dat6",
					OMAP_PIN_INPUT_PULLUP);
				omap_mux_init_signal("sdmmc2_dat7.sdmmc2_dat7",
					OMAP_PIN_INPUT_PULLUP);
			}
		}

		/*
		 * For MMC3 the pins need to be muxed in the board-*.c files
		 */
	}

	if (cpu_is_ti816x()) {
		omap_mux_init_signal("mmc_pow", OMAP_PULL_ENA );
		omap_mux_init_signal("mmc_clk", OMAP_PIN_OUTPUT );
		omap_mux_init_signal("mmc_cmd", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat0", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat1_sdirq", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat2_sdrw", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat3", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_sdcd", OMAP_PULL_ENA );
		omap_mux_init_signal("mmc_sdwp", OMAP_PULL_ENA );
	}

#if 0
	if (cpu_is_ti814x()) {
		omap_mux_init_signal("mmc_pow", OMAP_PULL_ENA );
		omap_mux_init_signal("mmc_clk", OMAP_PIN_OUTPUT );
		omap_mux_init_signal("mmc_cmd", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat0", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat1_sdirq", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat2_sdrw", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_dat3", OMAP_PULL_UP );
		omap_mux_init_signal("mmc_sdcd", OMAP_PULL_ENA );
		omap_mux_init_signal("mmc_sdwp", OMAP_PULL_ENA );
	}
#endif
}

void __init omap2_init_mmc(struct omap_mmc_platform_data **mmc_data,
			int nr_controllers)
{
	int i;
	char *name;

	for (i = 0; i < nr_controllers; i++) {
		unsigned long base, size;
		unsigned int irq = 0;

		if (!mmc_data[i])
			continue;

		omap2_mmc_mux(mmc_data[i], i);

		switch (i) {
		case 0:
			if (!cpu_is_ti81xx()) {
				base = OMAP2_MMC1_BASE;
				irq = INT_24XX_MMC_IRQ;
			} else if (cpu_is_ti816x()) {
				base = TI816X_MMC1_BASE;
				irq = TI81XX_IRQ_SD;
			} else if (cpu_is_ti814x()) {
				base = TI814X_MMC1_BASE;
				irq = TI814X_IRQ_SD1;
			}
			break;
		case 1:
			base = OMAP2_MMC2_BASE;
			irq = INT_24XX_MMC2_IRQ;
			break;
		case 2:
			if (!cpu_is_omap44xx() && !cpu_is_omap34xx())
				return;
			base = OMAP3_MMC3_BASE;
			irq = INT_34XX_MMC3_IRQ;
			break;
		case 3:
			if (!cpu_is_omap44xx())
				return;
			base = OMAP4_MMC4_BASE;
			irq = OMAP44XX_IRQ_MMC4;
			break;
		case 4:
			if (!cpu_is_omap44xx())
				return;
			base = OMAP4_MMC5_BASE;
			irq = OMAP44XX_IRQ_MMC5;
			break;
		default:
			continue;
		}

		if (cpu_is_omap2420()) {
			size = OMAP2420_MMC_SIZE;
			name = "mmci-omap";
		} else if (cpu_is_omap44xx()) {
			if (i < 3)
				irq += OMAP44XX_IRQ_GIC_START;
			size = OMAP4_HSMMC_SIZE;
			name = "mmci-omap-hs";
		} else if (cpu_is_ti81xx()) {
			size = TI81XX_HSMMC_SIZE;
			name = "mmci-omap-hs";
		} else {
			size = OMAP3_HSMMC_SIZE;
			name = "mmci-omap-hs";
		}
		omap_mmc_add(name, i, base, size, irq, mmc_data[i]);
	};
}

#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_HDQ_MASTER_OMAP) || defined(CONFIG_HDQ_MASTER_OMAP_MODULE)
#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3430)
#define OMAP_HDQ_BASE	0x480B2000
#endif
static struct resource omap_hdq_resources[] = {
	{
		.start		= OMAP_HDQ_BASE,
		.end		= OMAP_HDQ_BASE + 0x1C,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_HDQ_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};
static struct platform_device omap_hdq_dev = {
	.name = "omap_hdq",
	.id = 0,
	.dev = {
		.platform_data = NULL,
	},
	.num_resources	= ARRAY_SIZE(omap_hdq_resources),
	.resource	= omap_hdq_resources,
};
static inline void omap_hdq_init(void)
{
	(void) platform_device_register(&omap_hdq_dev);
}
#else
static inline void omap_hdq_init(void) {}
#endif

/*---------------------------------------------------------------------------*/

#if defined(CONFIG_VIDEO_OMAP2_VOUT) || \
	defined(CONFIG_VIDEO_OMAP2_VOUT_MODULE)
#if defined(CONFIG_FB_OMAP2) || defined(CONFIG_FB_OMAP2_MODULE)
static struct resource omap_vout_resource[3 - CONFIG_FB_OMAP2_NUM_FBS] = {
};
#else
static struct resource omap_vout_resource[2] = {
};
#endif

static struct platform_device omap_vout_device = {
	.name		= "omap_vout",
	.num_resources	= ARRAY_SIZE(omap_vout_resource),
	.resource 	= &omap_vout_resource[0],
	.id		= -1,
};
static void omap_init_vout(void)
{
	if (platform_device_register(&omap_vout_device) < 0)
		printk(KERN_ERR "Unable to register OMAP-VOUT device\n");
}
#else
static inline void omap_init_vout(void) {}
#endif

#ifdef CONFIG_ARCH_TI816X

#define TI816X_EMAC1_BASE		(0x4A100000)
#define TI816X_EMAC2_BASE		(0x4A120000)
#define TI816X_EMAC_CNTRL_OFFSET	(0x0)
#define TI816X_EMAC_CNTRL_MOD_OFFSET	(0x900)
#define TI816X_EMAC_CNTRL_RAM_OFFSET	(0x2000)
#define TI816X_EMAC_MDIO_OFFSET		(0x800)
#define TI816X_EMAC_CNTRL_RAM_SIZE	(0x2000)
#define TI816X_EMAC1_HW_RAM_ADDR	(0x4A102000)
#define TI816X_EMAC2_HW_RAM_ADDR	(0x4A122000)

#define TI816X_EMAC_PHY_MASK		(0xF)
#define TI816X_EMAC_MDIO_FREQ		(1000000)

static struct mdio_platform_data ti816x_mdio_pdata = {
	.bus_freq       = TI816X_EMAC_MDIO_FREQ,
};

 static struct resource ti816x_mdio_resources[] = {
	{
		.start  = TI816X_EMAC1_BASE + TI816X_EMAC_MDIO_OFFSET,
		.end  	= TI816X_EMAC1_BASE + TI816X_EMAC_MDIO_OFFSET +
				SZ_256 - 1,
		.flags  = IORESOURCE_MEM,
	}
 };

static struct platform_device ti816x_mdio_device = {
	.name           = "davinci_mdio",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ti816x_mdio_resources),
	.resource  	= ti816x_mdio_resources,
	.dev.platform_data = &ti816x_mdio_pdata,
};

static struct emac_platform_data ti816x_emac1_pdata = {
	.rmii_en	=	0,
	.phy_id		= 	"0:01",
};

static struct emac_platform_data ti816x_emac2_pdata = {
	.rmii_en	=	0,
	.phy_id		= 	"0:02",
};

static struct resource ti816x_emac1_resources[] = {
	{
		.start	=	TI816X_EMAC1_BASE,
		.end	=	TI816X_EMAC1_BASE + 0x3FFF,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.start	=	TI816X_IRQ_MACRXTHR0,
		.end	=	TI816X_IRQ_MACRXTHR0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACRXINT0,
		.end	=	TI816X_IRQ_MACRXINT0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACTXINT0,
		.end	=	TI816X_IRQ_MACTXINT0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACMISC0,
		.end	=	TI816X_IRQ_MACMISC0,
		.flags	=	IORESOURCE_IRQ,
	},
};

static struct resource ti816x_emac2_resources[] = {
	{
		.start	=	TI816X_EMAC2_BASE,
		.end	=	TI816X_EMAC2_BASE + 0x3FFF,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.start	=	TI816X_IRQ_MACRXTHR1,
		.end	=	TI816X_IRQ_MACRXTHR1,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACRXINT1,
		.end	=	TI816X_IRQ_MACRXINT1,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACTXINT1,
		.end	=	TI816X_IRQ_MACTXINT1,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACMISC1,
		.end	=	TI816X_IRQ_MACMISC1,
		.flags	=	IORESOURCE_IRQ,
	},
};
static struct platform_device ti816x_emac1_device = {
	.name	=	"davinci_emac",
	.id	=	0,
	.num_resources	=	ARRAY_SIZE(ti816x_emac1_resources),
	.resource	=	ti816x_emac1_resources,
};

static struct platform_device ti816x_emac2_device = {
	.name	=	"davinci_emac",
	.id	=	1,
	.num_resources	=	ARRAY_SIZE(ti816x_emac2_resources),
	.resource	=	ti816x_emac2_resources,
};

void ti816x_emac_mux(void)
{
       omap_mux_init_signal("gmii1_rxclk", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd0", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd1", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd2", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd3", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd4", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd5", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd6", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxd7", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxdv", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_gtxclk", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd0", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd1", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd2", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd3", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd4", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd5", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd6", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txd7", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txen", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_txclk", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_col", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_crs", OMAP_MUX_MODE1);
       omap_mux_init_signal("gmii1_rxer", OMAP_MUX_MODE1);
}



void ti816x_ethernet_init(void)
{
	u32 mac_lo, mac_hi;

	mac_lo = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID0_LO);
	mac_hi = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID0_HI);
	ti816x_emac1_pdata.mac_addr[0] = mac_hi & 0xFF;
	ti816x_emac1_pdata.mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	ti816x_emac1_pdata.mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	ti816x_emac1_pdata.mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	ti816x_emac1_pdata.mac_addr[4] = mac_lo & 0xFF;
	ti816x_emac1_pdata.mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	ti816x_emac1_pdata.ctrl_reg_offset = TI816X_EMAC_CNTRL_OFFSET;
	ti816x_emac1_pdata.ctrl_mod_reg_offset = TI816X_EMAC_CNTRL_MOD_OFFSET;
	ti816x_emac1_pdata.ctrl_ram_offset = TI816X_EMAC_CNTRL_RAM_OFFSET;
	ti816x_emac1_pdata.ctrl_ram_size = TI816X_EMAC_CNTRL_RAM_SIZE;
	ti816x_emac1_pdata.version = EMAC_VERSION_2;
	ti816x_emac1_pdata.hw_ram_addr = TI816X_EMAC1_HW_RAM_ADDR;
	ti816x_emac1_pdata.interrupt_enable = NULL;
	ti816x_emac1_pdata.interrupt_disable = NULL;
	ti816x_emac1_device.dev.platform_data = &ti816x_emac1_pdata;
	platform_device_register(&ti816x_emac1_device);

	platform_device_register(&ti816x_mdio_device);
	clk_add_alias(NULL, dev_name(&ti816x_mdio_device.dev),
			NULL, &ti816x_emac1_device.dev);

	mac_lo = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID1_LO);
	mac_hi = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID1_HI);
	ti816x_emac2_pdata.mac_addr[0] = mac_hi & 0xFF;
	ti816x_emac2_pdata.mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	ti816x_emac2_pdata.mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	ti816x_emac2_pdata.mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	ti816x_emac2_pdata.mac_addr[4] = mac_lo & 0xFF;
	ti816x_emac2_pdata.mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	ti816x_emac2_pdata.ctrl_reg_offset = TI816X_EMAC_CNTRL_OFFSET;
	ti816x_emac2_pdata.ctrl_mod_reg_offset = TI816X_EMAC_CNTRL_MOD_OFFSET;
	ti816x_emac2_pdata.ctrl_ram_offset = TI816X_EMAC_CNTRL_RAM_OFFSET;
	ti816x_emac2_pdata.ctrl_ram_size = TI816X_EMAC_CNTRL_RAM_SIZE;
	ti816x_emac2_pdata.version = EMAC_VERSION_2;
	ti816x_emac2_pdata.hw_ram_addr = TI816X_EMAC2_HW_RAM_ADDR;
	ti816x_emac2_pdata.interrupt_enable = NULL;
	ti816x_emac2_pdata.interrupt_disable = NULL;
	ti816x_emac2_device.dev.platform_data = &ti816x_emac2_pdata;
	platform_device_register(&ti816x_emac2_device);

	ti816x_emac_mux();
}
#else
static inline void ti816x_ethernet_init(void) {}
#endif

#if defined(CONFIG_ARCH_TI81XX)

#define TI81XX_TPCC_BASE		0x49000000
#define TI81XX_TPTC0_BASE		0x49800000
#define TI81XX_TPTC1_BASE		0x49900000
#define TI81XX_TPTC2_BASE		0x49a00000
#define TI81XX_TPTC3_BASE		0x49b00000

#define TI81XX_SCM_BASE_EDMA		0x00000f90

static struct resource ti81xx_edma_resources[] = {
	{
		.name	= "edma_cc0",
		.start	= TI81XX_TPCC_BASE,
		.end	= TI81XX_TPCC_BASE + SZ_32K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc0",
		.start	= TI81XX_TPTC0_BASE,
		.end	= TI81XX_TPTC0_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc1",
		.start	= TI81XX_TPTC1_BASE,
		.end	= TI81XX_TPTC1_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc2",
		.start	= TI81XX_TPTC2_BASE,
		.end	= TI81XX_TPTC2_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc3",
		.start	= TI81XX_TPTC3_BASE,
		.end	= TI81XX_TPTC3_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma0",
		.start	= TI81XX_IRQ_EDMA_COMP,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "edma0_err",
		.start	= TI81XX_IRQ_EDMA_ERR,
		.flags	= IORESOURCE_IRQ,
	},
};

static const s16 ti816x_dma_rsv_chans[][2] = {
	/* (offset, number) */
	{ 0,  4},	/* !@@@ TODO replace as appropriate - Sundaram*/
	//{24,  4},
	{30,  2},
	{54,  3},
	{-1, -1}
};

static const s16 ti816x_dma_rsv_slots[][2] = {
	/* (offset, number) */
	{ 0,  4},	/* !@@@ TODO replace as appropriate - Sundaram*/
//	{24,  4},
	{30,  2},
	{54,  3},
	{128, 384},
	{-1, -1}
};

/* Four Transfer Controllers on TI816X */
static const s8 ti816x_queue_tc_mapping[][2] = {
	/* {event queue no, TC no} */
	{0, 0},
	{1, 1},
	{2, 2},
	{3, 3},
	{-1, -1}
};

static const s8 ti816x_queue_priority_mapping[][2] = {
	/* {event queue no, Priority} */
	{0, 4},	/* !@@@ TODO replace as appropriate - Sundaram*/
	{1, 0},
	{2, 5},
	{3, 1},
	{-1, -1}
};

static struct edma_soc_info ti816x_edma_info[] = {
	{
		.n_channel		= 64,
		.n_region		= 5,	/* 0-2, 4-5 */
		.n_slot			= 512,
		.n_tc			= 4,
		.n_cc			= 1,
		.rsv_chans		= ti816x_dma_rsv_chans,
		.rsv_slots		= ti816x_dma_rsv_slots,
		.queue_tc_mapping	= ti816x_queue_tc_mapping,
		.queue_priority_mapping	= ti816x_queue_priority_mapping,
	},
};

static struct platform_device ti816x_edma_device = {
	.name		= "edma",
	.id		= -1,	/* !@@@ TODO replace as appropriate - Sundaram*/
	.dev = {
		.platform_data = ti816x_edma_info,
	},
	.num_resources	= ARRAY_SIZE(ti81xx_edma_resources),
	.resource	= ti81xx_edma_resources,
};

static const s16 ti814x_dma_rsv_chans[][2] = {
	/* (offset, number) */
	//{ 0,  4},	/* !@@@ TODO replace as appropriate - Sundaram*/
	{24,  4},
	{30,  2},
	{54,  3},
	{-1, -1}
};

static const s16 ti814x_dma_rsv_slots[][2] = {
	/* (offset, number) */
	//{ 0,  4},	/* !@@@ TODO replace as appropriate - Sundaram*/
	{24,  4},
	{30,  2},
	{54,  3},
	{128, 384},
	{-1, -1}
};

/* Four Transfer Controllers on TI814X */
static const s8 ti814x_queue_tc_mapping[][2] = {
	/* {event queue no, TC no} */
	{0, 0},
	{1, 1},
	{2, 2},
	{3, 3},
	{-1, -1}
};

static const s8 ti814x_queue_priority_mapping[][2] = {
	/* {event queue no, Priority} */
	{0, 4},	/* !@@@ TODO replace as appropriate - Sundaram*/
	{1, 0},
	{2, 5},
	{3, 1},
	{-1, -1}
};

static unsigned ti814x_xbar_event_mapping[][2] = {
	/* {xbar event no, Channel} */
	{1, -1},	/* !@@@ TODO replace as appropriate - Sundaram*/
	{2, -1},
	{3, -1},
	{4, -1},
	{5, -1},
	{6, -1},
	{7, -1},
	{8, -1},
	{9, -1},
	{10, -1},
	{11, -1},
	{12, -1},
	{13, -1},
	{14, -1},
	{15, -1},
	{16, -1},
	{17, -1},
	{18, -1},
	{19, -1},
	{20, -1},
	{21, -1},
	{22, -1},
	{23, -1},
	{24, -1},
	{25, -1},
	{26, -1},
	{27, -1},
	{28, -1},
	{29, -1},
	{30, -1},
	{31, -1},
	{-1, -1}
};

/**
 * map_xbar_event_to_channel - maps a crossbar event to a DMA channel
 * according to the configuration provided
 * @event: the event number for which mapping is required
 * @channel: channel being activated
 * @xbar_event_mapping: array that has the event to channel map
 *
 * Events that are routed by default are not mapped. Only events that
 * are crossbar mapped are routed to available channels according to
 * the configuration provided
 *
 * Returns zero on success, else negative errno.
 */
int map_xbar_event_to_channel(unsigned event, unsigned *channel, unsigned *xbar_event_mapping[])
{
	unsigned ctrl = 0;
	unsigned val = 0;
	unsigned offset = 0;

	ctrl = EDMA_CTLR(event);
	val = event - (edma_info[ctrl]->num_channels);

	if (event < edma_info[ctrl]->num_channels) {
		*channel = event;
	}
	else if (event < edma_info[ctrl]->num_events){
		*channel = xbar_event_mapping[val][1];
		offset = (*channel)*4;
		__raw_writel(xbar_event_mapping[val][0], TI81XX_SCM_BASE + TI81XX_SCM_BASE_EDMA + offset);
		return 0;
	}
	else {
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL(map_xbar_event_to_channel);

static struct edma_soc_info ti814x_edma_info[] = {
	{
		.n_channel		= 64,
		.n_region		= 5,	/* 0-2, 4-5 */
		.n_slot			= 512,
		.n_tc			= 4,
		.n_cc			= 1,
		.rsv_chans		= ti814x_dma_rsv_chans,
		.rsv_slots		= ti814x_dma_rsv_slots,
		.queue_tc_mapping	= ti814x_queue_tc_mapping,
		.queue_priority_mapping	= ti814x_queue_priority_mapping,
		.is_xbar		= 1,
		.n_events		= 95,
		.xbar_event_mapping	= ti814x_xbar_event_mapping,
		.map_xbar_channel	= map_xbar_event_to_channel,
	},
};

static struct platform_device ti814x_edma_device = {
	.name		= "edma",
	.id		= -1,	/* !@@@ TODO replace as appropriate - Sundaram*/
	.dev = {
		.platform_data = ti814x_edma_info,
	},
	.num_resources	= ARRAY_SIZE(ti81xx_edma_resources),
	.resource	= ti81xx_edma_resources,
};

int __init ti81xx_register_edma(void)
{
	struct platform_device *pdev;
	static struct clk *edma_clk;

	if (cpu_is_ti816x())
		pdev = &ti816x_edma_device;
	else if (cpu_is_ti814x())
		pdev = &ti814x_edma_device;
	else {
		pr_err("%s: platform not supported\n", __func__);
		return -ENODEV;
	}

        edma_clk = clk_get(NULL, "tpcc_ick");
         if (IS_ERR(edma_clk)) {
                 printk(KERN_ERR "EDMA: Failed to get clock\n");
                 return -EBUSY;
        }
        clk_enable(edma_clk);
        edma_clk = clk_get(NULL, "tptc0_ick");
         if (IS_ERR(edma_clk)) {
                 printk(KERN_ERR "EDMA: Failed to get clock\n");
                 return -EBUSY;
        }
        clk_enable(edma_clk);
        edma_clk = clk_get(NULL, "tptc1_ick");
         if (IS_ERR(edma_clk)) {
                 printk(KERN_ERR "EDMA: Failed to get clock\n");
                 return -EBUSY;
        }
        clk_enable(edma_clk);
        edma_clk = clk_get(NULL, "tptc2_ick");
         if (IS_ERR(edma_clk)) {
                 printk(KERN_ERR "EDMA: Failed to get clock\n");
                 return -EBUSY;
        }
        clk_enable(edma_clk);
        edma_clk = clk_get(NULL, "tptc3_ick");
         if (IS_ERR(edma_clk)) {
                 printk(KERN_ERR "EDMA: Failed to get clock\n");
                 return -EBUSY;
        }
        clk_enable(edma_clk);


	return platform_device_register(pdev);
}

#else
static inline void ti81xx_register_edma(void) {}
#endif

/*-------------------------------------------------------------------------*/

#ifdef CONFIG_ARCH_TI814X
#define TI814X_CPSW_BASE		(0x4A100000)
#define TI814X_CPSW_MDIO_BASE		(0x4A100800)
#define	TI814X_CPSW_SS_BASE		(0x4A100900)
#define TI814X_EMAC_MDIO_FREQ		(1000000)

static u64 cpsw_dma_mask = DMA_BIT_MASK(32);
/* TODO : Verify the offsets */
struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs  = 0x50,
		.sliver_reg_ofs = 0x700,
		.phy_id		= "0:01",
	},
	{
		.slave_reg_ofs  = 0x90,
		.sliver_reg_ofs = 0x740,
		.phy_id		= "0:00",
	},
};

static struct cpsw_platform_data ti814x_cpsw_pdata = {
	.ss_reg_ofs		= 0x900,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x100,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0x600,
	.ale_entries		= 1024,
	.host_port_reg_ofs      = 0x28,
	.hw_stats_reg_ofs       = 0x400,
	.bd_ram_ofs		= 0x2000,
	.bd_ram_size		= SZ_8K,
	.rx_descs               = 64,
	.mac_control            = BIT(5), /* MIIEN */
	.gigabit_en		= 1,
	.host_port_num		= 0,
	.no_bd_ram		= false,
};

static struct mdio_platform_data cpsw_mdio_pdata = {
	.bus_freq       = TI814X_EMAC_MDIO_FREQ,
};

static struct resource cpsw_mdio_resources[] = {
	{
		.start  = TI814X_CPSW_MDIO_BASE,
		.end    = TI814X_CPSW_MDIO_BASE + SZ_256 - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device cpsw_mdio_device = {
	.name           = "davinci_mdio",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(cpsw_mdio_resources),
	.resource       = cpsw_mdio_resources,
	.dev.platform_data = &cpsw_mdio_pdata,
};

static struct resource ti814x_cpsw_resources[] = {
	{
		.start  = TI814X_CPSW_BASE,
		.end    = TI814X_CPSW_BASE + SZ_2K - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = TI814X_CPSW_SS_BASE,
		.end    = TI814X_CPSW_SS_BASE + SZ_256 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start	= TI814X_IRQ_GSWRXTHR0,
		.end	= TI814X_IRQ_GSWRXTHR0,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= TI814X_IRQ_GSWRXINT0,
		.end	= TI814X_IRQ_GSWRXINT0,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= TI814X_IRQ_GSWTXINT0,
		.end	= TI814X_IRQ_GSWTXINT0,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= TI814X_IRQ_GSWMISC0,
		.end	= TI814X_IRQ_GSWMISC0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device ti814x_cpsw_device = {
	.name		=	"cpsw",
	.id		=	0,
	.num_resources	=	ARRAY_SIZE(ti814x_cpsw_resources),
	.resource	=	ti814x_cpsw_resources,
	.dev		=	{
					.platform_data	   = &ti814x_cpsw_pdata,
					.dma_mask	   = &cpsw_dma_mask,
					.coherent_dma_mask = DMA_BIT_MASK(32),
				},
};

void ti814x_cpsw_mux(void)
{
#if 0 /* No pinmux for now */
	omap_mux_init_signal("gmii1_rxclk", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd0", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd1", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd2", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd3", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd4", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd5", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd6", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd7", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxdv", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_gtxclk", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd0", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd1", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd2", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd3", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd4", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd5", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd6", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd7", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txen", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txclk", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_col", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_crs", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxer", OMAP_MUX_MODE1);
#endif
}

void ti814x_cpsw_init(void)
{
	u32 mac_lo, mac_hi;

	mac_lo = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID0_LO);
	mac_hi = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID0_HI);
	cpsw_slaves[0].mac_addr[0] = mac_hi & 0xFF;
	cpsw_slaves[0].mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	cpsw_slaves[0].mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	cpsw_slaves[0].mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	cpsw_slaves[0].mac_addr[4] = mac_lo & 0xFF;
	cpsw_slaves[0].mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	mac_lo = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID1_LO);
	mac_hi = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID1_HI);
	cpsw_slaves[1].mac_addr[0] = mac_hi & 0xFF;
	cpsw_slaves[1].mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	cpsw_slaves[1].mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	cpsw_slaves[1].mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	cpsw_slaves[1].mac_addr[4] = mac_lo & 0xFF;
	cpsw_slaves[1].mac_addr[5] = (mac_lo & 0xFF00) >> 8;
#if 0
	ti814x_cpsw_mux();
#endif
	platform_device_register(&cpsw_mdio_device);
	platform_device_register(&ti814x_cpsw_device);
	clk_add_alias(NULL, dev_name(&cpsw_mdio_device.dev),
			NULL, &ti814x_cpsw_device.dev);
}
#else
static inline void ti814x_cpsw_init(void) {}
#endif

#ifdef CONFIG_ARCH_TI81XX
static void ti81xx_ethernet_init(void)
{
	if (cpu_is_ti816x())
		ti816x_ethernet_init();
	else
		ti814x_cpsw_init();
}
#endif

#define P0PHYCR		0x178  /* SATA PHY0 Control Register offset
                                * from AHCI base)
                                */
#define P1PHYCR		0x1F8  /* SATA PHY0 Control Register offset
                                * from AHCI base)
                                */

#define PHY_ENPLL	1 /* bit0        1 */
#define PHY_MPY		8 /* bits4:1     4 Clock Sources at 100MHz */
#define PHY_LB		0 /* bits6:5     2 */
#define PHY_CLKBYP	0 /* bits8:7     2 */
#define PHY_RXINVPAIR	0 /* bit9        1 */
#define PHY_LBK		0 /* bits11:10   2 */
#define PHY_RXLOS	1 /* bit12	 1 */
#define PHY_RXCDR	4 /* bits15:13   3 */
#define PHY_RXEQ	1 /* bits19:16   4 */
#define PHY_RxENOC	1 /* bit20       1 */
#define PHY_TXINVPAIR	0 /* bit21	 1 */
#define PHY_TXCM	0 /* bit22       1 */
#define PHY_TXSWING	0x7 /* bits26:23   4 */
#define PHY_TXDE	0x0 /* bits31:27   5 */

#define PHY_CFGRX0_VAL		0x00C7CC22
#define PHY_CFGRX1_VAL		0x008E0500
#define PHY_CFGRX2_VAL		0x7BDEF000
#define PHY_CFGRX3_VAL		0x1F180B0F
#define PHY_CFGTX0_VAL		0x01001622
#define PHY_CFGTX1_VAL		0x40000002
#define PHY_CFGTX2_VAL		0x073CE39E

static int ti81xx_ahci_plat_init(void)
{
	u32     	phy_val = 0;
	void __iomem	*base;
	struct clk *sata_clk;

	sata_clk = clk_get(NULL, "sata_ick");
	if (IS_ERR(sata_clk)) {
		pr_err("ahci : Failed to get AHCI clock\n");
		return -1;
	}

	if (clk_enable(sata_clk)) {
		pr_err("ahci : Clock Enable Failed\n");
		return -1;
	}

	phy_val = PHY_ENPLL << 0 | PHY_MPY << 1 | PHY_LB << 5 |
			PHY_CLKBYP << 7 | PHY_RXINVPAIR << 9 |
			PHY_LBK  << 10 | PHY_RXLOS << 12 |
			PHY_RXCDR << 13 | PHY_RXEQ << 16 |
			PHY_RxENOC << 20 | PHY_TXINVPAIR << 21 |
			PHY_TXCM << 22 | PHY_TXSWING  << 23 | PHY_TXDE << 27;
	base = ioremap(TI81XX_SATA_BASE, 0x10ffff);
	if (!base) {
		printk(KERN_WARNING
				"%s: Unable to map SATA, "
				"cannot turn on PHY.\n",  __func__);
		return -1;
	}

	if (cpu_is_ti816x()) {
		/* Initialize the SATA PHY */
		writel(phy_val,	base + P0PHYCR);

		/* ti816x platform has 2 SATA PHY's Initialize the second instance */
		if (cpu_is_ti81xx())
			writel(phy_val, base + P1PHYCR);
	}

	if (cpu_is_ti814x()) {
		/* Configuring PHY registers for SATA */
		writel(PHY_CFGRX0_VAL, base + TI814X_SATA_PHY_CFGRX0_OFFSET);
		writel(PHY_CFGRX1_VAL, base + TI814X_SATA_PHY_CFGRX1_OFFSET);
		writel(PHY_CFGRX2_VAL, base + TI814X_SATA_PHY_CFGRX2_OFFSET);
		writel(PHY_CFGRX3_VAL, base + TI814X_SATA_PHY_CFGRX3_OFFSET);
		writel(PHY_CFGTX0_VAL, base + TI814X_SATA_PHY_CFGTX0_OFFSET);
		writel(PHY_CFGTX1_VAL, base + TI814X_SATA_PHY_CFGTX1_OFFSET);
		writel(PHY_CFGTX2_VAL, base + TI814X_SATA_PHY_CFGTX2_OFFSET);
	}

	iounmap(base);

	return 0;
}

static struct resource ti81xx_ahci_resources[] = {
	{
		.start	=	TI81XX_SATA_BASE,
		.end	=	TI81XX_SATA_BASE + 0x10fff,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.start	=	TI81XX_IRQ_SATA,
		.flags	=	IORESOURCE_IRQ,
	}
};

static struct platform_device ti81xx_ahci_device = {
	.name	=	"ahci",
	.dev	=	{
				.coherent_dma_mask = DMA_BIT_MASK(32),
			},
	.num_resources = ARRAY_SIZE(ti81xx_ahci_resources),
	.resource	= ti81xx_ahci_resources,
};

int __init ti81xx_init_ahci(void)
{
	if (ti81xx_ahci_plat_init() != 0)
		return -1;

	return platform_device_register(&ti81xx_ahci_device);
}

#if defined(CONFIG_ARCH_TI81XX)

static struct resource ti81xx_mcasp_resource[] = {
	{
		.name = "mcasp",
		.start = TI81XX_ASP2_BASE,
		.end = TI81XX_ASP2_BASE + (SZ_1K * 12) - 1,
		.flags = IORESOURCE_MEM,
	},
	/* TX event */
	{
		.start = TI81XX_DMA_MCASP2_AXEVT,
		.end = TI81XX_DMA_MCASP2_AXEVT,
		.flags = IORESOURCE_DMA,
	},
	/* RX event */
	{
		.start = TI81XX_DMA_MCASP2_AREVT,
		.end = TI81XX_DMA_MCASP2_AREVT,
		.flags = IORESOURCE_DMA,
	},
};

static struct platform_device ti81xx_mcasp_device = {
	.name = "davinci-mcasp",
	.id = 2,
	.num_resources = ARRAY_SIZE(ti81xx_mcasp_resource),
	.resource = ti81xx_mcasp_resource,
};

void __init ti81xx_register_mcasp(int id, struct snd_platform_data *pdata)
{
	ti81xx_mcasp_device.dev.platform_data = pdata;
	platform_device_register(&ti81xx_mcasp_device);
}
#endif

#if defined(CONFIG_ARCH_TI816X) && defined(CONFIG_PCI)
static struct ti816x_pcie_data ti816x_pcie_data = {
	        .msi_irq_base	= MSI_IRQ_BASE,
		.msi_irq_num	= MSI_NR_IRQS,
};

static struct resource ti816x_pcie_resources[] = {
	{
		/* Register space */
		.name		= "pcie-regs",
		.start		= TI816X_PCIE_REG_BASE,
		.end		= TI816X_PCIE_REG_BASE + SZ_16K - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		/* Non-prefetch memory */
		.name		= "pcie-nonprefetch",
		.start		= TI816X_PCIE_MEM_BASE,
		.end		= TI816X_PCIE_MEM_BASE + SZ_256M - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		/* IO window */
		.name		= "pcie-io",
		.start		= TI816X_PCIE_IO_BASE,
		.end		= TI816X_PCIE_IO_BASE + SZ_2M + SZ_1M - 1,
		.flags		= IORESOURCE_IO,
	},
	{
		/* Inbound memory window */
		.name		= "pcie-inbound0",
		.start		= PHYS_OFFSET,
		.end		= PHYS_OFFSET + SZ_2G - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		/* Legacy Interrupt */
		.name		= "legacy_int",
		.start		= TI81XX_IRQ_PCIINT0,
		.end		= TI81XX_IRQ_PCIINT0,
		.flags		= IORESOURCE_IRQ,
	},
#ifdef CONFIG_PCI_MSI
	{
		/* MSI Interrupt Line */
		.name		= "msi_int",
		.start		= TI81XX_IRQ_PCIINT1,
		.end		= TI81XX_IRQ_PCIINT1,
		.flags		= IORESOURCE_IRQ,
	},
#endif
};

static struct platform_device ti816x_pcie_device = {
	.name		= "ti816x_pcie",
	.id		= 0,
	.dev		= {
		.platform_data = &ti816x_pcie_data,
	},
	.num_resources	= ARRAY_SIZE(ti816x_pcie_resources),
	.resource	= ti816x_pcie_resources,
};

static inline void ti816x_init_pcie(void)
{
	if (cpu_is_ti816x()) {
		omap_ctrl_writel(TI816X_PCIE_PLLMUX_25X |
				TI81XX_PCIE_DEVTYPE_RC,
				TI816X_CONTROL_PCIE_CFG);

		platform_device_register(&ti816x_pcie_device);
	}
}
#else
static inline void ti816x_init_pcie(void) {}
#endif

static int __init omap2_init_devices(void)
{
	/*
	 * please keep these calls, and their implementations above,
	 * in alphabetical order so they're easier to sort through.
	 */
	omap_hsmmc_reset();
#if !defined(CONFIG_ARCH_TI81XX)
        omap_init_audio();
#endif
	omap_init_camera();
	omap_init_mbox();
	omap_init_mcspi();
	omap_init_pmu();
	omap_hdq_init();
	omap_init_sti();
	omap_init_sham();
	omap_init_aes();
	omap_init_vout();
#ifdef CONFIG_ARCH_TI81XX
	ti81xx_ethernet_init();
	ti816x_init_pcie();
	ti81xx_register_edma();
	ti81xx_init_pcm();
	ti81xx_init_ahci();
#endif
	return 0;
}
arch_initcall(omap2_init_devices);

#if defined(CONFIG_OMAP_WATCHDOG) || defined(CONFIG_OMAP_WATCHDOG_MODULE)
struct omap_device_pm_latency omap_wdt_latency[] = {
	[0] = {
		.deactivate_func = omap_device_idle_hwmods,
		.activate_func   = omap_device_enable_hwmods,
		.flags		 = OMAP_DEVICE_LATENCY_AUTO_ADJUST,
	},
};

static int __init omap_init_wdt(void)
{
	int id = -1;
	struct omap_device *od;
	struct omap_hwmod *oh;
	char *oh_name = "wd_timer2";
	char *dev_name = "omap_wdt";

	if (!cpu_class_is_omap2())
		return 0;

	oh = omap_hwmod_lookup(oh_name);
	if (!oh) {
		pr_err("Could not look up wd_timer%d hwmod\n", id);
		return -EINVAL;
	}

	od = omap_device_build(dev_name, id, oh, NULL, 0,
				omap_wdt_latency,
				ARRAY_SIZE(omap_wdt_latency), 0);
	WARN(IS_ERR(od), "Cant build omap_device for %s:%s.\n",
				dev_name, oh->name);
	return 0;
}
subsys_initcall(omap_init_wdt);
#endif
