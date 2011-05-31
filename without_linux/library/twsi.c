/*******************************************************************************
 *
 * Filename: twsi.c
 *
 * Instantiation of the TWI temp sensor functions.
 *
 * Revision information:
 *
 * 20JAN2005	kb_admin	initial creation - development stage only
 *
 * BEGIN_KBDD_BLOCK
 * No warranty, expressed or implied, is included with this software.  It is
 * provided "AS IS" and no warranty of any kind including statutory or aspects
 * relating to merchantability or fitness for any purpose is provided.  All
 * intellectual property rights of others is maintained with the respective
 * owners.  This software is not copyrighted and is intended for reference
 * only.
 * END_BLOCK
 ******************************************************************************/


#include "AT91RM9200.h"
#include "prompt.h"

/* Master clock frequency at power-up */
#ifdef USE_80P_20M_CLOCKS
#define AT91C_MASTER_CLOCK 20000000
#else
#define AT91C_MASTER_CLOCK 60000000
#endif

/* TWSI Clock frequency */
#define AT91C_TWSI_CLOCK	100000

#define TWSI_THERMO_ADDRESS	0x49

#define SwapBytesHalfWord(x) ((((x & 0xFF00) >> 8) | ((x & 0xFF) << 8)))

parse_function_t	twsi_function;

__inline void AT91F_TWI_Configure ( AT91PS_TWI pTWI )          // \arg pointer to a TWI controller
{
    //* Disable interrupts
	pTWI->TWI_IDR = (unsigned int) -1;

    //* Reset peripheral
	pTWI->TWI_CR = AT91C_TWI_SWRST;

	//* Set Master mode
	pTWI->TWI_CR = AT91C_TWI_MSEN | AT91C_TWI_SVDIS;

}

__inline void AT91F_PIO_CfgOpendrain(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int multiDrvEnable) // \arg pio to be configured in open drain
{
	// Configure the multi-drive option
	pPio->PIO_MDDR = ~multiDrvEnable;
	pPio->PIO_MDER = multiDrvEnable;
}

__inline void AT91F_PIO_CfgPeriph(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int periphAEnable,  // \arg PERIPH A to enable
	unsigned int periphBEnable)  // \arg PERIPH B to enable

{
	pPio->PIO_ASR = periphAEnable;
	pPio->PIO_BSR = periphBEnable;
	pPio->PIO_PDR = (periphAEnable | periphBEnable); // Set in Periph mode
}

__inline void AT91F_TWI_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TWI));
}

__inline void AT91F_TWI_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA25_TWD     ) |
		((unsigned int) AT91C_PA26_TWCK    ), // Peripheral A
		0); // Peripheral B
}

void AT91F_SetTwiClock(const AT91PS_TWI pTwi) {
	int sclock;

	/* Here, CKDIV = 1 and CHDIV=CLDIV  ==> CLDIV = CHDIV = 1/4*((Fmclk/FTWI) -6)*/

	sclock = (10*AT91C_MASTER_CLOCK /AT91C_TWSI_CLOCK);
	if (sclock % 10 >= 5)
		sclock = (sclock /10) - 5;
	else
		sclock = (sclock /10)- 6;
	sclock = (sclock + (4 - sclock %4)) >> 2;	// div 4

	pTwi->TWI_CWGR = 0x00010000 | sclock | (sclock << 8);
}


static void AT91F_TWI_Read(const AT91PS_TWI pTwi , int address, char *data, int size) {
	unsigned status;

	pTwi->TWI_CR = AT91C_TWI_MSEN | AT91C_TWI_SVDIS;

	// Set the TWI Master Mode Register
	pTwi->TWI_MMR = (address <<16) | AT91C_TWI_IADRSZ_NO | AT91C_TWI_MREAD;	

	// Start transfer
	pTwi->TWI_CR = AT91C_TWI_START;

	status = pTwi->TWI_SR;

	while (size-- > 1) {

		// Wait RHR Holding register is full
		while (!(pTwi->TWI_SR & AT91C_TWI_RXRDY));

		// Read byte
		*(data++) = pTwi->TWI_RHR;

	}

	pTwi->TWI_CR = AT91C_TWI_STOP;
	status = pTwi->TWI_SR;

	// Wait transfer is finished
	while (!(pTwi->TWI_SR & AT91C_TWI_TXCOMP));

	// Read last byte
	*data = pTwi->TWI_RHR;
}

/*
 * void TWSI_Test(void)
 */

void TWSI_Test(void) {
	unsigned short read, iterations = 5;
	unsigned lastSecs;

	// Configure TWI PIOs
	AT91F_TWI_CfgPIO ();
	AT91F_PIO_CfgOpendrain(AT91C_BASE_PIOA, (unsigned int) AT91C_PA25_TWD);

	// Configure PMC by enabling TWI clock
	AT91F_TWI_CfgPMC ();

	// Configure TWI in master mode
	AT91F_TWI_Configure (AT91C_BASE_TWI);

	// Set TWI Clock Waveform Generator Register	
	AT91F_SetTwiClock(AT91C_BASE_TWI);

	DebugPrint("TWSI Test: reading device ");
	DebugPrintHex(2, TWSI_THERMO_ADDRESS);
	DebugPrint("\n\r");
/*
	// Write a byte and read it 
	AT91F_TWI_Write(AT91C_BASE_TWI, TWSI_THERMO_ADDRESS,
		(char *)&write, 2);
*/

	lastSecs = GetSeconds();

	while (iterations--) {

		while (GetSeconds() == lastSecs) ;

		lastSecs = GetSeconds();

		AT91F_TWI_Read(AT91C_BASE_TWI, TWSI_THERMO_ADDRESS,
			(char *)&read, 2);

		DebugPrintHex(4, (SwapBytesHalfWord(read) >> 7) & 0x1FF);
		DebugPrint("\n\r");
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * int twsi_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int twsi_function_parse(int argc, char *argv[]) {

	TWSI_Test();
	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void twsi_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void twsi_function_help(void) {
	DebugPrint("\tTWSI Temp sensor test\n\r");
	DebugPrint("\t\ttemp\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void InitTWSI(void)
 *  This global function initializes the generic support functions.
 * .KB_C_FN_DEFINITION_END
 */
void InitTWSI(void) {

	twsi_function.f_string = "temp";
	twsi_function.parse_function = twsi_function_parse;
	twsi_function.help_function = twsi_function_help;
	RegisterFunction(&twsi_function);
}
