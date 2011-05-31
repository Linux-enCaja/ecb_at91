/****************************************************************************
*
*   Copyright (c) 2006 Dave Hylands     <dhylands@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation.
*
*   Alternatively, this software may be distributed under the terms of BSD
*   license.
*
*
****************************************************************************/
/**
*
*  robostix_drv.c
*
*  PURPOSE:
*
*   This implements a driver for programming ECBOT's AVRs 
*
*   Initially, this contains the required support to emulate enough of the
*   parallel port interface to allow uisp to program the ATMega168.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <linux/sysctl.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/delay.h>
#include <asm/uaccess.h>
#include <asm/arch/hardware.h>
#include <asm/leds.h>
#include <asm/arch/gpio.h>

#include "robostix_drv.h"


#define PPI_DATA_VCC_MASK       ( 0x01 | 0x02 | 0x04 | 0x08 )
#define PPI_DATA_RESET_MASK    	( 0x20 )
#define PPI_DATA_SCK_MASK       ( 0x40 )
#define PPI_DATA_MOSI_MASK      ( 0x80 )
#define PPI_STATUS_MISO_MASK  	( 0x40 )

#define SET_GPIO( pin, val )    do { if ( val ) { at91_set_gpio_value( pin, 1 ); } else { at91_set_gpio_value( pin, 0 ); }} while(0)
#define GET_GPIO( pin )         (( GPLR( pin ) & GPIO_bit( pin )) != 0 )

// The Alternate function register is 2 bits per pin, so we can't use the
// GPIO_bit macro.

#define GPIO_AF_shift(x)        		(((x) & 0x0F ) << 1 )
#define GPIO_AF_mask(x)         	( 3 << GPIO_AF_shift( x ))

/*
 * Define the mappings between various GPIO pins and functions on the robostix
 * board.
 */
//Common signals
#define ROBOSTIX_GPIO_ATM_RESET	AT91_PIN_PC26    // output active low
#define ROBOSTIX_GPIO_ATM_MISO  AT91_PIN_PC27    // input

int ROBOSTIX_GPIO_ATM_MOSI[]  = {AT91_PIN_PC28, AT91_PIN_PC28, AT91_PIN_PC28, AT91_PIN_PC28, AT91_PIN_PC28}; // UM1, US1, US2, US3, US4
int ROBOSTIX_GPIO_ATM_SCK[]   = {AT91_PIN_PC29, AT91_PIN_PC29, AT91_PIN_PC29, AT91_PIN_PC29, AT91_PIN_PC29}; // UM1, US1, US2, US3, US4

static struct avr_dev
{
  char   device_number;  /*Device ID*/
  struct cdev cdev;
} *avr_devices;

typedef enum
{
    RoboStixGpioIn,
    RoboStixGpioOut,
} PinMode_e;

#define ROBOSTIX_DEV_NAME   "robostix"


dev_t           gRobostixDevNum;
struct  class  *gRobostixClass;
static  int    major = 0;
static  int    minor = 0;   // Select the UM's ISP signals
static  int    ndevs = 5;   // The ECBOT have 5 AVRs

/* ---- Private Function Prototypes -------------------------------------- */

static  void    robostix_configure_pin( int pin, PinMode_e pinMode );

static  void    robostix_exit( void );
static  int     robostix_init( void );
static  int     robostix_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg );
static  int     robostix_open( struct inode *inode, struct file *file );
static  int     robostix_release( struct inode *inode, struct file *file );

/****************************************************************************
*
*   File Operations (these are the device driver entry points)
*
*****************************************************************************/

static struct file_operations robostix_fops =
{
    owner:      THIS_MODULE,
    ioctl:      robostix_ioctl,
    open:       robostix_open,
    release:    robostix_release,
};


void robostix_configure_pin( int pin, PinMode_e pinMode )
{
    if ( pinMode == RoboStixGpioIn )
    {
        at91_set_gpio_input(pin,0);
    }
    else
    {
        at91_set_gpio_output(pin,1);
    }

} // robostix_configure_pin


void robostix_exit( void )
{
  int i;

  for (i = 0; i < ndevs; i++)
    cdev_del(&avr_devices[i].cdev); /* borra el registro del char device en el kernel*/

  unregister_chrdev_region(MKDEV(major, 0), ndevs);

} // robostix_exit


