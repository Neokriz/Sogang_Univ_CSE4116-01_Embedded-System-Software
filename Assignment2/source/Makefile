#CROSS_COMPILER = arm-none-linux-gnueabi-
#CC = $(CROSS_COMPILER)gcc
#CFLAGS = -static -Wall
#DEPS = hw2_20171660.h

obj-m	:= dev_driver.o
KDIR	:=/work/achroimx_kernel
PWD		:=$(shell pwd)

all: driver app 

driver: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

app:  
	arm-none-linux-gnueabi-gcc -static -Wall -o app main.c 

clean:
	rm -rf *.ko
	rm -rf *.o
	rm -rf *.mod.*
	rm -rf *.cmd
	rm -rf .dev_*
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .tmp*
	rm -rf app
