/*
 *  jtag driver
 *
 *  Author:     Elphe, Carlos Camargo
 *  Created:    September 30, 2005
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO */
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/interrupt.h>  /* We want an interrupt */
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/delay.h>
#include <asm/uaccess.h>  
#include <asm/io.h>


//#include <asm/arch/gumstix.h>
//#include <asm/arch/pxa-regs.h>

#include <asm/arch/gpio.h>
#include "jtag_fpga.h"


#define SUCCESS 0
#define DEVICE_NAME "jtag" /* Dev name as it appears in /proc/jtags   */
#define BUF_LEN 80    /* Max length of the message from the jtag */


//static unsigned int ioaddress;
void __iomem *ioaddress;
static unsigned long porta_shadow=0x0000;  // only fpga uses port B
static  int prev32;
static int is_jtag_open = 0; /* Used to prevent multiple access to jtag */
static int Major;
static AT91PS_PIO pio;

static int readhead13 (void)
{
	int t;
	/* verify header is correct */
	t = memcmp(&bitstream_data[0], head13, 13);
	if (t)
	{
		return -1;
	}
	
	return 0;
}

static unsigned int ScanForByte(unsigned char SearchType){ /*length << 8 | position*/
	unsigned char counter;
	for( counter = 0; counter <FJTAG_MAX_HEAD; counter ++ ){
		if ( SearchType != 'd'){
		  if( ( bitstream_data[counter] == SearchType ) & ( bitstream_data[counter+1] == 0x00 ) )
				break;
		}
		else {
		  if( ( bitstream_data[counter -1 ] == 0x00 ) & ( bitstream_data[counter] == SearchType ) & ( bitstream_data[counter+1] == 0x00 ) )
				break;
		}
	}
	return ( ( ( counter + 3 ) << 8) ) | ( bitstream_data[counter+2] );

}


