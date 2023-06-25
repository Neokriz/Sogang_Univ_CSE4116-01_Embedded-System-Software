/* Driver for FPGA devices
   using in final project.
FILE : sim_driver.c 
AUTH : neo7k@sogang.ac.kr */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/path.h>
#include <linux/namei.h>
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
#include <linux/init.h>
#include <linux/version.h>

#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/workqueue.h>

#include "./sim_driver.h"
#include "./fpga_dot_font.h"

#define SIM_MAJOR 242	// sim device major number
//#define SIM_NAME "sim"	// sim device name

#define DIGIT 4
#define FND_ADDRESS 0x08000004 // fnd pysical address
#define LED_ADDRESS 0x08000016 // led pysical address
#define DOT_ADDRESS 0x08000210 // dot matrix pysical address
#define LCD_ADDRESS 0x08000090 // text lcd pysical address
#define STM_ADDRESS 0x0800000C // step motor pysical address

// user defined structures for timer
static struct device_timer {
	struct timer_list timer;
	struct file* inode;
	int t_sec;
} dev_timer;

//Global variable
static int sim_port_usage = 0;
static unsigned char *fnd_addr;
static unsigned char *led_addr;
static unsigned char *dot_addr;
static unsigned char *lcd_addr;
static unsigned char *stm_addr;

int _car_rpm = 0;
int _car_speed = 0;
char _car_gear;
int _command;
unsigned char _data_init[4];
_Bool USER_WRITE = true;

struct file *_inode;//temporary

// define functions...
int sim_open(struct inode *minode, struct file *mfile);
int sim_release(struct inode *minode, struct file *mfile);
ssize_t sim_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t sim_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
//long sim_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param) {

void fnd_write(char* value);
void led_write(void);
void dot_write(void);
void stm_write(void);

static void kernel_timer_repeat(unsigned long);

// interrupt handlers...
irqreturn_t inter_handler_VP(int irq, void* dev_id);
irqreturn_t inter_handler_VM(int irq, void* dev_id);

irqreturn_t inter_handler_VP(int irq, void* dev_id) {

	return IRQ_HANDLED;
}

irqreturn_t inter_handler_VM(int irq, void* dev_id) {
	
	return IRQ_HANDLED;
}

// when sim device open ,call this function
int sim_open(struct inode *minode, struct file *mfile) {
	int ret;
	int irq;

	if(sim_port_usage != 0) return -EBUSY;

	sim_port_usage = 1;
	
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
	ret=request_irq(irq, inter_handler_VM, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "voldown", 0);

	return 0;
}

// when sim device close, call this function
int sim_release(struct inode *minode, struct file *mfile) {

	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

	sim_port_usage = 0;
	_car_rpm = 0;
	del_timer_sync(&dev_timer.timer);

	return 0;
}

// when write to sim device, call this function
ssize_t sim_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	unsigned char value_fnd[5];
//const char *tmp = gdata;

	memset(value_fnd, 0x00, sizeof(char) * 5);

	//printk("log: sim_write: [0]gdata : [%s]\n", gdata);

	if(USER_WRITE) {
		//printk("log: sim_write user: [1]tmp : [%s]\n", tmp);
		//if (copy_from_user(&value_fnd, tmp, DIGIT))
		//return -EFAULT;
	}
	else 
		USER_WRITE = true;

	// display rpm to FND
	sprintf(value_fnd, "%4d", _car_rpm);
	//printk("log: sim_write: [1]value : [%s]\n", value_fnd);
	fnd_write(value_fnd);
	// light up led according to rpm
	led_write();	
	// display gear on DOT matrix
	dot_write();
	// spin step motor according to rpm
	stm_write();
	return length;
}

void fnd_write(char* value) {
	unsigned short value_short = 0;
	int i;
	if(_car_rpm == 0){
		for(i=0; i<4; ++i)
			value[i] = 0x30;
	}
	for(i=0; i<4; ++i)
		value[i] -= 0x30;
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)fnd_addr);	    
}

void led_write() {
	unsigned short value_short = 0;
	int count = _car_rpm / 740;
	int led[8] = {16, 32, 64, 128, 1, 2, 4, 8};
	int i;

	for(i = 0; i < count; i++){
		value_short += led[i];
	}

	if(count == 8 && dev_timer.t_sec == 0) {
		value_short = 0;

	}

	outw(value_short, (unsigned int)led_addr);
}

void dot_write() {
	char value[10];
	unsigned short value_short = 0;
	int redzone;
	int i, str_size = 10;

	redzone = _car_rpm > 6000 ? 1 : 0;

	if(_car_rpm) {
		switch (_car_gear) {
			case 'P':
				memcpy(value, fpga_gear[0], sizeof(char)*10);
				break;
			case 'R':
				memcpy(value, fpga_gear[1], sizeof(char)*10);
				break;
			case 'N':
				memcpy(value, fpga_gear[2], sizeof(char)*10);
				break;
			case 'D':
				memcpy(value, fpga_gear[3], sizeof(char)*10);
				break;
			default :
				memcpy(value, fpga_number[_car_gear - '0'], sizeof(char)*10);
		}
	}
	else 
		memcpy(value, fpga_set_blank, sizeof(char)*10);

	if(redzone && dev_timer.t_sec == 1) {
		memcpy(value, fpga_set_blank, sizeof(char)*10);
	}

	for(i=0; i<str_size; ++i) {
		//printk("%02x ", value_d[i]);//
		value_short = value[i] & 0x7F;
		outw(value_short, (unsigned int)dot_addr+i*2);
	}
}

