#/******************************************************************************
# *
# * Filename: Makefile
# *
# * Classic makefile intended for use in gnu environment.  The entries listed
# *  here are intentionally simple to aid the novice user in his/her own
# * development - for easy modification.
# *
# * Revision information:
# *
# * 31DEV2004	kb_admin	initial creation
# *
# * BEGIN_KBDD_BLOCK
# * No warranty, expressed or implied, is included with this software.  It is
# * provided "AS IS" and no warranty of any kind including statutory or aspects
# * relating to merchantability or fitness for any purpose is provided.  All
# * intellectual property rights of others is maintained with the respective
# * owners.  This software is not copyrighted and is intended for reference
# * only.
# * END_BLOCK
# ******************************************************************************
# 

SHELL = /bin/sh

basetoolsdir = /usr/local/arm/2.95.3
bindir = ${basetoolsdir}/bin
applibdir = /tools_source/work/kb9200-uclibc-dev/lib
clibdir = ${basetoolsdir}/lib/gcc-lib/arm-linux/2.95.3

CC = ${bindir}/arm-linux-gcc
AS = ${bindir}/arm-linux-as
LD = ${bindir}/arm-linux-ld
OBJCOPY = ${bindir}/arm-linux-objcopy

CFILES = main.o
APPLIBS=${applibdir}/libc.a ${applibdir}/crt0.o
CLIBS=${clibdir}/libgcc.a
LIBS=${APPLIBS} ${CLIBS}

sample_drv: ${CFILES} ${LIBS}
	${LD} -o sample_drv.out ${CFILES} ${LIBS}

main.o: main.c
	${CC} -c $?

clean:
	rm -f *.o
	rm -f *.out
