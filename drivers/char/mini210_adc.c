/*
 * linux/drivers/char/mini210_adc.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>

#include <plat/adc.h>

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK(x...) {printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...) (void)(0)
#endif

#define DEVICE_NAME	"mini210-adc"

typedef struct {
	struct mutex lock;
	struct s3c_adc_client *client;
	int channel;
} ADC_DEV;

static ADC_DEV adcdev;

static inline int exynos_adc_read_ch(void) {
	int ret;

	ret = mutex_lock_interruptible(&adcdev.lock);
	if (ret < 0)
		return ret;

	ret = s3c_adc_read(adcdev.client, adcdev.channel);
	mutex_unlock(&adcdev.lock);

	return ret;
}

static inline void exynos_adc_set_channel(int channel) {
	if (channel < 0 || channel > 9)
		return;

	adcdev.channel = channel;
}

static ssize_t exynos_adc_read(struct file *filp, char *buffer,
		size_t count, loff_t *ppos)
{
	char str[20];
	int value;
	size_t len;

	value = exynos_adc_read_ch();

	len = sprintf(str, "%d\n", value);
	if (count >= len) {
		int r = copy_to_user(buffer, str, len);
		return r ? r : len;
	} else {
		return -EINVAL;
	}
}

static long exynos_adc_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
#define ADC_SET_CHANNEL		0xc000fa01
#define ADC_SET_ADCTSC		0xc000fa02

	switch (cmd) {
		case ADC_SET_CHANNEL:
			exynos_adc_set_channel(arg);
			break;
		case ADC_SET_ADCTSC:
			/* do nothing */
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int exynos_adc_open(struct inode *inode, struct file *filp)
{
	exynos_adc_set_channel(0);

	DPRINTK("adc opened\n");
	return 0;
}

static int exynos_adc_release(struct inode *inode, struct file *filp)
{
	DPRINTK("adc closed\n");
	return 0;
}

static struct file_operations adc_dev_fops = {
	owner:	THIS_MODULE,
	open:	exynos_adc_open,
	read:	exynos_adc_read,	
	unlocked_ioctl:	exynos_adc_ioctl,
	release:	exynos_adc_release,
};

static struct miscdevice misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "adc",
	.fops	= &adc_dev_fops,
};

static int __devinit exynos_adc_probe(struct platform_device *dev)
{
	int ret;

	mutex_init(&adcdev.lock);

	/* Register with the core ADC driver. */
	adcdev.client = s3c_adc_register(dev, NULL, NULL, 0);
	if (IS_ERR(adcdev.client)) {
		printk("mini210_adc: cannot register adc\n");
		ret = PTR_ERR(adcdev.client);
		goto err_mem;
	}

	ret = misc_register(&misc);

	printk(DEVICE_NAME"\tinitialized\n");

err_mem:
	return ret;
}

static int __devexit exynos_adc_remove(struct platform_device *dev)
{
	misc_deregister(&misc);
	s3c_adc_release(adcdev.client);

	return 0;
}

static struct platform_driver exynos_adc_driver = {
	.driver = {
		.name		= "mini210_adc",
		.owner		= THIS_MODULE,
	},
	.probe		= exynos_adc_probe,
	.remove		= __devexit_p(exynos_adc_remove),
};

static int __init exynos_adc_init(void)
{
	return platform_driver_register(&exynos_adc_driver);
}

static void __exit exynos_adc_exit(void)
{
	platform_driver_unregister(&exynos_adc_driver);
}

module_init(exynos_adc_init);
module_exit(exynos_adc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");