static unsigned char read_bit_stream_info (void)
{
	unsigned int SecInfo, idcode;
	
/*  Bitstream header
	00000000  00 09 0f f0 0f f0 0f f0  0f f0 00 00 01 61 00 0b  |.............a..|
	00000010  73 79 73 74 65 6d 2e 6e  63 64 00 62 00 0b 33 73  |system.ncd.b..3s|
	00000020  34 30 30 74 71 31 34 34  00 63 00 0b 32 30 30 35  |400tq144.c..2005|
	00000030  2f 20 38 2f 32 34 00 64  00 09 31 34 3a 20 30 3a  |/ 8/24.d..14: 0:|
	00000040  32 34 00 65 00 03 3d a8  ff ff ff ff aa 99 55 66  |24.e..=.......Uf|	
*/
	
/*	Bitstream Header and configuration options
	Dummy word                             FFFF FFFFh
	Synchronization word                   AA99 5566h
	Packet Header: Write to CMD register   3000 8001h
	Packet Data: RCRC (Reset CRS Register) 0000 0007h
	Packet Header: Write to FLR register   3001 6001h
	Packet Data: Frame Length              0000 00--h
	Packet Header: Write to COR            3001 2001h

	DONE_PIPE   30  0: No pipeline stage for DONEIN.
			1: Add pipeline stage to DONEIN.
			FPGA waits on DONE that is delayed by one (1) cycle of the StartupClk
			rather than the pin itself. Use this option when StartupClk is running at high
			speeds.
	DRIVE_DONE   29  0: DONE pin is open drain.
			 1: DONE pin is actively driven high.
			 SINGLE    28  Readback capture is one-shot.
			 OSCFSEL  27:22 Select CCLK frequency in Master Serial configuration mode.
	SSCLKSRC  21:20 Startup sequence clock source
			00: Cclk
			01: UserClk
			1x: JTAGClk
	LOCK_WAIT 19:16 4-bit mask indicating which DLL lock signals to wait for during LCK cycle.
			The 4 bits (from MSB to LSB) correspond to DLLs TL=3, TR=2, BL=1,
			BR=0 (top-left, top-right, etc.). In Virtex-E devices where there are 8 DLLs,
			each mask bit applies to the 2 DLLs in that quadrant of the device. The
			default is not to wait for any DLL lock signals.
	SHUTDOWN    15  Indicate whether doing a startup or shutdown sequence.
			0: Startup
			1: Shutdown sequence
	DONE_CYCLE 14:12 Startup phase in which DONE pin is released
	LCK_CYCLE  11:9 Stall in this startup phase until DLL locks are asserted.
	GTS_CYCLE  8:6  Startup phase in which I/Os switch from tri-state to user design
	GWE_CYCLE  5:3  Startup phase in which the global write-enable is asserted
	GSR_CYCLE  2:0  Startup phase in which the global set/reset is negated

	
	Packet Data: Configuration options   ---- ----h
	Packet Header: Write to MASK         3000 C001h
	Packet Data: CTL mask                0000 0000h
	Packet Header: Write to CMD register 3000 8001h
	Packet Data: SWITCH                  0000 0009h
	Packet Header: Write to FAR register 3000 2001h
	Packet Data: Frame address           0000 0000h
	Packet Header: Write to CMD register 3000 8001h
	Packet Data: WCFG                    0000 0001h
*/
	
	SecInfo = ScanForByte( 'a' );
	memcpy( filename, &bitstream_data[SEC_POS(SecInfo)], SEC_SIZE(SecInfo) ); // filename
	
	SecInfo = ScanForByte( 'b' );
	memcpy( BSpart, &bitstream_data[SEC_POS(SecInfo)], SEC_SIZE(SecInfo) ); // part
	
	SecInfo = ScanForByte( 'c' );
	memcpy( date, &bitstream_data[SEC_POS(SecInfo)], SEC_SIZE(SecInfo) ); // date
	
	SecInfo = ScanForByte( 'd' );
	memcpy( time, &bitstream_data[SEC_POS(SecInfo)], SEC_SIZE(SecInfo) ); // time
	
	SecInfo = ScanForByte( 'e' );
	
	bit_stream_length = ( bitstream_data[SEC_POS(SecInfo)-1] << 16 ) + \
			( bitstream_data[SEC_POS(SecInfo)] << 8 ) + bitstream_data[SEC_POS(SecInfo)+1]; //Bitstream size
	
	StartData = SEC_POS(SecInfo) +2;
	
	printk( "\nBitstream created from file %s on %s at %s\n", filename, date, time );
	printk( "Target Device = %s\n", BSpart);
	printk("Bitstream length is %d bytes.\n", bit_stream_length );
	
	printk("Startup CLK= 0x%02X\n", bitstream_data[StartData+29]  );
	if( ( bitstream_data[StartData+29] & 0x30 ) >> 5 != 1 ){
		printk("WARNING!! Startup Clock has been changed to Jtag CLK in the bitstream \
			\nstored in memory, but the original bitstream file remains unchanged\n");
// 		bitstream_data[StartData+29] |= 0x20;
	}
	printk("Startup CLK= 0x%02X\n", bitstream_data[StartData+29]  );
	
	
	if( ( buf8i - StartData ) == bit_stream_length )
		return RIGHT_BIT_STREAM;
	else{
		printk("Warning!! incorrect bistream size: Abort Configure process\n" );
		printk("buf8i=%d StartData=%d  ( buf8i - StartData ) = %d \n",buf8i, StartData,( buf8i - StartData ) );
	}
	return WRONG_BIT_STREAM;
}


int jtag_send (int arg) {  /* send 1..8 bits through JTAG */
	
	int d,i,m;
	int r=0;
	
	porta_shadow = (  PGM_OFF | INIT_OFF | TCK_OFF  | TMS_OFF | TDI_OFF );
	
	porta_shadow |= FPGA_JTAG_TMS(arg) << 2;	//Set TMS
	i=FPGA_JTAG_LEN(arg);
	if (i==0) i=8;
	d=FPGA_JTAG_DW(arg);

	for (;i>0;i--){ // The State of TMS pin at the rising edge of the TCK determine the state transition
		porta_shadow = ( porta_shadow & 0xfff5 ) | ( ( ( d<<=1 ) >> 7 ) & 0x02 ) ;
		writeb( porta_shadow, ioaddress );
		writeb( ( porta_shadow | 8 ), ioaddress );	//TCK = 1
		r= ( r<<1 )+ ( readb(ioaddress) & 0x01 );
	}

	porta_shadow = (  PGM_OFF | INIT_OFF | TCK_OFF  | TMS_OFF | TDI_OFF );
	
	m = ( r ^ ( prev32>>24 ) ) & 0xff;

	prev32= (prev32<<8) | (arg & 0xff);
	return (m ?0x100:0) |(r & 0xff);
}


