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

// user defined structure for timer
static struct timer_with_counter {
	struct timer_list timer;
	struct file* inode;
	int count;
} dev_timer;

// Global variables
static int fpga_multi_dev_usage = 0;

static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;

int _timer_interval;
int _timer_cnt; 
int _init_pos;
int _init_symbol;
int _command;
unsigned char _timer_init[5]; 

// define functions...
ssize_t iom_dev_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_dev_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_dev_open(struct inode *minode, struct file *mfile);
int iom_dev_release(struct inode *minode, struct file *mfile);

static void kernel_timer_repeat(unsigned long);
static void kernel_timer_blank(unsigned long);

// when devices open, call this function
int iom_dev_open(struct inode *minode, struct file *mfile) {
	if(fpga_multi_dev_usage != 0)
		return -EBUSY;

	fpga_multi_dev_usage = 1;

	return 0;
};

// when devices close, call this function
int iom_dev_release(struct inode *minode, struct file *mfile) {
	fpga_multi_dev_usage = 0;
	
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

	unsigned char value[5];
	unsigned char value_d[10];
	unsigned char value_t[33];

	unsigned char my_num[9], my_name[14];
	unsigned short _s_value;
	unsigned short symbol_num;
	unsigned short i, j;
	const char *tmp = gdata;
	int str_size;
	int order;

	memset(value, 0x00, sizeof(char)*5);
	memset(value_t, 0x00, TEXT_LENGTH);
	symbol_num = 0; order = 0;

	printk("log: iom_dev_write: [1]in write\n");
	//printk("log: iom_dev_write: [2]tmp : %s\n", tmp);

	strcpy(my_num, "20171660");
	strcpy(my_name, "Yoo Honghyeon");

	//printk("log: iom_dev_write: [3]gdata : %s\n", gdata);

	if(copy_from_user(&value, tmp, length))
		return -EFAULT;
	//strncpy(value, tmp, length);


	printk("log: iom_dev_write: [4]value : %s\n", value);

	// convert symbol to integer
	for(i=0; i<4; ++i) {
		if(value[i] != 0x30)
			symbol_num = value[i] - 0x30;
	}
	printk("log: iom_dev_write: [5]symbol_num : %d\n", symbol_num);
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
	if(symbol_num)
		memcpy(value_d, fpga_number[symbol_num], sizeof(char)*str_size);
	else
		memcpy(value_d, fpga_set_blank, sizeof(char)*str_size);
	// display code on DOT 
	for(i=0; i<str_size; ++i) {
		//printk("%02x ", value_d[i]);//
		_s_value = value_d[i] & 0x7F;
		outw(_s_value, (unsigned int)iom_fpga_dot_addr+i*2);
	}
	

	//printk("log: iom_dev_write: [6]strlen(my_num) : %d\n", strlen(my_num));
	if(symbol_num) {
		order = _timer_cnt - dev_timer.count;
		for(i=0; i<blank_upper[order % 16]; ++i)
			value_t[i] = ' ';
		for(j=0; j<strlen(my_num); ++i, ++j) 
			value_t[i] = my_num[j]; 
		for(j=0; j<(8 - blank_upper[order % 16]); ++i, ++j)
			value_t[i] = ' ';
		for(j=0; j<blank_lower[order % 6]; ++i, ++j)
			value_t[i] = ' ';
		for(j=0; j<strlen(my_name); ++i, ++j)
			value_t[i] = my_name[j]; 
		for(j=0; j<(3 - blank_lower[order % 6]); ++i, ++j)
			value_t[i] = ' ';
		value_t[TEXT_LENGTH] = 0;
	}
	else {
		memset(value_t, ' ', TEXT_LENGTH+1);
	}

	// display on text_lcd
	for(i=0; i<TEXT_LENGTH; ++i) {
		_s_value = ((value_t[i] & 0xFF) << 8) | (value_t[i + 1] & 0xFF);
		outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr+i);
		i++;
	}


	//printk("log: iom_dev_write: [7]value_t : [%s]\n", value_t);
	printk("log: iom_dev_write: [7]value_t : [");
	for(i=0; i<16; ++i)
		printk("%c", value_t[i]);
	printk("|");
	for(i=16; i<32; ++i)
		printk("%c", value_t[i]);
	printk("]\n");


	return length;
};

ssize_t iom_dev_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {

	return length;
};

