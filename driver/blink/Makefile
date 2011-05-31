#Combiar las variables de entorno en (/install_dir/gumstix-buildroot/build_arm_nofpu/linux-2.6.11gum/Makefile):
#ARCH=arm
#CROSS=arm-linux-
obj-m +=  blinker.o #jtag_fpga.o
all:
	make -C /home/cain/Embedded/AT91/work/linux/linux-2.6.17 M=$(PWD) modules
clean:
	make -C /home/cain/Embedded/AT91/work/linux/linux-2.6.17   M=$(PWD) clean
		

