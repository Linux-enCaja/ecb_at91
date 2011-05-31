SHELL = /bin/sh

basetoolsdir = /home/at91/gcc-3.4.5-glibc-2.3.6/arm-softfloat-linux-gnu/
bindir = ${basetoolsdir}/bin
libdir = ${basetoolsdir}/lib/gcc/arm-softfloat-linux-gnu/3.4.5/

CC = arm-softfloat-linux-gnu-gcc 
AS = arm-softfloat-linux-gnu-as 
LD = arm-softfloat-linux-gnu-ld
OBJCOPY = arm-softfloat-linux-gnu-objcopy



OBJS = \
	main.o 			\
	debug_io.o 		\
	at91rm9200_lowlevel.o 	\
	p_string.o 		\
	spi_flash.o 		\
	led.o 			\
	prompt.o
	
ASFILES = arm_init.o

LIBS=${libdir}/libgcc.a

all: ramMonitor

ramMonitor: ${OBJS} ${ASFILES} ${LIBS}
	${LD} -e 0 -o ramMonitor.out -T linker.cfg ${ASFILES} ${OBJS} ${LIBS}
	${OBJCOPY} -O binary ramMonitor.out ramMonitor.bin

main.o: main.c
	${CC} -c $?

debug_io.o: debug_io.c
	${CC} -c $?

at91rm9200_lowlevel.o: at91rm9200_lowlevel.c
	${CC} -c $?

prompt.o: prompt.c
	${CC} -c $?

p_string.o: p_string.c
	${CC} -c $?

xmodem.o: xmodem.c
	${CC} -c $?

eeprom.o: eeprom.c
	${CC} -c $?

flash.o: flash.c
	${CC} -c $?

emac.o: emac.c
	${CC} -c $?

test_uhp.o: test_uhp.c
	${CC} -c $?

twsi.o: twsi.c
	${CC} -c $?

irda.o: irda.c
	${CC} -c $?

spi_flash.o: spi_flash.c
	${CC} -c $?

processor.o: processor.c
	${CC} -c $?

lcd.o: lcd.c
	${CC} -c $?

led.o: led.c
	${CC} -c $?

arm_init.o: arm_init.s
	${AS} -o $@ $?

clean:
	rm -f *.o
