/* linux/arch/arm/mach-s5pv210/mach-mini210.c
 *
 * Copyright (c) 2011 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/max8698.h>
#include <mach/power-domain.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/console.h>
#include <linux/sysdev.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>
#include <linux/usb/ch9.h>
#include <linux/spi/spi.h>
#include <linux/gpio_keys.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>
#include <../../../drivers/video/samsung/s3cfb.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/spi-clocks.h>
#include <mach/regs-fb.h>

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/s3c64xx-spi.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/ata.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/mfc.h>
#include <plat/s5p-time.h>
#include <plat/sdhci.h>
#include <plat/fimc.h>
#include <plat/csis.h>
#include <plat/jpeg.h>
#include <plat/clock.h> 
#include <plat/regs-otg.h>
#include <plat/otg.h>
#include <plat/ehci.h>
#include <plat/ohci.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <plat/media.h>
#include <mach/media.h>
#include <mach/gpio-smdkc110.h>

extern struct s3cfb_lcd *mini210_get_lcd(void);

#ifdef CONFIG_TOUCHSCREEN_EGALAX
#include <linux/i2c/egalax.h>
#define EETI_TS_DEV_NAME		"egalax_i2c"

static struct egalax_i2c_platform_data egalax_platdata = {
	.gpio_int		= EGALAX_IRQ,
	.gpio_en		= NULL,
	.gpio_rst		= NULL,
};
#endif

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define MINI210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define MINI210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define MINI210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg mini210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
};

#if defined(CONFIG_REGULATOR_MAX8698)
/* LDO */
static struct regulator_consumer_supply mini210_ldo3_consumer[] = {
	REGULATOR_SUPPLY("pd_io", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_io", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_io", "s5p-ehci"),
};

static struct regulator_consumer_supply mini210_ldo5_consumer[] = {
	REGULATOR_SUPPLY("AVDD", "0-001b"),
	REGULATOR_SUPPLY("DVDD", "0-001b"),
};

static struct regulator_consumer_supply mini210_ldo8_consumer[] = {
	REGULATOR_SUPPLY("pd_core", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_core", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_core", "s5p-ehci"),
};

