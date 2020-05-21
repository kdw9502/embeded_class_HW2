#Makefile for a basic kernel module

obj-m   := module.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver app
#all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm
app:
	arm-none-linux-gnueabi-gcc -static -o app app.c
