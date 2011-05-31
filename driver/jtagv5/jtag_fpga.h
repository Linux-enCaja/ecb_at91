/*
jtag_fpga.h
 Precautions When Using the JTAG Port in 3.3V Environments:

The JTAG port is powered by the +2.5V VCCAUX power supply. When connecting to a 3.3V interface, the JTAG input
pins must be current-limited to 10 mA or less using a series resistor. Similarly, the TDO pin is a CMOS output
powered from +2.5V. The TDO output can directly drive a 3.3V input but with reduced noise immunity. See the
3.3V-Tolerant Configuration Interface section in Module 2: Functional Description for additional details.

The following interface precautions are recommended when connecting the JTAG port to a 3.3V interface.
1. Set any inactive JTAG signals, including TCK, Low when not actively used. 2. Limit the drive current into a 
JTAG input to no more than 10 mA.
*/

#ifndef _JTAG_FPGA_H
#define _JTAG_FPGA_H

#define FPGACONF_IOCTYPE     129
#define FPGA_PGM	0x3   /* set FPGA pgm pin */
#define FPGA_STAT	0x4   /* read FPGA pins status (bit 0 - INIT, bit 1 - DOME) */
#define FPGA_JTAG	0x5   /* shift byte to FPGA JTAG */
#define FPGA_PA_RD	0x6   /* write data to port A and shadow */
#define FPGA_PA_WR	0x7   /* write data to port A and shadow */

//  (7) PGM (6)NC (5) DONE (4) INIT | (3) TCK (2) TMS (1) TDI TDO (0)

#define FPGA_JTAG_ARG(tms, len, d) (((tms) << 11) | ((len) << 8) | ((d) & 0xff))
#define FPGA_JTAG_TMS(arg) ((arg >> 11) &    1)
#define FPGA_JTAG_LEN(arg) ((arg >> 8)  &    7)
#define FPGA_JTAG_DW(arg)  ( arg        & 0xff)
#define SEC_POS(arg)       ( (arg >> 8) & 0x00ff )
#define SEC_SIZE(arg)      ( arg & 0x00ff )

#define TDI	AT91_PIN_PC8
#define TDO	AT91_PIN_PC18
#define TMS	AT91_PIN_PC16
#define TCK	AT91_PIN_PC17
#define PGM	AT91_PIN_PC9
#define INIT AT91_PIN_PC7


#define XC2S300_BYTESIZE 212392
#define FJTAG_BUF_SIZE   0x40000
#define FJTAG_MAX_HEAD   0x0FF
#define FPGA_PART_NAME	 "3s400tq144"

#define JTAG_BASE	 PXA_CS1_PHYS
#define JTAG_PORT_WRITE  ioaddress
#define JTAG_PORT_READ   ioaddress
#define RIGHT_BIT_STREAM 0x1
#define WRONG_BIT_STREAM 0x1


#define NON_XILINX_DEVICE	0x00
#define DEVICE_NOT_READY	0x01
#define DEVICE_MISMATCH		0x02
#define UNKNOWN_DEVICE		0x03


#endif

static unsigned char head13[] = {0, 9, 15, 240, 15, 240, 15, 240, 15, 240, 0, 0, 1};
static unsigned char bitstream_data[FJTAG_BUF_SIZE];    // will fit bitstream and the header (if any)
static	int buf8i=0;					// current buffer length (in bytes!)
static unsigned char StartData;				// First data to write: bitstream_data[StartData]
static unsigned int bitstream_size;


unsigned char filename[32] = "";
unsigned char BSpart[16]= "";
unsigned char BSpackage[16]= "";
unsigned char date[16]= "";
unsigned char time[16]= "";
unsigned int bit_stream_length=0;

typedef struct _idCodeName 
{
	unsigned long idcode;
	char* device_name;
	unsigned int bitstream_size;
}idCodeName;

idCodeName devices[] = {
	{0x00608093,"2s15",     24172},
	{0x0060C093,"2s30",     42096},
	{0x00610093,"2s50",     69900},
	{0x00614093,"2s100",    97652},
	{0x00618093,"2s150",   130012},
	{0x0061C093,"2s200",   166980},
	
	{0x00A10093,"2s50e",    78756},
	{0x00A14093,"2s100e",  107980},
	{0x00A18093,"2s150e",  141812},
	{0x00A1C093,"2s200e",  180252},
	{0x00A20093,"2s300e",  234456},
	
	{0x0140C093,"3s50",     54908},
	{0x01414093,"3s200",   130952},
	{0x0141C093,"3s400",   212392},
	{0,0,0}
};


void bin_prnt_byte(unsigned char x)
{
	int n;
	for(n=0; n < 8; n++)
	{
		if((x & 0x80) !=0)
		{
			printk("1");
		}
		else
		{
			printk("0");
		}
		if (n==3)
		{
			printk(" "); /* insert a space between nybbles */
		}
		x = x<<1;
	}
}

void bin_prnt_int(int x)
{
	unsigned char hi, lo;
	hi=(x>>8) & 0xff;
	lo=x&0xff;
	bin_prnt_byte(hi);
	printk(" ");
	bin_prnt_byte(lo);
}


