#Makefile for a basic kernel module

obj-m   := stopwatch.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf *.cmd
	rm -rf .dev_*
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .tmp*
	rm -rf .stopwatch*
