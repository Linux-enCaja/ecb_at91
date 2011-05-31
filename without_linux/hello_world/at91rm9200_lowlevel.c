/*******************************************************************************
 *
 * Filename: at91rm9200_lowlevel.c
 *
 * Instantiation of low-level routines to access the chip-specific 
 * functions/registers.
 *
 * Revision information:
 *
 * 20AUG2004	kb_admin	initial creation
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
#include "at91rm9200_lowlevel.h"
#include "debug_io.h"


/* ****************************** GLOBALS *************************************/

/* ***********************PRIVATE FUNCTIONS/DATA ******************************/

#define AT91C_US_ASYNC_MODE (AT91C_US_USMODE_NORMAL + AT91C_US_NBSTOP_1_BIT + AT91C_US_PAR_NONE + AT91C_US_CHRL_8_BITS + AT91C_US_CLKS_CLOCK)


/*
 * .KB_C_FN_DEFINITION_START
 * void Enable_Debug_USART(AT91PS_USART pUSART)
 *  This private function enables and configures the debug uart.
 * .KB_C_FN_DEFINITION_END
 */
static void Enable_Debug_USART(AT91PS_USART pUSART) {

	AT91PS_PDC pPDC = (AT91PS_PDC)&(pUSART->US_RPR);
	AT91PS_PIO pPio = (AT91PS_PIO)AT91C_BASE_PIOA;

	pPio->PIO_ASR =
		((unsigned)AT91C_PA31_DTXD) | ((unsigned)AT91C_PA30_DRXD);
	pPio->PIO_BSR = 0;
	pPio->PIO_PDR =
		((unsigned)AT91C_PA31_DTXD) | ((unsigned)AT91C_PA30_DRXD);

	pUSART->US_IDR = (unsigned int) -1;

	pUSART->US_CR =
		AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;

	pUSART->US_BRGR = ((((AT91C_MASTER_CLOCK*10)/(115200*16))+5)/10);

	pUSART->US_TTGR = 0;

	pPDC->PDC_PTCR = AT91C_PDC_RXTDIS;
	pPDC->PDC_PTCR = AT91C_PDC_TXTDIS;

	pPDC->PDC_TNPR = 0;
	pPDC->PDC_TNCR = 0;

	pPDC->PDC_RNPR = 0;
	pPDC->PDC_RNCR = 0;

	pPDC->PDC_TPR = 0;
	pPDC->PDC_TCR = 0;

	pPDC->PDC_RPR = 0;
	pPDC->PDC_RCR = 0;

	pPDC->PDC_PTCR = AT91C_PDC_RXTEN;
	pPDC->PDC_PTCR = AT91C_PDC_TXTEN;

	pUSART->US_MR = AT91C_US_ASYNC_MODE;

	pUSART->US_CR = AT91C_US_TXEN;
	pUSART->US_CR = AT91C_US_RXEN;
}


/* ************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * unsigned GetSeconds(void)
 *  Returns a value read from the RTC for use as a rough seconds counter.
 * .KB_C_FN_DEFINITION_END
 */
