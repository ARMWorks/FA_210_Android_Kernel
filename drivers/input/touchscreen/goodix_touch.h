/*
 * include/linux/goodix_touch.h
 *
 * Copyright (C) 2008 Goodix, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __LINUX_GOODIX_TOUCH_H__
#define __LINUX_GOODIX_TOUCH_H__

#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <plat/goodix_touch.h>

#define GUITAR_GT80X		1
#define GOODIX_I2C_NAME		"gt80x-ts"


/* Touch panel resolution */
#define TOUCH_MAX_WIDTH	 	5120
#define TOUCH_MAX_HEIGHT 	7680	

/* Screen resolution for coord. remap (disabled) */
#define SCREEN_MAX_WIDTH	800
#define SCREEN_MAX_HEIGHT	480

#if	0
#define SHUTDOWN_PORT		S5PV210_GPH1(6)
#define INT_PORT			S5PV210_GPH1(7)
#define INT_CFG				S3C_GPIO_SFN(0xf)
#endif


#define GOODIX_MULTI_TOUCH
#ifndef GOODIX_MULTI_TOUCH
	#define MAX_FINGER_NUM	1
#else
	#define MAX_FINGER_NUM	5
#endif

#if defined(INT_PORT)
	#if (MAX_FINGER_NUM <= 3)
	#define READ_BYTES_NUM	(1 + 2 + MAX_FINGER_NUM * 5)
	#elif (MAX_FINGER_NUM == 4)
	#define READ_BYTES_NUM	(1 + 28)
	#elif (MAX_FINGER_NUM == 5)
	#define READ_BYTES_NUM	(1 + 34)
	#endif
#else	
	#define READ_BYTES_NUM	(1 + 34)
#endif

#ifndef swap
#define swap(x, y) do { typeof(x) z = x; x = y; y = z; } while (0)
#endif


enum finger_state {
#define FLAG_MASK 0x01
	FLAG_UP = 0,
	FLAG_DOWN = 1,
	FLAG_INVALID = 2,
};


struct point_node {
	uint8_t id;
	enum finger_state state;
	uint8_t pressure;
	unsigned int x;
	unsigned int y;
};

struct goodix_ts_data {
	int retry;
	int panel_type;
	char phys[32];		

	struct i2c_client *client;
	struct input_dev *input_dev;

	uint8_t use_irq;
	uint8_t use_shutdown;

	uint32_t gpio_shutdown;
	uint32_t gpio_irq;
	uint32_t screen_width;
	uint32_t screen_height;

	struct hrtimer timer;
	struct work_struct  work;
	struct early_suspend early_suspend;

	int (*power)(struct goodix_ts_data * ts, int on);
};

#endif	// __LINUX_GOODIX_TOUCH_H__

