/* linux/drivers/media/video/ov9650.c
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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/ov9650_platform.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#include "ov9650.h"

#define OV9650_DRIVER_NAME	"ov9650"


/* Default resolution & pixelformat. plz ref ov9650_platform.h */
#if	0
#define DEFAULT_RES			WVGA				/* Index of resoultion */
#define DEFAUT_FPS_INDEX	OV9650_15FPS
#endif
#define DEFAULT_FMT			V4L2_PIX_FMT_YUYV	/* YUV422 */


/*
 * Specification
 * Parallel : ITU-R. 656/601 YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Serial : MIPI CSI2 (single lane) YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Resolution : 1280 (H) x 1024 (V)
 * Image control : Brightness, Contrast, Saturation, Sharpness, Glamour
 * Effect : Mono, Negative, Sepia, Aqua, Sketch
 * FPS : 15fps @full resolution, 30fps @VGA, 24fps @720p
 * Max. pixel clock frequency : 48MHz(upto)
 * Internal PLL (6MHz to 27MHz input frequency)
 */

static int ov9650_init(struct v4l2_subdev *sd, u32 val);

/* Camera functional setting values configured by user concept */
struct ov9650_userset {
	signed int		exposure_bias;	/* V4L2_CID_EXPOSURE */
	unsigned int	ae_lock;
	unsigned int	awb_lock;
	unsigned int	auto_wb;		/* V4L2_CID_CAMERA_WHITE_BALANCE */
	unsigned int	manual_wb;		/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int	wb_temp;		/* V4L2_CID_WHITE_BALANCE_TEMPERATURE */
	unsigned int	effect;			/* Color FX (AKA Color tone) */
	unsigned int	contrast;		/* V4L2_CID_CAMERA_CONTRAST */
	unsigned int	saturation;		/* V4L2_CID_CAMERA_SATURATION */
	unsigned int	sharpness;		/* V4L2_CID_CAMERA_SHARPNESS */
	unsigned int	glamour;
};

struct ov9650_state {
	struct ov9650_platform_data	*pdata;
	struct v4l2_subdev		sd;
	struct v4l2_pix_format	pix;
	struct v4l2_fract		timeperframe;
	struct ov9650_userset	userset;
	int				framesize_index;
	int				freq;	/* MCLK in KHz */
	int				is_mipi;
	int				isize;
	int				ver;
	int				fps;
	int				check_previewdata;
};

static inline struct ov9650_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct ov9650_state, sd);
}


/* Preview info. */
enum {
	OV9650_PREVIEW_VGA,
};

struct ov9650_enum_framesize {
	unsigned int	index;
	unsigned int	width;
	unsigned int	height;
};

struct ov9650_enum_framesize ov9650_framesize_list[] = {
	{ OV9650_PREVIEW_VGA, 640, 480 }
};


/*
 * OV965x register operators
 */
static int ov9650_reg_read(struct i2c_client *client, u8 reg, u8 *val)
{
	int ret;
	u8 data = reg;
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 1,
		.buf	= &data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	msg.flags = I2C_M_RD;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	*val = data;
	return 0;

err:
	dev_err(&client->dev, "Failed reading register 0x%02x!\n", reg);
	return ret;
}

static int ov9650_reg_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	u8 _val;
	unsigned char data[2] = { reg, val };
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

	/* we have to read the register back ... no idea why, maybe HW bug */
	ret = ov9650_reg_read(client, reg, &_val);
	if (ret)
		dev_err(&client->dev,
			"Failed reading back register 0x%02x!\n", reg);

	return 0;
}

/* Read a register, alter its bits, write it back */
static int ov9650_reg_rmw(struct i2c_client *client, u8 reg, u8 set, u8 unset)
{
	u8 val;
	int ret;

	ret = ov9650_reg_read(client, reg, &val);
	if (ret) {
		dev_err(&client->dev,
			"[Read]-Modify-Write of register %02x failed!\n", reg);
		return val;
	}

	val |= set;
	val &= ~unset;

	ret = ov9650_reg_write(client, reg, val);
	if (ret)
		dev_err(&client->dev,
			"Read-Modify-[Write] of register %02x failed!\n", reg);

	return ret;
}


