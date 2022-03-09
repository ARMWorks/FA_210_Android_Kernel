/* linux/drivers/media/video/ov9650.h
 *
 * Copyright (c) 2011 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *	         http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __OV9650_H__
#define __OV9650_H__

struct ov9650_reg {
	unsigned char	addr;
	unsigned char	val;
};


/*
 * Register definitions
 */
#define	OV9650_GAIN			0x00
#define	OV9650_BLUE			0x01
#define	OV9650_RED			0x02
#define	OV9650_VREF			0x03
#define	OV9650_COM1			0x04
#define	OV9650_BAVE			0x05
#define	OV9650_GEAVE		0x06
/* 0x07 - Reserved */
#define	OV9650_RAVE			0x08
#define	OV9650_COM2			0x09
#define	OV9650_PID			0x0a
#define	OV9650_VER			0x0b
#define	OV9650_COM3			0x0c
#define	OV9650_COM4			0x0d
#define	OV9650_COM5			0x0e
#define	OV9650_COM6			0x0f
#define	OV9650_AECH			0x10
#define	OV9650_CLKRC		0x11
#define	OV9650_COM7			0x12
#define	OV9650_COM8			0x13
#define	OV9650_COM9			0x14
#define	OV9650_COM10		0x15
/* 0x16 - Reserved */
#define	OV9650_HSTART		0x17
#define	OV9650_HSTOP		0x18
#define	OV9650_VSTART		0x19
#define	OV9650_VSTOP		0x1a
#define	OV9650_PSHFT		0x1b
#define	OV9650_MIDH			0x1c
#define	OV9650_MIDL			0x1d
#define	OV9650_MVFP			0x1e
#define	OV9650_LAEC			0x1f
#define	OV9650_BOS			0x20
#define	OV9650_GBOS			0x21
#define	OV9650_GROS			0x22
#define	OV9650_ROS			0x23
#define	OV9650_AEW			0x24
#define	OV9650_AEB			0x25
#define	OV9650_VPT			0x26
#define	OV9650_BBIAS		0x27
#define	OV9650_GBBIAS		0x28
#define	OV9650_GRCOM		0x29
#define	OV9650_EXHCH		0x2a
#define	OV9650_EXHCL		0x2b
#define	OV9650_RBIAS		0x2c
#define	OV9650_ADVFL		0x2d
#define	OV9650_ADVFH		0x2e
#define	OV9650_YAVE			0x2f
#define	OV9650_HSYST		0x30
#define	OV9650_HSYEN		0x31
#define	OV9650_HREF			0x32
#define	OV9650_CHLF			0x33
#define	OV9650_ARBLM		0x34
/* 0x35..0x36 - Reserved */
#define	OV9650_ADC			0x37
#define	OV9650_ACOM			0x38
#define	OV9650_OFON			0x39
#define	OV9650_TSLB			0x3a
#define	OV9650_COM11		0x3b
#define	OV9650_COM12		0x3c
#define	OV9650_COM13		0x3d
#define	OV9650_COM14		0x3e
#define	OV9650_EDGE			0x3f
#define	OV9650_COM15		0x40
#define	OV9650_COM16		0x41
#define	OV9650_COM17		0x42
/* 0x43..0x4e - Reserved */
#define	OV9650_MTX1			0x4f
#define	OV9650_MTX2			0x50
#define	OV9650_MTX3			0x51
#define	OV9650_MTX4			0x52
#define	OV9650_MTX5			0x53
#define	OV9650_MTX6			0x54
#define	OV9650_MTX7			0x55
#define	OV9650_MTX8			0x56
#define	OV9650_MTX9			0x57
#define	OV9650_MTXS			0x58
/* 0x59..0x61 - Reserved */
#define	OV9650_LCC1			0x62
#define	OV9650_LCC2			0x63
#define	OV9650_LCC3			0x64
#define	OV9650_LCC4			0x65
#define	OV9650_LCC5			0x66
#define	OV9650_MANU			0x67
#define	OV9650_MANV			0x68
#define	OV9650_HV			0x69
#define	OV9650_MBD			0x6a
#define	OV9650_DBLV			0x6b
#define	OV9650_GSP			0x6c	/* ... till 0x7b */
#define	OV9650_GST			0x7c	/* ... till 0x8a */
#define	OV9650_COM21		0x8b
#define	OV9650_COM22		0x8c
#define	OV9650_COM23		0x8d
#define	OV9650_COM24		0x8e
#define	OV9650_DBLC1		0x8f
#define	OV9650_DBLCB		0x90
#define	OV9650_DBLCR		0x91
#define	OV9650_DMLNL		0x92
#define	OV9650_DMLNH		0x93
/* 0x94..0x9c - Reserved */
#define	OV9650_LCCFB		0x9d
#define	OV9650_LCCFR		0x9e
#define	OV9650_DBLCGB		0x9f
#define	OV9650_DBLCGR		0xa0