int __init robostix_init( void )
{
    int ret_val;
    char i = 0;
    dev_t dev;    


    dev = MKDEV(major, 0);

    if (major)
      ret_val = register_chrdev_region(dev, ndevs, ROBOSTIX_DEV_NAME);
    else
    { // Request a major number
      ret_val = alloc_chrdev_region(&dev, 0, ndevs, ROBOSTIX_DEV_NAME);
      major = MAJOR(dev);
    }

    if (ret_val < 0)
    {
      printk(KERN_ALERT "Error registering %s: %d\n", ROBOSTIX_DEV_NAME, ret_val);
      return ret_val;
    }

    SET_GPIO( ROBOSTIX_GPIO_ATM_RESET, 0 );     // AVR held in Reset (active low)

    robostix_configure_pin( ROBOSTIX_GPIO_ATM_RESET, RoboStixGpioOut );
    robostix_configure_pin( ROBOSTIX_GPIO_ATM_MISO,  RoboStixGpioIn );
    for (i = 0; i < ndevs; i++)
    {
      robostix_configure_pin( ROBOSTIX_GPIO_ATM_SCK[i],  RoboStixGpioOut );
      robostix_configure_pin( ROBOSTIX_GPIO_ATM_MOSI[i], RoboStixGpioOut );
    }

    avr_devices = kmalloc(ndevs * sizeof(struct avr_dev), GFP_KERNEL);

    if (!avr_devices)
    {
      ret_val = -ENOMEM;
      goto init_fail_malloc;
    }
    
    memset(avr_devices, 0, ndevs * sizeof(struct avr_dev)); // Initialize to 0s

    for (i = 0; i < ndevs; i++)
    {
      struct avr_dev *devs = avr_devices + i;

      devs->device_number = i;

      cdev_init(&devs->cdev, &robostix_fops);
      devs->cdev.owner = THIS_MODULE;
      ret_val = cdev_add(&devs->cdev, MKDEV(major, i), 1);

      if (ret_val)
        printk(KERN_NOTICE "Error %d adding %s%d\n", ret_val, ROBOSTIX_DEV_NAME, i);
    }
    printk(KERN_INFO "Device %s registration OK\n", ROBOSTIX_DEV_NAME);

    for (i = 0; i < ndevs; i++)
      printk(KERN_INFO "mknod /dev/%s%d c %d %d\n", ROBOSTIX_DEV_NAME, i, major, i);

    return 0;

  init_fail_malloc: /* Salida de la inicialización si un kmalloc falla */

    unregister_chrdev_region(dev, ndevs);

    if (avr_devices)
      kfree(avr_devices);

    return ret_val;

} // robostix_init

/****************************************************************************
*
*   robostix_ioctl
*
*****************************************************************************/