static struct regulator_init_data mini210_ldo2_data = {
	.constraints	= {
		.name		= "VALIVE_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data mini210_ldo3_data = {
	.constraints	= {
		.name		= "VUOTG_D+VUHOST_D_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(mini210_ldo3_consumer),
	.consumer_supplies	= mini210_ldo3_consumer,
};

static struct regulator_init_data mini210_ldo4_data = {
	.constraints	= {
		.name		= "V_MIPI_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data mini210_ldo5_data = {
	.constraints	= {
		.name		= "VMMC+VEXT_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(mini210_ldo5_consumer),
	.consumer_supplies	= mini210_ldo5_consumer,
};

static struct regulator_init_data mini210_ldo6_data = {
	.constraints	= {
		.name		= "VCC_2.6V",
		.min_uV		= 2600000,
		.max_uV		= 2600000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	 = {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data mini210_ldo7_data = {
	.constraints	= {
		.name		= "VDAC_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data mini210_ldo8_data = {
	.constraints	= {
		.name		= "VUOTG_A+VUHOST_A_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(mini210_ldo8_consumer),
	.consumer_supplies	= mini210_ldo8_consumer,
};

static struct regulator_init_data mini210_ldo9_data = {
	.constraints	= {
		.name		= "VADC+VSYS+VKEY_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

/* BUCK */
static struct regulator_consumer_supply mini210_buck1_consumer =
	REGULATOR_SUPPLY("vddarm", NULL);

static struct regulator_consumer_supply mini210_buck2_consumer =
	REGULATOR_SUPPLY("vddint", NULL);

static struct regulator_init_data mini210_buck1_data = {
	.constraints	= {
		.name		= "VCC_ARM",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1250000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &mini210_buck1_consumer,
};

static struct regulator_init_data mini210_buck2_data = {
	.constraints	= {
		.name		= "VCC_INTERNAL",
		.min_uV		= 950000,
		.max_uV		= 1200000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1100000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &mini210_buck2_consumer,
};

static struct regulator_init_data mini210_buck3_data = {
	.constraints	= {
		.name		= "VCC_MEM",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.always_on	= 1,
		.apply_uV	= 1,
		.state_mem	= {
			.uV	= 1800000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

static struct max8698_regulator_data mini210_regulators[] = {
	{ MAX8698_LDO2,  &mini210_ldo2_data },
	{ MAX8698_LDO3,  &mini210_ldo3_data },
	{ MAX8698_LDO4,  &mini210_ldo4_data },
	{ MAX8698_LDO5,  &mini210_ldo5_data },
	{ MAX8698_LDO6,  &mini210_ldo6_data },
	{ MAX8698_LDO7,  &mini210_ldo7_data },
	{ MAX8698_LDO8,  &mini210_ldo8_data },
	{ MAX8698_LDO9,  &mini210_ldo9_data },
	{ MAX8698_BUCK1, &mini210_buck1_data },
	{ MAX8698_BUCK2, &mini210_buck2_data },
	{ MAX8698_BUCK3, &mini210_buck3_data },
};

static struct max8698_platform_data mini210_max8698_pdata = {
	.num_regulators	= ARRAY_SIZE(mini210_regulators),
	.regulators		= mini210_regulators,

	/* 1GHz default voltage */
	.dvsarm1		= 0xa,  /* 1.25v */
	.dvsarm2		= 0x9,  /* 1.20V */
	.dvsarm3		= 0x6,  /* 1.05V */
	.dvsarm4		= 0x4,  /* 0.95V */
	.dvsint1		= 0x7,  /* 1.10v */
	.dvsint2		= 0x5,  /* 1.00V */

	.set1			= S5PV210_GPH1(6),
	.set2			= S5PV210_GPH1(7),
	.set3			= S5PV210_GPH0(4),
};
#endif

#if defined(CONFIG_S5PV210_SETUP_IDE)
static struct s3c_ide_platdata mini210_ide_pdata __initdata = {
	.setup_gpio		= s5pv210_ide_setup_gpio,
};
#endif

#ifdef CONFIG_KEYBOARD_GPIO
static struct gpio_keys_button gpio_buttons[] = {
	{
		.gpio		= S5PV210_GPH2(0),
		.code		= 158,
		.desc		= "BACK",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH2(1),
		.code		= 102,
		.desc		= "HOME",
		.active_low	= 1,
		.wakeup		= 1,
	}, {
		.gpio		= S5PV210_GPH2(2),
		.code		= 139,
		.desc		= "MENU",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH2(3),
		.code		= 232,
		.desc		= "DPAD_CENTER",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH3(0),
		.code		= 105,
		.desc		= "DPAD_LEFT",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH3(1),
		.code		= 108,
		.desc		= "DPAD_DOWN",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH3(2),
		.code		= 103,
		.desc		= "DPAD_UP",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH3(3),
		.code		= 106,
		.desc		= "DPAD_RIGHT",
		.active_low	= 1,
		.wakeup		= 0,
	}, {
		.gpio		= S5PV210_GPH1(7),
		.code		= 102,
		.desc		= "HOME",
		.active_low	= 1,
		.wakeup		= 1,
	}
};

static struct gpio_keys_platform_data gpio_button_data = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
};

static struct platform_device s3c_device_gpio_btns = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &gpio_button_data,
	}
};
#endif

static struct platform_device s3c_device_1wire = {
	.name			= "mini210_1wire",
	.id				= -1,
	.num_resources	= 0,
};

static struct platform_device mini210_device_adc = {
	.name			= "mini210_adc",
	.id				= -1,
	.num_resources	= 0,
};

#if defined(CONFIG_KEYBOARD_SAMSUNG)
static uint32_t mini210_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_1), KEY(0, 4, KEY_2), KEY(0, 5, KEY_3),
	KEY(0, 6, KEY_4), KEY(0, 7, KEY_5),
	KEY(1, 3, KEY_A), KEY(1, 4, KEY_B), KEY(1, 5, KEY_C),
	KEY(1, 6, KEY_D), KEY(1, 7, KEY_E), KEY(7, 1, KEY_LEFTBRACE)
};

static struct matrix_keymap_data mini210_keymap_data __initdata = {
	.keymap			= mini210_keymap,
	.keymap_size	= ARRAY_SIZE(mini210_keymap),
};

static struct samsung_keypad_platdata mini210_keypad_data __initdata = {
	.keymap_data	= &mini210_keymap_data,
	.rows			= 8,
	.cols			= 8,
};
#endif

#ifdef CONFIG_REGULATOR_FIXED_VOLTAGE
static struct regulator_consumer_supply mini210_b_pwr_5v_consumers[] = {
	{
		/* WM8580 */
		.supply		= "PVDD",
		.dev_name	= "0-001b",
	},
};

static struct regulator_init_data mini210_b_pwr_5v_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(mini210_b_pwr_5v_consumers),
	.consumer_supplies	= mini210_b_pwr_5v_consumers,
};

static struct fixed_voltage_config mini210_b_pwr_5v_pdata = {
	.supply_name	= "B_PWR_5V",
	.microvolts		= 5000000,
	.gpio			= -EINVAL,
	.init_data		= &mini210_b_pwr_5v_data,
};

static struct platform_device mini210_b_pwr_5v = {
	.name			= "reg-fixed-voltage",
	.id				= -1,
	.dev = {
		.platform_data = &mini210_b_pwr_5v_pdata,
	},
};
#endif

#ifdef CONFIG_TOUCHSCREEN_EGALAX
static struct i2c_gpio_platform_data i2c5_platdata = {
	.sda_pin		= S5PV210_GPB(6),
	.scl_pin		= S5PV210_GPB(7),
	.udelay			= 2,
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0.
};

struct platform_device s3c_device_i2c5 = {
	.name			= "i2c-gpio",
	.id				= 5,
	.dev.platform_data	= &i2c5_platdata,
};

static struct i2c_board_info i2c_devs5[] __initdata = {
	{
		I2C_BOARD_INFO(EETI_TS_DEV_NAME, 0x04),
		.platform_data	= &egalax_platdata,
		.irq		= IRQ_EINT6,
	},
};
#endif

/* Multimedia support */
#define LCD_WIDTH				800
#define LCD_HEIGHT				480
#define BYTES_PER_PIXEL			4
#define NUM_BUFFER_OVLY		\
		(CONFIG_FB_S3C_NUM_OVLY_WIN * CONFIG_FB_S3C_NUM_BUF_OVLY_WIN)
#define NUM_BUFFER			\
		(CONFIG_FB_S3C_NR_BUFFERS + NUM_BUFFER_OVLY)
#define PXL2FIMD(pixels)	\
		((pixels) * BYTES_PER_PIXEL * NUM_BUFFER)

#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0		( 6144 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1		(   16 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2		( 6144 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0		(36864 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1		(36864 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD		PXL2FIMD(LCD_WIDTH * LCD_HEIGHT)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG		( 8192 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D		( 8192 * SZ_1K)
#define S5PV210_VIDEO_SAMSUNG_MEMSIZE_TEXSTREAM	( 4096 * SZ_1K)
#define S5PV210_ANDROID_PMEM_MEMSIZE_PMEM		( 5550 * SZ_1K)
#define S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1	( 1800 * SZ_1K)	//3300
#define S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_ADSP	( 1500 * SZ_1K)

static struct s5p_media_device mini210_media_devs[] = {
#if defined(CONFIG_VIDEO_MFC50)
	{
		.id			= S5P_MDEV_MFC,
		.name		= "mfc",
		.bank		= 0,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0,
		.paddr		= 0,
	},
	{
		.id			= S5P_MDEV_MFC,
		.name		= "mfc",
		.bank		= 1,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1,
		.paddr		= 0,
	},
#endif
#if defined(CONFIG_VIDEO_FIMC)
	{
		.id			= S5P_MDEV_FIMC0,
		.name		= "fimc0",
		.bank		= 1,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0,
		.paddr		= 0,
	},
	{
		.id			= S5P_MDEV_FIMC1,
		.name		= "fimc1",
		.bank		= 1,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1,
		.paddr		= 0,
	},
	{
		.id			= S5P_MDEV_FIMC2,
		.name		= "fimc2",
		.bank		= 1,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2,
		.paddr		= 0,
	},
#endif
#if defined(CONFIG_VIDEO_JPEG_V2)
	{
		.id			= S5P_MDEV_JPEG,
		.name		= "jpeg",
		.bank		= 0,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG,
		.paddr		= 0,
	},
#endif
	{
		.id			= S5P_MDEV_FIMD,
		.name		= "fimd",
		.bank		= 1,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD,
		.paddr		= 0,
	},
#if defined(CONFIG_VIDEO_G2D)
	{
		.id			= S5P_MDEV_G2D,
		.name		= "g2d",
		.bank		= 0,
		.memsize	= S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D,
		.paddr		= 0,
	},
#endif
#if defined(CONFIG_ANDROID_PMEM)
	{
		.id			= S5P_MDEV_PMEM_GPU1,
		.name		= "pmem_gpu1",
		.bank		= 0, /* OneDRAM */
		.memsize	= S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1,
		.paddr		= 0,
	},
#endif
};

static void mini210_fixup_bootmem(int id, unsigned int size) {
	int i;

	for (i = 0; i < ARRAY_SIZE(mini210_media_devs); i++) {
		if (mini210_media_devs[i].id == id) {
			mini210_media_devs[i].memsize = size;
		}
	}
}

#if defined(CONFIG_ANDROID_PMEM)
static struct android_pmem_platform_data pmem_pdata = {
	.name			= "pmem",
	.no_allocator	= 1,
	.cached			= 1,
	.start			= 0,
	.size			= 0,
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name			= "pmem_gpu1",
	.no_allocator	= 1,
	.cached			= 1,
	.buffered		= 1,
	.start			= 0,
	.size			= 0,
};

static struct android_pmem_platform_data pmem_adsp_pdata = {
	.name			= "pmem_adsp",
	.no_allocator	= 1,
	.cached			= 1,
	.buffered		= 1,
	.start			= 0,
	.size			= 0,
};      

static struct platform_device pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &pmem_adsp_pdata },
};

static void __init android_pmem_set_platdata(void)
{
#if	0
	pmem_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM, 0);
	pmem_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM, 0);
#endif

	pmem_gpu1_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_GPU1, 0);
	pmem_gpu1_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_GPU1, 0);

#if	0
	pmem_adsp_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_ADSP, 0);
	pmem_adsp_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_ADSP, 0);
#endif
}
#endif

/* LCD on/off or backlight control */
#define MINI210_BL_GPIO			S5PV210_GPD0(1)
#define MINI210_BL_PWM			1

static void mini210_lcd_set_power(struct plat_lcd_data *pd,
		unsigned int power)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	gpio_request(MINI210_BL_GPIO, "GPD0");
	gpio_direction_output(MINI210_BL_GPIO, power);

	gpio_free(MINI210_BL_GPIO);
#endif
}

static struct plat_lcd_data mini210_lcd_data = {
	.set_power		= mini210_lcd_set_power,
};

static struct platform_device mini210_lcd_dev = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &mini210_lcd_data,
};