#define	OV9650_CLKRC_DPLL_EN			0x80
#define	OV9650_CLKRC_DIRECT			0x40
#define	OV9650_CLKRC_DIV(x)	((x) & 0x3f)

#define	OV9650_PSHFT_VAL(x)	((x) & 0xff)

#define	OV9650_ACOM_2X_ANALOG			0x80
#define	OV9650_ACOM_RSVD			0x12

#define	OV9650_MVFP_V				0x10
#define	OV9650_MVFP_H				0x20

#define	OV9650_COM1_HREF_NOSKIP			0x00
#define	OV9650_COM1_HREF_2SKIP			0x04
#define	OV9650_COM1_HREF_3SKIP			0x08
#define	OV9650_COM1_QQFMT			0x20

#define	OV9650_COM2_SSM				0x10

#define	OV9650_COM3_VP				0x04

#define	OV9650_COM4_QQ_VP			0x80
#define	OV9650_COM4_RSVD			0x40

#define	OV9650_COM5_SYSCLK			0x80
#define	OV9650_COM5_LONGEXP			0x01

#define	OV9650_COM6_OPT_BLC			0x40
#define	OV9650_COM6_ADBLC_BIAS			0x08
#define	OV9650_COM6_FMT_RST			0x82
#define	OV9650_COM6_ADBLC_OPTEN			0x01

#define	OV9650_COM7_RAW_RGB			0x01
#define	OV9650_COM7_RGB				0x04
#define	OV9650_COM7_QCIF			0x08
#define	OV9650_COM7_QVGA			0x10
#define	OV9650_COM7_CIF				0x20
#define	OV9650_COM7_VGA				0x40
#define	OV9650_COM7_SCCB_RESET			0x80

#define	OV9650_TSLB_YVYU_YUYV			0x04
#define	OV9650_TSLB_YUYV_UYVY			0x08

#define	OV9650_COM12_YUV_AVG			0x04
#define	OV9650_COM12_RSVD			0x40

#define	OV9650_COM13_GAMMA_NONE			0x00
#define	OV9650_COM13_GAMMA_Y			0x40
#define	OV9650_COM13_GAMMA_RAW			0x80
#define	OV9650_COM13_RGB_AVG			0x20
#define	OV9650_COM13_MATRIX_EN			0x10
#define	OV9650_COM13_Y_DELAY_EN			0x08
#define	OV9650_COM13_YUV_DLY(x)	((x) & 0x07)

#define	OV9650_COM15_OR_00FF			0x00
#define	OV9650_COM15_OR_01FE			0x40
#define	OV9650_COM15_OR_10F0			0xc0
#define	OV9650_COM15_RGB_NORM			0x00
#define	OV9650_COM15_RGB_565			0x10
#define	OV9650_COM15_RGB_555			0x30

#define	OV9650_COM16_RB_AVG			0x01

/* IDs */
#define OV9652				0x9652
#define VERSION(pid, ver)	(((pid) << 8) | ((ver) & 0xFF))

/*
 * User defined commands
 */
/* S/W defined features for tune */
#define REG_DELAY			0xFF00	/* in ms */
#define REG_CMD				0xFFFF	/* Followed by command */

/* Following order should not be changed */
enum image_size_ov9650 {
	/* This SoC supports upto SXGA (1280*1024) */
	QQVGA,	/* 160*120 */
	QCIF,	/* 176*144 */
	QVGA,	/* 320*240 */
	CIF,	/* 352*288 */
	VGA,	/* 640*480 */
	SXGA,	/* 1280*1024 */
#if	0
	SVGA,	/* 800*600 */
	HD720P,	/* 1280*720 */
	UXGA,	/* 1600*1200 */
#endif
};

/*
 * Following values describe controls of camera
 * in user aspect and must be match with index of ov9650_regset[]
 * These values indicates each controls and should be used
 * to control each control
 */