static unsigned long GetIdcode(void){
	int j;
	unsigned long id = 0;
	int manuf, size, family, version;
	
	jtag_send(FPGA_JTAG_ARG( 1, 5, 0   )); //step 1 - set Test-Logic-Reset state
	jtag_send(FPGA_JTAG_ARG( 0, 1, 0   )); //step 2 - set Run-Test-Idle state
	jtag_send(FPGA_JTAG_ARG( 1, 2, 0   )); //step 3 - set SELECT-IR state
	jtag_send(FPGA_JTAG_ARG( 0, 2, 0   )); //step 4 - set SHIFT-IR state
	jtag_send(FPGA_JTAG_ARG( 0, 5, 0x90)); //step 5 - start of CFG_IN
	jtag_send(FPGA_JTAG_ARG( 1, 1, 0   )); //step 6 - finish CFG_IN
	jtag_send(FPGA_JTAG_ARG( 1, 2, 0   )); //step 7 - set SELECT-DR state
	jtag_send(FPGA_JTAG_ARG( 0, 2, 0   )); //step 8 - set SHIFT-DR state	
	
	
	for(j=0; j < 32; j++){
	  writeb( ( PGM_OFF | INIT_OFF | TCK_ON  | TMS_OFF | TDI_OFF ), ioaddress );
	  writeb( ( PGM_OFF | INIT_OFF | TCK_OFF | TMS_OFF | TDI_OFF ), ioaddress );
	  id= (id >> 1) | ( ( readb(ioaddress) & 0x01 ) << 31 );
	}
	
	//                          |<----  part number ----->|
	//               | version | family code | part size | manufacturer ID | 1
	//             bit: 31-28 |   27 - 21   |  20 - 12  |     11 - 1      | 0 
	//ex: XC2S100       0000 |   000-0011  |0-0001-0100|  000-0100-1001  | 1
	
	manuf   = ( id >> 01 ) & 0x07ff; 	//0x49 = Xilinx
	size    = ( id >> 12 ) & 0x01ff; 	//
	family  = ( id >> 21 ) & 0x007f; 	//0x03 = Spartan II ;  0x0A = Spartan III
	version = ( id >> 28 ) & 0x000f; 	//

	
	printk("Device ID: %x\n", id);
	printk("Manuf: %x, Part Size: %x, Family Code: %x, Version: %0d\n", manuf, size, family, version);
	return id;
	
}

static unsigned char VerifyDevice( unsigned long id ){
	
	unsigned char i;
	
	if( ( ( id >> 01 ) & 0x07ff ) != 0x049 )
	  return NON_XILINX_DEVICE;
	
	for(i=0; devices[i].idcode !=0; i++ )
		if(devices[i].idcode==id){
		  bitstream_size = devices[i].bitstream_size;
		  printk("Bit Stream Size = 0x%04x\n", bitstream_size);
	    	  return 0;
		}
	return DEVICE_MISMATCH;
}

