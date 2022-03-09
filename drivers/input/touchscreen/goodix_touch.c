/*----------------------------------------------------------------------------
 * driver/input/touchscreen/goodix_touch.c
 *
 * Copyright(c) 2010 Goodix Technology Corp.
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
 * ChangeLog: 
 *	2010.11.11, add point_queue's definiens.
 *
 *	2011.03.09, rewrite point_queue's definiens.
 *
 *	2011.05.12, delete point_queue for Android 2.2/Android 2.3 and so on.
 *
 *	2012.02.22, FriendlyARM support
 *
 *---------------------------------------------------------------------------*/
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/ctouch.h>

#include "goodix_touch.h"

#ifndef GUITAR_GT80X
#error "Goodix touch driver does not match the hardware version."
#endif

/* Required for current configs according to test */
#define SWAP_XY			1
#define INVERT_X		1	/* 0 - 1R07; 1 - 1R08 (TBD: or later ?) */


const char *s3c_ts_name = "goodix_ts_80x";
static struct workqueue_struct *goodix_wq;

/* used by GT80X-IAP module */
struct i2c_client *i2c_connect_client = NULL;
EXPORT_SYMBOL(i2c_connect_client);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h);
static void goodix_ts_late_resume(struct early_suspend *h);
#endif


/* Chip I/O operations */
static int i2c_read_bytes(struct i2c_client *client,
		uint8_t *buf, int16_t len)
{
	struct i2c_msg msgs[2];
	int ret = -1;

	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr = client->addr;
	msgs[0].len = 1;
	msgs[0].buf = buf;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = client->addr;
	msgs[1].len = len - 1;
	msgs[1].buf = buf + 1;

	ret = i2c_transfer(client->adapter, msgs, 2);
	return ret;
}

static int i2c_write_bytes(struct i2c_client *client,
		uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret = -1;

	msg.flags = !I2C_M_RD;
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	return ret;
}

static int goodix_init_panel(struct goodix_ts_data *ts) {
	int ret = -1;
	int count;

	/* NOTE: Get configs from Goodix and update it */
	uint8_t config_info[54] = { 0x30,
		0x0F, 0x05, 0x06, 0x28, 0x02, 0x14, 0x14, 0x10,		/* Reg 0x30 ~ 0x37 */
		0x28, 0xB2,
#if	0
		/* 1024 x 600 */
		0x02, 0x58,											/* XMaxH, XMaxL */
		0x04, 0x00,											/* YMaxH, YMaxL */
#else
		/*  800 x 480 */
		0x01, 0xe0,
		0x03, 0x20,
#endif
		0xED, 0xCB,
		0xA9, 0x87, 0x65, 0x43, 0x21, 0x00, 0x00, 0x00,		/* Reg 0x40 ~ 0x47 */
		0x00, 0x00, 0x4D, 0xC1, 0x20, 0x01, 0x01, 0x41,		/* Reg 0x48 ~ 0x4f */
		0x64, 0x3C, 0x1E, 0x28, 0x0E, 0x00, 0x00, 0x00,		/* Reg 0x50 ~ 0x57 */
		0x00, 0x50, 0x3C, 0x32, 0x71, 0x00, 0x00, 0x00,		/* Reg 0x58 ~ 0x5f */
		0x00, 0x00, 0x00, 0x00, 0x01						/* Reg 0x60 ~ 0x64 */
	};

	/* Update Xmax and Ymax */
#define	XMAX_H		(0x3a - 0x30 + 1)
#ifdef SWAP_XY
	config_info[XMAX_H] = ts->screen_height >> 8;
	config_info[XMAX_H + 1] = ts->screen_height & 0xff;
	config_info[XMAX_H + 2] = ts->screen_width >> 8;
	config_info[XMAX_H + 3] = ts->screen_width & 0xff;
#else
	config_info[XMAX_H] = ts->screen_width >> 8;
	config_info[XMAX_H + 1] = ts->screen_width & 0xff;
	config_info[XMAX_H + 2] = ts->screen_height >> 8;
	config_info[XMAX_H + 3] = ts->screen_height & 0xff;
#endif

	for (count = 5; count > 0; count--) {
		ret = i2c_write_bytes(ts->client, config_info, 54);
		if (ret == 1)
			break;
		else
			msleep(10);
	}

	return (ret == 1) ? 0 : -1;
}

