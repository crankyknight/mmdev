DEBUG=y
ifeq ($(DEBUG),y)
	EXTRA_CFLAGS= -g 
endif

ifneq ($(KERNELRELEASE),)
obj-m += mmdev.o
else 
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	$(RM) *.o *.ko *.mod.c *.order *.symvers