/* sysfs support */
#define	OV9650_REG_NUM		0x100
static ssize_t ov9650_reg_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 val;
	int i, len = 0;

	for (i = 0; i < OV9650_REG_NUM; i++) {
		ov9650_reg_read(client, i, &val);
		len += sprintf(buf + len, "%02x  %02x\n", i, val);
	}

	return len;
}

static ssize_t ov9650_reg_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int reg, val;
	u8 new;

	if (sscanf(buf, "%x %x", &reg, &val) != 2) {
		return -EINVAL;
	}

	if (reg < 0 || reg >= OV9650_REG_NUM) {
		return -EINVAL;
	}

	ov9650_reg_write(client, reg, val);
	ov9650_reg_read(client, reg, &new);
	printk("%02x  %02x (%02x)\n", reg, new, val);

	return strnlen(buf, count);
}

static DEVICE_ATTR(ov9650_reg, S_IRUGO | S_IWUSR, ov9650_reg_show, ov9650_reg_store);


/* Soft reset the camera. This has nothing to do with the RESET pin! */
static int ov9650_reset(struct i2c_client *client)
{
	int ret;

	ret = ov9650_reg_write(client, OV9650_COM7, OV9650_COM7_SCCB_RESET);
	if (ret)
		dev_err(&client->dev,
			"An error occured while entering soft reset!\n");

	return ret;
}

static int ov9650_chip_probe(struct i2c_client *client)
{
	const char	*devname;
	u8 pid, ver, midh, midl;
	int	ret = 0;

	/*
	 * check and show product ID and manufacturer ID 
	 */

	ret = ov9650_reg_read(client, OV9650_PID, &pid);
	if (ret)
		goto err;

	ret = ov9650_reg_read(client, OV9650_VER, &ver);
	if (ret)
		goto err;

	ret = ov9650_reg_read(client, OV9650_MIDH, &midh);
	if (ret)
		goto err;

	ret = ov9650_reg_read(client, OV9650_MIDL, &midl);
	if (ret)
		goto err;

	switch (VERSION(pid, ver)) {
		case OV9652:
			devname = "ov9652";
			break;
		default:
			dev_err(&client->dev, "Product ID error %x:%x\n", pid, ver);
			ret = -ENODEV;
			goto err;
	}

	dev_info(&client->dev, "%s Product ID %0x:%0x Manufacturer ID %x:%x\n",
			devname, pid, ver, midh, midl);

err:
	return ret;
}


#if	0
static int ov9650_write_regs(struct v4l2_subdev *sd, unsigned char regs[],
		int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

	for (i = 0; i < size; i++) {
		err = ov9650_i2c_write(sd, &regs[i], sizeof(regs[i]));
		if (err < 0)
			v4l_info(client, "%s: register set failed\n", __func__);
	}

	return 0;
}

static int ov9650_reset(struct v4l2_subdev *sd)
{
	return ov9650_init(sd, 0);
}

static const char *ov9650_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};

static const char *ov9650_querymenu_effect_mode[] = {
	"Effect Sepia", "Effect Aqua", "Effect Monochrome",
	"Effect Negative", "Effect Sketch", NULL
};

static const char *ov9650_querymenu_ev_bias_mode[] = {
	"-3EV",	"-2,1/2EV", "-2EV", "-1,1/2EV",
	"-1EV", "-1/2EV", "0", "1/2EV",
	"1EV", "1,1/2EV", "2EV", "2,1/2EV",
	"3EV", NULL
};

