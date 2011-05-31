/*
 *  QEM ADC/FFT  device driver
 *
 *  Author:     Andres Calderon
 *  Created:    September 16, 2005
 *  Copyright:  (C) 2005 emQbit Ltda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO */
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/interrupt.h>  /* We want an interrupt */
#include <linux/config.h>
#include <linux/platform_device.h>
#include <linux/fs.h>

#include <asm/hardware.h>
#include <asm/delay.h>
#include <asm/uaccess.h>  
#include <asm/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <asm/arch/at91rm9200.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#include <asm/arch/board.h>
#include <asm/arch/gpio.h>



#define FPGA_IRQ_PIN          AT91_PIN_PC19     // you can choice between PC19, PC20 & PC21

#define FPGA_IRQ     			AT91_ID_PIOC

#define FPGA_BASE             0x30000000//AT91_FPGA_BASE  //

#define SUCCESS 0
#define DEVICE_NAME "fpga" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80    /* Max length of the message from the device */



static struct resource fpga_resources[] = {
  [0] = {
    .start  = FPGA_BASE + 0x00000000,
    .end    = FPGA_BASE + 0x00004000,
    .flags  = IORESOURCE_MEM,
  },
  [1] = {
    .start  = FPGA_IRQ,
    .end    = FPGA_IRQ,
    .flags  = IORESOURCE_IRQ,
  }  
};


static struct platform_device fpga_device = {
  .name           = DEVICE_NAME,
  .id             = -1,
  .num_resources  = ARRAY_SIZE(fpga_resources),
  .resource       = fpga_resources,
};

   
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int irq_enabled = 0;
static int is_device_open = 0; /* Is device open?  Used to prevent multiple access to device */
static int Major; 

void __iomem *ioaddress;
static int interrupt_counter = 0;

static DECLARE_WAIT_QUEUE_HEAD(wq);


struct file_operations fops = {
  .owner	 = THIS_MODULE,
  .read   = device_read,
  .write  = device_write,
  .open   = device_open,
  .release = device_release
};



irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
  if(irq_enabled)
  {
    interrupt_counter++;

    //printk(KERN_INFO "interrupt_counter=%d\n",interrupt_counter);
	 printk("\n kernel: IREG_LP:%X, ISRCF:%X, IRQEn:%X \n", readb(&ioaddress[0x40]), readb(&ioaddress[0x42]), readb(&ioaddress[0x44]) );
    printk("\n kernel: Data Read= %X, %X, %X, %X\n", readb(&ioaddress[0]), readb(&ioaddress[16]), readb(&ioaddress[32]), readb(&ioaddress[48])   );
    wake_up_interruptible(&wq);
  }
  
  return IRQ_HANDLED;
}


static int __init qem_init(void)
{
  int res;
  printk(KERN_INFO "FPGA module is Up.\n");
         
  Major = register_chrdev(0, DEVICE_NAME, &fops);         
  
  	 if (Major < 0) {
      printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  } 
  
  printk(KERN_ALERT  "I was assigned major number %d. To talk to\n", Major);
  printk(KERN_ALERT  "the driver, create a dev file with\n");
  printk(KERN_ALERT  "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
  
  
  /* Set up the FGPA irq line */ 
    at91_set_gpio_input(FPGA_IRQ_PIN, 0);
    at91_set_deglitch(FPGA_IRQ_PIN, 1);    
    
	
  res = request_irq(AT91_PIN_PC19, irq_handler, SA_INTERRUPT, "FPGA - IRQ", NULL);
         
  printk("res= %d\n", res);
         
  set_irq_type(FPGA_IRQ, IRQT_FALLING);  

  ioaddress = ioremap(FPGA_BASE, 0x4000);
       
  return 0;
  //return platform_device_register(&fpga_device);
}


static void __exit qem_exit(void)
{
  int ret;
  /*Tho order for free_irq, iounmap & unregister is very important */
  free_irq(AT91_PIN_PC19, NULL);         
  iounmap(ioaddress);
  ret = unregister_chrdev(Major, DEVICE_NAME);  
  if (ret < 0)
    printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
  printk(KERN_INFO "FPGA driver is down...\n");
}


static int device_open(struct inode *inode, struct file *file)
{
  if (is_device_open)
    return -EBUSY;

  is_device_open = 1;

  try_module_get(THIS_MODULE);

  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{ 
  is_device_open = 0;  

  module_put(THIS_MODULE);

  return 0;
}



static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */
         char *buffer,  /* buffer to fill with data */
         size_t count, /* length of the buffer     */
         loff_t * offset)
{ 
  if(irq_enabled)
  {
    wait_event_interruptible(wq, interrupt_counter!=0);
  
    copy_to_user ( buffer, &interrupt_counter, sizeof(interrupt_counter) );  
  
    interrupt_counter=0;  
  }
  else  
  {   
    interrupt_counter = -1;
    copy_to_user ( buffer, &interrupt_counter, sizeof(interrupt_counter) );  
  } 
  
  return sizeof(interrupt_counter);
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t count, loff_t * off)
{
  const char cmd = buff[0];
  
  if(cmd=='Q')
  {
  /* Reset the FPGA  */

    irq_enabled       = 1;
    interrupt_counter = 0;
    printk(KERN_INFO "FPGA irq_enabled...\n");
        
  }
  else
    if(cmd=='S'){
      irq_enabled=0;
      printk(KERN_INFO "FPGA irq disabled.\n");
    }
  
  return 1;
}

module_init(qem_init);
module_exit(qem_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andres Calderon <andresn@emqbit.com>");
MODULE_DESCRIPTION("FPGA' IRQ driver");
MODULE_VERSION("1:0.1");






