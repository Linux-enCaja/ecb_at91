#############################################################################
#
#	Makefile for building the robostix module
#
#############################################################################

ifeq ($(DEBUG),y)
	EXTRA_CFLAGS += -O -g		# -O is need to expand inlines
else
	EXTRA_CFLAGS += -O2
endif
											
CC = $(CROSS_COMPILE)gcc
GUMSTIX_BUILDROOT	= $(PWD)/../../gumstix-buildroot
BUILD_ARM		= $(GUMSTIX_BUILDROOT)/build_arm_nofpu
KERNELDIR 		= /home/cain/Embedded/AT91/work/linux/linux-2.6.24.4
CROSS_COMPILE		= arm-linux-

TARGET_ARCH=-Os -march=arm920t -Wa

obj-m	:= robostix_drv.o # robostix-spi.o
PWD       := $(shell pwd)

.PHONY: all kernel-module

all:	 kernel-module


kernel-module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions robostix sertest *.symvers

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