enum ov9650_control {
	OV9650_INIT,
	OV9650_EV,
	OV9650_AWB,
	OV9650_MWB,
	OV9650_EFFECT,
	OV9650_CONTRAST,
	OV9650_SATURATION,
	OV9650_SHARPNESS,
};

#define OV9650_REGSET(x)	{	\
	.regset = x,			\
	.len = sizeof(x)/sizeof(ov9650_reg),}


/*
 * User tuned register setting values
 */
#define	CHIP_DELAY			0xff

static unsigned char ov9650_init_reg[][2] = {
	{ 0x12, 0x80 },			// Camera Soft reset. Self cleared after reset
	{ CHIP_DELAY, 10 },		// unknown
	//{ 0x11, 0x80 },
	{ 0x6a, 0x3e },			// Manual banding filter value
	{ 0x3b, 0x09 },			// Night mode disable, manual banding filter enable
	{ 0x13, 0x8f },			// Fast AGC/AEC, AGC, AWB, AEC
	{ 0x01, 0x80 },			// Blue channel gain
	{ 0x02, 0x80 },			// Red  channel gain
	{ 0x00, 0x00 },			// Gain control
	{ 0x10, 0x00 },			// Exposure Value
	{ 0x35, 0x91 },			// R
	{ 0x0e, 0xa0 },			// System clock selection ?
	{ 0x1e, 0x34 },			// Mirror, VFlip
	{ 0xA8, 0x80 },			// R
	/* --- VGA Begin --- */
	{ 0x04, 0x00 },
	{ 0x0c, 0x04 },
	{ 0x0d, 0x80 },
	{ 0x11, 0x81 },
	{ 0x12, 0x40 },
	{ 0x37, 0x91 },
	{ 0x38, 0x12 },
	{ 0x39, 0x43 },
	/* --- VGA End ----- */
	{ 0x18, 0xc6 },			// H-stop
	{ 0x17, 0x26 },			// H-start
	{ 0x32, 0xad },			// H-ref
	{ 0x03, 0x00 },			// V-ref
	{ 0x1a, 0x3d },			// V-stop
	{ 0x19, 0x01 },			// V-start
	{ 0x3f, 0xa6 },
	{ 0x14, 0x2e },
	{ 0x15, 0x10 },			// PCLK reverse
	{ 0x41, 0x02 },			// Color matrix coefficient
	{ 0x42, 0x08 },			// def
	{ 0x1b, 0x00 },			// Pixel delay select, def
	{ 0x16, 0x06 },			// R
	{ 0x33, 0xe2 },			// R
	{ 0x34, 0xbf },			// R
	{ 0x96, 0x04 },			// R
	{ 0x3a, 0x00 },			// TSLB: Normal UV, YUYV
	{ 0x8e, 0x00 },			// Reserved
	{ 0x3c, 0x77 },			// Enable UV average
	{ 0x8B, 0x06 },			// R
	{ 0x94, 0x88 },			// R
	{ 0x95, 0x88 },			// R
	{ 0x40, 0xc1 },			// Range 00~FF, Reserved
	{ 0x29, 0x3f },			// Bypass ...
	{ 0x0f, 0x42 },			// Reset all timing when format changes
	{ 0x3d, 0x92 },			// Gamma for Raw, color matrix
	{ 0x69, 0x40 },			// Manual banding filter
	{ 0x5C, 0xb9 },			// R
	{ 0x5D, 0x96 },			// R
	{ 0x5E, 0x10 },			// R
	{ 0x59, 0xc0 },			// R
	{ 0x5A, 0xaf },			// R
	{ 0x5B, 0x55 },			// R
	{ 0x43, 0xf0 },			// R
	{ 0x44, 0x10 },			// R
	{ 0x45, 0x68 },			// R
	{ 0x46, 0x96 },			// R
	{ 0x47, 0x60 },			// R
	{ 0x48, 0x80 },			// R
	{ 0x5F, 0xe0 },			// R
	{ 0x60, 0x8c },			// R
	{ 0x61, 0x20 },			// R
	{ 0xa5, 0xd9 },			// R
	{ 0xa4, 0x74 },			// R
	{ 0x8d, 0x02 },			// Digital color gain enable
	//{ 0x13, 0xe7 },
	{ 0x4f, 0x3a },			// Matrix
	{ 0x50, 0x3d },			// Matrix
	{ 0x51, 0x03 },			// Matrix
	{ 0x52, 0x12 },			// Matrix
	{ 0x53, 0x26 },			// Matrix
	{ 0x54, 0x36 },			// Matrix
	{ 0x55, 0x45 },			// Matrix
	{ 0x56, 0x40 },			// Matrix
	{ 0x57, 0x40 },			// Matrix
	{ 0x58, 0x0d },			// Matrix coefficient sign for coefficient 9 to 2
	{ 0x8C, 0x23 },
	{ 0x3E, 0x02 },			// Enable edge enhancement for YUV output
	{ 0xa9, 0xb8 },			// R
	{ 0xaa, 0x92 },			// R
	{ 0xab, 0x0a },			// R
	{ 0x8f, 0xdf },			// Digital BLC
	{ 0x90, 0x00 },			// Digital BLC
	{ 0x91, 0x00 },			// Digital BLC
	{ 0x9f, 0x00 },			// Digital BLC
	{ 0xa0, 0x00 },			// Digital BLC
	{ 0x3A, 0x01 },
	{ 0x24, 0x70 },
	{ 0x25, 0x64 },
	{ 0x26, 0xc3 },
	{ 0x2a, 0x00 },
	{ 0x2b, 0x00 },
#if	0
	/* --- Gamma curve begin --- */
	{ 0x6c, 0x40 },
	{ 0x6d, 0x30 },
	{ 0x6e, 0x4b },
	{ 0x6f, 0x60 },
	{ 0x70, 0x70 },
	{ 0x71, 0x70 },
	{ 0x72, 0x70 },
	{ 0x73, 0x70 },
	{ 0x74, 0x60 },
	{ 0x75, 0x60 },
	{ 0x76, 0x50 },
	{ 0x77, 0x48 },
	{ 0x78, 0x3a },
	{ 0x79, 0x2e },
	{ 0x7a, 0x28 },
	{ 0x7b, 0x22 },
	{ 0x7c, 0x04 },
	{ 0x7d, 0x07 },
	{ 0x7e, 0x10 },
	{ 0x7f, 0x28 },
	{ 0x80, 0x36 },
	{ 0x81, 0x44 },
	{ 0x82, 0x52 },
	{ 0x83, 0x60 },
	{ 0x84, 0x6c },
	{ 0x85, 0x78 },
	{ 0x86, 0x8c },
	{ 0x87, 0x9e },
	{ 0x88, 0xbb },
	{ 0x89, 0xd2 },
	{ 0x8a, 0xe6 },
	/* --- Gamma curve end ----- */
#endif
	//{ 0x15, 0x12 },	// PCLK reverse
};

