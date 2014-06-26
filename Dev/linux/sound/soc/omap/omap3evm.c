/*
 * omap3evm.c  -- ALSA SoC support for OMAP3 EVM
 *
 * Author: Anuj Aggarwal <anuj.aggarwal@ti.com>
 *
 * Based on sound/soc/omap/beagle.c by Steve Sakoman
 *
 * Added support for MCBSP1 <-> WL1271 BT by Mistral
 *
 * Copyright (C) 2008 Texas Instruments, Incorporated
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"

#if defined(CONFIG_SND_SOC_WL1271BT)
#include <plat/control.h>
#include "../codecs/wl1271bt.h"
#endif

static int omap3evm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "Can't set codec system clock\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops omap3evm_ops = {
	.hw_params = omap3evm_hw_params,
};

#if defined(CONFIG_SND_SOC_WL1271BT)
static int omap3evm_wl1271bt_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	/* Set cpu DAI configuration for WL1271 Bluetooth codec */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration for " \
						"WL1271 Bluetooth codec \n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops omap3evm_wl1271bt_pcm_ops = {
	.hw_params = omap3evm_wl1271bt_pcm_hw_params,
};

#endif


/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link omap3evm_dai = {
	.name 		= "TWL4030",
	.stream_name 	= "TWL4030",
	.cpu_dai_name = "omap-mcbsp-dai.1",
	.codec_dai_name = "twl4030-hifi",
	.platform_name = "omap-pcm-audio",
	.codec_name = "twl4030-codec",
	.ops 		= &omap3evm_ops,

static struct snd_soc_dai_link omap3evm_dai[] = {
	{
		.name = "TWL4030",
		.stream_name = "TWL4030",
		.cpu_dai = &omap_mcbsp_dai[0],
		.codec_dai = &twl4030_dai[TWL4030_DAI_HIFI],
		.ops = &omap3evm_ops,
	},

#if defined(CONFIG_SND_SOC_WL1271BT)
	/* Connects WL1271 Bluetooth codec <--> CPU */
	{
		.name = "WL1271BTPCM",
		.stream_name = "WL1271 BT PCM",
		.cpu_dai = &omap_mcbsp_dai[1],
		.codec_dai = &wl1271bt_dai,
		.ops = &omap3evm_wl1271bt_pcm_ops,
	},
#endif
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_omap3evm = {
	.name = "omap3evm",
	.dai_link = &omap3evm_dai[0],
	.num_links = ARRAY_SIZE(omap3evm_dai),
};

static struct platform_device *omap3evm_snd_device;

static int __init omap3evm_soc_init(void)
{
	int ret;
#if defined(CONFIG_SND_SOC_WL1271BT)
	u16 reg;
	u32 val;
#endif

	if (!machine_is_omap3evm())
		return -ENODEV;
	pr_info("OMAP3 EVM SoC init\n");

	omap3evm_snd_device = platform_device_alloc("soc-audio", -1);
	if (!omap3evm_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

#if defined(CONFIG_SND_SOC_WL1271BT)
/*
 * Set DEVCONF0 register to connect
 * MCBSP1_CLKR -> MCBSP1_CLKX & MCBSP1_FSR -> MCBSP1_FSX
 */
	reg = OMAP2_CONTROL_DEVCONF0;
	val = omap_ctrl_readl(reg);
	val = val | 0x18;
	omap_ctrl_writel(val, reg);
#endif


	platform_set_drvdata(omap3evm_snd_device, &snd_soc_omap3evm);
	*(unsigned int *)omap3evm_dai[0].cpu_dai->private_data = 1; /* McBSP2 */
#if defined(CONFIG_SND_SOC_WL1271BT)
	*(unsigned int *)omap3evm_dai[1].cpu_dai->private_data = 0; /* McBSP1 */
#endif
	ret = platform_device_add(omap3evm_snd_device);
	if (ret)
		goto err1;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(omap3evm_snd_device);

	return ret;
}

static void __exit omap3evm_soc_exit(void)
{
	platform_device_unregister(omap3evm_snd_device);
}

module_init(omap3evm_soc_init);
module_exit(omap3evm_soc_exit);

MODULE_AUTHOR("Anuj Aggarwal <anuj.aggarwal@ti.com>");
MODULE_DESCRIPTION("ALSA SoC OMAP3 EVM");
MODULE_LICENSE("GPL v2");
