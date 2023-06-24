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
static struct stopwatch_timer {
	struct timer_list timer;
	struct file* inode;
	int min;
	int sec;
	int t_sec;
} stw_timer;

static struct exit_timer {
	struct timer_list timer;
	int count;
} ex_timer; 

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
int _run_check = 0;
int _pause_check = 0;
int _command;
unsigned char _data_init[4];
_Bool USER_WRITE = true;

struct file *_inode;//temporary

// initialize wait queue
/*
wait_queue_head_t app_waitqueue;
init_waitqueue_head(&app_waitqueue);
*/
DECLARE_WAIT_QUEUE_HEAD(app_waitqueue); // do both job(line 65, line 66)

// work queues
static struct workqueue_struct *stw_wq;
static struct workqueue_struct *stw_wq2;

// define functions...
int sim_open(struct inode *minode, struct file *mfile);
int sim_release(struct inode *minode, struct file *mfile);
ssize_t sim_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t sim_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);

static void wq_func1(struct work_struct *work);
static void wq_func2(struct work_struct *work);
static void wq_func1(struct work_struct *work);
static void kernel_timer_repeat(unsigned long);
static void kernel_timer_count(unsigned long);
static void sim_blank(struct stopwatch_timer *tdata);
//long sim_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param) {

// interrupt handlers...
irqreturn_t inter_handler_VP(int irq, void* dev_id);
irqreturn_t inter_handler_VM(int irq, void* dev_id);
/*
irqreturn_t inter_handler_HM(int irq, void* dev_id) {
	static struct work_struct task;
	int ret;

	//printk(KERN_ALERT "interrupt HM!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	
	if(_run_check == 0) {	
		_run_check = 1;

		INIT_WORK(&task, wq_func1);

		ret = queue_work(stw_wq, &task);
	}
			
	return IRQ_HANDLED;
}

irqreturn_t inter_handler_BK(int irq, void* dev_id) {
	int ret, i;
	//printk(KERN_ALERT "interrupt BK!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));

	flush_workqueue(stw_wq);
	ret = del_timer(&stw_timer.timer);

	_run_check = 0;
	_pause_check = 1;

	if(ret) {
		printk(KERN_ALERT "\n");
		for(i=0; i<stw_timer.t_sec; ++i) 
			printk(KERN_CONT "  ");
	}

	return IRQ_HANDLED;
}
*/
irqreturn_t inter_handler_VP(int irq, void* dev_id) {
	flush_workqueue(stw_wq);
	_run_check = 0;
	sim_blank(&stw_timer);
	stw_timer.min = stw_timer.sec = stw_timer.t_sec = 0;

	return IRQ_HANDLED;
}

irqreturn_t inter_handler_VM(int irq, void* dev_id) {
	static struct work_struct task;
	int val, ret;
	
	//printk(KERN_ALERT "interrupt VM!!! = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
	
	val = gpio_get_value(IMX_GPIO_NR(5,14));
	
	if(!val) {
		INIT_WORK(&task, wq_func2);
		ret = queue_work(stw_wq2, &task);
	}
	else {
		del_timer(&ex_timer.timer);
		flush_workqueue(stw_wq2);
		ex_timer.count = 0;
	}

	return IRQ_HANDLED;
}

// when sim device open ,call this function
int sim_open(struct inode *minode, struct file *mfile) {
	int ret;
	int irq;

	if(sim_port_usage != 0) return -EBUSY;

	sim_port_usage = 1;
	
	// create workqueue
	stw_wq = create_workqueue("stw_workqueue");
	stw_wq2= create_workqueue("stw_workqueue2");

/*  gpio_request(IMX_GPIO_NR(1,11), "GPIO_home");
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_HM, IRQF_TRIGGER_FALLING, "home", 0);

	gpio_request(IMX_GPIO_NR(1,12), "GPIO_back");
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler_BK, IRQF_TRIGGER_FALLING, "back", 0);
*/
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
	ret=request_irq(irq, inter_handler_VM, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "voldown", 0);

	return 0;
}