void stm_write() {
	char value[3];
	unsigned short value_short = 0;
	int input_range, motor_range;

	input_range = 6500;
	motor_range = 250;

	value[0] = _car_rpm > 0 ? 1 : 0;
	value[1] = 1;
	value[2] = ((_car_rpm) * (-150) / 6300) + 150;

    value_short = value[0] & 0xF;
    outw(value_short,(unsigned int)stm_addr);
    value_short = value[1] & 0xF;
    outw(value_short,(unsigned int)stm_addr + 2);
    value_short = value[2] & 0xFF;
    outw(value_short,(unsigned int)stm_addr + 4);	
}

// when read to sim device, call this function
ssize_t sim_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {

	return length;
}

// timer function that is called every 1/10 second
static void kernel_timer_repeat(unsigned long tdata) {
	struct device_timer *t_ptr = (struct device_timer*)tdata;
	int len = DIGIT;
	char data[4];
	int sec = t_ptr->t_sec;

	t_ptr->t_sec = sec > 1 ? 0 : sec + 1 ;

	sprintf(data, "%4d", _car_rpm);
	sim_write(t_ptr->inode, data, len, NULL);
	t_ptr->timer.expires = get_jiffies_64() + (1 * HZ) / 10;
	t_ptr->timer.data = (unsigned long)t_ptr;
	t_ptr->timer.function = kernel_timer_repeat;

	add_timer(&dev_timer.timer);

	return;
}

long sim_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param) {
	// switch according to the ioctl called.
	switch(ioctl_num) {
		case SET_DATA: {
			char __user *tmp = (char __user *)ioctl_param;
			char msg[11];
			char *in_msg = msg; 
			char *str;
			int len = 11;
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
						if(kstrtoint(str, 10, &_car_rpm) != 0) {
							printk("kstrtoint failed\n");
							return -1;
						}
						break;
					case 2:
						if(kstrtoint(str, 10, &_car_speed) != 0) {
							printk("kstrtoint failed\n");
							return -1;
						}
						break;
					case 3:
						_car_gear = *str;
						break;
					}
				}
			}
			//printk("log: iom_dev_ioctl: [2]\n\t\t_car_rpm:%d,\n\t\t_car_speed:%d,\n\t\t_car_gear:%c\n\n", _car_rpm, _car_speed, _car_gear);

			break;
		}
		case COMMAND: {
			char __user *tmp = (char __user *)ioctl_param;
			char c_arr[2];

			if(copy_from_user(&c_arr, tmp, sizeof(char) * 2)) {
				printk("failed\n");
				return -EFAULT;
			}
			
			if(kstrtoint(c_arr, 10, &_command) != 0) {
				printk("kstrtoint failed\n");
				return -1;
			}

			del_timer_sync(&dev_timer.timer);

			dev_timer.inode = inode;
			dev_timer.t_sec = 0;
			dev_timer.timer.expires = get_jiffies_64() + (0 * HZ);
			dev_timer.timer.data = (unsigned long)&dev_timer;
			dev_timer.timer.function = kernel_timer_repeat;

			add_timer(&dev_timer.timer);

			//printk("log: iom_dev_ioctl: [2]_command: %d\n", _command);
			break;
		}
	}
	return 0;
}

// define file_operations structure 
struct file_operations sim_fops = {
	.owner		=	THIS_MODULE,
	.open		=	sim_open,
	.release	=	sim_release,
	.write		=	sim_write,	
	.read		=	sim_read,	
	.unlocked_ioctl = sim_ioctl,
};

int __init sim_init(void) {
	int result;
    struct path file_path;
    struct inode *inode;
    int perm_ret;

    const char *module_path = "/dev/sim_driver";

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
	

	result = register_chrdev(MAJOR_NUM, DEV_NAME, &sim_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	fnd_addr = ioremap(FND_ADDRESS, 0x4);
	led_addr = ioremap(LED_ADDRESS, 0x1);
	dot_addr = ioremap(DOT_ADDRESS, 0x10);
	lcd_addr = ioremap(LCD_ADDRESS, 0x32);
	stm_addr = ioremap(STM_ADDRESS, 0x4);
	
	init_timer(&dev_timer.timer);

	printk(KERN_ALERT "Init module, %s Major number : %d\n", DEV_NAME, MAJOR_NUM);
	return 0;
}

void __exit sim_exit(void) {
	iounmap(fnd_addr);
	iounmap(led_addr);
	iounmap(dot_addr);
	iounmap(lcd_addr);
	iounmap(stm_addr);

	sim_port_usage = 0;
	del_timer(&dev_timer.timer);
	unregister_chrdev(MAJOR_NUM, DEV_NAME);

	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(sim_init);
module_exit(sim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YooHonghyeon");
