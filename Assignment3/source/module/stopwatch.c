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


#define STOPWATCH_MAJOR 242		// stopwatch device major number
#define STOPWATCH_NAME "stopwatch"	// stopwatch device name

#define STOPWATCH_ADDRESS 0x08000004 // pysical address

//Global variable
static int stopwatch_port_usage = 0;
static unsigned char *stopwatch_addr;

// define functions...
ssize_t stopwatch_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t stopwatch_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int stopwatch_open(struct inode *minode, struct file *mfile);
int stopwatch_release(struct inode *minode, struct file *mfile);

// define file_operations structure 
struct file_operations stopwatch_fops =
{
	.owner		=	THIS_MODULE,
	.open		=	stopwatch_open,
	.write		=	stopwatch_write,	
	.read		=	stopwatch_read,	
	.release	=	stopwatch_release,
};

// when stopwatch device open ,call this function
int stopwatch_open(struct inode *minode, struct file *mfile) 
{	
	if(stopwatch_port_usage != 0) return -EBUSY;

	stopwatch_port_usage = 1;

	return 0;
}

// when stopwatch device close, call this function
int stopwatch_release(struct inode *minode, struct file *mfile) 
{
	stopwatch_port_usage = 0;

	return 0;
}

// when write to stopwatch(fnd) device, call this function
ssize_t stopwatch_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, 4))
		return -EFAULT;

    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)stopwatch_addr);	    

	return length;
}

// when read to stopwatch device, call this function
ssize_t stopwatch_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
{
	int i;
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

int __init stopwatch_init(void) {
	int result;

	result = register_chrdev(STOPWATCH_MAJOR, STOPWATCH_NAME, &stopwatch_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	stopwatch_addr = ioremap(STOPWATCH_ADDRESS, 0x4);
	printk(KERN_ALERT "Init module, %s Major number : %d\n", STOPWATCH_NAME, STOPWATCH_MAJOR);
	return 0;
}

void __exit stopwatch_exit(void) 
{
	iounmap(stopwatch_addr);
	unregister_chrdev(STOPWATCH_MAJOR, STOPWATCH_NAME);
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(stopwatch_init);
module_exit(stopwatch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