unsigned GetSeconds(void) {
	return (((AT91PS_RTC)AT91C_BASE_RTC)->RTC_TIMR & AT91C_RTC_SEC);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void DefaultSystemInit(void)
 *  If no system config info is found, config the board for default parameters.
 * .KB_C_FN_DEFINITION_END
 */
void DefaultSystemInit(void) {
	register unsigned	value;

	// configure clocks
	// assume:
	//    main osc = 10Mhz
	//    PLLB configured for 96MHz (48MHz after div)
	//    CSS = PLLB
	
	// set PLLA = 180MHz
	// assume main osc = 10Mhz
	// div = 5 , out = 2 (150MHz = 240MHz)
	value = ((AT91PS_CKGR)AT91C_BASE_CKGR)->CKGR_PLLAR;
	value &= ~AT91C_CKGR_DIVA;
	value &= ~AT91C_CKGR_OUTA;
#ifdef USE_80P_20M_CLOCKS
	value |= (OSC_MAIN_FREQ_DIV | AT91C_CKGR_OUTA_0);
#else
	value |= (OSC_MAIN_FREQ_DIV | AT91C_CKGR_OUTA_2);
#endif
	value |= AT91C_CKGR_SRCA;
	((AT91PS_CKGR)AT91C_BASE_CKGR)->CKGR_PLLAR = value;

	// mul = 90
	value = ((AT91PS_CKGR)AT91C_BASE_CKGR)->CKGR_PLLAR;
	value &= ~AT91C_CKGR_MULA;
#ifdef USE_80P_20M_CLOCKS
	value |= (39 << 16);
#else
	value |= (89 << 16);
#endif
	((AT91PS_CKGR)AT91C_BASE_CKGR)->CKGR_PLLAR = value;

	// wait for lock
	while (!((((AT91PS_PMC)AT91C_BASE_PMC)->PMC_SR) & AT91C_PMC_LOCKA)) ;

	// change divider = 3, pres = 1
	value = ((AT91PS_PMC)AT91C_BASE_PMC)->PMC_MCKR;
	value &= ~AT91C_PMC_MDIV;
#ifdef USE_80P_20M_CLOCKS
	value |= AT91C_PMC_MDIV_4;
#else
	value |= AT91C_PMC_MDIV_3;
#endif
	value &= ~AT91C_PMC_PRES;
	value |= AT91C_PMC_PRES_CLK;
	((AT91PS_PMC)AT91C_BASE_PMC)->PMC_MCKR = value;

	// wait for update
	while (!((((AT91PS_PMC)AT91C_BASE_PMC)->PMC_SR) & AT91C_PMC_MCKRDY)) ;

	// change CSS = PLLA
	value &= ~AT91C_PMC_CSS;
	value |= AT91C_PMC_CSS_PLLA_CLK;
	((AT91PS_PMC)AT91C_BASE_PMC)->PMC_MCKR = value;

	// wait for update
	while (!((((AT91PS_PMC)AT91C_BASE_PMC)->PMC_SR) & AT91C_PMC_MCKRDY)) ;

	// setup flash access (allow ample margin)
	// 7 wait states, 1 setup, 0 hold, 1 float for 8-bit device
	((AT91PS_SMC2)AT91C_BASE_SMC2)->SMC2_CSR[0] =
		AT91C_SMC2_WSEN |
		(7 & AT91C_SMC2_NWS) |
		((1 << 8) & AT91C_SMC2_TDF) |
		AT91C_SMC2_DBW_8 |
		((1 << 24) & AT91C_SMC2_RWSETUP) |
		((0 << 29) & AT91C_SMC2_RWHOLD);

	// setup SDRAM access
	// EBI chip-select register (CS1 = SDRAM controller)
	// 9 col, 13row, 4 bank, CAS2
	// write recovery = 2 (Twr)
	// row cycle = 5 (Trc)
	// precharge delay = 2 (Trp)
	// row to col delay 2 (Trcd)
	// active to precharge = 4 (Tras)
	// exit self refresh to active = 6 (Txsr)
	value = ((AT91PS_EBI)AT91C_BASE_EBI)->EBI_CSA;
	value &= ~AT91C_EBI_CS1A;
	value |= AT91C_EBI_CS1A_SDRAMC;
	((AT91PS_EBI)AT91C_BASE_EBI)->EBI_CSA = value;

	((AT91PS_SDRC)AT91C_BASE_SDRC)->SDRC_CR =
		AT91C_SDRC_NC_9 |
		AT91C_SDRC_NR_13 |
		AT91C_SDRC_NB_4_BANKS |
		AT91C_SDRC_CAS_2 |
		((2 << 7) & AT91C_SDRC_TWR) |
		((5 << 11) & AT91C_SDRC_TRC) |
		((2 << 15) & AT91C_SDRC_TRP) |
		((2 << 19) & AT91C_SDRC_TRCD) |
		((4 << 23) & AT91C_SDRC_TRAS) |
		((6 << 27) & AT91C_SDRC_TXSR);


	((AT91PS_SDRC)AT91C_BASE_SDRC)->SDRC_MR =
		AT91C_SDRC_DBW_16_BITS | AT91C_SDRC_MODE_PRCGALL_CMD;
	*(unsigned short*)SDRAM_BASE = 0;

	((AT91PS_SDRC)AT91C_BASE_SDRC)->SDRC_MR = 
		AT91C_SDRC_DBW_16_BITS | AT91C_SDRC_MODE_RFSH_CMD;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;
	*(unsigned short*)SDRAM_BASE = 0;

	((AT91PS_SDRC)AT91C_BASE_SDRC)->SDRC_MR =
		AT91C_SDRC_DBW_16_BITS | AT91C_SDRC_MODE_LMR_CMD;
	*(unsigned short*)SDRAM_BASE = 0;

	((AT91PS_SDRC)AT91C_BASE_SDRC)->SDRC_TR =
		7 * AT91C_MASTER_CLOCK / 1000000;

	*(unsigned short*)SDRAM_BASE = 0;

	((AT91PS_SDRC)AT91C_BASE_SDRC)->SDRC_MR =
		AT91C_SDRC_DBW_16_BITS | AT91C_SDRC_MODE_NORMAL_CMD;
	*(unsigned short*)SDRAM_BASE = 0;

	// Configure DBGU -use local routine optimized for space
	Enable_Debug_USART((AT91PS_USART)AT91C_BASE_DBGU);

	DebugPrint("\n\rDefault system configuration complete.\n\r");
}