static void lcd_cfg_gpio(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
	}

	/* mDNIe SEL: why we shall write 0x2 ? */
	writel(0x2, S5P_MDNIE_SEL);

	/* drive strength to 2x ....(max for smdkv210) */
	writel(0xaaaaaaaa, S5PV210_GPF0_BASE + 0xc);
	writel(0xaaaaaaaa, S5PV210_GPF1_BASE + 0xc);
	writel(0xaaaaaaaa, S5PV210_GPF2_BASE + 0xc);
	writel(0x000000aa, S5PV210_GPF3_BASE + 0xc);
}

static int lcd_backlight_on(struct platform_device *pdev)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(MINI210_BL_GPIO, "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
				"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(MINI210_BL_GPIO, 1);
	gpio_free(MINI210_BL_GPIO);
#endif

	return 0;
}

static int lcd_backlight_off(struct platform_device *pdev, int onoff)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(MINI210_BL_GPIO, "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
				"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(MINI210_BL_GPIO, 0);
	gpio_free(MINI210_BL_GPIO);
#endif

	return 0;
}

static int lcd_reset_lcd(struct platform_device *pdev)
{
	return 0;
}

static struct s3c_platform_fb mini210_fb_data __initdata = {
	.hw_ver			= 0x62,
	.clk_name       = "sclk_fimd",
	.nr_wins		= 5,
	.default_win	= CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap			= FB_SWAP_WORD | FB_SWAP_HWORD,

	.cfg_gpio		= lcd_cfg_gpio,
	.backlight_on	= lcd_backlight_on,
	.backlight_onoff= lcd_backlight_off,
	.reset_lcd		= lcd_reset_lcd,
};

#ifdef CONFIG_BACKLIGHT_PWM
static int mini210_backlight_init(struct device *dev)
{
	int ret;

	ret = gpio_request(MINI210_BL_GPIO, "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPD for backlight\n");
		return ret;
	}

	/* Configure GPIO pin as XpwmTOUT */
	s3c_gpio_cfgpin(MINI210_BL_GPIO, S3C_GPIO_SFN(2));

	return 0;
}

static void mini210_backlight_exit(struct device *dev)
{
	s3c_gpio_cfgpin(MINI210_BL_GPIO, S3C_GPIO_OUTPUT);
	gpio_free(MINI210_BL_GPIO);
}

static struct platform_pwm_backlight_data mini210_backlight_data = {
	.pwm_id			= MINI210_BL_PWM,
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 78770,
	.init		= mini210_backlight_init,
	.exit		= mini210_backlight_exit,
};

static struct platform_device mini210_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent		= &s3c_device_timer[MINI210_BL_PWM].dev,
		.platform_data	= &mini210_backlight_data,
	},
};
#endif

/* NAND Controller */
static struct resource s3c_nand_resource[] = {
	[0] = {
		.start	= S5PV210_PA_NAND,
		.end	= S5PV210_PA_NAND + S5PV210_SZ_NAND - 1,
		.flags	= IORESOURCE_MEM,
	}
};

struct platform_device s3c_device_nand = {
	.name		= "s5pv210-nand",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(s3c_nand_resource),
	.resource	= s3c_nand_resource,
};

