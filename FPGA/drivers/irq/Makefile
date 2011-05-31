CFLAGS += -mcpu=arm920t -Wall -I/home/at91/DISTRO/filesystem/buildroot/rootfs_debian/usr/include/
CC = /home/at91/DISTRO/filesystem/buildroot/home/at91/br_build_build_arm/staging_dir/bin/arm-linux-gcc
obj-m +=  irq.o 
all: driver main

driver:
	make  -C /home/cain/Embedded/AT91/work/linux/linux-2.6.17 M=$(PWD) modules
clean:
	make -C /home/cain/Embedded/AT91/work/linux/linux-2.6.17   M=$(PWD) clean
	rm -rf *.o main.o main irq.ko Modules.symvers

main: main.o

PREPROCESS.c = $(CCB) $(CFLAGS) $(TARGET_ARCH) -E -Wp,-C,-dD,-dI
%.pp : %.c FORCE
	$(PREPROCESS.c) $< > $@
	
		