#ifdef __FA_DISABLED__

/*
 * EV bias
 */

static const struct ov9650_reg ov9650_ev_m6[] = {
};

static const struct ov9650_reg ov9650_ev_m5[] = {
};

static const struct ov9650_reg ov9650_ev_m4[] = {
};

static const struct ov9650_reg ov9650_ev_m3[] = {
};

static const struct ov9650_reg ov9650_ev_m2[] = {
};

static const struct ov9650_reg ov9650_ev_m1[] = {
};

static const struct ov9650_reg ov9650_ev_default[] = {
};

static const struct ov9650_reg ov9650_ev_p1[] = {
};

static const struct ov9650_reg ov9650_ev_p2[] = {
};

static const struct ov9650_reg ov9650_ev_p3[] = {
};

static const struct ov9650_reg ov9650_ev_p4[] = {
};

static const struct ov9650_reg ov9650_ev_p5[] = {
};

static const struct ov9650_reg ov9650_ev_p6[] = {
};

/* Order of this array should be following the querymenu data */
static const unsigned char *ov9650_regs_ev_bias[] = {
	(unsigned char *)ov9650_ev_m6, (unsigned char *)ov9650_ev_m5,
	(unsigned char *)ov9650_ev_m4, (unsigned char *)ov9650_ev_m3,
	(unsigned char *)ov9650_ev_m2, (unsigned char *)ov9650_ev_m1,
	(unsigned char *)ov9650_ev_default, (unsigned char *)ov9650_ev_p1,
	(unsigned char *)ov9650_ev_p2, (unsigned char *)ov9650_ev_p3,
	(unsigned char *)ov9650_ev_p4, (unsigned char *)ov9650_ev_p5,
	(unsigned char *)ov9650_ev_p6,
};

