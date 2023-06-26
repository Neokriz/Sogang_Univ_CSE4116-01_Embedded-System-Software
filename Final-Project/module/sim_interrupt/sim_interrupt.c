#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>

#include "./sim_interrupt.h"

//static int result;
static int sim_int_port_usage;

int sim_int_open(struct inode *, struct file *);
int sim_int_release(struct inode *, struct file *);
int sim_int_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
int sim_int_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);

int interruptCount=0;
int interruptNumber=0;

wait_queue_head_t wq_read;
DECLARE_WAIT_QUEUE_HEAD(wq_read);

// interrupt handlers...
irqreturn_t inter_handler_HM(int irq, void* dev_id);
irqreturn_t inter_handler_VP(int irq, void* dev_id);
irqreturn_t inter_handler_VM(int irq, void* dev_id);

irqreturn_t inter_handler_HM(int irq, void* dev_id) {
	//printk(KERN_ALERT "ipnterrupt PW!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	interruptNumber = 3;
	if(interruptCount==1) {
		interruptCount = 0;
		__wake_up(&wq_read, 1, 1, NULL);	
		printk("interrupt handler3\n");
	}
	return IRQ_HANDLED;
}

irqreturn_t inter_handler_VP(int irq, void* dev_id) {
	//printk(KERN_ALERT "interrupt VP!!! = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
	interruptNumber = 1;
	if(interruptCount==1) {
		interruptCount = 0;
		__wake_up(&wq_read, 1, 1, NULL);	
		printk("interrupt handler1\n");
	}
	return IRQ_HANDLED;
}

irqreturn_t inter_handler_VM(int irq, void* dev_id) {
	//printk(KERN_ALERT "interrupt VM... = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
	interruptNumber = 2;
	if(interruptCount==1) {
		interruptCount = 0;
		__wake_up(&wq_read, 1, 1, NULL);	
		printk("interrupt handler2\n");
	}
	return IRQ_HANDLED;
}


// define file_operations structure 
struct file_operations sim_int_fops = {
	.owner		=	THIS_MODULE,
	.release	=	sim_int_release,
	.open		=	sim_int_open,
	.read		=	sim_int_read,	
	.write		=	sim_int_write,	
};

// when sim interrupt device open ,call this function
int sim_int_open(struct inode *minode, struct file *mfile) {
	int ret;
	int irq;

	if(sim_int_port_usage != 0) return -EBUSY;
	sim_int_port_usage = 1;

	// int3
	gpio_request(IMX_GPIO_NR(1,11), "GPIO_home");
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_HM, IRQF_TRIGGER_FALLING, "home", 0);
	
	// int1
	gpio_request(IMX_GPIO_NR(2,15), "GPIO_volup");
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_VP, IRQF_TRIGGER_FALLING, "volup", 0);

	// int2
	gpio_request(IMX_GPIO_NR(5,14), "GPIO_voldown");
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_VM, IRQF_TRIGGER_FALLING, "voldown", 0);

	return 0;
}

// when sim interrupt device close, call this function
int sim_int_release(struct inode *minode, struct file *mfile) {

	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL); // VP
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL); // VM
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL); // HM

	sim_int_port_usage = 0;

	return 0;
}

// when write to sim interrupt device, call this function
int sim_int_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	return length;
}

// when read to sim interrupt device, call this function
int sim_int_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	/*
	unsigned char value;
	unsigned short value_short;
	char *tmp = gdata;
	*/
	interruptNumber = 0;
	if(interruptCount == 0) {
		interruptCount = 1;
		printk("sim_interrupt sleep on\n");
		interruptible_sleep_on(&wq_read);
	}
	//printk("sim_interrupt read\n");

	return interruptNumber;
}

int __init sim_int_init(void) {
	int result;
    struct path file_path;
    struct inode *inode;
    int perm_ret;

    const char *module_path = "/dev/sim_interrupt";
	result = register_chrdev(MAJOR_NUM, DEV_NAME, &sim_int_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

    perm_ret = kern_path(module_path, 0, &file_path);
    if (perm_ret != 0) {
        pr_err("Failed to get path for driver file\n");
        return perm_ret;
    }

    inode = file_path.dentry->d_inode;
    inode->i_mode |= S_IRWXU | S_IRWXG | S_IRWXO;

    perm_ret = inode_permission(inode, MAY_WRITE | MAY_EXEC);
    if (perm_ret != 0) {
        pr_err("Failed to change permissions of driver file\n");
        path_put(&file_path);
        return perm_ret;
    }
	else 
		printk("Permission changed successfully!\n");

    path_put(&file_path);

	printk(KERN_ALERT "Init Interrupt module, %s Major number : %d\n", DEV_NAME, MAJOR_NUM);
	return 0;
}

void __exit sim_int_exit(void) {
	sim_int_port_usage = 0;
	unregister_chrdev(MAJOR_NUM, DEV_NAME);

	printk(KERN_ALERT "Remove Interrupt Module Success \n");
}

module_init(sim_int_init);
module_exit(sim_int_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YooHonghyeon");