#ifdef CONFIG_DM9000
#include <linux/dm9000.h>

/* physical address for dm9000a ...kgene.kim@samsung.com */
#define S5PV210_PA_DM9000_A		(0x88001000)
#define S5PV210_PA_DM9000_F		(S5PV210_PA_DM9000_A + 0x300C)

static struct resource mini210_dm9000_resources[] = {
	[0] = {
		.start	= S5PV210_PA_DM9000_A,
		.end	= S5PV210_PA_DM9000_A + SZ_1K*4 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5PV210_PA_DM9000_F,
		.end	= S5PV210_PA_DM9000_F + SZ_1K*4 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT(7),
		.end	= IRQ_EINT(7),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct dm9000_plat_data mini210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x08, 0x90, 0x00, 0xa0, 0x02, 0x10 },
};

struct platform_device mini210_dm9000 = {
	.name		= "dm9000",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(mini210_dm9000_resources),
	.resource	= mini210_dm9000_resources,
	.dev		= {
		.platform_data	= &mini210_dm9000_platdata,
	},
};
#endif

static struct platform_device *mini210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_nand,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	&s3c_device_i2c5,
#endif
	&s3c_device_rtc,
	&s3c_device_wdt,
	&s5pv210_device_iis0,
#ifdef CONFIG_SND_SAMSUNG_AC97
	&s5pv210_device_ac97,
#endif
#ifdef CONFIG_SND_SAMSUNG_SPDIF
	&s5pv210_device_spdif,
#endif
#ifdef CONFIG_SND_SAMSUNG_PCM
#ifdef CONFIG_SND_SAMSUNG_PCM_USE_I2S1_MCLK
	&s5pv210_device_pcm0,
#endif
#endif 	/*end of CONFIG_SND_SAMSUNG_PCM*/
	&samsung_asoc_dma,
#ifdef CONFIG_KEYBOARD_GPIO
    &s3c_device_gpio_btns,
#endif
	&s3c_device_1wire,
	&mini210_device_adc,
#ifdef CONFIG_TOUCHSCREEN_S3C2410
	&s3c_device_ts,
#endif
#ifdef CONFIG_KEYBOARD_SAMSUNG
	&samsung_device_keypad,
#endif
	&s3c_device_timer[0],
#ifdef CONFIG_BACKLIGHT_PWM
	&s3c_device_timer[MINI210_BL_PWM],
	&mini210_backlight_device,
#endif
	&s5p_device_ehci,
	&s5p_device_ohci,
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_DM9000
	&mini210_dm9000,
#endif
	&s3c_device_fb,
	&mini210_lcd_dev,
#ifdef CONFIG_VIDEO_FIMC
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_fimc2,
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	&s3c_device_csis,
#endif
#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif
#ifdef CONFIG_VIDEO_MFC50
	&s3c_device_mfc,
#endif
#ifdef CONFIG_ANDROID_PMEM
	&pmem_gpu1_device,
#endif
#ifdef CONFIG_SPI_S3C64XX
	&s5pv210_device_spi0,
	&s5pv210_device_spi1,
#endif
#ifdef CONFIG_REGULATOR_FIXED_VOLTAGE
	&mini210_b_pwr_5v,
#endif
#ifdef CONFIG_S5PV210_POWER_DOMAIN
	&s5pv210_pd_tv,
	&s5pv210_pd_lcd,
	&s5pv210_pd_g3d,
	&s5pv210_pd_mfc,
	&s5pv210_pd_audio,
#endif
	&s3c_device_g3d,
#ifdef CONFIG_VIDEO_G2D
	&s3c_device_g2d,
#endif
#ifdef CONFIG_VIDEO_TV20
	&s5p_device_tvout,
	&s5p_device_cec,
	&s5p_device_hpd,
#endif
};

#ifdef CONFIG_VIDEO_S5K4EA
/* Set for MIPI-CSI Camera module Power Enable */
static int mini210_mipi_cam_pwr_en(int enabled)
{
	int err;

	err = gpio_request(S5PV210_GPH1(2), "GPH1");
	if (err)
		printk(KERN_ERR "#### failed to request(GPH1)for CAM_2V8\n");

	s3c_gpio_setpull(S5PV210_GPH1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPH1(2), enabled);
	gpio_free(S5PV210_GPH1(2));

	return 0;
}

/* Set for MIPI-CSI Camera module Reset */
static int mini210_mipi_cam_rstn(int enabled)
{
	int err;

	err = gpio_request(S5PV210_GPH0(3), "GPH0");
	if (err)
		printk(KERN_ERR "#### failed to reset(GPH0) for MIPI CAM\n");

	s3c_gpio_setpull(S5PV210_GPH0(3), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPH0(3), enabled);
	gpio_free(S5PV210_GPH0(3));

	return 0;
}

/* MIPI-CSI Camera module Power up/down sequence */
static int mini210_mipi_cam_power(int on)
{
	if (on) {
		mini210_mipi_cam_pwr_en(1);
		mdelay(5);
		mini210_mipi_cam_rstn(1);
	} else {
		mini210_mipi_cam_rstn(0);
		mdelay(5);
		mini210_mipi_cam_pwr_en(0);
	}
	return 0;
}
#endif

#ifdef CONFIG_VIDEO_S5K4BA
#include <media/s5k4ba_platform.h>
#undef	CAM_ITU_CH_A
#define	CAM_ITU_CH_B