static struct v4l2_queryctrl ov9650_controls[] = {
	{
		/*
		 * For now, we just support in preset type
		 * to be close to generic WB system,
		 * we define color temp range for each preset
		 */
		.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "White balance in kelvin",
		.minimum = 0,
		.maximum = 10000,
		.step = 1,
		.default_value = 0,	/* FIXME */
	},
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(ov9650_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_CAMERA_WHITE_BALANCE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Auto white balance",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Exposure bias",
		.minimum = 0,
		.maximum = ARRAY_SIZE(ov9650_querymenu_ev_bias_mode) - 2,
		.step = 1,
		.default_value =
			(ARRAY_SIZE(ov9650_querymenu_ev_bias_mode) - 2) / 2,
			/* 0 EV */
	},
	{
		.id = V4L2_CID_CAMERA_EFFECT,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Image Effect",
		.minimum = 0,
		.maximum = ARRAY_SIZE(ov9650_querymenu_effect_mode) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_CAMERA_CONTRAST,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Contrast",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_SATURATION,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Saturation",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_SHARPNESS,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Sharpness",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
};

const char **ov9650_ctrl_get_menu(u32 id)
{
	switch (id) {
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return ov9650_querymenu_wb_preset;

	case V4L2_CID_CAMERA_EFFECT:
		return ov9650_querymenu_effect_mode;

	case V4L2_CID_EXPOSURE:
		return ov9650_querymenu_ev_bias_mode;

	default:
		return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *ov9650_find_qctrl(int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov9650_controls); i++)
		if (ov9650_controls[i].id == id)
			return &ov9650_controls[i];

	return NULL;
}

static int ov9650_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov9650_controls); i++) {
		if (ov9650_controls[i].id == qc->id) {
			memcpy(qc, &ov9650_controls[i],
				sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int ov9650_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;

	qctrl.id = qm->id;
	ov9650_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, ov9650_ctrl_get_menu(qm->id));
}
#endif

static int ov9650_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	return err;
}

static int ov9650_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	return err;
}

static int ov9650_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	struct ov9650_state *state = to_state(sd);
	struct ov9650_enum_framesize *elem;
	int num_entries = ARRAY_SIZE(ov9650_framesize_list);
	int index = 0;
	int i = 0;

	/* The camera interface should read this value, this is the resolution
	 * at which the sensor would provide framedata to the camera i/f
	 *
	 * In case of image capture,
	 * this returns the default camera resolution (VGA)
	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	index = state->framesize_index;

	for (i = 0; i < num_entries; i++) {
		elem = &ov9650_framesize_list[i];
		if (elem->index == index) {
			fsize->discrete.width = ov9650_framesize_list[index].width;
			fsize->discrete.height = ov9650_framesize_list[index].height;
			return 0;
		}
	}

	return -EINVAL;
}

static int ov9650_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *fival)
{
	int err = 0;

	return err;
}

static int ov9650_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int err = 0;

	return err;
}

static int ov9650_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	return err;
}

static int ov9650_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	dev_dbg(&client->dev, "%s\n", __func__);

	return err;
}

static int ov9650_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	dev_dbg(&client->dev, "%s: numerator %d, denominator: %d\n",
			__func__, param->parm.capture.timeperframe.numerator,
			param->parm.capture.timeperframe.denominator);

	return err;
}

static int ov9650_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov9650_state *state = to_state(sd);
	struct ov9650_userset userset = state->userset;
	int err = 0;

	switch (ctrl->id) {
		case V4L2_CID_CAMERA_WHITE_BALANCE:
			ctrl->value = userset.auto_wb;
			break;
		case V4L2_CID_WHITE_BALANCE_PRESET:
			ctrl->value = userset.manual_wb;
			break;
		case V4L2_CID_CAMERA_EFFECT:
			ctrl->value = userset.effect;
			break;
		case V4L2_CID_CAMERA_CONTRAST:
			ctrl->value = userset.contrast;
			break;
		case V4L2_CID_CAMERA_SATURATION:
			ctrl->value = userset.saturation;
			break;
		case V4L2_CID_CAMERA_SHARPNESS:
			ctrl->value = userset.saturation;
			break;
		case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		case V4L2_CID_CAM_JPEG_THUMB_SIZE:
		case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
		case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		case V4L2_CID_CAM_JPEG_MEMSIZE:
		case V4L2_CID_CAM_JPEG_QUALITY:
		case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT_FIRST:
		case V4L2_CID_CAM_DATE_INFO_YEAR:
		case V4L2_CID_CAM_DATE_INFO_MONTH:
		case V4L2_CID_CAM_DATE_INFO_DATE:
		case V4L2_CID_CAM_SENSOR_VER:
		case V4L2_CID_CAM_FW_MINOR_VER:
		case V4L2_CID_CAM_FW_MAJOR_VER:
		case V4L2_CID_CAM_PRM_MINOR_VER:
		case V4L2_CID_CAM_PRM_MAJOR_VER:
		case V4L2_CID_ESD_INT:
		case V4L2_CID_CAMERA_GET_ISO:
		case V4L2_CID_CAMERA_GET_SHT_TIME:
		case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
		case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
			ctrl->value = 0;
			break;
		case V4L2_CID_EXPOSURE:
			ctrl->value = userset.exposure_bias;
			break;
		default:
			dev_err(&client->dev, "%s: no such ctrl\n", __func__);
			/* err = -EINVAL; */
			break;
	}

	return err;
}