// when sim device close, call this function
int sim_release(struct inode *minode, struct file *mfile) {
	flush_workqueue(stw_wq);
	destroy_workqueue(stw_wq);
	flush_workqueue(stw_wq2);
	destroy_workqueue(stw_wq2);

//free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
//free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

	sim_port_usage = 0;

	return 0;
}

// when write to sim device, call this function
ssize_t sim_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	int i;
	unsigned char value_fnd[5];
	unsigned char value_dot[10];

	unsigned short value_short = 0;
	const char *tmp = gdata;
	int str_size;

	memset(value_fnd, 0x00, sizeof(char) * 5);

	//printk("log: sim_write: [0]gdata : [%s]\n", gdata);

	if(USER_WRITE) {
		//printk("log: sim_write user: [1]tmp : [%s]\n", tmp);
		if (copy_from_user(&value_fnd, tmp, DIGIT))
			return -EFAULT;
	}
	else 
		USER_WRITE = true;

	// display rpm to FND
	sprintf(value_fnd, "%4d", _car_rpm);
	//printk("log: sim_write: [1]value : [%s]\n", value);
	for(i=0; i<4; ++i)
		value_fnd[i] -= 0x30;
    value_short = value_fnd[0] << 12 | value_fnd[1] << 8 |value_fnd[2] << 4 |value_fnd[3];
    outw(value_short,(unsigned int)fnd_addr);	    
	/////////////////////
	
	// display gear on DOT matrix
	str_size = 10;
	if(_car_rpm) {
		switch (_car_gear) {
			case 'P':
				memcpy(value_dot, fpga_gear[0], sizeof(char)*10);
				break;
			case 'R':
				memcpy(value_dot, fpga_gear[1], sizeof(char)*10);
				break;
			case 'N':
				memcpy(value_dot, fpga_gear[2], sizeof(char)*10);
				break;
			case 'D':
				memcpy(value_dot, fpga_gear[3], sizeof(char)*10);
				break;
			default :
				memcpy(value_dot, fpga_number[_car_rpm - '0'], sizeof(char)*10);
		}
	}
	else 
		memcpy(value_dot, fpga_set_blank, sizeof(char)*10);
	for(i=0; i<str_size; ++i) {
		//printk("%02x ", value_d[i]);//
		value_short = value_dot[i] & 0x7F;
		outw(value_short, (unsigned int)dot_addr+i*2);
	}
	/////////////////////
	return length;
}

// when read to sim device, call this function
ssize_t sim_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	unsigned char value[4];
	unsigned short int value_short = 0;
	char *tmp = gdata;

    //value_short = inw((unsigned int)stopwatch_addr);	    
    value[0] =(value_short >> 12) & 0xF;
    value[1] =(value_short >> 8) & 0xF;
    value[2] =(value_short >> 4) & 0xF;
    value[3] = value_short & 0xF;

    if (copy_to_user(tmp, value, 4))
        return -EFAULT;

	return length;
}

static void wq_func1(struct work_struct *work) {
	//del_timer(&stw_timer.timer);
	del_timer_sync(&stw_timer.timer);

	stw_timer.timer.expires = get_jiffies_64() + (0 * HZ);
	stw_timer.timer.data = (unsigned long)&stw_timer;
	stw_timer.timer.function = kernel_timer_repeat;

	add_timer(&stw_timer.timer);

	return;
}
static void wq_func2(struct work_struct *work) {
	del_timer_sync(&ex_timer.timer);

	ex_timer.count = 0;	

	ex_timer.timer.expires = get_jiffies_64() + (0 * HZ);
	ex_timer.timer.data = (unsigned long)&ex_timer;
	ex_timer.timer.function = kernel_timer_count;

	add_timer(&ex_timer.timer);

	return;
}