/*
 * External camera reset
 * Because the most of cameras take i2c bus signal, so that
 * you have to reset at the boot time for other i2c slave devices.
 * This function also called at fimc_init_camera()
 * Do optimization for cameras on your platform.
*/
#ifdef CAM_ITU_CH_A
static int smdkv210_cam0_power(int onoff)
{
	int err;

	/* Camera A */
	err = gpio_request(GPIO_PS_VOUT, "GPH0");
	if (err)
		printk(KERN_ERR "failed to request GPH0 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_PS_VOUT, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_PS_VOUT, 0);
	gpio_direction_output(GPIO_PS_VOUT, 1);
	gpio_free(GPIO_PS_VOUT);

	return 0;
}
#else
static int smdkv210_cam1_power(int onoff)
{
	int err;

	/* S/W workaround for the SMDK_CAM4_type board
	 * When SMDK_CAM4 type module is initialized at power reset,
	 * it needs the cam_mclk.
	 *
	 * Now cam_mclk is set as below, need only set the gpio mux.
	 * CAM_SRC1 = 0x0006000, CLK_DIV1 = 0x00070400.
	 * cam_mclk source is SCLKMPLL, and divider value is 8.
	 */

	/* gpio mux select the cam_mclk */
	err = gpio_request(GPIO_PS_ON, "GPJ1");
	if (err)
		printk(KERN_ERR "failed to request GPJ1 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_PS_ON, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_PS_ON, (0x3<<16));

	/* Camera B */
	err = gpio_request(GPIO_BUCK_1_EN_A, "GPH0");
	if (err)
		printk(KERN_ERR "failed to request GPH0 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_BUCK_1_EN_A, S3C_GPIO_PULL_NONE);

	/*
	 * set the direction of the GPQ as output and write the data
	 */
	gpio_direction_output(GPIO_BUCK_1_EN_A, 0);
	gpio_direction_output(GPIO_BUCK_1_EN_A, 1);
	udelay(1000);

	gpio_free(GPIO_PS_ON);
	gpio_free(GPIO_BUCK_1_EN_A);

	return 0;
}
#endif

static int s5k5ba_power_en(int onoff)
{
	if (onoff) {
#ifdef CAM_ITU_CH_A
		smdkv210_cam0_power(onoff);
#else
		smdkv210_cam1_power(onoff);
#endif
	} else {
#ifdef CAM_ITU_CH_A
		smdkv210_cam0_power(onoff);
#else
		smdkv210_cam1_power(onoff);
#endif
	}

	return 0;
}

static struct s5k4ba_platform_data s5k4ba_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 44000000,
	.is_mipi = 0,
};

static struct i2c_board_info  s5k4ba_i2c_info = {
	I2C_BOARD_INFO("S5K4BA", 0x2d),
	.platform_data = &s5k4ba_plat,
};

static struct s3c_platform_camera s5k4ba = {
	#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
	#else
	.id		= CAMERA_PAR_B,
	#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &s5k4ba_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	/* .srclk_name	= "xusbxti", */
	.clk_name	= "sclk_cam1",
	.clk_rate	= 44000000,
	.line_length	= 1920,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= s5k5ba_power_en,
};
#endif

/* 2 MIPI Cameras */
#ifdef CONFIG_VIDEO_S5K4EA
static struct s5k4ea_platform_data s5k4ea_plat = {
	.default_width = 1920,
	.default_height = 1080,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 1,
};

static struct i2c_board_info  s5k4ea_i2c_info = {
	I2C_BOARD_INFO("S5K4EA", 0x2d),
	.platform_data = &s5k4ea_plat,
};

static struct s3c_platform_camera s5k4ea = {
	.id		= CAMERA_CSI_C,
	.type		= CAM_TYPE_MIPI,
	.fmt		= MIPI_CSI_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &s5k4ea_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam0",
	/* .clk_name	= "sclk_cam1", */
	.clk_rate	= 48000000,
	.line_length	= 1920,
	.width		= 1920,
	.height		= 1080,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1920,
		.height	= 1080,
	},

	.mipi_lanes	= 2,
	.mipi_settle	= 12,
	.mipi_align	= 32,

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= mini210_mipi_cam_power,
};
#endif

#ifdef CONFIG_VIDEO_OV9650
#include <media/ov9650_platform.h>

#define CAM_ITU_CH_A	1
#define CAMA_PWR_EN		S5PV210_GPJ2(4)

static int ov9650_power_en(int onoff) {
	int ret;

	ret = gpio_request(CAMA_PWR_EN, "GPJ2_4");
	if (ret) {
		printk("request GPJ2_4 failed for camera power, %d\n", ret);
	}

	gpio_direction_output(CAMA_PWR_EN, onoff);
	gpio_free(CAMA_PWR_EN);

	printk("ov9650: power %s\n", onoff ? "ON" : "Off");
	return 0;
}

static struct ov9650_platform_data ov9650_plat = {
	.default_width = 1280,
	.default_height = 1024,
	.pixelformat = V4L2_PIX_FMT_YUYV,
	.freq = 40000000,
	.is_mipi = 0,
};

static struct i2c_board_info ov9650_i2c_info = {
	I2C_BOARD_INFO("ov9650", 0x30),
	.platform_data = &ov9650_plat,
};

static struct s3c_platform_camera ov9650 = {
	#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
	#else
	.id		= CAMERA_PAR_B,
	#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.i2c_busnum	= 0,
	.info		= &ov9650_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_YUYV,
	.srclk_name	= "mout_mpll",
	/* .srclk_name	= "xusbxti", */
	.clk_name	= "sclk_cam1",
	.clk_rate	= 40000000,
	.line_length	= 1920,
	.width		= 1280,
	.height		= 1024,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1280,
		.height	= 1024,
	},

	/* Polarity */
	.inv_pclk	= 1,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= ov9650_power_en,
};
#endif

#ifdef CONFIG_VIDEO_TVP5150
#ifndef CAM_ITU_CH_A
#define CAM_ITU_CH_A	2
#define CAM_ITU_CH_A
#endif

static int tvp5150_power_en(int onoff)
{
	printk("tvp5150: power %s\n", onoff ? "ON" : "Off");
	return 0;
}

static struct i2c_board_info tvp5150_i2c_info = {
	I2C_BOARD_INFO("tvp5150", (0xb8 >> 1)),
};

static struct s3c_platform_camera tvp5150 = {
	#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
	#else
	.id		= CAMERA_PAR_B,
	#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_656_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &tvp5150_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_YUYV,
	.srclk_name	= "mout_mpll",
	/* .srclk_name	= "xusbxti", */
	.clk_name	= "sclk_cam1",
	.clk_rate	= 40000000,
	.line_length	= 1920,
	.width		= 1280,
	.height		= 1024,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1280,
		.height	= 1024,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 0,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= tvp5150_power_en,
};
#endif

