/*
 * mini210_wm8960.c
 *   based on smdkv2xx_wm8580mst.c and herring-wm8994.c
 *
 * Copyright (c) 2011 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2009 Samsung Electronics Co. Ltd
 * Author: Jaswinder Singh <jassi.brar@samsung.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <linux/io.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <plat/regs-iis.h>

#include "dma.h"
#include "s5pc1xx-i2s.h"
#include "wm8960.h"

#define I2S_NUM				0


static int mini210_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	u32 sample_rate = params_rate(params);
	u32 rclk, src_clk;
	int bfs, rfs, psr;
	int ret;
#ifndef USE_CLKAUDIO
	struct *clk_epll;
#endif

	switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S16_LE:
			/* Can take any RFS value for AP */
			bfs = 32;
			break;

		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S20_3LE:
			bfs = 48;
			break;

		default:
			return -EINVAL;
	}

	/* Select rfs to a minimum value */
	switch (sample_rate) {
		case 44100:
		case 22050:
		case 48000:
		case 16000:
		case 24000:
		case 32000:
		case 88200:
		case 96000:
			if (bfs == 48)
				rfs = 384;
			else
				rfs = 256;
			break;
		case 8000:
		case 11025:
		case 12000:
			if (bfs == 48)
				rfs = 768;
			else
				rfs = 512;
			break;
		case 64000:
			rfs = 384;
			break;
		default:
			return -EINVAL;
	}

    rclk = sample_rate * rfs;

	/* config codec (wm8960) as slave */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk("%s: Codec DAI configuration error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk("%s: AP DAI configuration error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C64XX_CLKSRC_CDCLK,
			sample_rate, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk("%s: AP sycclk CDCLK setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}

#ifdef USE_CLKAUDIO
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C_CLKSRC_CLKAUDIO,
			sample_rate, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk("%s: AP sysclk CLKAUDIO setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	if (sample_rate & 0xf) {
		src_clk = 67738000;
	} else {
		src_clk = 49152000;
	}

#else
	clk_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(clk_epll)) {
		printk(KERN_ERR "failed to get fout_epll\n");
		clk_put(clk_out);
		return -EBUSY;
	}

	src_clk = clk_get_rate(clk_epll);
	clk_put(clk_epll);
#endif

	psr = src_clk / rclk;

#if 0
	printk("ASoC_mini210: SCLK=%d PSR=%d RCLK=%d RFS=%d BFS=%d\n",
			src_clk, psr, rclk, rfs, bfs);
#endif

	/* Set codec clock dividers */
	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_SYSCLKDIV, 0);
	if (ret < 0) {
        printk("%s: Codec SYSCLKDIV setting error, %d\n", __FUNCTION__, ret);
        return ret;
	}

	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, 0);
	if (ret < 0) {
        printk("%s: Codec DACDIV setting error, %d\n", __FUNCTION__, ret);
        return ret;
	}

	snd_soc_dai_set_clkdiv(codec_dai, WM8960_DCLKDIV, rclk);

    /* Set the AP Prescalar/Pll */
    ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_PRESCALER, psr - 1);
    if (ret < 0) {
        printk("%s: AP prescalar setting error, %d\n", __FUNCTION__, ret);
        return ret;
    }

    /* Set the AP RFS */
    ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_RCLK, rfs);
    if (ret < 0) {
        printk("%s: AP RFS setting error, %d\n", __FUNCTION__, ret);
        return ret;
    }

    /* Set the AP BFS */
    ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_BCLK, bfs);
    if (ret < 0) {
        printk("%s: AP BFS setting error, %d\n", __FUNCTION__, ret);
        return ret;
    }

	return 0;
}

/*
 * Machine (mini210 <--> wm8960) stream operations.
 */
static struct snd_soc_ops mini210_ops = {
	.hw_params = mini210_hw_params,
};


/* MINIV210 Playback widgets */
static const struct snd_soc_dapm_widget wm8960_dapm_widgets_pbk[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_SPK("Speaker_L", NULL),
	SND_SOC_DAPM_SPK("Speaker_R", NULL),
};

/* MINIV210 Capture widgets */
static const struct snd_soc_dapm_widget wm8960_dapm_widgets_cpt[] = {
	SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_LINE("Line Input 3 (FM)", NULL),
};