/*
 * Auto White Balance configure
 */
static const struct ov9650_reg ov9650_awb_off[] = {
};

static const struct ov9650_reg ov9650_awb_on[] = {
};

static const unsigned char *ov9650_regs_awb_enable[] = {
	(unsigned char *)ov9650_awb_off,
	(unsigned char *)ov9650_awb_on,
};

/*
 * Manual White Balance (presets)
 */
static const struct ov9650_reg ov9650_wb_tungsten[] = {

};

static const struct ov9650_reg ov9650_wb_fluorescent[] = {

};

static const struct ov9650_reg ov9650_wb_sunny[] = {

};

static const struct ov9650_reg ov9650_wb_cloudy[] = {

};

/* Order of this array should be following the querymenu data */
static const unsigned char *ov9650_regs_wb_preset[] = {
	(unsigned char *)ov9650_wb_sunny,
	(unsigned char *)ov9650_wb_cloudy,
	(unsigned char *)ov9650_wb_tungsten,
	(unsigned char *)ov9650_wb_fluorescent,
};

/*
 * Color Effect (COLORFX)
 */
static const struct ov9650_reg ov9650_color_normal[] = {
};

static const struct ov9650_reg ov9650_color_monochrome[] = {
};

static const struct ov9650_reg ov9650_color_sepia[] = {
};

static const struct ov9650_reg ov9650_color_aqua[] = {
};

static const struct ov9650_reg ov9650_color_negative[] = {
};

static const struct ov9650_reg ov9650_color_sketch[] = {
};

/* Order of this array should be following the querymenu data */
static const unsigned char *ov9650_regs_color_effect[] = {
	(unsigned char *)ov9650_color_normal,
	(unsigned char *)ov9650_color_monochrome,
	(unsigned char *)ov9650_color_sepia,
	(unsigned char *)ov9650_color_aqua,
	(unsigned char *)ov9650_color_sketch,
	(unsigned char *)ov9650_color_negative,
};

/*
 * Contrast bias
 */
static const struct ov9650_reg ov9650_contrast_m2[] = {
};

static const struct ov9650_reg ov9650_contrast_m1[] = {
};

static const struct ov9650_reg ov9650_contrast_default[] = {
};

static const struct ov9650_reg ov9650_contrast_p1[] = {
};

static const struct ov9650_reg ov9650_contrast_p2[] = {
};

static const unsigned char *ov9650_regs_contrast_bias[] = {
	(unsigned char *)ov9650_contrast_m2,
	(unsigned char *)ov9650_contrast_m1,
	(unsigned char *)ov9650_contrast_default,
	(unsigned char *)ov9650_contrast_p1,
	(unsigned char *)ov9650_contrast_p2,
};

/*
 * Saturation bias
 */
static const struct ov9650_reg ov9650_saturation_m2[] = {
};

static const struct ov9650_reg ov9650_saturation_m1[] = {
};

static const struct ov9650_reg ov9650_saturation_default[] = {
};

static const struct ov9650_reg ov9650_saturation_p1[] = {
};

static const struct ov9650_reg ov9650_saturation_p2[] = {
};

static const unsigned char *ov9650_regs_saturation_bias[] = {
	(unsigned char *)ov9650_saturation_m2,
	(unsigned char *)ov9650_saturation_m1,
	(unsigned char *)ov9650_saturation_default,
	(unsigned char *)ov9650_saturation_p1,
	(unsigned char *)ov9650_saturation_p2,
};

/*
 * Sharpness bias
 */
static const struct ov9650_reg ov9650_sharpness_m2[] = {
};

static const struct ov9650_reg ov9650_sharpness_m1[] = {
};

static const struct ov9650_reg ov9650_sharpness_default[] = {
};

static const struct ov9650_reg ov9650_sharpness_p1[] = {
};

static const struct ov9650_reg ov9650_sharpness_p2[] = {
};

static const unsigned char *ov9650_regs_sharpness_bias[] = {
	(unsigned char *)ov9650_sharpness_m2,
	(unsigned char *)ov9650_sharpness_m1,
	(unsigned char *)ov9650_sharpness_default,
	(unsigned char *)ov9650_sharpness_p1,
	(unsigned char *)ov9650_sharpness_p2,
};
#endif

#endif
