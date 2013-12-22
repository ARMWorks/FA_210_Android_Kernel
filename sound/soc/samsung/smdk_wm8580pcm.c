/*
 *  sound/soc/samsung/smdk_wm8580pcm.c
 *
 *  Copyright (c) 2011 Samsung Electronics Co. Ltd
 *
 *  This program is free software; you can redistribute  it and/or  modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>



#include <mach/map.h>
#include <mach/regs-clock.h>

#include "../codecs/wm8580.h"
#include "dma.h"
#include "pcm.h"

#define SMDK_WM8580_XTI_FREQ	24000000
#define SMDK_WM8580_PCM_PORT	0	/* PCM0 */

#define SMDK_WM8580_EXT_OSC 12000000
#define SMDK_WM8580_EXT_MCLK 4096000
#define SMDK_WM8580_EXT_VOICE 2048000

static unsigned long mclk_freq;
static unsigned long xtal_freq;

/*
 * If MCLK clock directly gets from XTAL, we don't have to use PLL
 * to make MCLK, but if XTAL clock source connects with other codec
 * pin (like XTI), we should have to set codec's PLL to make MCLK.
 * Because Samsung SoC does not support pcmcdclk output like I2S.
 */

static int set_epll_rate(unsigned long rate)
{
	struct clk *fout_epll;
	fout_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(fout_epll)) {
		printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
		return -ENOENT;
	}

//	if (rate == clk_get_rate(fout_epll))
//		goto out;

	clk_disable(fout_epll);
	clk_set_rate(fout_epll, rate);
	clk_enable(fout_epll);
out:
	clk_put(fout_epll);

	return 0;
}

/* Note that PCM works __only__ in AP-Master mode */
static int smdk_wm8580_pcm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int rfs, ret;
	unsigned long epll_out_rate;

	switch (params_rate(params)) {
	case 8000:
		mclk_freq = 49152000;
                break;
	case 44100:
                mclk_freq = 67738000;
		break;
	default:
		printk(KERN_ERR "%s:%d Sampling Rate %u not supported!\n",
		__func__, __LINE__, params_rate(params));
		return -EINVAL;
	}

        switch (params_rate(params)) {
        case 16000:
        case 22050:
        case 22025:
        case 32000:
        case 44100:
        case 48000:
        case 96000:
        case 24000:
                rfs = 256;
                break;
        case 64000:
                rfs = 384;
                break;
        case 8000:
        case 11025:
        case 12000:
                rfs = 512;
                break;
        case 88200:
                rfs = 128;
                break;
        default:
                printk(KERN_ERR "%s:%d Sampling Rate %u not supported!\n",
                        __func__, __LINE__, params_rate(params));
                return -EINVAL;
        }


	/* Set the codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_DSP_B
				| SND_SOC_DAIFMT_IB_NF
				| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* Set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_DSP_B
					 | SND_SOC_DAIFMT_IB_NF
					 | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

mclk_freq= 4* params_rate(params) * rfs ;

    /* Set EPLL clock rate */
    ret = set_epll_rate(67738000 /*mclk_freq*/);
    if (ret < 0)
        return ret;


#if 0

	if (mclk_freq == xtal_freq) {
		ret = snd_soc_dai_set_sysclk(codec_dai, WM8580_CLKSRC_MCLK,
						mclk_freq, SND_SOC_CLOCK_IN);
		if (ret < 0)
			return ret;

		ret = snd_soc_dai_set_clkdiv(codec_dai, WM8580_MCLK,
						WM8580_CLKSRC_MCLK);
		if (ret < 0)
			return ret;
	} else {
		ret = snd_soc_dai_set_sysclk(codec_dai, WM8580_CLKSRC_PLLA,
						mclk_freq, SND_SOC_CLOCK_IN);
		if (ret < 0)
			return ret;

		ret = snd_soc_dai_set_clkdiv(codec_dai, WM8580_MCLK,
						WM8580_CLKSRC_PLLA);
		if (ret < 0)
			return ret;

		ret = snd_soc_dai_set_pll(codec_dai, WM8580_PLLA, 0,
						xtal_freq, mclk_freq);
		if (ret < 0)
			return ret;
	}
#endif
		xtal_freq = SMDK_WM8580_EXT_OSC;


	ret = snd_soc_dai_set_pll(codec_dai, WM8580_PLLA, 0,
					xtal_freq, mclk_freq);
	if (ret < 0)
		return ret;

	/* Set PCM source clock on CPU */
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C_PCM_CLKSRC_MUX,
					mclk_freq, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

        ret = snd_soc_dai_set_sysclk(codec_dai, WM8580_CLKSRC_PLLA,
                                     mclk_freq/4, SND_SOC_CLOCK_IN);
        if (ret < 0)
                return ret;

	/* Set SCLK_DIV for making bclk */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_PCM_SCLK_PER_FS, rfs);
	if (ret < 0)
		return ret;

	return 0;
}

static struct snd_soc_ops smdk_wm8580_pcm_ops = {
	.hw_params = smdk_wm8580_pcm_hw_params,
};

