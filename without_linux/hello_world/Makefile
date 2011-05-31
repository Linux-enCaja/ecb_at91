SHELL = /bin/sh

basetoolsdir = /home/at91/gcc-3.4.5-glibc-2.3.6/arm-softfloat-linux-gnu
bindir = ${basetoolsdir}/bin
libdir = ${basetoolsdir}/lib/gcc/arm-softfloat-linux-gnu/3.4.5

CC = arm-softfloat-linux-gnu-gcc 
AS = arm-softfloat-linux-gnu-as 
LD = arm-softfloat-linux-gnu-ld
OBJCOPY = arm-softfloat-linux-gnu-objcopy

OBJS = \
	main.o 			\
	debug_io.o 		\
	at91rm9200_lowlevel.o 	\
	p_string.o

ASFILES = arm_init.o

LIBS=${libdir}/libgcc.a

all: hello_world 

hello_world: ${OBJS} ${ASFILES} ${LIBS}
	${LD} -e 0 -o hello_world.out -T linker.cfg ${ASFILES} ${OBJS} ${LIBS}
	${OBJCOPY} -O binary hello_world.out hello_world.bin

clean:
	rm -f *.o *~ hello_world.*

PREPROCESS.c = $(CC) $(CPPFLAGS) $(TARGET_ARCH) -E -Wp,-C,-dD,-dI

%.pp : %.c FORCE
	$(PREPROCESS.c) $< > $@
