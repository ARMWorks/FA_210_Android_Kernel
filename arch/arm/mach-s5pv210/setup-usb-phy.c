/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/usb/ch9.h>
#include <plat/map-base.h>
#include <plat/map-s5p.h>
#include <mach/regs-clock.h>
#include <plat/regs-usb-hsotg-phy.h>
#include <plat/cpu.h>
#include <plat/usb-phy.h>
#include <plat/regs-otg.h>
#include <linux/regulator/consumer.h>

static struct regulator *usb_vcc_d;
static struct regulator *usb_vcc_a;
static int host_phy_initialized;

static int s5p_usb_power(struct platform_device *pdev, int on)
{
#ifdef CONFIG_REGULATOR
#if !defined(CONFIG_MACH_MINI210)
	usb_vcc_d = regulator_get(&pdev->dev, "pd_io");
	usb_vcc_a = regulator_get(&pdev->dev, "pd_core");
	if (IS_ERR_OR_NULL(usb_vcc_d) || IS_ERR_OR_NULL(usb_vcc_a)) {
		pr_err("failed to find udc vcc source\n");
		return -ENOENT;
	}

	if (on) {
		regulator_enable(usb_vcc_d);
		regulator_enable(usb_vcc_a);
	}
	else {
		regulator_disable(usb_vcc_d);
		regulator_disable(usb_vcc_a);
	}

	regulator_put(usb_vcc_d);
	regulator_put(usb_vcc_a);
#endif
#endif

	return 0;
}

int s5p_usb_phy_init(struct platform_device *pdev, int type)
{
	struct clk *otg_clk;
	int err;

	err = s5p_usb_power(pdev, 1);
	if (err) {
		pr_err("Failed to power up USB\n");
	}	

	if (type == S5P_USB_PHY_HOST && host_phy_initialized != 0) {
		host_phy_initialized++;
		return 0;
	}

	otg_clk = clk_get(&pdev->dev, "otg");
	if (IS_ERR(otg_clk)) {
		dev_err(&pdev->dev, "Failed to get otg clock\n");
		return PTR_ERR(otg_clk);
	}

	err = clk_enable(otg_clk);
	if (err) {
		clk_put(otg_clk);
		return err;
	}

	if (type == S5P_USB_PHY_HOST) {
		if (readl(S5P_USB_PHY_CONTROL) & (0x1<<1))
			return -1;

		__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL) | (0x1<<1),
				S5P_USB_PHY_CONTROL);
		__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
					& ~(0x1<<7) & ~(0x1<<6)) | (0x1<<8) | (0x1<<5),
				S3C_USBOTG_PHYPWR);
		__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) & ~(0x1<<7)) | (0x3<<0),
				S3C_USBOTG_PHYCLK);
		__raw_writel((__raw_readl(S3C_USBOTG_RSTCON)) | (0x1<<4) | (0x1<<3),
				S3C_USBOTG_RSTCON);
		udelay(10);

		__raw_writel(__raw_readl(S3C_USBOTG_RSTCON) & ~(0x1<<4) & ~(0x1<<3),
				S3C_USBOTG_RSTCON);

		host_phy_initialized++;
	} else if (type == S5P_USB_PHY_DEVICE) {

		/* USB PHY0 Enable */
		writel(readl(S5P_USB_PHY_CONTROL) | (0x1<<0),
				S5P_USB_PHY_CONTROL);
		writel((readl(S3C_USBOTG_PHYPWR) & ~(0x3<<3) & ~(0x1<<0)) | (0x1<<5),
				S3C_USBOTG_PHYPWR);
		writel((readl(S3C_USBOTG_PHYCLK) & ~(0x5<<2)) | (0x3<<0),
				S3C_USBOTG_PHYCLK);
		writel((readl(S3C_USBOTG_RSTCON) & ~(0x3<<1)) | (0x1<<0),
				S3C_USBOTG_RSTCON);
		msleep(1);

		writel(readl(S3C_USBOTG_RSTCON) & ~(0x7<<0),
				S3C_USBOTG_RSTCON);
		msleep(1);

		/* rising/falling time */
		writel(readl(S3C_USBOTG_PHYTUNE) | (0x1<<20),
				S3C_USBOTG_PHYTUNE);

		/* set DC level as 6 (6%) */
		writel((readl(S3C_USBOTG_PHYTUNE) & ~(0xf)) | (0x1<<2) | (0x1<<1),
				S3C_USBOTG_PHYTUNE);
	}

	udelay(80);

	clk_disable(otg_clk);
	clk_put(otg_clk);

	return 0;
}

int s5p_usb_phy_exit(struct platform_device *pdev, int type)
{
	struct clk *otg_clk;
	int err;

	otg_clk = clk_get(&pdev->dev, "otg");
	if (IS_ERR(otg_clk)) {
		dev_err(&pdev->dev, "Failed to get otg clock\n");
		return PTR_ERR(otg_clk);
	}

	err = clk_enable(otg_clk);
	if (err) {
		clk_put(otg_clk);
		return err;
	}

	if (type == S5P_USB_PHY_HOST) {
		host_phy_initialized--;
		if (host_phy_initialized !=0)
			goto end;

		__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR) | (0x1<<7)|(0x1<<6),
				S3C_USBOTG_PHYPWR);
		__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL) & ~(1<<1),
				S5P_USB_PHY_CONTROL);

	} else if (type == S5P_USB_PHY_DEVICE) {
		writel(readl(S3C_PHYPWR) | (0x3<<3),
				S3C_PHYPWR);

		writel(readl(S5P_USB_PHY_CONTROL) & ~(1<<0),
				S5P_USB_PHY_CONTROL);
	}

end:
	clk_disable(otg_clk);
	clk_put(otg_clk);

	err = s5p_usb_power(pdev, 0);
	if (err) {
		pr_err("Failed to power down USB\n");
	}

	return 0;
}

/* USB Control request data struct must be located here for DMA transfer */
struct usb_ctrlrequest usb_ctrl __attribute__((aligned(64)));