// timer function that is called INIT_CNT times
static void kernel_timer_repeat(unsigned long tdata) {
	struct timer_with_counter *t_ptr = (struct timer_with_counter*)tdata; 
	int len = strlen(_timer_init);
	int order, pos, symbol_num;
	
	order = _timer_cnt - t_ptr->count;
	symbol_num = 0;

	printk("log: kernel_timer_repeat %d\n", t_ptr->count);

	pos = (_init_pos + order / 8) % 4;
	symbol_num = (_init_symbol - 1 + order) % 8;
	iom_dev_write(dev_timer.inode, fnd_code[pos][symbol_num], len, NULL);

	t_ptr->count--;
	// turn off(reset) devices [_command] seconds after task finish. 
	if(t_ptr->count < 1) {
		printk("\nCounter expired. Shutting down in %d seconds...\n", _command);
		dev_timer.timer.expires = get_jiffies_64() + (_command * HZ);
		dev_timer.timer.data = (unsigned long)&dev_timer;
		dev_timer.timer.function = kernel_timer_blank;

		add_timer(&dev_timer.timer);
		return;
	}

	dev_timer.timer.expires = get_jiffies_64() + (_timer_interval * HZ) / 10;
	dev_timer.timer.data = (unsigned long)&dev_timer;
	dev_timer.timer.function = kernel_timer_repeat;

	add_timer(&dev_timer.timer);
	printk("______________________________________________________________________\n");
	return;
}

// reset(turn off) devices
static void kernel_timer_blank(unsigned long tdata) {
	char blank[5] = {0x30, 0x30, 0x30, 0x30, '\0'};
	printk("______________________________________________________________________\n");
	iom_dev_write(dev_timer.inode, blank, 4, NULL);
	return;
}

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

			//printk("log: iom_dev_ioctl: [1]msg : [%s]\n", msg);

			str = in_msg;
			while(str != NULL) {
				str = strsep(&in_msg, " ");
				if((str != NULL) && strcmp("", str)) {
					//printk("log: iom_dev_ioctl: [n]str: [%s]\n", str);
					argument_num++;
					switch(argument_num) {
					case 1:
						if(kstrtoint(str, 10, &_timer_interval) != 0) {
							//printk("kstrtoint failed\n");
							return -1;
						}
						break;
					case 2:
						if(kstrtoint(str, 10, &_timer_cnt) != 0) {
							//printk("kstrtoint failed\n");
							return -1;
						}
						break;
					case 3:
						strcpy(_timer_init, str);	
						break;
					}
				}
			}
			printk("log: iom_dev_ioctl: [2]\n\t\t_timer_interval:%d,\n\t\t_timer_cnt:%d,\n\t\t_timer_init:%s\n", _timer_interval, _timer_cnt, _timer_init);

			break;
		}
		case COMMAND: {
			char __user *tmp = (char __user *)ioctl_param;
			char c_arr[3];
			int i;

			if(copy_from_user(&c_arr, tmp, sizeof(char) * 3))
				return -EFAULT;
			if(kstrtoint(c_arr, 10, &_command) != 0) {
				printk("kstrtoint failed\n");
				return -1;
			}
			printk("log: iom_dev_ioctl: [3]_command: %d\n", _command);

			for(i=0; i<4; ++i) {
				if(_timer_init[i] != 0x30) {
					_init_pos = i;
					_init_symbol = _timer_init[i] - 0x30;
					break;
				}
			}
			printk("log: iom_dev_ioctl: [4]_init_pos: %d, _init_symbol: %d\n", _init_pos, _init_symbol);

			dev_timer.inode = inode;
			dev_timer.count = _timer_cnt;
			printk("log: iom_dev_ioctl: [5]dev_timer.count: %d\n\n", dev_timer.count);

			del_timer_sync(&dev_timer.timer);
			
			//dev_timer.timer.expires = get_jiffies_64() + (1 * HZ);
			printk("\nCounter set. Starting after %d seconds...\n", _command);
			dev_timer.timer.expires = get_jiffies_64() + (_command * HZ);
			dev_timer.timer.data = (unsigned long)&dev_timer;
			dev_timer.timer.function = kernel_timer_repeat;

			add_timer(&dev_timer.timer);

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

	init_timer(&(dev_timer.timer));

	printk("init module, %s major number : %d\n", IOM_DEV_NAME, IOM_DEV_MAJOR);

	return 0;
}

void __exit iom_dev_exit(void) {
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_text_lcd_addr);
	
	printk("module exit\n\n");
	fpga_multi_dev_usage = 0;
	del_timer(&dev_timer.timer);

	unregister_chrdev(IOM_DEV_MAJOR, IOM_DEV_NAME);
}

module_init(iom_dev_init);
module_exit(iom_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("YooHonghyeon");