int robostix_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg )
{
    int err;

    if (( _IOC_TYPE( cmd ) != ROBOSTIX_IOCTL_MAGIC )
    ||  ( _IOC_NR( cmd ) < ROBOSTIX_CMD_FIRST )
    ||  ( _IOC_NR( cmd ) >= ROBOSTIX_CMD_LAST ))
    {
        // Since we emulate some of the parallel port commands, we need to allow
        // those as well.

        if (( _IOC_TYPE( cmd ) != PP_IOCTL )
        ||  ( _IOC_NR( cmd ) < 0x80 )
        ||  ( _IOC_NR( cmd ) >= 0x9b ))
        {
            return -ENOTTY;
        }
    }

    // Note that _IOC_DIR Read/Write is from the perspective of userland. access_ok
    // is from the perspective of kernelland.

    err = 0;
    if (( _IOC_DIR( cmd ) & _IOC_READ ) != 0 )
    {
        err |= !access_ok( VERIFY_WRITE, (void *)arg, _IOC_SIZE( cmd ));
    }
    if (( _IOC_DIR( cmd ) & _IOC_WRITE ) != 0 )
    {
        err |= !access_ok( VERIFY_READ, (void *)arg, _IOC_SIZE( cmd ));
    }
    if ( err )
    {
        return -EFAULT;
    }

    switch ( cmd )
    {
        //-------------------------------------------------------------------
        //
        // Parallel port interface. Some documentation on these ioctls can
        // be found here: 
        //  http://www.kernelnewbies.org/documents/kdoc/parportbook/x623.html
        //

        case PPRSTATUS:     // Read status register
        {
            unsigned char   statusReg = 0;
            int             miso;

            // The only thing mapped into the status register, is MISO.

            miso = at91_get_gpio_value( ROBOSTIX_GPIO_ATM_MISO );


            if ( miso )
            {
                statusReg |= PPI_STATUS_MISO_MASK;
            }

            if ( copy_to_user( (unsigned char *)arg, &statusReg, sizeof( statusReg )) != 0 )
            {
                return -EFAULT;
            }
            break;
        }

        case PPRCONTROL:    // Read control register
        {
            // Called once to initialize avrdude's shadow registers

            unsigned char controlReg = 0;

            if ( copy_to_user( (unsigned char *)arg, &controlReg, sizeof( controlReg )) != 0 )
            {
                return -EFAULT;
            }
            break;
        }

        case PPWCONTROL:    // Write control register
        {
            unsigned char controlReg = 0;

            if ( copy_from_user( &controlReg, (unsigned char *)arg, sizeof( controlReg )) != 0 )
            {
                return -EFAULT;
            }

            break;
        }

        case PPRDATA:   // Read data register
        {

            unsigned char   dataReg = 0;
            int sck, reset, mosi;
            int power = 0;

            sck   = at91_get_gpio_value( ROBOSTIX_GPIO_ATM_SCK[minor] );
            reset = at91_get_gpio_value( ROBOSTIX_GPIO_ATM_RESET );
            mosi  = at91_get_gpio_value( ROBOSTIX_GPIO_ATM_MOSI[minor] );

            if ( power )
            {
                dataReg |= PPI_DATA_VCC_MASK;
            }
            if ( reset )
            {
                dataReg |= PPI_DATA_RESET_MASK;
            }
            if ( sck )
            {
                dataReg |= PPI_DATA_SCK_MASK;
            }
            if ( mosi )
            {
                dataReg |= PPI_DATA_MOSI_MASK;
            }

            if ( copy_to_user( (unsigned char *)arg, &dataReg, sizeof( dataReg )) != 0 )
            {
                return -EFAULT;
            }
            break;
        }

        case PPWDATA:   // Write data register
        {
            unsigned char   dataReg = 0;
            int             power, sck, reset, mosi;
            
            if ( copy_from_user( &dataReg, (unsigned char *)arg, sizeof( dataReg )) != 0 )
            {
                return -EFAULT;
            }

            power = ( dataReg & PPI_DATA_VCC_MASK ) != 0;
            sck   = ( dataReg & PPI_DATA_SCK_MASK ) != 0;
            reset = ( dataReg & PPI_DATA_RESET_MASK ) != 0;
            mosi  = ( dataReg & PPI_DATA_MOSI_MASK ) != 0;

            SET_GPIO( ROBOSTIX_GPIO_ATM_SCK[minor], sck );
            SET_GPIO( ROBOSTIX_GPIO_ATM_RESET, reset );
            SET_GPIO( ROBOSTIX_GPIO_ATM_MOSI[minor], mosi );
            break;
        }

        case PPCLAIM:       // Claim the parallel port
        {
            robostix_configure_pin( ROBOSTIX_GPIO_ATM_MOSI[minor], RoboStixGpioOut );
            robostix_configure_pin( ROBOSTIX_GPIO_ATM_MISO, RoboStixGpioIn );
            break;
        }

        case PPRELEASE:     // Release the parallel port
        {
            // TODO: Set all programmer signals to secure state

            //robostix_set_pin_config();
            break;
        }

        case PPDATADIR:
        {
            int dataDirReg;

            if ( copy_from_user( &dataDirReg, (int *)arg, sizeof( dataDirReg )) != 0 )
            {
                return -EFAULT;
            }

            break;
        }

        default:
        {
            return -ENOTTY;
        }
    }

    return 0;

} // robostix_ioctl

int robostix_open( struct inode *inode, struct file *file )
{
    struct avr_dev *dev;
    dev = container_of(inode->i_cdev, struct avr_dev, cdev);
    printk(KERN_ALERT "/dev/robostix%d is calling the driver \n", dev->device_number);
    minor = dev->device_number;
    return 0;

} // robostix_open

int robostix_release( struct inode *inode, struct file *file )
{
    return 0;

} // robostix_release

/****************************************************************************/

module_init(robostix_init);
module_exit(robostix_exit);

MODULE_AUTHOR("Dave Hylands");
MODULE_DESCRIPTION("gumstix/robostix driver");
MODULE_LICENSE("Dual BSD/GPL");