static int ov9650_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov9650_state *state = to_state(sd);
	int err = 0;
	int value = ctrl->value;

	printk("ov9650_s_ctrl: %p ... ID=%08x value=%d\n", sd, ctrl->id, value);

	return 0;
#if	0
	switch (ctrl->id) {

	case V4L2_CID_CAMERA_FLASH_MODE:
	case V4L2_CID_CAMERA_BRIGHTNESS:
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_WHITE_BALANCE\n",
			__func__);

		if (value <= WHITE_BALANCE_AUTO) {
			err = ov9650_write_regs(sd,
			(unsigned char *) ov9650_regs_awb_enable[value],
				sizeof(ov9650_regs_awb_enable[value]));
		} else {
			err = ov9650_write_regs(sd,
			(unsigned char *) ov9650_regs_wb_preset[value-2],
				sizeof(ov9650_regs_wb_preset[value-2]));
		}
		break;
	case V4L2_CID_CAMERA_EFFECT:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_EFFECT\n", __func__);
		err = ov9650_write_regs(sd,
		(unsigned char *) ov9650_regs_color_effect[value-1],
			sizeof(ov9650_regs_color_effect[value-1]));
		break;
	case V4L2_CID_CAMERA_ISO:
	case V4L2_CID_CAMERA_METERING:
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_CONTRAST\n", __func__);
		err = ov9650_write_regs(sd,
		(unsigned char *) ov9650_regs_contrast_bias[value],
			sizeof(ov9650_regs_contrast_bias[value]));
		break;
	case V4L2_CID_CAMERA_SATURATION:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_SATURATION\n", __func__);
		err = ov9650_write_regs(sd,
		(unsigned char *) ov9650_regs_saturation_bias[value],
			sizeof(ov9650_regs_saturation_bias[value]));
		break;
	case V4L2_CID_CAMERA_SHARPNESS:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_SHARPNESS\n", __func__);
		err = ov9650_write_regs(sd,
		(unsigned char *) ov9650_regs_sharpness_bias[value],
			sizeof(ov9650_regs_sharpness_bias[value]));
		break;
	case V4L2_CID_CAMERA_WDR:
	case V4L2_CID_CAMERA_FACE_DETECTION:
	case V4L2_CID_CAMERA_FOCUS_MODE:
	case V4L2_CID_CAM_JPEG_QUALITY:
	case V4L2_CID_CAMERA_SCENE_MODE:
	case V4L2_CID_CAMERA_GPS_LATITUDE:
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
	case V4L2_CID_CAMERA_FRAME_RATE:
		break;
	case V4L2_CID_CAM_PREVIEW_ONOFF:
		if (state->check_previewdata == 0)
			err = 0;
		else
			err = -EIO;
		break;
	case V4L2_CID_CAMERA_CHECK_DATALINE:
	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
		break;
	case V4L2_CID_CAMERA_RESET:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_RESET\n", __func__);
		err = ov9650_reset(sd);
		break;
	case V4L2_CID_EXPOSURE:
		dev_dbg(&client->dev, "%s: V4L2_CID_EXPOSURE\n", __func__);
		err = ov9650_write_regs(sd,
		(unsigned char *) ov9650_regs_ev_bias[value],
			sizeof(ov9650_regs_ev_bias[value]));
		break;
	default:
		dev_err(&client->dev, "%s: no such control\n", __func__);
		/* err = -EINVAL; */
		break;
	}

	if (err < 0)
		dev_dbg(&client->dev, "%s: vidioc_s_ctrl failed\n", __func__);

	return err;
#endif
}

