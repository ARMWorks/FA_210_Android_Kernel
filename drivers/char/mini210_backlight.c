#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK(x...)	{printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...)	(void)(0)
#endif

#define DEVICE_NAME		"backlight"

#define GPIO_BL			S5PV210_GPD0(1)


static unsigned int bl_state;

static inline void set_bl(int state)
{
	bl_state = !!state;
	//printk(DEVICE_NAME ": %s\n", state ? "ON" : "OFF");

	gpio_set_value(GPIO_BL, bl_state);
}

static inline unsigned int get_bl(void)
{
	return bl_state;
}

static ssize_t bl_dev_write(struct file *file, const char *buffer,
		size_t count, loff_t *ppos)
{
	unsigned char ch;
	int ret;

	if (count == 0) {
		return count;
	}

	ret = copy_from_user(&ch, buffer, sizeof ch) ? -EFAULT : 0;
	if (ret) {
		return ret;
	}

	ch &= 0x01;
	set_bl(ch);

	return count;
}

static ssize_t bl_dev_read(struct file *filp, char *buffer,
		size_t count, loff_t *ppos)
{
	int ret;
	unsigned char str[] = { '0', '1' };

	if (count == 0) {
		return 0;
	}

	ret = copy_to_user(buffer, str + get_bl(), sizeof(unsigned char)) ? -EFAULT : 0;
	if (ret) {
		return ret;
	}

	return sizeof(unsigned char);
}

static struct file_operations bl_dev_fops = {
	owner:	THIS_MODULE,
	read:	bl_dev_read,	
	write:	bl_dev_write,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &bl_dev_fops,
};

static int __init bl_dev_init(void)
{
	int ret;

	ret = gpio_request(GPIO_BL, "backlight");
	if (ret) {
		printk("%s: request GPIO %d for backlight failed, ret = %d\n",
				DEVICE_NAME, GPIO_BL, ret);
		return ret;
	}

	s3c_gpio_cfgpin(GPIO_BL, S3C_GPIO_OUTPUT);
	set_bl(1);

	gpio_free(GPIO_BL);

	ret = misc_register(&misc);

	printk(DEVICE_NAME"\tinitialized\n");

	return ret;
}


static void __exit bl_dev_exit(void)
{
	misc_deregister(&misc);
}

module_init(bl_dev_init);
module_exit(bl_dev_exit);

MODULE_AUTHOR("FriendlyARM Inc.");
MODULE_LICENSE("GPL");

