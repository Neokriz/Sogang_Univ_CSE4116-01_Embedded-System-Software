/* FPGA Devices Ioremap Control
 * Devices : FND, LED, DOT, TEXT_LCD
 * FILE : fpga_dev_driver.c
 * AUTH : neo7k@sogang.ac.kr */

#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/param.h>

// custom header for dot matrix
#include "./fpga_dot_font.h"

#include "./dev.h"

// ioboard fpga device major numbers
#define IOM_DEV_MAJOR 242

// ioboard fpga device names
#define IOM_DEV_NAME "fpga_dev"
#define IOM_LED_NAME "fpga_led"
#define IOM_FND_NAME "fpga_fnd"
#define IOM_DOT_NAME "fpga_dot"
#define IOM_TEXT_LCD_NAME "fpga_text_lcd"

// physical addresses - 32 Byte (16 * 2)
#define IOM_LED_ADDRESS 0x08000016
#define IOM_FND_ADDRESS 0x08000004
#define IOM_DOT_ADDRESS 0x08000210
#define IOM_TEXT_LCD_ADDRESS 0x08000090

#define TEXT_LENGTH 32
#define LINE_LENGTH 16 

// Global variables
/*static int fpga_led_port_usage = 0;
static int fpga_fnd_port_usage = 0;
static int fpga_dot_port_usage = 0;
static int fpga_text_lcd_port_usage = 0; */
static int fpga_multi_dev_usage = 0;

static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;

int _timer_interval;
int _timer_cnt; 
unsigned char _timer_init[5]; 

// define functions...
ssize_t iom_dev_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_dev_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_dev_open(struct inode *minode, struct file *mfile);
int iom_dev_release(struct inode *minode, struct file *mfile);

// when devices open, call this function
int iom_dev_open(struct inode *minode, struct file *mfile) {
	/*
	if(fpga_led_port_usage != 0 || \
	   fpga_fnd_port_usage != 0 || \
	   fpga_dot_port_usage != 0 || \
	   fpga_text_lcd_port_usage != 0) return -EBUSY;

	fpga_led_port_usage = 1;
	fpga_fnd_port_usage = 1;
	fpga_dot_port_usage = 1;
	fpga_text_lcd_port_usage = 1;
	*/
	if(fpga_multi_dev_usage != 0)
		return -EBUSY;

	fpga_multi_dev_usage = 1;

	return 0;
};

// when devices close, call this function
int iom_dev_release(struct inode *minode, struct file *mfile) {
	/*
	fpga_led_port_usage = 0;
	fpga_fnd_port_usage = 0;
	fpga_dot_port_usage = 0;
	fpga_text_lcd_port_usage = 0;
	*/
	fpga_multi_dev_usage = 1;

	return 0;
};

ssize_t iom_dev_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	/* value of each original driver :
	 * 	led - char
	 * 	fnd - char[4]
	 * 	dot - char[10]
	 * 	text_lcd - char[33]
	 *
	 * 	in this module, gdata came in as char[4]
	 */

	unsigned char value[4];
	unsigned char value_d[10];
	unsigned char value_t[33];

	unsigned char my_num[9], my_name[14];
	unsigned short _s_value;
	unsigned short symbol_num;
	unsigned short i;
	const char *tmp = gdata;
	int str_size;

	//value_t[0] = '\0';
	memset(value_t, 0x00, TEXT_LENGTH);
	symbol_num = 0;

	printk("log: iom_dev_write: [1]in write\n");
	printk("log: iom_dev_write: [2]tmp : %s\n", tmp);

	strcpy(my_num, "20171660");
	strcpy(my_name, "Yoo Honghyeon");

	printk("log: iom_dev_write: [3]gdata : %s\n", gdata);

	/*
	if(copy_from_user(&msg, tmp, length))
		return -EFAULT;
	*/
	strncpy(value, tmp, length);

	// convert symbol to integer
	for(i=0; i<4; ++i) {
		if(value[i] != 0x30)
			symbol_num = value[i] - 0x30;
	}
	printk("log: iom_dev_write: [4]symbol_num : %d\n", symbol_num);
	/* number - led value mapping
	 *   1		128  [1000 0000]
	 *   2       64  [0100 0000]
	 *   3       32  [0010 0000]
	 *   4       16  [0001 0000]
	 *   5        8  [0000 1000]
	 *   6        4  [0000 0100]
	 *   7        2  [0000 0010]
	 *   8        1  [0000 0001]
	 */
	
	_s_value = 256 >> symbol_num;
	// light up led
	outw(_s_value, (unsigned int)iom_fpga_led_addr);

	
	for(i=0; i<4; ++i)
		value[i] -= 0x30;

	_s_value = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
	// display code on FND
	outw(_s_value, (unsigned int)iom_fpga_fnd_addr);

	
	str_size = sizeof(fpga_number[symbol_num]);	// 10
	memcpy(value_d, fpga_number[symbol_num], sizeof(char)*10);
	// display code on DOT 
	for(i=0; i<str_size; ++i) {
		//printk("%02x ", value_d[i]);//
		_s_value = value_d[i] & 0x7F;
		outw(_s_value, (unsigned int)iom_fpga_dot_addr+i*2);
	}

	
	printk("log: iom_dev_write: [5]strlen(my_num) : %d\n", strlen(my_num));
	strcat(value_t, my_num);
	memset(value_t+strlen(my_num), ' ', LINE_LENGTH-strlen(my_num));
	strcat(value_t, my_name);
	memset(value_t+LINE_LENGTH+strlen(my_name), ' ', LINE_LENGTH-strlen(my_name));
	value_t[TEXT_LENGTH] = 0;
	printk("log: iom_dev_write: [6]value_t : [%s]\n", value_t);
	
	for(i=0; i<TEXT_LENGTH; ++i) {
		_s_value = ((value_t[i] & 0xFF) << 8) | (value_t[i + 1] & 0xFF);
		outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr+i);
		i++;
	}

	return length;
};

