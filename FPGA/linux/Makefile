ifeq ($(DEBUG),y)
	CFLAGS += -O -g		# -O is need to expand inlines
else
	CFLAGS += -O2
endif

PWD       := $(shell pwd)

CROSS_COMPILE		=arm-softfloat-linux-gnu-


CPPFLAGS +=-mcpu=arm920t -I. -I/home/at91/DISTRO/filesystem/buildroot/rootfs_debian/usr/include/
CFLAGS	 += -mcpu=arm920t -Wall -I/home/at91/DISTRO/filesystem/buildroot/rootfs_debian/usr/include/

TARGET_ARCH=-Os 
CC = $(CROSS_COMPILE)gcc

all: ncs0 pck blink sram pc19 irda

ncs0:ncs0.o

pck:pck.o

blink:blink.o

sram: sram.o

pc19: pc19.o

irda: irda.o

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions svn-version.h ncs0 pck sram blink irda 


PREPROCESS.c = $(CC) $(CPPFLAGS) $(TARGET_ARCH) -E -Wp,-C,-dD,-dI

%.pp : %.c FORCE
	$(PREPROCESS.c) $< > $@
