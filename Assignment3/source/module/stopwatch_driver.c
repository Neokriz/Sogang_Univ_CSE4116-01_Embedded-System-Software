/* Stopwatch using FPGA FND 
FILE : stopwatch.c 
AUTH : neo7k@sogang.ac.kr */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/param.h>
#include <mach/gpio.h>

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/string.h>

#include "./stopwatch.h"

//#define STOPWATCH_MAJOR 242		// stopwatch device major number
//#define STOPWATCH_NAME "stopwatch"	// stopwatch device name

#define DIGIT 4
#define STOPWATCH_ADDRESS 0x08000004 // pysical address

// user defined structure for timer
static struct stopwatch_timer {
	struct timer_list timer;
	struct file* inode;
	int min;
	int sec;
	int t_sec;
} stw_timer;

//Global variable
static int stopwatch_port_usage = 0;
static unsigned char *stopwatch_addr;
int _interrupt_count = 0;
int _command;
unsigned char _data_init[4];
_Bool USER_WRITE = true;


struct file *_inode;//temporary


wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

// define functions...
int stopwatch_open(struct inode *minode, struct file *mfile);
int stopwatch_release(struct inode *minode, struct file *mfile);
ssize_t stopwatch_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t stopwatch_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
//long stopwatch_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param) {

static void kernel_timer_repeat(unsigned long);
static void kernel_timer_blank(unsigned long);

// interrupt handlers...
irqreturn_t inter_handler_HM(int irq, void* dev_id);
irqreturn_t inter_handler_BK(int irq, void* dev_id);
irqreturn_t inter_handler_VP(int irq, void* dev_id);
irqreturn_t inter_handler_VM(int irq, void* dev_id);

irqreturn_t inter_handler_HM(int irq, void* dev_id) {
	printk(KERN_ALERT "interrupt HM!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	
	if(_interrupt_count == 0) {	
		del_timer_sync(&stw_timer.timer);

		stw_timer.timer.expires = get_jiffies_64() + (0 * HZ);
		stw_timer.timer.data = (unsigned long)&stw_timer;
		stw_timer.timer.function = kernel_timer_repeat;

		add_timer(&stw_timer.timer);
	}
			
	return IRQ_HANDLED;
}

irqreturn_t inter_handler_BK(int irq, void* dev_id) {
	printk(KERN_ALERT "interrupt BK!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));

	if(++_interrupt_count>=3) {
		_interrupt_count=0;
                __wake_up(&wq_write, 1, 1, NULL);
		//wake_up_interruptible(&wq_write);
		printk("wake up\n");
        }

	return IRQ_HANDLED;
}

irqreturn_t inter_handler_VP(int irq, void* dev_id) {
	printk(KERN_ALERT "interrupt VP!!! = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
	return IRQ_HANDLED;
}

irqreturn_t inter_handler_VM(int irq, void* dev_id) {
	printk(KERN_ALERT "interrupt VM!!! = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
	return IRQ_HANDLED;
}

// when stopwatch device open ,call this function
int stopwatch_open(struct inode *minode, struct file *mfile) {
	int ret;
	int irq;

	if(stopwatch_port_usage != 0) return -EBUSY;

	stopwatch_port_usage = 1;

	// int1
	gpio_request(IMX_GPIO_NR(1,11), "GPIO_home");
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_HM, IRQF_TRIGGER_FALLING, "home", 0);

	// int2
	gpio_request(IMX_GPIO_NR(1,12), "GPIO_back");
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_BK, IRQF_TRIGGER_FALLING, "back", 0);

	// int3
	gpio_request(IMX_GPIO_NR(2,15), "GPIO_volup");
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_VP, IRQF_TRIGGER_FALLING, "volup", 0);

	// int4
	gpio_request(IMX_GPIO_NR(5,14), "GPIO_voldown");
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_VM, IRQF_TRIGGER_FALLING, "voldown", 0);

	return 0;
}

// when stopwatch device close, call this function
int stopwatch_release(struct inode *minode, struct file *mfile) {
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

	stopwatch_port_usage = 0;

	return 0;
}

// when write to stopwatch(fnd) device, call this function
ssize_t stopwatch_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	int i;
	unsigned char value[5];
	unsigned short int value_short = 0;
	const char *tmp = gdata;

	memset(value, 0x00, sizeof(char) * 5);

	//printk("log: stopwatch_write: [0]gdata : [%s]\n", gdata);

	if(USER_WRITE) {
		if (copy_from_user(&value, tmp, DIGIT))
			return -EFAULT;
	}
	else 
		USER_WRITE = true;

	strncpy(value, tmp, DIGIT);
	printk("log: stopwatch_write: [1]value : [%s]\n", value);

	for(i=0; i<4; ++i)
		value[i] -= 0x30;

    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)stopwatch_addr);	    

	return length;
}