// timer function that is called every 1/10 second
static void kernel_timer_repeat(unsigned long tdata) {
	struct stopwatch_timer *t_ptr = (struct stopwatch_timer*)tdata;
	char data[4];
	int len = DIGIT;
	int i;

	t_ptr->t_sec++;
	//printk("log: kernel_timer_repeat:: t_sec: %d\n", t_ptr->t_sec);
	//printk(KERN_CONT "\r\t\t\t%d", t_ptr->t_sec);
	if(!_run_check) {
		printk(KERN_ALERT "\n");
		for(i=0; i<stw_timer.t_sec; ++i) 
			printk(KERN_CONT "  ");
	}
	printk(KERN_CONT " %d", t_ptr->t_sec);
	if(t_ptr->t_sec == 10) {
		t_ptr->t_sec = 0;
		printk("\n");
		t_ptr->sec++;
		if(t_ptr->sec == 60) {
			t_ptr->min++; 
			t_ptr->sec = 0;
		}
		if(t_ptr->min == 60) 
			t_ptr->min = 0;

		sprintf(data, "%02d%02d", t_ptr->min, t_ptr->sec);
		
		printk("\t\t\t[time  %02d:%02d]\n", t_ptr->min, t_ptr->sec);
		sim_write(t_ptr->inode, data, len, NULL);
	}

	t_ptr->timer.expires = get_jiffies_64() + (1 * HZ) / 10;
	t_ptr->timer.data = (unsigned long)t_ptr;
	t_ptr->timer.function = kernel_timer_repeat;

	add_timer(&stw_timer.timer);

	return;
}

static void kernel_timer_count(unsigned long tdata) {
	struct exit_timer *t_ptr = (struct exit_timer*)tdata;
	int i;
	//printk("log: kernel_timer_count: t_ptr->count(ex_timer.count): %d", t_ptr->count);
	printk("\n\r\t\tVOL- button pressed : about to shut down in %d second(s)\n", 3-t_ptr->count);
	if(_run_check || _pause_check) {
		_pause_check = !_run_check;
		printk(KERN_ALERT "\n");
		for(i=0; i<stw_timer.t_sec; ++i) 
			printk(KERN_CONT "  ");
	}
	t_ptr->timer.expires = get_jiffies_64() + (1 * HZ);
	t_ptr->timer.data = (unsigned long)t_ptr;
	t_ptr->timer.function = kernel_timer_count;

	t_ptr->count++;
	if(t_ptr->count < 4) 
		add_timer(&ex_timer.timer);
	
	if(ex_timer.count >= 3) {
		del_timer(&ex_timer.timer);
		del_timer(&stw_timer.timer);
		sim_blank(&stw_timer);

		_run_check = 0;

		__wake_up(&app_waitqueue, 1, 1, NULL);
		//wake_up_interruptible(&app_waitqueue);
		printk("wake up\n");
	}

	return;
}


// reset(turn off) devices
static void sim_blank(struct stopwatch_timer *tdata) {
	struct stopwatch_timer *t_ptr = (struct stopwatch_timer*)tdata;
	static char blank[4] = {0x30, 0x30, 0x30, 0x30};

	del_timer(&t_ptr->timer);

	printk("\nStopwatch stopped - %02d:%02d.%02d\n", t_ptr->min, t_ptr->sec, t_ptr->t_sec);
	printk("______________________________________________________________________\n");
	
	sim_write(t_ptr->inode, blank, 4, NULL);

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

			printk("log: iom_dev_ioctl: [1]msg : [%s]\n", msg);

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
						//strcpy(&_car_gear, str);	
						_car_gear = *str;
						break;
					}
				}
			}
			printk("log: iom_dev_ioctl: [2]\n\t\t_car_rpm:%d,\n\t\t_car_speed:%d,\n\t\t_car_gear:%c\n", _car_rpm, _car_speed, _car_gear);

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
			//printk("log: iom_dev_ioctl: [2]_command: %d\n", _command);
			
			//printk("sleep on\n");
			//interruptible_sleep_on(&app_waitqueue);
			//printk("waked up\n");

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

	init_timer(&stw_timer.timer);
	init_timer(&ex_timer.timer);
	printk(KERN_ALERT "Init module, %s Major number : %d\n", DEV_NAME, MAJOR_NUM);
	return 0;
}

void __exit sim_exit(void) {
	iounmap(fnd_addr);
	iounmap(led_addr);
	iounmap(dot_addr);
	iounmap(lcd_addr);
	iounmap(stm_addr);

	del_timer(&stw_timer.timer);
	del_timer(&ex_timer.timer);
	unregister_chrdev(MAJOR_NUM, DEV_NAME);

	sim_port_usage = 0;
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(sim_init);
module_exit(sim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YooHonghyeon");
