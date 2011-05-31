--- new_linux/drivers/char/Config.in	Thu Dec 30 19:55:22 2004
+++ linux/drivers/char/Config.in	Tue Dec 28 16:46:46 2004
@@ -136,6 +136,9 @@
 if [ "$CONFIG_ARCH_AT91RM9200" = "y" ]; then
   tristate 'AT91RM9200 SPI device interface' CONFIG_AT91_SPIDEV
 fi
+if [ "$CONFIG_ARCH_AT91RM9200" = "y" ]; then
+  tristate 'KB9200 Sample device driver' CONFIG_AT91_BASIC
+fi
 
 source drivers/serial/Config.in
 
diff -uN new_linux/drivers/at91/Makefile linux/drivers/at91/Makefile
--- new_linux/drivers/at91/Makefile	Thu Dec 30 19:55:22 2004
+++ linux/drivers/at91/Makefile	Tue Dec 28 16:17:36 2004
@@ -7,13 +7,14 @@
 
 O_TARGET := at91drv.o
 
-subdir-y :=	serial net watchdog rtc usb i2c spi mtd
+subdir-y :=	serial net watchdog rtc usb i2c spi mtd basic
 subdir-m :=	$(subdir-y)
 
 obj-$(CONFIG_SERIAL_AT91)		+= serial/at91serial.o
 obj-$(CONFIG_AT91_ETHER)		+= net/at91net.o
 obj-$(CONFIG_AT91_WATCHDOG)		+= watchdog/at91wdt.o
 obj-$(CONFIG_AT91_RTC)			+= rtc/at91rtc.o
+obj-$(CONFIG_AT91_BASIC)		+= basic/at91basic.o
 obj-$(CONFIG_USB)			+= usb/at91usb.o
 obj-$(CONFIG_I2C_AT91)			+= i2c/at91i2c.o
 obj-$(CONFIG_AT91_SPIDEV)		+= spi/at91spi.o

diff -uN new_linux/drivers/at91/basic/Makefile linux/drivers/at91/basic/Makefile
--- new_linux/drivers/at91/basic/Makefile	Wed Dec 31 19:00:00 1969
+++ linux/drivers/at91/basic/Makefile	Tue Dec 28 16:17:24 2004
@@ -0,0 +1,15 @@
+# File: drivers/at91/basic/Makefile
+#
+# Makefile for the KB9200 sample device driver
+#
+
+O_TARGET := at91basic.o
+
+obj-y	:=
+obj-m	:=
+obj-n	:=
+obj-	:=
+
+obj-$(CONFIG_AT91_BASIC) += at91_basic.o
+
+include $(TOPDIR)/Rules.make
diff -uN new_linux/drivers/at91/basic/at91_basic.c linux/drivers/at91/basic/at91_basic.c
--- new_linux/drivers/at91/basic/at91_basic.c	Wed Dec 31 19:00:00 1969
+++ linux/drivers/at91/basic/at91_basic.c	Tue Dec 28 16:15:40 2004
@@ -0,0 +1,91 @@
+/*
+ *	Sample device driver for the KB9200
+ *
+ *	This source is intended for reference only.
+ *
+ */
+
+#include <linux/module.h>
+#include <linux/fs.h>
+#include <linux/miscdevice.h>
+#include <linux/string.h>
+#include <linux/init.h>
+#include <linux/poll.h>
+#include <linux/proc_fs.h>
+#include <asm/bitops.h>
+#include <asm/hardware.h>
+
+static int at91_basic_open(struct inode *inode, struct file *file)
+{
+	printk(KERN_INFO "KB9200 Sample driver open\n");
+
+	return 0;
+}
+
+static int at91_basic_release(struct inode *inode, struct file *file)
+{
+	printk(KERN_INFO "KB9200 Sample driver release\n");
+
+	return 0;
+}
+
+/* The read call always returns the same string and does not account
+ *  for file offset.
+ */
+ssize_t at91_basic_read(struct file * file, char *buf, size_t count, loff_t * ppos)
+{
+	char	returnString[]="KB9200 Sample driver return string\n";
+
+	if (count > sizeof(returnString))
+		count = sizeof(returnString);
+
+	printk(KERN_INFO "Displaying kernel message on read call execution.\n");
+	copy_to_user(buf, returnString, count);
+
+	return (count);
+}
+
+static struct file_operations at91_basic_fops = {
+	owner:THIS_MODULE,
+	read:at91_basic_read,
+	open:at91_basic_open,
+	release:at91_basic_release
+};
+
+static struct miscdevice at91_basic_miscdev = {
+	minor:BASIC_MINOR,
+	name:"basic",
+	fops:&at91_basic_fops,
+};
+
+/*
+ * Initialize and install KB9200 sample driver
+ */
+static int __init at91_basic_init(void)
+{
+	misc_register(&at91_basic_miscdev);
+
+	printk(KERN_INFO "KB9200 Sample driver init\n");
+
+	return 0;
+}
+
+/*
+ * Disable and remove the RTC driver
+ */
+static void __exit at91_basic_exit(void)
+{
+	printk(KERN_INFO "KB9200 Sample driver exit\n");
+
+	misc_deregister(&at91_basic_miscdev);
+}
+
+module_init(at91_basic_init);
+module_exit(at91_basic_exit);
+
+MODULE_AUTHOR("kb9200_dev");
+MODULE_DESCRIPTION("KB9200 Sample device driver: at91_basic");
+MODULE_LICENSE("GPL");
+EXPORT_NO_SYMBOLS;
+
+
--- new_linux/include/linux/miscdevice.h	Sun Jan  2 15:11:49 2005
+++ linux/include/linux/miscdevice.h	Wed Dec 29 20:05:18 2004
@@ -25,6 +25,7 @@
 #define MICROCODE_MINOR		184
 #define MWAVE_MINOR		219	/* ACP/Mwave Modem */
 #define MPT_MINOR		220
+#define BASIC_MINOR		221	/* KB9200 Sample device driver */
 #define MISC_DYNAMIC_MINOR	255
 
 #define SGI_GRAPHICS_MINOR	146