/* Interface setting */
static struct s3c_platform_fimc fimc_plat_lsi = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc",
	.lclk_name	= "fimc",
	.clk_rate	= 166750000,
#if defined(CONFIG_VIDEO_S5K4EA)
	.default_cam	= CAMERA_CSI_C,
#else
#ifdef CAM_ITU_CH_A
	.default_cam	= CAMERA_PAR_A,
#else
	.default_cam	= CAMERA_PAR_B,
#endif
#endif
	.camera		= {
#ifdef CONFIG_VIDEO_S5K4ECGX
			&s5k4ecgx,
#endif
#ifdef CONFIG_VIDEO_S5KA3DFX
			&s5ka3dfx,
#endif
#ifdef CONFIG_VIDEO_S5K4BA
			&s5k4ba,
#endif
#ifdef CONFIG_VIDEO_S5K4EA
			&s5k4ea,
#endif
#ifdef CONFIG_VIDEO_TVP5150
			&tvp5150,
#endif
#ifdef CONFIG_VIDEO_OV9650
			&ov9650,
#endif
	},
	.hw_ver		= 0x43,
};

#ifdef CONFIG_VIDEO_JPEG_V2
static struct s3c_platform_jpeg jpeg_plat __initdata = {
	.max_main_width	= 800,
	.max_main_height	= 480,
	.max_thumb_width	= 320,
	.max_thumb_height	= 240,
};
#endif

static void __init mini210_wifi_init(void)
{
	/* WIFI 0 (builtin): block power --> PDn --> RESETn */
	gpio_request(S5PV210_GPJ3(5), "GPJ3_5");
	gpio_direction_output(S5PV210_GPJ3(5), 1);
	udelay(10);
	gpio_free(S5PV210_GPJ3(5));

	gpio_request(S5PV210_GPJ3(7), "GPJ3_7");
	gpio_direction_output(S5PV210_GPJ3(7), 1);
	udelay(10);
	gpio_free(S5PV210_GPJ3(7));

	gpio_request(S5PV210_GPJ3(6), "GPJ3_6");
	gpio_direction_output(S5PV210_GPJ3(6), 1);
	gpio_free(S5PV210_GPJ3(6));

	/* WIFI 1 (external): PDn --> RESETn */
	gpio_request(S5PV210_GPJ4(1), "GPJ4_1");
	gpio_direction_output(S5PV210_GPJ4(1), 1);
	udelay(50);
	gpio_free(S5PV210_GPJ4(1));

	gpio_request(S5PV210_GPJ4(3), "GPJ4_3");
	gpio_direction_output(S5PV210_GPJ4(3), 0);
	udelay(100);
	gpio_set_value(S5PV210_GPJ4(3), 1);
	gpio_free(S5PV210_GPJ4(3));
}

#ifdef CONFIG_DM9000
static int __init dm9000_set_mac(char *str) {
	unsigned char addr[6];
	unsigned int val;
	int idx = 0;
	char *p = str, *end;

	while (*p && idx < 6) {
		val = simple_strtoul(p, &end, 16);
		if (end <= p) {
			/* convert failed */
			break;
		} else {
			addr[idx++] = val;
			p = end;
			if (*p == ':'|| *p == '-') {
				p++;
			} else {
				break;
			}
		}
	}

	if (idx == 6) {
		printk("Setup ethernet address to %pM\n", addr);
		memcpy(mini210_dm9000_platdata.param_addr, addr, 6);
	}

	return 1;
}

__setup("ethmac=", dm9000_set_mac);

static void __init mini210_dm9000_init(void)
{
	unsigned int tmp;

	gpio_request(S5PV210_MP01(1), "nCS1");
	s3c_gpio_cfgpin(S5PV210_MP01(1), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(1));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC1);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS1__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS1__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);
}
#endif

#ifdef CONFIG_SND_SOC_WM8960_MINI210
#include <sound/wm8960.h>
static struct wm8960_data wm8960_pdata = {
	.capless		= 0,
	.dres			= WM8960_DRES_400R,
};
#endif

#ifdef CONFIG_SENSORS_MMA7660
#include <linux/mma7660.h>
static struct mma7660_platform_data mma7660_pdata = {
	.irq			= IRQ_EINT9,
	.poll_interval	= 100,
	.input_fuzz		= 4,
	.input_flat		= 4,
};
#endif

static struct i2c_board_info mini210_i2c_devs0[] __initdata = {
	{	I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
#ifdef CONFIG_SND_SOC_WM8580
	{	I2C_BOARD_INFO("wm8580", 0x1b), },
#endif
#ifdef CONFIG_SND_SOC_WM8960_MINI210
	{
		I2C_BOARD_INFO("wm8960", 0x1a),
		.platform_data = &wm8960_pdata,
	},
#endif
#ifdef CONFIG_SENSORS_MMA7660
	{
		I2C_BOARD_INFO("mma7660", 0x4c),
		.platform_data = &mma7660_pdata,
	},
#endif
};

static struct i2c_board_info mini210_i2c_devs1[] __initdata = {
#ifdef CONFIG_VIDEO_TV20
	{
		I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),
	},
#endif
};

#ifdef CONFIG_TOUCHSCREEN_GOODIX
#include <plat/ctouch.h>
#include <plat/goodix_touch.h>
static struct goodix_i2c_platform_data goodix_pdata = {
	.gpio_irq		= S5PV210_GPH1(6),
	.irq_cfg		= S3C_GPIO_SFN(0xf),
	.gpio_shutdown	= 0,
	.shutdown_cfg	= S3C_GPIO_OUTPUT,
	.screen_width	= 800,
	.screen_height	= 480,
};
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5X0X
#include <plat/ft5x0x_touch.h>
static struct ft5x0x_i2c_platform_data ft5x0x_pdata = {
	.gpio_irq		= S5PV210_GPH1(6),
	.irq_cfg		= S3C_GPIO_SFN(0xf),
	.screen_max_x	= 480,
	.screen_max_y	= 272,
	.pressure_max	= 255,
};
#endif

#include <plat/regs-iic.h>
static struct s3c2410_platform_i2c i2c2_data __initdata = {
	.flags			= 0,
	.bus_num		= 2,
	.slave_addr		= 0x10,
	.frequency		= 400*1000,
	.sda_delay		= S3C2410_IICLC_SDA_DELAY5 | S3C2410_IICLC_FILTER_ON,
};