static int ov9650_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov9650_state *state = to_state(sd);
	int err = -EINVAL, i;

	v4l_info(client, "%s: camera initialization start\n", __func__);

	for (i = 0; i < ARRAY_SIZE(ov9650_init_reg); i++) {
		err = ov9650_reg_write(client, ov9650_init_reg[i][0],
					ov9650_init_reg[i][1]);
		if (err < 0)
			v4l_info(client, "%s: register set failed\n", __func__);
	}

	if (err < 0) {
		/* This is preview fail */
		state->check_previewdata = 100;
		v4l_err(client,
			"%s: camera initialization failed. err(%d)\n",
			__func__, state->check_previewdata);
		return -EIO;
	}

	/* This is preview success */
	state->check_previewdata = 0;
	return 0;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize every single opening time
 * therefor, it is not necessary to be initialized on probe time.
 * except for version checking.
 * NOTE: version checking is optional
 */
static int ov9650_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov9650_state *state = to_state(sd);
	struct ov9650_platform_data *pdata;

	dev_info(&client->dev, "fetching platform data\n");

	pdata = client->dev.platform_data;

	if (!pdata) {
		dev_err(&client->dev, "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	if (!(pdata->default_width && pdata->default_height)) {
		/* TODO: assign driver default resolution */
	} else {
		state->pix.width = pdata->default_width;
		state->pix.height = pdata->default_height;
	}

	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = 24000000;	/* 24MHz default */
	else
		state->freq = pdata->freq;

	if (!pdata->is_mipi) {
		state->is_mipi = 0;
		dev_info(&client->dev, "parallel mode\n");
	} else
		state->is_mipi = pdata->is_mipi;

	return 0;
}

static const struct v4l2_subdev_core_ops ov9650_core_ops = {
	.init			= ov9650_init,	/* initializing API */
	.s_config		= ov9650_s_config,	/* Fetch platform data */
//	.queryctrl		= ov9650_queryctrl,
//	.querymenu		= ov9650_querymenu,
	.g_ctrl			= ov9650_g_ctrl,
	.s_ctrl			= ov9650_s_ctrl,

//	.g_chip_ident	= ov9640_g_chip_ident,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register		= ov9640_get_register,
	.s_register		= ov9640_set_register,
#endif
};

static const struct v4l2_subdev_video_ops ov9650_video_ops = {
	.g_fmt			= ov9650_g_fmt,
	.s_fmt			= ov9650_s_fmt,
	.enum_framesizes	= ov9650_enum_framesizes,
	.enum_frameintervals= ov9650_enum_frameintervals,
	.enum_fmt		= ov9650_enum_fmt,
	.try_fmt		= ov9650_try_fmt,
	.g_parm			= ov9650_g_parm,
	.s_parm 		= ov9650_s_parm,
};

static const struct v4l2_subdev_ops ov9650_ops = {
	.core			= &ov9650_core_ops,
	.video			= &ov9650_video_ops,
};


/*
 * ov9650_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int ov9650_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct ov9650_state *state;
	struct v4l2_subdev *sd;

	state = kzalloc(sizeof(struct ov9650_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	sd = &state->sd;
	strcpy(sd->name, OV9650_DRIVER_NAME);

#if	0
	{
		int i;
		u8 val;

		ov9650_reset(client);

		for (i = 0; i < OV9650_REG_NUM; i++) {
			ov9650_reg_read(client, i, &val);
			printk("%02x  %02x\n", i, val);
		}
	}
#endif

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &ov9650_ops);

	ov9650_chip_probe(client);

	if (device_create_file(&client->dev, &dev_attr_ov9650_reg)) {
		dev_err(&client->dev, "Create sysfs file failed\n");
	}

	dev_info(&client->dev, "ov9650 has been probed\n");
	return 0;
}

static int ov9650_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}


static const struct i2c_device_id ov9650_id[] = {
	{ OV9650_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, ov9650_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = OV9650_DRIVER_NAME,
	.probe = ov9650_probe,
	.remove = __devexit_p(ov9650_remove),
	.id_table = ov9650_id,
};


MODULE_DESCRIPTION("OmniVision OV9650 SXGA camera driver");
MODULE_AUTHOR("FriendlyARM <www.arm9.net>");
MODULE_LICENSE("GPL");

