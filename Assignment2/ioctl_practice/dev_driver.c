/* FPGA Devices Ioremap Control
 * Devices : FND, LED, DOT, TEXT_LCD
 * FILE : fpga_dev_driver.c
 * AUTH : neo7k@sogang.ac.kr */

/*
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
*/

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

ssize_t iom_dev_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	/* device별 value값 :
	 * 	led - char
	 * 	fnd - char[4]
	unsigned char value;
	unsigned short _s_value;
	char *tmp = gdata;

	_s_value = inw((unsigned int)iom_fpga_led_addr);

	value = _s_value & 0xF;

	if(copy_to_user(tmp, &value, 1))
		return -EFAULT;

	return length;
};

int iom_dev_open(struct inode *minode, struct file *mfile) {

};