static int goodix_read_version(struct goodix_ts_data *ts) {
#define GT80X_VERSION_LENGTH	40
	uint8_t version[2] = { 0x69, 0xff };
	uint8_t version_data[GT80X_VERSION_LENGTH + 1] = { 0x6A };
	int ret;

	memset(version_data+1, 0, GT80X_VERSION_LENGTH);
	ret = i2c_write_bytes(ts->client, version, 2);
	if (ret != 1) {
		return ret;
	}

	msleep(50);
	ret = i2c_read_bytes(ts->client, version_data, GT80X_VERSION_LENGTH);
	if (ret != 2) {
		strncpy(version_data+1, "NULL", 4);
	}

	dev_info(&ts->client->dev, "GT80X Version: %s\n", version_data + 1);
	version[1] = 0x00;
	i2c_write_bytes(ts->client, version, 2);

	return 0;
}

static void goodix_report_fingers(struct goodix_ts_data *ts,
		struct point_node *points)
{
	struct point_node *p = points;
	int num_fingers = 0;
	int i;

#ifndef GOODIX_MULTI_TOUCH
	if (p->state == FLAG_DOWN) {
		input_report_abs(ts->input_dev, ABS_X, p->x);
		input_report_abs(ts->input_dev, ABS_Y, p->y);
	} 

	input_report_abs(ts->input_dev, ABS_PRESSURE, p->pressure);
	input_report_key(ts->input_dev, BTN_TOUCH,
			(p->state == FLAG_INVALID) ? FLAG_UP : p->state);

#else
	/* Multi-touch support */
	for (i = 0; i < MAX_FINGER_NUM; i++, p++) {
		if (p->state != FLAG_DOWN)
			continue;

		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, p->x);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, p->y);

		input_report_abs(ts->input_dev, ABS_MT_PRESSURE, p->pressure);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, p->pressure);
		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, p->id);

		input_mt_sync(ts->input_dev);
		num_fingers++;
	}

	if (!num_fingers) {
		input_mt_sync(ts->input_dev);
	}
#endif

	input_sync(ts->input_dev);
}

static void goodix_ts_work_func(struct work_struct *work)
{
	static struct point_node pointer[MAX_FINGER_NUM];
	static uint8_t finger_last = 0;
	struct goodix_ts_data *ts = container_of(work, struct goodix_ts_data, work);
	struct point_node *p = NULL;
	uint8_t read_pos = 0;
	uint8_t point_data[READ_BYTES_NUM] = { 0 };
	uint8_t finger, finger_current;
	uint8_t check_sum = 0;
	unsigned int x, y;
	int count = 0;
	int ret = -1; 

	if (ts->use_shutdown && gpio_get_value(ts->gpio_shutdown)) {
		goto NO_ACTION;
	}

	ret = i2c_read_bytes(ts->client, point_data, sizeof(point_data));
	if (ret <= 0) {
		dev_dbg(&(ts->client->dev), "I2C transfer error, %d\n", ret);

		if (++ts->retry > 100) {
			dev_err(&(ts->client->dev), "Reset the chip for I2C error\n");
			ts->retry = 0;

			if (ts->power) {
				ts->power(ts, 0);
				ts->power(ts, 1);
			} else {
				goodix_init_panel(ts);
				msleep(200);
			}
		}

		goto XFER_ERROR;
	}

	if (!ts->use_irq) {
		switch (point_data[1] & 0x1f) {
			case 0:
				break;
			case 1:
				read_pos = 8;
				break;
			case 2:
			case 3:
				read_pos = 13;
				break;
			default:
				read_pos = 34;
				break;
		}

		for (count = 1; count < read_pos; count++) {
			check_sum += (int) point_data[count];
		}

		if (check_sum != point_data[read_pos]) {
			dev_dbg(&(ts->client->dev), "goodix: check sum error %02x <--> %02x\n",
					check_sum, point_data[read_pos]);
			goto XFER_ERROR;
		}
	}

	finger_current = point_data[1] & 0x1f;
	finger = finger_current ^ finger_last;
	if (unlikely(finger == 0)) {
		if (finger_current == 0)
			goto NO_ACTION;

		goto BIT_NO_CHANGE;
	}

	/* check which point(s) DOWN or UP */
	for (count = 0, p = pointer; count < MAX_FINGER_NUM; count++, p++) {
		p->id = count;
		if (finger_current & FLAG_MASK) {
			p->state = FLAG_DOWN;
		} else {
			if (finger & FLAG_MASK)
				p->state = FLAG_UP;
			else
				p->state = FLAG_INVALID;
		}

		finger >>= 1;
		finger_current >>= 1;
	}

	/* save current finger state for furture */
	finger_last = point_data[1] & 0x1f;

BIT_NO_CHANGE:
	for (count = 0; count < MAX_FINGER_NUM; count++) {
		p = &pointer[count];
		if (p->state == FLAG_INVALID)
			continue;

		if (p->state == FLAG_UP) {
			x = y = 0;
			p->pressure = 0;
			continue;
		}

		if (p->id < 3)
			read_pos = p->id * 5 + 3;
		else
			read_pos = 29;

		if (p->id != 3) {
			x = point_data[read_pos] << 8;
			x+= point_data[read_pos + 1];

			y = point_data[read_pos + 2] << 8;
			y+= point_data[read_pos + 3];

			p->pressure = point_data[read_pos + 4];
		}
#if MAX_FINGER_NUM > 3
		else {
			x = (unsigned int)(point_data[18]<<8) + (unsigned int)(point_data[25]);
			y = (unsigned int)(point_data[26]<<8) + (unsigned int)(point_data[27]);
			p->pressure = point_data[28];
		}
#endif

#if	0
		/* Touch coord. --> LCD coordinates */
		x = (TOUCH_MAX_WIDTH - x) * SCREEN_MAX_WIDTH / TOUCH_MAX_WIDTH;
		y = y * SCREEN_MAX_HEIGHT / TOUCH_MAX_HEIGHT;
#endif

#ifdef SWAP_XY
		p->x = y;
		p->y = x;
#else
		p->x = x;
		p->y = y;
#endif
#ifdef INVERT_X
		p->x = ts->screen_width - p->x;
#endif
	}

	goodix_report_fingers(ts, pointer);

XFER_ERROR:
NO_ACTION:
	if (ts->use_irq)
		enable_irq(ts->client->irq);
}

