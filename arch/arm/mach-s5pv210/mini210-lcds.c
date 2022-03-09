/*
 * linux/arch/arm/mach-s5pv210/mini210-lcds.c
 *
 * Copyright (c) 2011 FriendlyARM (www.arm9.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include <plat/fb.h>
#include <plat/ctouch.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>

#include <../../../drivers/video/samsung/s3cfb.h>


/* s3cfb configs for supported LCD */

static struct s3cfb_lcd wvga_w50 = {
	.width= 800,
	.height = 480,
	.p_width = 108,
	.p_height = 64,
	.bpp = 32,
	.freq = 61,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 48,
		.v_fp = 20,
		.v_fpe = 1,
		.v_bp = 20,
		.v_bpe = 1,
		.v_sw = 12,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_w500 = {
	.width = 800,
	.height = 480,
	.p_width = 108,
	.p_height = 65,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 80,
		.h_bp = 36,
		.h_sw = 10,
		.v_fp = 22,
		.v_fpe = 1,
		.v_bp = 15,
		.v_bpe = 1,
		.v_sw = 8,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_a70 = {
	.width = 800,
	.height = 480,
	.p_width = 152,
	.p_height = 90,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 48,
		.v_fp = 17,
		.v_fpe = 1,
		.v_bp = 29,
		.v_bpe = 1,
		.v_sw = 24,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_s70 = {
	.width = 800,
	.height = 480,
	.p_width = 154,
	.p_height = 96,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 80,
		.h_bp = 36,
		.h_sw = 10,
		.v_fp = 22,
		.v_fpe = 1,
		.v_bp = 15,
		.v_bpe = 1,
		.v_sw = 8,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_s70d = {
	.width = 800,
	.height = 480,
	.p_width = 154,
	.p_height = 96,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 80,
		.h_bp = 78,
		.h_sw = 10,
		.v_fp = 22,
		.v_fpe = 1,
		.v_bp = 24,
		.v_bpe = 1,
		.v_sw = 8,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_h43 = {
	.width = 480,
	.height = 272,
	.p_width = 96,
	.p_height = 54,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp =  5,
		.h_bp = 40,
		.h_sw =  2,
		.v_fp =  8,
		.v_fpe = 1,
		.v_bp =  8,
		.v_bpe = 1,
		.v_sw =  2,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_a97 = {
	.width = 1024,
	.height = 768,
	.p_width = 200,
	.p_height = 150,
	.bpp = 32,
	.freq = 62,

	.timing = {
		.h_fp = 12,
		.h_bp = 12,
		.h_sw = 4,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw =  4,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_a121 = {
	.width = 1024,
	.height = 768,
	.p_width = 200,
	.p_height = 150,
	.bpp = 32,
	.freq = 63,

	.timing = {
		.h_fp = 32,
		.h_bp = 32,
		.h_sw = 64,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw =  16,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* VGAs */
static struct s3cfb_lcd wvga_l80 = {
	.width= 640,
	.height = 480,
	.p_width = 160,
	.p_height = 120,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 35,
		.h_bp = 53,
		.h_sw = 73,
		.v_fp = 3,
		.v_fpe = 1,
		.v_bp = 29,
		.v_bpe = 1,
		.v_sw = 6,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_g10 = {
	.width= 640,
	.height = 480,
	.p_width = 213,
	.p_height = 160,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 0x3c,
		.h_bp = 0x63,
		.h_sw = 1,
		.v_fp = 0x0a,
		.v_fpe = 1,
		.v_bp = 0x22,
		.v_bpe = 1,
		.v_sw = 1,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_a56 = {
	.width= 640,
	.height = 480,
	.p_width = 112,
	.p_height = 84,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp = 16,
		.h_bp = 134,
		.h_sw = 10,
		.v_fp = 32,
		.v_fpe = 1,
		.v_bp = 11,
		.v_bpe = 1,
		.v_sw = 2,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_w101 = {
	.width= 1024,
	.height = 600,
	.p_width = 204,
	.p_height = 120,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 120,
		.v_fp = 10,
		.v_fpe = 1,
		.v_bp = 10,
		.v_bpe = 1,
		.v_sw = 12,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_w35 = {
	.width= 320,
	.height = 240,
	.p_width = 70,
	.p_height = 52,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp =  4,
		.h_bp = 70,
		.h_sw =  4,
		.v_fp =  4,
		.v_fpe = 1,
		.v_bp = 12,
		.v_bpe = 1,
		.v_sw =  4,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_p35 = {
	.width= 320,
	.height = 240,
	.p_width = 70,
	.p_height = 52,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp =  4,
		.h_bp =  4,
		.h_sw =  4,
		.v_fp =  4,
		.v_fpe = 1,
		.v_bp =  9,
		.v_bpe = 1,
		.v_sw =  4,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_p43 = {
	.width = 480,
	.height = 272,
	.p_width = 96,
	.p_height = 54,
	.bpp = 32,
	.freq = 65,

	.timing = {
		.h_fp =  5,
		.h_bp = 40,
		.h_sw =  2,
		.v_fp =  8,
		.v_fpe = 1,
		.v_bp =  9,
		.v_bpe = 1,
		.v_sw =  2,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_td35 = {
	.width= 240,
	.height = 320,
	.p_width = 52,
	.p_height = 70,
	.bpp = 32,
	.freq = 61,

	.timing = {
		.h_fp =  101,
		.h_bp =  1,
		.h_sw =  5,
		.v_fp =  1,
		.v_fpe = 1,
		.v_bp =  1,
		.v_bpe = 1,
		.v_sw =  10,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* HDMI */
static struct s3cfb_lcd hdmi_def = {
	.width = 1920,
	.height = 1080,
	.p_width = 480,
	.p_height = 320,
	.bpp = 32,
	.freq = 62,

	.timing = {
		.h_fp = 12,
		.h_bp = 12,
		.h_sw = 4,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw =  4,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* -------------------------------------------------------------- */

static struct hdmi_config {
	char *name;
	int width;
	int height;
} panel_hdmi_modes[] = {
	{ "HDMI1080P60",	1920, 1080 },
	{ "HDMI1080I60",	1920, 1080 },
	{ "HDMI1080P30",	1920, 1080 },

	{ "HDMI1080P60D",	 960,  536 },
	{ "HDMI1080I60D",	 960,  536 },
	{ "HDMI1080P30D",	 960,  536 },

	{ "HDMI720P60",		1280,  720 },
	{ "HDMI720P60D",	 640,  360 },

	{ "HDMI576P16X9",	 720,  576 },
	{ "HDMI576P16X9D",	 720,  576 },
	{ "HDMI576P4X3",	 720,  576 },
	{ "HDMI576P4X3D",	 720,  576 },

	{ "HDMI480P16X9",	 720,  480 },
	{ "HDMI480P16X9D",	 720,  480 },
	{ "HDMI480P4X3",	 720,  480 },
	{ "HDMI480P4X3D",	 720,  480 },
};


/* Try to guess LCD panel by kernel command line, or
 * using *W50* as default */

static struct {
	char *name;
	struct s3cfb_lcd *lcd;
	int ctp;
} panel_lcd_list[] = {
	{ "W50",  &wvga_w50,  0 },
	{ "A70",  &wvga_a70,  0 },
	{ "S70",  &wvga_s70,  1 },
	{ "S70D", &wvga_s70d, 1 },
	{ "S702", &wvga_s70,  CTP_AUTO  },
	{ "S701", &wvga_s70,  CTP_GT9XX },
	{ "W500", &wvga_w500, CTP_GT9XX },
	{ "H43",  &wvga_h43,  1 },
	{ "A97",  &wvga_a97,  0 },
	{ "L80",  &wvga_l80,  0 },
	{ "G10",  &wvga_g10,  0 },
	{ "A56",  &wvga_a56,  0 },
	{ "A121", &wvga_a121, 1 },
	{ "W101", &wvga_w101, 1 },
	{ "W35",  &wvga_w35,  0 },
	{ "P35",  &wvga_p35,  0 },
	{ "P43",  &wvga_p43,  0 },
	{ "TD35", &wvga_td35, 0 },
	{ "HDM",  &hdmi_def,  0 },	/* Pls keep it at last */
};

static int lcd_idx = 2;

static int __init panel_setup_lcd(char *str)
{
	char *delim;
	int i;

	delim = strchr(str, ',');
	if (delim)
		*delim++ = '\0';

	if (!strncasecmp("HDMI", str, 4)) {
		struct hdmi_config *cfg = &panel_hdmi_modes[0];
		struct s3cfb_lcd *lcd;

		lcd_idx = ARRAY_SIZE(panel_lcd_list) - 1;
		lcd = panel_lcd_list[lcd_idx].lcd;
		lcd->args = lcd_idx;

		for (i = 0; i < ARRAY_SIZE(panel_hdmi_modes); i++, cfg++) {
			if (!strcasecmp(cfg->name, str)) {
				lcd->width = cfg->width;
				lcd->height = cfg->height;
				goto __ret;
			}
		}
	}

	for (i = 0; i < ARRAY_SIZE(panel_lcd_list); i++) {
		if (!strcasecmp(panel_lcd_list[i].name, str)) {
			lcd_idx = i;
			break;
		}
	}

__ret:
	if (panel_lcd_list[lcd_idx].ctp >= CTP_AUTO)
		panel_set_touch_id(panel_lcd_list[lcd_idx].ctp);

	printk("MINI210: %s selected\n", panel_lcd_list[lcd_idx].name);
	return 0;
}
early_param("lcd", panel_setup_lcd);

struct s3cfb_lcd *mini210_get_lcd(void)
{
	return panel_lcd_list[lcd_idx].lcd;
}

void mini210_get_lcd_res(int *w, int *h)
{
	struct s3cfb_lcd *lcd = panel_lcd_list[lcd_idx].lcd;

	if (w)
		*w = lcd->width;
	if (h)
		*h = lcd->height;

	return;
}
EXPORT_SYMBOL(mini210_get_lcd_res);

/* -------------------------------------------------------------- */

/* Touch panel type */
static unsigned int ctp_type = CTP_NONE;

unsigned int panel_get_touch_id(void)
{
	if (panel_lcd_list[lcd_idx].ctp)
		return ctp_type;
	else
		return CTP_NONE;
}

void panel_set_touch_id(int type)
{
	if (type > 0 && type < CTP_MAX) {
		ctp_type = type;
	}
}

static int __init panel_setup_touch_id(char *str)
{
	unsigned int val;
	char *p = str, *end;

	val = simple_strtoul(p, &end, 10);
	if (end <= p) {
		return 1;
	}

	if (ctp_type < CTP_AUTO && val < CTP_MAX) {
		ctp_type = val;
	} else if (val == CTP_NONE) {
		ctp_type = CTP_NONE;
	}

	return 1;
}
__setup("ctp=", panel_setup_touch_id);

unsigned int mini210_get_ctp(void)
{
	return ctp_type;
}
EXPORT_SYMBOL(mini210_get_ctp);

