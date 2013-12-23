# Driver code taken from:
# https://github.com/5victor/linux-tiny210v2/blob/master/sound/soc/s5pv2xx/mini210_wm8960.c

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "s5pc1xx-i2s.h"
#include "../codecs/wm8960.h"

static const struct snd_soc_dapm_widget wm8960_dapm_widgets_cpt[] = {
  SND_SOC_DAPM_MIC("Mic Jack", NULL),
  SND_SOC_DAPM_LINE("Line Input 3 (FM)", NULL),
};

static const struct snd_soc_dapm_widget wm8960_dapm_widgets_cpt2[] = {
  SND_SOC_DAPM_HP("Headphone Jack", NULL),
  SND_SOC_DAPM_SPK("Speaker_L", NULL),
  SND_SOC_DAPM_SPK("Speaker_R", NULL),
};

static const struct snd_soc_dapm_route audio_map_tx[] = {
  {"Headphone Jack", NULL, "HP_L"},
  {"Headphone Jack", NULL, "HP_R"},
  {"Speaker_L", NULL, "SPK_LP"},
  {"Speaker_L", NULL, "SPK_LN"},
  {"Speaker_R", NULL, "SPK_RP"},
  {"Speaker_R", NULL, "SPK_RN"},
  {"LINPUT1", NULL, "MICB"},
  {"MICB", NULL, "Mic Jack"},
};

static int mini210_wm8960_init(struct snd_soc_pcm_runtime *rtd)
{
  struct snd_soc_codec *codec = rtd->codec;
  struct snd_soc_dapm_context *dapm = &codec->dapm;

  snd_soc_dapm_nc_pin(dapm, "RINPUT1");
  snd_soc_dapm_nc_pin(dapm, "LINPUT2");
  snd_soc_dapm_nc_pin(dapm, "RINPUT2");
  snd_soc_dapm_nc_pin(dapm, "OUT3");
  snd_soc_dapm_new_controls(dapm, wm8960_dapm_widgets_cpt,
      ARRAY_SIZE(wm8960_dapm_widgets_cpt));
  snd_soc_dapm_new_controls(dapm, wm8960_dapm_widgets_cpt2,
      ARRAY_SIZE(wm8960_dapm_widgets_cpt2));

  snd_soc_dapm_add_routes(dapm, audio_map_tx, ARRAY_SIZE(audio_map_tx));

  snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
  snd_soc_dapm_enable_pin(dapm, "Mic Jack");
  snd_soc_dapm_enable_pin(dapm, "Speaker_L");
  snd_soc_dapm_enable_pin(dapm, "Speaker_R");
  snd_soc_dapm_enable_pin(dapm, "Line Input 3 (FM)");

  snd_soc_dapm_sync(dapm);
  return 0;
}

static int mini210_hw_params(struct snd_pcm_substream *substream,
  struct snd_pcm_hw_params *params)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
  struct snd_soc_dai *codec_dai = rtd->codec_dai;

  int ret, pre, pre_n, rfs, bfs, codec_pre;