static enum hrtimer_restart goodix_ts_timer_func(struct hrtimer *timer)
{
	struct goodix_ts_data *ts = container_of(timer,
			struct goodix_ts_data, timer);

	queue_work(goodix_wq, &ts->work);
	if (ts->timer.state != HRTIMER_STATE_INACTIVE)
		hrtimer_start(&ts->timer, ktime_set(0, 16000000), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

static irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{
	struct goodix_ts_data *ts = dev_id;

	disable_irq_nosync(ts->client->irq);
	queue_work(goodix_wq, &ts->work);

	return IRQ_HANDLED;
}

static int goodix_ts_power(struct goodix_ts_data * ts, int on)
{
	int ret = 0;

	if (!ts->use_shutdown)
		return -1;

	switch (on) {
		case 0:
			gpio_set_value(ts->gpio_shutdown, 1);
			msleep(5);
			if (gpio_get_value(ts->gpio_shutdown))	//has been suspended
				ret = 0;
			break;

		case 1:
			gpio_set_value(ts->gpio_shutdown, 0);
			msleep(5);
			if (gpio_get_value(ts->gpio_shutdown))	//has been waked up
				ret = -1;
			else 
				msleep(200);
			break;
	}

	dev_dbg(&ts->client->dev, "Set Guitar's shutdown to %s (%d)\n",
			on ? "HIGH" : "LOW", ret);

	return ret;
}

/* Detecting goodix device by writing test data
 * Note: Before it SHUTDOWN port Must be low state 30ms or more.
 */
static int goodix_i2c_test(struct i2c_client * client)
{
	int ret, retry;
	uint8_t test_data[1] = { 0 };

	for (retry = 0; retry < 5; retry++) {
		ret = i2c_write_bytes(client, test_data, 1);
		if (ret == 1)
			break;
		msleep(5);
	}

	return (ret == 1) ? 0 : -1;
}


/*
 * I2C client driver functions
 */

static int goodix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct goodix_i2c_platform_data *pdata;
	struct goodix_ts_data *ts;
	int ret = 0;

	dev_dbg(&client->dev, "Install touchscreen driver for guitar.\n");

	if (mini210_get_ctp() != CTP_GT80X)
		return -ENODEV;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "System need I2C function.\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	pdata = client->dev.platform_data;
	if (pdata != NULL) {
		ts->screen_width = pdata->screen_width;
		ts->screen_height = pdata->screen_height;

		ts->gpio_shutdown = pdata->gpio_shutdown;
		ts->gpio_irq = pdata->gpio_irq;

		if (pdata->irq_cfg) {
			s3c_gpio_cfgpin(ts->gpio_irq, pdata->irq_cfg);
			s3c_gpio_setpull(ts->gpio_irq, S3C_GPIO_PULL_NONE);
		}

		if (ts->gpio_irq) {
			client->irq = gpio_to_irq(ts->gpio_irq);
		}
	} else {
#ifdef SHUTDOWN_PORT
		ts->gpio_shutdown = SHUTDOWN_PORT;
#endif
#ifdef INT_PORT
		ts->gpio_irq = INT_PORT;
		s3c_gpio_cfgpin(ts->gpio_irq, INT_CFG);
		client->irq = gpio_to_irq(ts->gpio_irq);
#endif
	}

	/* Initialize default screen resolution */
	if (ts->screen_width == 0)
		ts->screen_width = SCREEN_MAX_WIDTH;
	if (ts->screen_height == 0)
		ts->screen_height = SCREEN_MAX_HEIGHT;

	if (ts->gpio_shutdown) {
		ret = gpio_request(ts->gpio_shutdown, "TS_SHUTDOWN");
		if (ret < 0) {
			dev_err(&client->dev, "Request GPIO %d failed, %d\n", ts->gpio_shutdown, ret);
			goto err_gpio_request;
		}

		gpio_direction_output(ts->gpio_shutdown, 1);
		msleep(100);

		/* output low to wake up device */
		gpio_direction_output(ts->gpio_shutdown, 0);
		msleep(50);
		ret = gpio_get_value(ts->gpio_shutdown);
		if (ret) {
			dev_err(&client->dev, "Can't set touchscreen to work\n");
			goto err_i2c_failed;
		}

		ts->use_shutdown = true;

		/* waiting for initialization of Guitar */
		msleep(25);
	} else {
		ts->use_shutdown = false;
	}

	i2c_connect_client = client;

	ret = goodix_i2c_test(client);
	if (ret) {
		dev_err(&client->dev, "Warning: I2C connection might be something wrong!\n");
		goto err_i2c_failed;
	}

	if (ts->use_shutdown) {
		gpio_set_value(ts->gpio_shutdown, 1);
	}

	INIT_WORK(&ts->work, goodix_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		dev_dbg(&client->dev, "Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

#ifndef GOODIX_MULTI_TOUCH
	ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);

	input_set_abs_params(ts->input_dev, ABS_X, 0, SCREEN_MAX_HEIGHT, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, SCREEN_MAX_WIDTH, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);

#else
	ts->input_dev->absbit[0] = BIT_MASK(ABS_MT_TRACKING_ID) |
		BIT_MASK(ABS_MT_TOUCH_MAJOR)| BIT_MASK(ABS_MT_WIDTH_MAJOR) |
		BIT_MASK(ABS_MT_POSITION_X) | BIT_MASK(ABS_MT_POSITION_Y);

	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0,
			ts->screen_width, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0,
			ts->screen_height, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0,
			MAX_FINGER_NUM, 0, 0);
#endif

	sprintf(ts->phys, "input/goodix-ts");
	ts->input_dev->name = s3c_ts_name;
	ts->input_dev->phys = ts->phys;
	ts->input_dev->id.bustype = BUS_I2C;
	ts->input_dev->id.vendor = 0xDEAD;
	ts->input_dev->id.product = 0xBEEF;
	ts->input_dev->id.version = 0x1105;

	ret = input_register_device(ts->input_dev);
	if (ret) {
		dev_err(&client->dev, "Register input device %s failed, %d\n",
				ts->input_dev->name, ret);
		goto err_input_register_device_failed;
	}

	ts->use_irq = false;
	if (client->irq) {
		ret = gpio_request(ts->gpio_irq, "TS_INT");
		if (ret < 0) {
			dev_err(&client->dev, "Request GPIO %d failed, %d\n", ts->gpio_irq, ret);
			goto err_int_request_failed;
		}

		ret = request_irq(client->irq, goodix_ts_irq_handler, IRQ_TYPE_EDGE_FALLING,
				client->name, ts);
		if (ret != 0) {
			gpio_direction_input(ts->gpio_irq);
			gpio_free(ts->gpio_irq);

			dev_err(&client->dev, "Request IRQ %d failed, %d\n", client->irq, ret);
			goto err_int_request_failed;
		}

		disable_irq(client->irq);
		ts->use_irq = true;
	}

err_int_request_failed:
	if (!ts->use_irq) {
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = goodix_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

	flush_workqueue(goodix_wq);
	if (ts->use_shutdown) {
		gpio_set_value(ts->gpio_shutdown, 0);
		ts->power = goodix_ts_power;
		msleep(30);
	}

	ret = goodix_init_panel(ts);
	if (ret) 
		goto err_init_godix_ts;

	if (ts->use_irq)
		enable_irq(client->irq);

	msleep(50);

	goodix_read_version(ts);
	msleep(260);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = goodix_ts_early_suspend;
	ts->early_suspend.resume = goodix_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	dev_dbg(&client->dev, "Start %s in %s mode\n", ts->input_dev->name, 
			ts->use_irq ? "Interrupt" : "Polling");

	return 0;

err_init_godix_ts:
	if (ts->use_irq) {
		free_irq(client->irq, ts);
		gpio_free(ts->gpio_irq);
	}

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
	i2c_set_clientdata(client, NULL);

err_i2c_failed:
	if (ts->use_shutdown) {
		gpio_direction_input(ts->gpio_shutdown);
		gpio_free(ts->gpio_shutdown);
	}

err_gpio_request:
	kfree(ts);

err_alloc_data_failed:
err_check_functionality_failed:

	return ret;
}

static int goodix_ts_remove(struct i2c_client *client)
{
	struct goodix_ts_data *ts = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif

	if (ts->use_irq) {
		free_irq(client->irq, ts);
		gpio_free(ts->gpio_irq);
	} else {
		hrtimer_cancel(&ts->timer);
	}

	if (ts->use_shutdown) {
		gpio_direction_input(ts->gpio_shutdown);
		gpio_free(ts->gpio_shutdown);
	}

	dev_notice(&client->dev, "The driver is removing...\n");

	i2c_set_clientdata(client, NULL);
	input_unregister_device(ts->input_dev);

	if (ts->input_dev)
		kfree(ts->input_dev);

	kfree(ts);

	return 0;
}

/* Power management support */
static int goodix_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct goodix_ts_data *ts = i2c_get_clientdata(client);

	if (ts->use_irq)
		disable_irq(client->irq);
	else if (ts->timer.state)
		hrtimer_cancel(&ts->timer);

	ret = cancel_work_sync(&ts->work);
	if (ret && ts->use_irq)		//irq was disabled twice.
		enable_irq(client->irq);

	if (ts->power) {
		ret = ts->power(ts,0);
		if (ret < 0)
			dev_warn(&client->dev, "%s power off failed\n", s3c_ts_name);
	}

	return 0;
}

static int goodix_ts_resume(struct i2c_client *client)
{
	int ret;
	struct goodix_ts_data *ts = i2c_get_clientdata(client);

	if (ts->power) {
		ret = ts->power(ts, 1);
		if (ret < 0)
			dev_warn(&client->dev, "%s power on failed\n", s3c_ts_name);
	}

	if (ts->use_irq)
		enable_irq(client->irq);
	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h)
{
	struct goodix_ts_data *ts;
	ts = container_of(h, struct goodix_ts_data, early_suspend);
	goodix_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void goodix_ts_late_resume(struct early_suspend *h)
{
	struct goodix_ts_data *ts;
	ts = container_of(h, struct goodix_ts_data, early_suspend);
	goodix_ts_resume(ts->client);
}
#endif


static const struct i2c_device_id goodix_ts_id[] = {
	{ GOODIX_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver goodix_ts_driver = {
	.probe		= goodix_ts_probe,
	.remove		= goodix_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= goodix_ts_suspend,
	.resume		= goodix_ts_resume,
#endif
	.id_table	= goodix_ts_id,
	.driver = {
		.name	= GOODIX_I2C_NAME,
		.owner = THIS_MODULE,
	},
};


static int __devinit goodix_ts_init(void) {
	int ret;

	goodix_wq = create_singlethread_workqueue("goodix_wq");
	if (!goodix_wq) {
		printk(KERN_ALERT "Create %s workqueue failed.\n", s3c_ts_name);
		return -ENOMEM;
	}

	ret = i2c_add_driver(&goodix_ts_driver);
	return ret; 
}

static void __exit goodix_ts_exit(void) {
	i2c_del_driver(&goodix_ts_driver);

	if (goodix_wq) {
		destroy_workqueue(goodix_wq);
	}
}

late_initcall(goodix_ts_init);
module_exit(goodix_ts_exit);

MODULE_DESCRIPTION("Goodix Touchscreen Driver");
MODULE_LICENSE("GPL v2");