ssize_t iom_dev_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {

	return length;
};

long iom_dev_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param) {
	// switch according to the ioctl called.
	switch(ioctl_num) {
		case SET_OPTION: {
			char __user *tmp = (char __user *)ioctl_param;
			char msg[13];
			char *in_msg = msg; 
			char *str;
			int len = 13;
			int argument_num = 0;
			
			if(copy_from_user(&msg, tmp, sizeof(char)*len))
				return -EFAULT;

			printk("log: iom_dev_ioctl: [1]msg : [%s]\n", msg);

			str = in_msg;
			while(str != NULL) {
				str = strsep(&in_msg, " ");
				if((str != NULL) && strcmp("", str)) {
					printk("log: iom_dev_ioctl: [n]str: [%s]\n", str);
					argument_num++;
					switch(argument_num) {
					case 1:
						//_timer_interval = atoi(str);
						if(kstrtoint(str, 10, &_timer_interval) != 0) {
							printk("kstrtoint failed\n");
							return -1;
						}
						break;
					case 2:
						//_timer_cnt = atoi(str);
						if(kstrtoint(str, 10, &_timer_cnt) != 0) {
							printk("kstrtoint failed\n");
							return -1;
						}
						break;
					case 3:
						strcpy(_timer_init, str);	
						break;
					}
				}
			}
			printk("log: iom_dev_ioctl: [2]_timer_interval:%d,_timer_cnt:%d,_timer_init:%s\n", _timer_interval, _timer_cnt, _timer_init);

			//strncpy(value, str, 4);

			len = strlen(_timer_init);
			iom_dev_write(inode, _timer_init, len, NULL);
			break;
		}
		case COMMAND: {

			break;
		}
	}
	return 0;
}

// define file_operations structure
struct file_operations iom_dev_fops = {
	.owner		=	THIS_MODULE,
	.open		=	iom_dev_open,
	.release	=	iom_dev_release, 
	.unlocked_ioctl = iom_dev_ioctl,
	.read		=	iom_dev_read,
	.write		=	iom_dev_write,
};

int __init iom_dev_init(void) {
	int result;

	result = register_chrdev(IOM_DEV_MAJOR, IOM_DEV_NAME, &iom_dev_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_dot_addr = ioremap(IOM_DOT_ADDRESS, 0x10);
	iom_fpga_text_lcd_addr = ioremap(IOM_TEXT_LCD_ADDRESS, 0x32);

	printk("init module, %s major number : %d\n", IOM_DEV_NAME, IOM_DEV_MAJOR);

	return 0;
}

void __exit iom_dev_exit(void) {
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_text_lcd_addr);

	unregister_chrdev(IOM_DEV_MAJOR, IOM_DEV_NAME);
}

module_init(iom_dev_init);
module_exit(iom_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("YooHonghyeon");
