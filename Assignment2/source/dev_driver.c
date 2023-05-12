/* FPGA Devices Ioremap Control
 * Devices : FND, LED, DOT, TEXT_LCD
 * FILE : fpga_dev_driver.c
 * AUTH : neo7k@sogang.ac.kr */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_devices.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

// custom header for dot matrix
//#include "./fpga_dot_font.h"

// ioboard fpga device major numbers
#define IOM_LED_MAJOR 260
#define IOM_FND_MAJOR 261	
#define IOM_DOT_MAJOR 262
#define IOM_TEXT_LCD_MAJOR 263

// ioboard fpga device names
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

// Global variables
static int fpga_led_port_usage = 0;
static int fpga_fnd_port_usage = 0;
static int fpga_dot_port_usage = 0;
static int fpga_text_lcd_port_usage = 0;

static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;

// define functions...
ssize_t iom_dev_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_dev_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_dev_open(struct inode *minode, struct file *mfile);
int iom_dev_release(struct inode *minode, struct file *mfile);

// define file_operations structure
struct file_operations iom_dev_fops = {
	.owner		=	THIS_MODULE,
	.open		=	iom_dev_open,
	.write		=	iom_dev_write,
	.read		=	iom_dev_read,
	.release	=	iom_led_release,
};

// when devices open, call this function
int iom_dev_open(struct inode *minode, struct file *mfile) {
	if(fpga_led_port_usage != 0 || \
	   fpga_fnd_port_usage != 0 || \
	   fpga_dot_port_usage != 0 || \
	   fpga_text_lcd_port_usage != 0) return -EBUSY;

	fpga_led_port_usage = 1;
	fpga_fnd_port_usage = 1;
	fpga_dot_port_usage = 1;
	fpga_text_lcd_port_usage = 1;

	return 0;
};

// when devices close, call this function
int iom_dev_release(struct inode *minode, struct file *mfile) {
	fpga_led_port_usage = 0;
	fpga_fnd_port_usage = 0;
	fpga_dot_port_usage = 0;
	fpga_text_lcd_port_usage = 0;

	return 0;
};

ssize_t iom_dev_write(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	/* value of each original driver :
	 * 	led - char
	 * 	fnd - char[4]
	 * 	dot - char[10]
	 * 	text_lcd - char[33]
	 */

	unsigned char value[4];
	unsigned char value_d[10];
	unsigned char value_t[33];
	unsigned short _s_value;
	unsigned short symbol_num;
	unsigned short i;
	char *tmp;
	char *my_num, *my_name;
	int str_size;

	*tmp = gdata;
	strcpy(my_num, "20171660");
	strcpy(my_name, "Yoo Honghyeon");

	if(copy_from_user(&value, tmp, 4))
		return -EFAULT;


	// convert symbol to integer
	for(i=0; i<4; ++i) {
		if(value[i] != 0x30)
			symbol_num = value[i] - 0x30;
	}
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

	_s_value = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
	// display code on FND
	outw(_s_value, (unsigned int)iom_fpga_fnd_addr);

	
	str_size = sizeof(fpga_number[symbol_num]);	
	memcpy(value_d, value, 10);
	// display code on DOT 
	for(i=0; i<str_size; ++i) {
		_s_value = value[i] & 0x7F;
		outw(_s_value, (unsigned int)iom_fpga_dot_addr+i*2);
	}

	value_t[TEXT_LENGTH] = 0;
	for(i=0; i<TEXT_LENGTH; ++i) {
		_s_value = (value[i] & 0xFF) << 8 | value_t[i + 1] & 0xFF;
		outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr+i);
	}


	return length;
};

int iom_dev_read(struct inode *inode, char *gdata, size_t length, loff_t *off_what) {

};