/* Playback widgets */
static const struct snd_soc_dapm_widget wm8580_dapm_widgets_pbk[] = {
	SND_SOC_DAPM_HP("Front-L/R", NULL),
};

/* Capture widgets */
static const struct snd_soc_dapm_widget wm8580_dapm_widgets_cpt[] = {
	SND_SOC_DAPM_MIC("MicIn", NULL),
	SND_SOC_DAPM_LINE("LineIn", NULL),
};

/* TX connections */
static const struct snd_soc_dapm_route audio_map_tx[] = {
	/* MicIn feeds AINL */
	{"AINL", NULL, "MicIn"},

	/* LineIn feeds AINL/R */
	{"AINL", NULL, "LineIn"},
	{"AINR", NULL, "LineIn"},
};

/* RX connections */
static const struct snd_soc_dapm_route audio_map_rx[] = {
	/* Front Left/Right are fed VOUT1L/R */
	{"Front-L/R", NULL, "VOUT1L"},
	{"Front-L/R", NULL, "VOUT1R"},
};

static int smdk_wm8580_pcm_init(struct snd_soc_codec *codec)
{

	/* Add smdk specific Playback widgets */
	snd_soc_dapm_new_controls(codec, wm8580_dapm_widgets_pbk,
				  ARRAY_SIZE(wm8580_dapm_widgets_pbk));

	/* Add smdk specific Capture widgets */
	snd_soc_dapm_new_controls(codec, wm8580_dapm_widgets_cpt,
				  ARRAY_SIZE(wm8580_dapm_widgets_cpt));

	/* Set up TX audio path */
	snd_soc_dapm_add_routes(codec, audio_map_tx, ARRAY_SIZE(audio_map_tx));

	/* Set up RX audio path */
	snd_soc_dapm_add_routes(codec, audio_map_rx, ARRAY_SIZE(audio_map_rx));

	/* All enabled by default */
	snd_soc_dapm_enable_pin(codec, "Front-L/R");
	snd_soc_dapm_enable_pin(codec, "MicIn");
	snd_soc_dapm_enable_pin(codec, "LineIn");

	/* signal a DAPM event */
	snd_soc_dapm_sync(codec);

	return 0;
}
extern struct snd_soc_dai s3c_pcm_dai[];
static struct snd_soc_dai_link smdk_dai[] = {
	{
		.name = "WM8580 PAIF PCM RX",
		.stream_name = "Playback",
		.cpu_dai_name = "samsung-pcm.0",
		.codec_dai_name = "wm8580-hifi-playback",
		.platform_name = "samsung-audio",
		.codec_name = "wm8580-codec.0-001b",
		.ops = &smdk_wm8580_pcm_ops,
	}, {
		.name = "WM8580 PAIF PCM TX",
		.stream_name = "Capture",
		.cpu_dai_name = "samsung-pcm.0",
		.codec_dai_name = "wm8580-hifi-capture",
		.platform_name = "samsung-audio",
		.codec_name = "wm8580-codec.0-001b",
		.ops = &smdk_wm8580_pcm_ops,
	},
};

static struct snd_soc_card smdk_pcm = {
	.name = "SMDK-PCM",
	.dai_link = smdk_dai,
	.num_links = 2,
};

/*
 * After SMDKC110 Base Board's Rev is '0.1', 12MHz External OSC(X1)
 * is absent (or not connected), so we connect EXT_VOICE_CLK(OSC4),
 * 2.0484Mhz, directly with MCLK both Codec and SoC.
 */
static int __devinit snd_smdk_probe(struct platform_device *pdev)
{
	int ret = 0;

	xtal_freq = SMDK_WM8580_EXT_OSC;
	mclk_freq = SMDK_WM8580_EXT_MCLK;

		xtal_freq = mclk_freq = SMDK_WM8580_EXT_VOICE;

	smdk_pcm.dev = &pdev->dev;
	ret = snd_soc_register_card(&smdk_pcm);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
		return ret;
	}

	return 0;
}

static int __devexit snd_smdk_remove(struct platform_device *pdev)
{
	snd_soc_unregister_card(&smdk_pcm);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver snd_smdk_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "samsung-smdk-pcm",
	},
	.probe = snd_smdk_probe,
	.remove = __devexit_p(snd_smdk_remove),
};
static struct platform_device *smdk_snd_device;

static int __init smdk_audio_init(void)
{
        int ret;

        smdk_snd_device = platform_device_alloc("soc-audio", -1);
        if (!smdk_snd_device)
                return -ENOMEM;

        platform_set_drvdata(smdk_snd_device, &smdk_pcm);
        ret = platform_device_add(smdk_snd_device);

        if (ret){
                platform_device_put(smdk_snd_device);
        }

        return ret;



#if 0
	return platform_driver_register(&snd_smdk_driver);
#endif
}

module_init(smdk_audio_init);

static void __exit smdk_audio_exit(void)
{
	platform_driver_unregister(&snd_smdk_driver);
}

module_exit(smdk_audio_exit);

MODULE_AUTHOR("Sangbeom Kim, <sbkim73@samsung.com>");
MODULE_DESCRIPTION("ALSA SoC SMDK WM8580 for PCM");
MODULE_LICENSE("GPL");