/* SMDK Audio map */
static const struct snd_soc_dapm_route audio_map[] = {
	/* Headphone */
	{ "Headphone Jack", NULL, "HP_L" },
	{ "Headphone Jack", NULL, "HP_R" },

	/* Speaker */
	{ "Speaker_L", NULL, "SPK_LP" },
	{ "Speaker_L", NULL, "SPK_LN" },
	{ "Speaker_R", NULL, "SPK_RP" },
	{ "Speaker_R", NULL, "SPK_RN" },

#if 0
	/* Line Input 3 (FM) - TODO: test it when FM is ready */
	{ "LINPUT3", NULL, "Line Input 3 (FM)" },
	{ "RINPUT3", NULL, "Line Input 3 (FM)" },
#endif

	/* MIC is connected to input 1 - with bias */
	{ "LINPUT1", NULL, "MICB"},
	{ "MICB", NULL, "Mic Jack"},
};

static int mini210_wm8960_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	int ret = 0;

	/* NC codec pins */
	snd_soc_dapm_nc_pin(dapm, "RINPUT1");
	snd_soc_dapm_nc_pin(dapm, "LINPUT2");
	snd_soc_dapm_nc_pin(dapm, "RINPUT2");
	snd_soc_dapm_nc_pin(dapm, "OUT3");

	/* Add mini210 specific Capture widgets */
	snd_soc_dapm_new_controls(dapm, wm8960_dapm_widgets_cpt,
			ARRAY_SIZE(wm8960_dapm_widgets_cpt));

	/* Add mini210 specific Playback widgets */
	snd_soc_dapm_new_controls(dapm, wm8960_dapm_widgets_pbk,
			ARRAY_SIZE(wm8960_dapm_widgets_pbk));

	/* Set up audio path */
	snd_soc_dapm_add_routes(dapm, audio_map, ARRAY_SIZE(audio_map));

	/* Set endpoints mode */
	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
	snd_soc_dapm_enable_pin(dapm, "Mic Jack");
	snd_soc_dapm_enable_pin(dapm, "Speaker_L");
	snd_soc_dapm_enable_pin(dapm, "Speaker_R");
	snd_soc_dapm_disable_pin(dapm, "Line Input 3 (FM)");

	/* signal a DAPM event */
	snd_soc_dapm_sync(dapm);

	return ret;
}

/* digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link mini210_dai = {
	.name = "MINI210",
	.stream_name = "WM8960 HiFi",
	.cpu_dai_name = "samsung-i2s.0",
	.codec_dai_name = "wm8960-hifi",
	.platform_name = "samsung-audio",
	.codec_name = "wm8960-codec.0-001a",
	.init = mini210_wm8960_init,
	.ops = &mini210_ops,
};

static struct snd_soc_card mini210 = {
	.name = "mini210",
	.dai_link = &mini210_dai,
	.num_links = 1,
};


/* audio subsystem */
static struct platform_device *mini210_snd_device;

static char mini210_snd_card[] = {
	/* Encoded "FriendlyARM\0" */
	0xb9, 0x85, 0x90, 0x9c, 0x91, 0x9b, 0x93, 0x80,
	0xb8, 0xa5, 0xb4, 0xf7,
};

static int __init mini210_audio_init(void)
{
	char *p = mini210_snd_card;
	volatile unsigned int mode = 0x10;
	int i;
	int ret;

	mode = mode ? (mode + 0xEA) : 0;
	for (i = 0; i < sizeof(mini210_snd_card); i++) {
		p[i] = (p[i] + 0x03) ^ mode;
	}
	printk("%s", p);

	mini210_snd_device = platform_device_alloc("soc-audio", -1);
	if (!mini210_snd_device)
		return -ENOMEM;

	platform_set_drvdata(mini210_snd_device, &mini210);

	printk(" http://www.arm9.net\n");

	ret = platform_device_add(mini210_snd_device);
	if (ret) {
		printk("%s: add device failed, %d\n", __FUNCTION__, ret);
		platform_device_put(mini210_snd_device);
	}

	return ret;
}

static void __exit mini210_audio_exit(void)
{
    platform_device_unregister(mini210_snd_device);
}

module_init(mini210_audio_init);
module_exit(mini210_audio_exit);

MODULE_AUTHOR("FriendlyARM (www.arm9.net)");
MODULE_DESCRIPTION("ALSA SoC MINI210 WM8960");
MODULE_LICENSE("GPL");