static unsigned char Configure(void){
	unsigned long flags;
	int i,j;
	int d=0;
	int r=0;
	unsigned long id = 0;
	
	printk( "Set nPROGRAM = 0\n" );
	
	writeb( ( porta_shadow  &= 0xff3f ), ioaddress );  //PGM = 0
	for (i=0;i<1000;i++);
	writeb( ( porta_shadow  |= 0x0080 ) , ioaddress);  //PGM = 1
	i=0;
	while ( ( i < 1000 ) && ( ( readb(ioaddress) & 0x10)==0) ) i ++; //Wait for INIT = 1
	if( ( readb(ioaddress) & 0x10 ) == 0) {
     
/*    printk(KERN_INFO "readl(ioaddress)   = 0x%X \n",readl(ioaddress)); 
		printk("Error: FPGA not ready for configuration (Init is always 0)\r\n");
		return DEVICE_NOT_READY;*/
	}
	
	save_flags(flags); // Disable Interrupts
	cli();
	
	id = GetIdcode();
	if ( VerifyDevice(id) ) return DEVICE_MISMATCH;

	
	// FPGA CONFIGURATION ALGORITHM....
	
	jtag_send(FPGA_JTAG_ARG( 1, 5, 0   )); //step 1 - set Test-Logic-Reset state
	jtag_send(FPGA_JTAG_ARG( 0, 1, 0   )); //step 2 - set Run-Test-Idle state
	jtag_send(FPGA_JTAG_ARG( 1, 2, 0   )); //step 3 - set SELECT-IR state
	jtag_send(FPGA_JTAG_ARG( 0, 2, 0   )); //step 4 - set SHIFT-IR state
	jtag_send(FPGA_JTAG_ARG( 0, 5, 0xa0)); //step 5 - start of CFG_IN
	jtag_send(FPGA_JTAG_ARG( 1, 1, 0   )); //step 6 - finish CFG_IN
	jtag_send(FPGA_JTAG_ARG( 1, 2, 0   )); //step 7 - set SELECT-DR state
	jtag_send(FPGA_JTAG_ARG( 0, 2, 0   )); //step 8 - set SHIFT-DR state
	// write data
	printk ("StartData = %x\n", StartData);  
	for (i=StartData;i<(StartData+8);i++) {
		porta_shadow =0xfff0;
		d=bitstream_data[i];
//  (7) PGM (6)NC (5) DONE (4) INIT | (3) TCK (2) TMS (1) TDI (0) TDO 
		
		for (j=0;j<8;j++) {
			porta_shadow = ( porta_shadow & 0xfff5 ) | ( ( ( d<<=1 ) >> 7 ) & 2 );
			writeb( porta_shadow , ioaddress );
/*			
			bin_prnt_byte(porta_shadow);
			printk ("\n" );
			*/
			writeb( (porta_shadow | 8 ), ioaddress ); // TCK = 1
/*			
			bin_prnt_byte(porta_shadow | 8);
			printk ("\n" );
			*/
//			r= (r<<1)+ ( readb(ioaddress) & 1 );
		}
// 		prev32= (prev32<<8) | (bitstream_data[i] & 0xff);
	}
	// and not begin comparing readback
	for (i=StartData+8;i<(buf8i-1);i++) {
		porta_shadow &=0xfff0;
		d=bitstream_data[i];
// 		printk ("\n    0x%02X\n", d );
		for (j=0;j<8;j++) {
			writeb( (porta_shadow = ( porta_shadow & 0xfff5 ) | ( ( ( d<<=1 ) >> 7 ) & 2 ) ), ioaddress );
			
/*			bin_prnt_byte(porta_shadow);
			printk ("\n" );*/
			
			writeb( (porta_shadow | 8 ), ioaddress ); // TCK = 1
			
/*			bin_prnt_byte(porta_shadow | 8);
			printk ("\n" );*/
			
//			r= (r<<1)+ ( readb(ioaddress) & 1 );
		}
// 		if ( ( r ^ (prev32>>24) ) & 0xff ) break; //readback mismatch
// 		prev32= (prev32<<8) | (bitstream_data[i] & 0xff);
 	}
/*	if (i<(buf8i-1)) { // abort due to mismatch
		restore_flags(flags);
		writeb( ( porta_shadow  &= 0xff3f ), ioaddress );	// any delays to extend pulse?
		for (j=0;j<1000;j++);
		writeb( ( porta_shadow  |= 0x0080 ), ioaddress );
		printk ("**** Configuration failed at byte # %d (%x)****\n", (i-StartData),(i-StartData));
		printk ("**** r= %x, prev32=%x ****\n", r,prev32);
	}*/
	d=bitstream_data[buf8i-1];
	// now write the last byte (7+1 bits)
	jtag_send(FPGA_JTAG_ARG(0, 7, d)); //shift 7 data bits
	jtag_send(FPGA_JTAG_ARG(1, 1, (d<<7))); //step 10 - shift last data bit
	jtag_send(FPGA_JTAG_ARG(1, 1, 0   )); //step 11 - set UPDATE-DR state
	jtag_send(FPGA_JTAG_ARG(1, 2, 0   )); //step 12 - set SELECT-IR state
	jtag_send(FPGA_JTAG_ARG(0, 2, 0   )); //step 13 - set SHIFT-IR state
	jtag_send(FPGA_JTAG_ARG(0, 5, 0x30)); //step 14 - start of JSTART
	jtag_send(FPGA_JTAG_ARG(1, 1, 0   )); //step 15 - finish JSTART
	jtag_send(FPGA_JTAG_ARG(1, 2, 0   )); //step 16 - set SELECT-DR state
	jtag_send(FPGA_JTAG_ARG(0, 0, 0   )); //step 17 - set SHIFT-DR , clock startup
	jtag_send(FPGA_JTAG_ARG(0, 0, 0   )); //step 17a - (total >=12 clocks)
	jtag_send(FPGA_JTAG_ARG(1, 2, 0   )); //step 18 - set UPDATE-DR state
	jtag_send(FPGA_JTAG_ARG(0, 1, 0   )); //step 19 - set Run-Test-Idle state
	jtag_send(FPGA_JTAG_ARG(0, 0, 0   )); //step 17a - (total >=12 clocks)
	
	restore_flags(flags);
	
	if ( ( readb(ioaddress) & 0x20 )==0x20) {
		printk("\n*** FPGA did not start after configuration ***\r\n");
	}
}


