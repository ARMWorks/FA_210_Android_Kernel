/* linux/arch/arm/plat-s5p/devs.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Base S5P platform device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>

#include <plat/devs.h>
#include <plat/fimc.h>
#include <plat/csis.h>
#include <plat/media.h>
#include <plat/jpeg.h>
#include <mach/media.h>

#if defined(CONFIG_VIDEO_FIMC) || defined(CONFIG_CPU_FREQ) /* TODO: use existing dev */
static struct resource s3c_fimc0_resource[] = {
	[0] = {
		.start	= S5P_PA_FIMC0,
		.end	= S5P_PA_FIMC0 + S5P_SZ_FIMC0 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_FIMC0,
		.end	= IRQ_FIMC0,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_fimc0 = {
	.name		= "s3c-fimc",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(s3c_fimc0_resource),
	.resource	= s3c_fimc0_resource,
};

static struct s3c_platform_fimc default_fimc0_data __initdata = {
	.default_cam	= CAMERA_PAR_A,
	.hw_ver	= 0x45,
};

void __init s3c_fimc0_set_platdata(struct s3c_platform_fimc *pd)
{
	struct s3c_platform_fimc *npd;

	if (!pd)
		pd = &default_fimc0_data;

	npd = kmemdup(pd, sizeof(struct s3c_platform_fimc), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);
	else {
		if (!npd->cfg_gpio)
			npd->cfg_gpio = s3c_fimc0_cfg_gpio;

		if (!npd->clk_on)
			npd->clk_on = s3c_fimc_clk_on;

		if (!npd->clk_off)
			npd->clk_off = s3c_fimc_clk_off;

		npd->hw_ver = 0x45;

		/* starting physical address of memory region */
		npd->pmem_start = s5p_get_media_memory_bank(S5P_MDEV_FIMC0, 1);
		/* size of memory region */
		npd->pmem_size = s5p_get_media_memsize_bank(S5P_MDEV_FIMC0, 1);

		s3c_device_fimc0.dev.platform_data = npd;
	}
}

static struct resource s3c_fimc1_resource[] = {
	[0] = {
		.start	= S5P_PA_FIMC1,
		.end	= S5P_PA_FIMC1 + S5P_SZ_FIMC1 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_FIMC1,
		.end	= IRQ_FIMC1,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_fimc1 = {
	.name		= "s3c-fimc",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(s3c_fimc1_resource),
	.resource	= s3c_fimc1_resource,
};

static struct s3c_platform_fimc default_fimc1_data __initdata = {
	.default_cam	= CAMERA_PAR_A,
	.hw_ver	= 0x50,
};

void __init s3c_fimc1_set_platdata(struct s3c_platform_fimc *pd)
{
	struct s3c_platform_fimc *npd;

	if (!pd)
		pd = &default_fimc1_data;

	npd = kmemdup(pd, sizeof(struct s3c_platform_fimc), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);
	else {
		if (!npd->cfg_gpio)
			npd->cfg_gpio = s3c_fimc1_cfg_gpio;

		if (!npd->clk_on)
			npd->clk_on = s3c_fimc_clk_on;

		if (!npd->clk_off)
			npd->clk_off = s3c_fimc_clk_off;

		npd->hw_ver = 0x50;

		/* starting physical address of memory region */
		npd->pmem_start = s5p_get_media_memory_bank(S5P_MDEV_FIMC1, 1);
		/* size of memory region */
		npd->pmem_size = s5p_get_media_memsize_bank(S5P_MDEV_FIMC1, 1);

		s3c_device_fimc1.dev.platform_data = npd;
	}
}

static struct resource s3c_fimc2_resource[] = {
	[0] = {
		.start	= S5P_PA_FIMC2,
		.end	= S5P_PA_FIMC2 + S5P_SZ_FIMC2 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_FIMC2,
		.end	= IRQ_FIMC2,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_fimc2 = {
	.name		= "s3c-fimc",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(s3c_fimc2_resource),
	.resource	= s3c_fimc2_resource,
};

static struct s3c_platform_fimc default_fimc2_data __initdata = {
	.default_cam	= CAMERA_PAR_A,
	.hw_ver	= 0x45,
};

void __init s3c_fimc2_set_platdata(struct s3c_platform_fimc *pd)
{
	struct s3c_platform_fimc *npd;

	if (!pd)
		pd = &default_fimc2_data;

	npd = kmemdup(pd, sizeof(struct s3c_platform_fimc), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);
	else {
		if (!npd->cfg_gpio)
			npd->cfg_gpio = s3c_fimc2_cfg_gpio;

		if (!npd->clk_on)
			npd->clk_on = s3c_fimc_clk_on;

		if (!npd->clk_off)
			npd->clk_off = s3c_fimc_clk_off;

		npd->hw_ver = 0x45;

		/* starting physical address of memory region */
		npd->pmem_start = s5p_get_media_memory_bank(S5P_MDEV_FIMC2, 1);
		/* size of memory region */
		npd->pmem_size = s5p_get_media_memsize_bank(S5P_MDEV_FIMC2, 1);

		s3c_device_fimc2.dev.platform_data = npd;
	}
}
static struct resource s3c_csis_resource[] = {
	[0] = {
		.start	= S5P_PA_CSIS,
		.end	= S5P_PA_CSIS + S5P_SZ_CSIS - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_MIPI_CSIS,
		.end	= IRQ_MIPI_CSIS,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_csis = {
	.name		= "s5p-mipi-csis",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_csis_resource),
	.resource	= s3c_csis_resource,
};

static struct s3c_platform_csis default_csis_data __initdata = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_csis",
	.clk_rate	= 166000000,
};

void __init s3c_csis_set_platdata(struct s3c_platform_csis *pd)
{
	struct s3c_platform_csis *npd;

	if (!pd)
		pd = &default_csis_data;

	npd = kmemdup(pd, sizeof(struct s3c_platform_csis), GFP_KERNEL);
	if (!npd) {
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);
		return;
	}

#if defined(CONFIG_S5PV210_SETUP_CSIS)
	npd->cfg_gpio = s3c_csis_cfg_gpio;
	npd->cfg_phy_global = s3c_csis_cfg_phy_global;
	npd->clk_on = s3c_csis_clk_on;
	npd->clk_off = s3c_csis_clk_off;
#endif

	s3c_device_csis.dev.platform_data = npd;
}
#endif

/* JPEG controller */
static struct s3c_platform_jpeg default_jpeg_data __initdata = {
	.max_main_width		= 800, //2560,
	.max_main_height	= 480, //1920,
	.max_thumb_width	= 320, //0,
	.max_thumb_height	= 240, //0,
};

void __init s3c_jpeg_set_platdata(struct s3c_platform_jpeg *pd)
{
	struct s3c_platform_jpeg *npd;

	if (!pd)
		pd = &default_jpeg_data;

	npd = kmemdup(pd, sizeof(struct s3c_platform_jpeg), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);
	else
		s3c_device_jpeg.dev.platform_data = npd;
}

static struct resource s3c_jpeg_resource[] = {
	[0] = {
		.start	= S5PV210_PA_JPEG,
		.end	= S5PV210_PA_JPEG + S5PV210_SZ_JPEG - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_JPEG,
		.end	= IRQ_JPEG,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_jpeg = {
	.name		= "s3c-jpg",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(s3c_jpeg_resource),
	.resource	= s3c_jpeg_resource,
};

/* G3D */
struct platform_device s3c_device_g3d = {
	.name		= "pvrsrvkm",
	.id		= -1,
};

struct platform_device s3c_device_lcd = {
	.name		= "s3c_lcd",
	.id		= -1,
};

static struct resource s3c_g2d_resources[] = {
	[0] = {
		.start	= S5P_PA_FIMG2D,
		.end	= S5P_PA_FIMG2D + S5P_SZ_FIMG2D - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_2D,
		.end	= IRQ_2D,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_g2d = {
	.name		= "s3c-g2d",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(s3c_g2d_resources),
	.resource	= s3c_g2d_resources,
};

/* TVOUT interface */
static struct resource s5p_tvout_resources[] = {
	[0] = {
		.start	= S5P_PA_TVENC,
		.end	= S5P_PA_TVENC + S5P_SZ_TVENC - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5P_PA_VP,
		.end	= S5P_PA_VP + S5P_SZ_VP - 1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= S5P_PA_MIXER,
		.end	= S5P_PA_MIXER + S5P_SZ_MIXER - 1,
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.start	= S5P_PA_HDMI,
		.end	= S5P_PA_HDMI + S5P_SZ_HDMI - 1,
		.flags	= IORESOURCE_MEM,
	},
	[4] = {
		.start	= S5P_I2C_HDMI_PHY,
		.end	= S5P_I2C_HDMI_PHY + S5P_I2C_HDMI_SZ_PHY - 1,
		.flags	= IORESOURCE_MEM,
	},
	[5] = {
		.start	= IRQ_MIXER,
		.end	= IRQ_MIXER,
		.flags	= IORESOURCE_IRQ,
	},
	[6] = {
		.start	= IRQ_HDMI,
		.end	= IRQ_HDMI,
		.flags	= IORESOURCE_IRQ,
	},
	[7] = {
		.start	= IRQ_TVENC,
		.end	= IRQ_TVENC,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device s5p_device_tvout = {
	.name		= "s5p-tvout",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(s5p_tvout_resources),
	.resource	= s5p_tvout_resources,
};

/* CEC */
static struct resource s5p_cec_resources[] = {
	[0] = {
		.start	= S5P_PA_CEC,
		.end	= S5P_PA_CEC + S5P_SZ_CEC - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_CEC,
		.end	= IRQ_CEC,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device s5p_device_cec = {
	.name		= "s5p-cec",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(s5p_cec_resources),
	.resource	= s5p_cec_resources,
};

/* HPD */
struct platform_device s5p_device_hpd = {
	.name		= "s5p-hpd",
	.id			= -1,
};