// when read to stopwatch device, call this function
ssize_t stopwatch_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	//int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	char *tmp = gdata;

    value_short = inw((unsigned int)stopwatch_addr);	    
    value[0] =(value_short >> 12) & 0xF;
    value[1] =(value_short >> 8) & 0xF;
    value[2] =(value_short >> 4) & 0xF;
    value[3] = value_short & 0xF;

    if (copy_to_user(tmp, value, 4))
        return -EFAULT;

	return length;
}

// timer function that is called INIT_CNT times
static void kernel_timer_repeat(unsigned long tdata) {
	struct stopwatch_timer *t_ptr = (struct stopwatch_timer*)tdata;
	char data[4];
	int len = DIGIT;

	stw_timer.t_sec++;
	//printk("log: kernel_timer_repeat:: t_sec: %d\n", stw_timer.t_sec);
	if(stw_timer.t_sec == 10) {
		stw_timer.t_sec = 0;

		stw_timer.sec++;
		if(stw_timer.sec == 60) {
			stw_timer.min++; 
			stw_timer.sec = 0;
		}
		if(stw_timer.min == 60) 
			stw_timer.min = 0;

		//printk("log: kernel_timer_repeat %d\n", stw_timer.sec);

		sprintf(data, "%02d%02d", stw_timer.min, stw_timer.sec);
		
		printk("log: kernel_timer_repeat:: m&s: %2d:%2d\n", stw_timer.min, stw_timer.sec);
		//printk("log: kernel_timer_repeat:: data: %c%c%c%c\n", data[0], data[1], data[2], data[3]);
		stopwatch_write(stw_timer.inode, data, len, NULL);
		printk("______________________________________________________________________\n");
	}
	// turn off(reset) devices [_command] seconds after task finish.
	if(_interrupt_count) {
		printk("\nCounter expired. Shutting down in %d seconds...\n", _command);
		stw_timer.timer.expires = get_jiffies_64() + (0 * HZ);
		stw_timer.timer.data = (unsigned long)&stw_timer;
		stw_timer.timer.function = kernel_timer_blank;

		add_timer(&stw_timer.timer);
		return;
	}

	stw_timer.timer.expires = get_jiffies_64() + (1 * HZ) / 10;
	stw_timer.timer.data = (unsigned long)&stw_timer;
	stw_timer.timer.function = kernel_timer_repeat;

	add_timer(&stw_timer.timer);

	return;
}

// reset(turn off) devices
static void kernel_timer_blank(unsigned long tdata) {
	char blank[5] = {0x30, 0x30, 0x30, 0x30, '\0'};
	printk("______________________________________________________________________\n");
	stopwatch_write(stw_timer.inode, blank, 4, NULL);
	return;
}

long stopwatch_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param) {
	// switch according to the ioctl called.
	switch(ioctl_num) {
		case SET_OPTION: {
			char __user *tmp = (char __user *)ioctl_param;
			char msg[4];
			
			if(copy_from_user(&msg, tmp, DIGIT))
				return -EFAULT;
	
			_inode = inode;
			//printk("log: stopwatch_ioctl: [1]msg : [%s]\n", msg);
			strncpy(_data_init, msg, 4);

			stw_timer.min = 0;
			stw_timer.sec = 0;
			stw_timer.t_sec = 0;

			USER_WRITE = false;
			stopwatch_write(inode, _data_init, DIGIT, NULL);

			break;
		}
		case COMMAND: {
			char __user *tmp = (char __user *)ioctl_param;
			char c_arr[2];

			stw_timer.inode = inode;

			if(copy_from_user(&c_arr, tmp, sizeof(char) * 2))
				return -EFAULT;
			
			if(kstrtoint(c_arr, 10, &_command) != 0) {
				printk("kstrtoint failed\n");
				return -1;
			}
			printk("log: iom_dev_ioctl: [2]_command: %d\n", _command);

			if(_interrupt_count == 0) {
				printk("sleep on\n");
				interruptible_sleep_on(&wq_write);
			}
			break;
		}
	}
	return 0;
}

// define file_operations structure 
struct file_operations stopwatch_fops = {
	.owner		=	THIS_MODULE,
	.open		=	stopwatch_open,
	.release	=	stopwatch_release,
	.write		=	stopwatch_write,	
	.read		=	stopwatch_read,	
	.unlocked_ioctl = stopwatch_ioctl,
};

int __init stopwatch_init(void) {
	int result;

	result = register_chrdev(MAJOR_NUM, DEV_NAME, &stopwatch_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	stopwatch_addr = ioremap(STOPWATCH_ADDRESS, 0x4);

	init_timer(&stw_timer.timer);
	printk(KERN_ALERT "Init module, %s Major number : %d\n", DEV_NAME, MAJOR_NUM);
	return 0;
}

void __exit stopwatch_exit(void) {
	iounmap(stopwatch_addr);

	del_timer(&stw_timer.timer);
	unregister_chrdev(MAJOR_NUM, DEV_NAME);

	stopwatch_port_usage = 0;
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(stopwatch_init);
module_exit(stopwatch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YooHonghyeon");