/*  if (params->0x24) {
    var1 = 0;
  } else {
    if (params->0x28) {
      goto err0x15;
    }
    var1 = 0x20;  
  }
????????params_format???
*/
        switch (params_format(params)) {
        case SNDRV_PCM_FORMAT_S24_LE:
        case SNDRV_PCM_FORMAT_S20_3LE:
                bfs = 48;
                break;
        case SNDRV_PCM_FORMAT_S16_LE:
                bfs = 32;
                break;
        default:
    printk(KERN_INFO "%s:params_format(bfs) INVAL\n", __func__);
                return -EINVAL;
        }

  switch (params_rate(params)) {
  case 12000:
  case 11025:
  case 8000:
    if (bfs == 0x30)
      rfs = 0x300;
    else
      rfs = 0x200;

    break;

  case 22050:
  case 16000:
  case 44100:
  case 32000:
  case 96000:
  case 88200:
  case 48000:
  case 24000:
    if (bfs == 0x30)
      rfs = 0x180;
    else
      rfs = 0x100;

    break;
  case 64000:
    rfs = 0x180;
    break;
  
  default:
                printk(KERN_INFO "%s:params_rate(rfs) INVAL\n", __func__);
                return -EINVAL; 
  }

        ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
                                         | SND_SOC_DAIFMT_NB_NF
                                         | SND_SOC_DAIFMT_CBS_CFS);
  if (ret < 0) {
    printk(KERN_INFO "%s:Codec DAI configuration error, %d\n",
      __func__, ret);
    goto exit;
  }

        ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
                                         | SND_SOC_DAIFMT_NB_NF
                                         | SND_SOC_DAIFMT_CBS_CFS);
  if (ret < 0) {
    printk(KERN_INFO "%s:AP configuration error, %d\n",
      __func__, ret);
    goto exit;
  }

  ret = snd_soc_dai_set_sysclk(cpu_dai, S3C64XX_CLKSRC_CDCLK,
      params_rate(params), SND_SOC_CLOCK_OUT);

  if (ret < 0) {
    printk(KERN_INFO "%s: AP sysclk CDCLK setting error,%d\n",
      __func__, ret);
    goto exit;
  }

  ret = snd_soc_dai_set_sysclk(cpu_dai, 0x400, params_rate(params),
        SND_SOC_CLOCK_OUT);

  if (ret < 0) {
    printk(KERN_INFO "%s: AP sysclk CLKAUDIO setting error,%d\n",
      __func__, ret);
    goto exit;
  }

  if (params_rate(params) & 0xF)
    pre_n = 0x4099990;
  else
    pre_n = 0x2ee0000;

  ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_SYSCLKDIV, 0);

  if (ret < 0) {
    printk(KERN_INFO "%s: Codec SYSCLKDIV setting error, %d\n",
      __func__, ret);
    goto exit;
  }

  ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, 0);

  if (ret < 0) {
    printk(KERN_INFO "%s: Codec DACDIV setting error, %d\n",
                        __func__, ret);
                goto exit;
  }

  codec_pre = params_rate(params) * rfs;
  snd_soc_dai_set_clkdiv(codec_dai, WM8960_DCLKDIV, codec_pre);

  pre = pre_n / codec_pre - 1;
  ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_PRESCALER, pre);

  if (ret < 0) {
    printk(KERN_INFO "%s: AP prescalar setting error,%d\n",
      __func__, ret);
    goto exit;
  }

  ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_RCLK, rfs);
  if (ret < 0) {
    printk(KERN_INFO "%s: AP RFS setting error, %d\n",
      __func__, ret);
    goto exit;
  }

  ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_BCLK, bfs);
  if (ret < 0) {
    printk(KERN_INFO "%s: AP BFS setting error, %d\n",
      __func__, ret);
    goto exit;
  }
  
  return 0;

exit:
  return ret;
}

static struct snd_soc_ops mini210_ops = {
  .hw_params  = mini210_hw_params,
};

static struct snd_soc_dai_link mini210_dai = {
  .name    = "MINI210",
  .stream_name  = "WM8960 HiFi",
  .codec_name  = "wm8960-codec.0-001a",
  .platform_name  = "samsung-audio",
  .cpu_dai_name  = "samsung-i2s.0",
  .codec_dai_name  = "wm8960-hifi",
  .init    = mini210_wm8960_init,
  .ops    = &mini210_ops,
};

static struct snd_soc_card snd_soc_mini210 = {
  .name = "mini210",
  .dai_link = &mini210_dai,
  .num_links = 1,
};

static struct platform_device *mini210_snd_device;

static int __init mini210_soc_init(void)
{
  int ret;

  if (!machine_is_mini210())
    return -ENODEV;

  mini210_snd_device = platform_device_alloc("soc-audio", -1);
  if (!mini210_snd_device) {
    printk(KERN_INFO "Platform device allocation failed\n");
    return -ENOMEM;
  }

  platform_set_drvdata(mini210_snd_device, &snd_soc_mini210);

  ret = platform_device_add(mini210_snd_device);

  if (ret)
    goto err1;

  return 0;

err1:
  printk(KERN_INFO "Unable to add platform device\n");
  platform_device_put(mini210_snd_device);
  return ret;
}

static void __exit mini210_soc_exit(void)
{
  platform_device_unregister(mini210_snd_device);
}

module_init(mini210_soc_init);
module_exit(mini210_soc_exit);

MODULE_LICENSE("GPL");
