/*
 * wl1271bt.c  --  ALSA SoC WL1271 Bluetooth codec driver for omap3evm board
 *
 * Author: Sinoj M. Issac, <sinoj at mistralsolutions.com>
 *
 * Based on sound/soc/codecs/twl4030.c by Steve Sakoman
 *
 * This file provides stub codec that can be used on OMAP3530 evm to
 * send/receive voice samples to/from WL1271 Bluetooth chip over PCM interface.
 * The Bluetoothchip codec interface is configured by HCI commands. ALSA is
 * configured and aligned to the codec interface.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <sound/soc.h>
#include <sound/pcm.h>

/*
 * Since WL1271 PCM interface is intended for Voice,
 * Support sampling rate 8K only
 */
#define WL1271BT_RATES		SNDRV_PCM_RATE_8000
#define WL1271BT_FORMATS	SNDRV_PCM_FMTBIT_S16_LE

struct snd_soc_dai wl1271bt_dai = {
	.name = "wl1271bt",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WL1271BT_RATES,
		.formats = WL1271BT_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WL1271BT_RATES,
		.formats = WL1271BT_FORMATS,},
};

static int __init wl1271bt_modinit(void)
{
	/* Register number of DAIs (wl1271bt_dai) with the ASoC core */
	return snd_soc_register_dais(&wl1271bt_dai, 1);
}

static void __exit wl1271bt_modexit(void)
{
	/* Unregister number of DAIs (wl1271bt_dai) from the ASoC core */
	snd_soc_unregister_dais(&wl1271bt_dai, 1);
}

module_init(wl1271bt_modinit);
module_exit(wl1271bt_modexit);