static int jtag_open(struct inode *inode, struct file *file)
{
  printk( KERN_INFO "Open JTAGER\n" );
  if (is_jtag_open)
    return -EBUSY;

  is_jtag_open = 1;

  buf8i=0;

  try_module_get(THIS_MODULE);

  return SUCCESS;
}

static int jtag_release(struct inode *inode, struct file *file)
{ 
	
   is_jtag_open = 0;

   if( readhead13() < 0 ) {
	   printk( "Error: Wrong header. \n" );
	   return 0;
   }
 
   if ( read_bit_stream_info() == RIGHT_BIT_STREAM ){
     printk ( "Bitstream is Valid\n" );
     printk ( "Configuring\n" );
     Configure();
   }

   module_put(THIS_MODULE);

   printk( KERN_INFO "Close JTAGER\n" );

   return 0;



}



static ssize_t jtag_write(struct file *filp, const char *buf, size_t count, loff_t * off)
{
  if ((buf8i+count)> FJTAG_BUF_SIZE) return -EFAULT;
  if (copy_from_user(&bitstream_data[buf8i],buf,count)) return -EFAULT;
  buf8i+=count;
  printk(".");

  return count;

}


static int jtag_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
// 	unsigned long flags;
// 	int i;
// 	printk("fjtag_ioctl cmd= %x, arg= %x\n\r",cmd,(int) arg);
// //	printk("fjtag_ioctl:  ((int *)file->private_data)[0]= %x\n\r",((int *)file->private_data)[0]);
// 	if(_IOC_TYPE(cmd) != FPGACONF_IOCTYPE) {
// 		return -EINVAL;
// 	}
// 	switch (_IOC_NR(cmd)) {
// 		case FPGA_PGM:
// 		{
// 			/* set PGM FPGA pin (arg==0 - off(high), arg==1 - on (low)*/
// 			if (arg & 1) {
// 				writeb( ioaddress, ( porta_shadow  &= 0xff3f ) );
// 			} else {
// 				writeb( ioaddress, ( porta_shadow  |= 0x0080 ) );
// 			}
// 			return 0;
// 		}					    
// 		case FPGA_STAT:
// 		{
// 			/* get INIT (bit 0) and DONE (bit 1) FPGA pins status */
// 			return ( ( readb(ioaddress) >> 4 ) & 3 );
// 		}					    
// 		case FPGA_JTAG:
// 		{
// 			save_flags(flags);
// 			cli();
// 			i=jtag_send ((int) arg);
// 			restore_flags(flags);
// 			return i;
// 		}					    
// 		default:
// 			return -EINVAL;
// 		}

	return 0;

}


struct file_operations fops = {
  .write   = jtag_write,
  .open    = jtag_open,
  .release = jtag_release,
  .ioctl   = jtag_ioctl
};

#define CPLD_nCS_MD   GPIO80_nCS_4_MD

static int __init jtag_init(void)
{
  printk(KERN_INFO "JTAG module is Up.\n");

  Major = register_chrdev(0, DEVICE_NAME, &fops);         


  if (Major < 0) {
    printk(KERN_ALERT "Registering char jtag failed with %d\n", Major);
    return Major;
  } 

  printk(KERN_ALERT  "I was assigned major number %d. To talk to\n", Major);
  printk(KERN_ALERT  "the driver, create a dev file with\n");
  printk(KERN_ALERT  "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

//  ioaddress = ioremap(JTAG_BASE, 0x10);
  if (!ioaddress) {
	  printk( "ioremap failed\n");
  }

  return 0;
}


static void __exit jtag_exit(void)
{
  int ret;

  ret = unregister_chrdev(Major, DEVICE_NAME);
  iounmap( ioaddress );


  if ( ret < 0 )
    printk( KERN_ALERT "Error in unregister_chrdev: %d\n", ret );


  printk( KERN_INFO "JTAG driver is down...\n" );
}


module_init(jtag_init);
module_exit(jtag_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Camargo <cicamargoba@gmail.com>");
MODULE_DESCRIPTION("JTAGER LED driver");
MODULE_VERSION("1:0.1");