static struct i2c_board_info mini210_i2c_devs2[] __initdata = {
#if defined(CONFIG_REGULATOR_MAX8698)
	{
		I2C_BOARD_INFO("max8698", 0xCC >> 1),
		.platform_data  = &mini210_max8698_pdata,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_GOODIX
	{
		I2C_BOARD_INFO("gt80x-ts", 0x55),
		.platform_data = &goodix_pdata,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
	{
		I2C_BOARD_INFO("ft5x0x_ts", (0x70 >> 1)),
		.platform_data = &ft5x0x_pdata,
	},
#endif
};

#ifdef CONFIG_SPI_S3C64XX
#define SMDK_MMCSPI_CS 0

static struct s3c64xx_spi_csinfo smdk_spi0_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(1),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

static struct s3c64xx_spi_csinfo smdk_spi1_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(5),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

static struct spi_board_info s3c_spi_devs[] __initdata = {
	{
		.modalias        = "spidev",		/* MMC SPI */
		.mode            = SPI_MODE_0,		/* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 0,
		.chip_select     = 0,
		.controller_data = &smdk_spi0_csi[SMDK_MMCSPI_CS],
	},
	{
		.modalias        = "spidev",		/* MMC SPI */
		.mode            = SPI_MODE_0,		/* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 1,
		.chip_select     = 0,
		.controller_data = &smdk_spi1_csi[SMDK_MMCSPI_CS],
	},
};
#endif

#ifdef CONFIG_TOUCHSCREEN_S3C2410
static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
	.delay			= 10000,
	.presc			= 49,
	.oversampling_shift	= 2,
	.cal_x_max		= 800,
	.cal_y_max		= 480,
	.cal_param = {
		-13357, -85, 53858048, -95, -8493, 32809514, 65536
	},
};
#endif

static void __init mini210_map_io(void)
{
	struct s3cfb_lcd *lcd = mini210_get_lcd();
	int frame_size, fimd_size;

	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(mini210_uartcfgs, ARRAY_SIZE(mini210_uartcfgs));
#ifndef CONFIG_S5P_HIGH_RES_TIMERS
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
#endif

	frame_size = lcd->width * lcd->height * BYTES_PER_PIXEL;
	fimd_size = ALIGN(frame_size, PAGE_SIZE) * NUM_BUFFER;
	if (frame_size > 0x200000) {
		fimd_size += ALIGN(frame_size, PAGE_SIZE) * 2;
	}
	/* Reserve 0x003f6000 bytes for PVR YUV video, and 1 page */
	fimd_size += ALIGN(1280*720, PAGE_SIZE) * 3;
	fimd_size += ALIGN(1280*360, PAGE_SIZE) * 3 + PAGE_SIZE;
	if (fimd_size != S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD) {
		mini210_fixup_bootmem(S5P_MDEV_FIMD, fimd_size);
	}
	if (lcd->width > 1280) {
		mini210_fixup_bootmem(S5P_MDEV_FIMC2, 12288 * SZ_1K);
	}
	s5p_reserve_bootmem(mini210_media_devs,
			ARRAY_SIZE(mini210_media_devs), S5P_RANGE_MFC);

#ifdef CONFIG_TOUCHSCREEN_GOODIX
	goodix_pdata.screen_width = lcd->width;
	goodix_pdata.screen_height = lcd->height;
#endif
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
	ft5x0x_pdata.screen_max_x = lcd->width;
	ft5x0x_pdata.screen_max_y = lcd->height;
#endif
}

static void __init mini210_fixup(struct machine_desc *desc,
		struct tag *tags, char **cmdline,
		struct meminfo *mi)
{
#ifdef CONFIG_ARCH_DISCONTIGMEM_ENABLE
	/*-------------------------------------------------------------
	 * fixup memory info here, for example:
	 +   mi->bank[0].start = 0x20000000;
	 +   mi->bank[0].size = 256 * SZ_1M;
	 +   mi->bank[0].node = 0;
	 +   mi->nr_banks = 1;
	 -------------------------------------------------------------*/
#else
//#warning "DRAM #1 may be inaccesaable, pls *CHECK* kernel config!"
#endif
}

#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata mini210_hsmmc0_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
#if defined(CONFIG_S5PV210_SD_CH0_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif
 
#ifdef CONFIG_S3C_DEV_HSMMC1
static struct s3c_sdhci_platdata mini210_hsmmc1_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
static struct s3c_sdhci_platdata mini210_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
#if defined(CONFIG_S5PV210_SD_CH2_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
static struct s3c_sdhci_platdata mini210_hsmmc3_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
};
#endif

static void setup_hsmmc_clock(struct platform_device *pdev)
{
	struct clk *pclk;
	struct clk *clk;

	/* set HSMMC clock */
	clk = clk_get(&pdev->dev, "sclk_mmc");
	pclk = clk_get(NULL, "mout_mpll");
	clk_set_parent(clk, pclk);
	clk_set_rate(clk, 50*MHZ);

	pr_info("hsmmc%d: %s: source is %s, rate is %ld\n", pdev->id,
			clk->name, clk->parent->name, clk_get_rate(clk));
}

static void __init mini210_setup_clocks(void)
{
	setup_hsmmc_clock(&s3c_device_hsmmc0);
	setup_hsmmc_clock(&s3c_device_hsmmc1);
	setup_hsmmc_clock(&s3c_device_hsmmc2);
	setup_hsmmc_clock(&s3c_device_hsmmc3);
}

/* USB EHCI */
static struct s5p_ehci_platdata mini210_ehci_pdata;
static void __init mini210_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &mini210_ehci_pdata;

	s5p_ehci_set_platdata(pdata);
}

/* USB OHCI */
static struct s5p_ohci_platdata mini210_ohci_pdata;
static void __init mini210_ohci_init(void)
{
	struct s5p_ohci_platdata *pdata = &mini210_ohci_pdata;

	s5p_ohci_set_platdata(pdata);
}

/* USB DEVICE */
static struct s5p_otg_platdata mini210_otg_pdata;
static void __init mini210_otg_init(void)
{
	struct s5p_otg_platdata *pdata = &mini210_otg_pdata;

	s5p_otg_set_platdata(pdata);
}

static bool console_flushed;

static void flush_console(void)
{
	if (console_flushed)
		return;

	console_flushed = true;

	printk("\n");
	pr_emerg("Restarting %s\n", linux_banner);
	if (!is_console_locked())
		return;

	mdelay(50);

	local_irq_disable();
	if (!console_trylock())
		pr_emerg("flush_console: console was locked! busting!\n");
	else
		pr_emerg("flush_console: console was locked!\n");
	console_unlock();
}

static void smdkc110_pm_restart(char mode, const char *cmd)
{
	flush_console();

	/* On a normal reboot, INFORM6 will contain a small integer
	 * reason code from the notifier hook.  On a panic, it will
	 * contain the 0xee we set at boot.  Write 0xbb to differentiate
	 * a watchdog-timeout-and-reboot (0xee) from a controlled reboot
	 * (0xbb)
	 */
	if (__raw_readl(S5P_INFORM6) == 0xee)
		__raw_writel(0xbb, S5P_INFORM6);

	arm_machine_restart(mode, cmd);
}

static void __init mini210_machine_init(void)
{
	arm_pm_restart = smdkc110_pm_restart;

	s3c_pm_init();

	mini210_wifi_init();
#ifdef CONFIG_DM9000
	mini210_dm9000_init();
#endif

	platform_add_devices(mini210_devices, ARRAY_SIZE(mini210_devices));

#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif

#ifdef CONFIG_KEYBOARD_SAMSUNG
	samsung_keypad_set_platdata(&mini210_keypad_data);
#endif
#ifdef CONFIG_TOUCHSCREEN_S3C2410
	s3c24xx_ts_set_platdata(&s3c_ts_platform);
#endif

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
#ifdef CONFIG_TOUCHSCREEN_GOODIX
	if (mini210_get_ctp() == CTP_GT80X) {
		i2c2_data.frequency = 260*1000;
	}
#endif
	s3c_i2c2_set_platdata(&i2c2_data);
	i2c_register_board_info(0, mini210_i2c_devs0,
			ARRAY_SIZE(mini210_i2c_devs0));
	i2c_register_board_info(1, mini210_i2c_devs1,
			ARRAY_SIZE(mini210_i2c_devs1));
	i2c_register_board_info(2, mini210_i2c_devs2,
			ARRAY_SIZE(mini210_i2c_devs2));
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));
#endif

#ifdef CONFIG_S5PV210_SETUP_IDE
	s3c_ide_set_platdata(&mini210_ide_pdata);
#endif

#ifdef CONFIG_FB_S3C_MINI210
	{
		struct s3cfb_lcd *mlcd = mini210_get_lcd();
		if (!(mlcd->args & 0x0f)) {
			if (readl(S5PV210_GPF0_BASE + 0x184) & 0x10)
				mlcd->args |= (1 << 7);
		}
		mini210_fb_data.lcd = mlcd;
		s3c_fb_set_platdata(&mini210_fb_data);
	}
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
	s3c_sdhci0_set_platdata(&mini210_hsmmc0_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s3c_sdhci1_set_platdata(&mini210_hsmmc1_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	s3c_sdhci2_set_platdata(&mini210_hsmmc2_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s3c_sdhci3_set_platdata(&mini210_hsmmc3_pdata);
#endif

#ifdef CONFIG_VIDEO_FIMC
	/* fimc */
	s3c_fimc0_set_platdata(&fimc_plat_lsi);
	s3c_fimc1_set_platdata(&fimc_plat_lsi);
	s3c_fimc2_set_platdata(&fimc_plat_lsi);
#ifdef CONFIG_VIDEO_S5K4BA
#ifdef CAM_ITU_CH_A
	smdkv210_cam0_power(1);
#else
	smdkv210_cam1_power(1);
#endif
#endif
#endif

#ifdef CONFIG_VIDEO_FIMC_MIPI
	s3c_csis_set_platdata(NULL);
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	s3c_jpeg_set_platdata(&jpeg_plat);
#endif
#ifdef CONFIG_VIDEO_MFC50
	/* mfc */
	s3c_mfc_set_platdata(NULL);
#endif

	/* spi */
#ifdef CONFIG_SPI_S3C64XX
	if (!gpio_request(S5PV210_GPB(1), "SPI_CS0")) {
		gpio_direction_output(S5PV210_GPB(1), 1);
		s3c_gpio_cfgpin(S5PV210_GPB(1), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPB(1), S3C_GPIO_PULL_UP);
		s5pv210_spi_set_info(0, S5PV210_SPI_SRCCLK_PCLK,
				ARRAY_SIZE(smdk_spi0_csi));
	}
	if (!gpio_request(S5PV210_GPB(5), "SPI_CS1")) {
		gpio_direction_output(S5PV210_GPB(5), 1);
		s3c_gpio_cfgpin(S5PV210_GPB(5), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPB(5), S3C_GPIO_PULL_UP);
		s5pv210_spi_set_info(1, S5PV210_SPI_SRCCLK_PCLK,
				ARRAY_SIZE(smdk_spi1_csi));
	}
	spi_register_board_info(s3c_spi_devs, ARRAY_SIZE(s3c_spi_devs));
#endif

	mini210_otg_init();
	mini210_ehci_init();
	mini210_ohci_init();
	clk_xusbxti.rate = 24000000;

	regulator_has_full_constraints();

	mini210_setup_clocks();

	/* write something into the INFORM6 register that we can use to
	 * differentiate an unclear reboot from a clean reboot (which
	 * writes a small integer code to INFORM6).
	 */
	__raw_writel(0xee, S5P_INFORM6);
}

MACHINE_START(MINI210, "MINI210")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.fixup			= mini210_fixup,
	.init_irq		= s5pv210_init_irq,
	.map_io			= mini210_map_io,
	.init_machine	= mini210_machine_init,
#ifdef CONFIG_S5P_HIGH_RES_TIMERS
	.timer			= &s5p_systimer,
#else
	.timer			= &s5p_timer,
#endif
MACHINE_END

