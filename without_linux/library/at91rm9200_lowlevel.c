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

/* Functions from Atmel's lib_AT91RM9200.h */

static unsigned int AT91F_CKGR_GetMainClock (
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock)  // \arg slowClock in Hz
{
	return ((pCKGR->CKGR_MCFR  & AT91C_CKGR_MAINF) * slowClock) >> 4;
}

static unsigned int AT91F_PMC_GetProcessorClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock)  // \arg slowClock in Hz
{
	unsigned int reg = pPMC->PMC_MCKR;
	unsigned int prescaler = (1 << ((reg & AT91C_PMC_PRES) >> 2));
	unsigned int pllDivider, pllMultiplier;

	switch (reg & AT91C_PMC_CSS) {
		case AT91C_PMC_CSS_SLOW_CLK: // Slow clock selected
			return slowClock / prescaler;
		case AT91C_PMC_CSS_MAIN_CLK: // Main clock is selected
			return AT91F_CKGR_GetMainClock(pCKGR, slowClock) / prescaler;
		case AT91C_PMC_CSS_PLLA_CLK: // PLLA clock is selected
			reg = pCKGR->CKGR_PLLAR;
			pllDivider    = (reg  & AT91C_CKGR_DIVA);
			pllMultiplier = ((reg  & AT91C_CKGR_MULA) >> 16) + 1;
			if (reg & AT91C_CKGR_SRCA) // Source is Main clock
				return AT91F_CKGR_GetMainClock(pCKGR, slowClock) / pllDivider * pllMultiplier / prescaler;
			else                       // Source is Slow clock
				return slowClock / pllDivider * pllMultiplier / prescaler;
		case AT91C_PMC_CSS_PLLB_CLK: // PLLB clock is selected
			reg = pCKGR->CKGR_PLLBR;
			pllDivider    = (reg  & AT91C_CKGR_DIVB);
			pllMultiplier = ((reg  & AT91C_CKGR_MULB) >> 16) + 1;
			return AT91F_CKGR_GetMainClock(pCKGR, slowClock) / pllDivider * pllMultiplier / prescaler;
	}
	return 0;
}

static unsigned int AT91F_PMC_GetMasterClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock)  // \arg slowClock in Hz
{
	return AT91F_PMC_GetProcessorClock(pPMC, pCKGR, slowClock) /
		(((pPMC->PMC_MCKR & AT91C_PMC_MDIV) >> 8)+1);
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
 * unsigned GetProcessorClockSpeed(void)
 *  Returns the frequency of the processor clock (Hz).
 * .KB_C_FN_DEFINITION_END
 */
unsigned GetProcessorClockSpeed(void) {

	return ((unsigned)AT91F_PMC_GetProcessorClock(
		(AT91PS_PMC)AT91C_BASE_PMC,
		(AT91PS_CKGR)AT91C_BASE_CKGR,
		32768));
}


/*
 * .KB_C_FN_DEFINITION_START
 * unsigned GetMasterClockSpeed(void)
 *  Returns the frequency of the master clock (Hz).
 * .KB_C_FN_DEFINITION_END
 */
unsigned GetMasterClockSpeed(void) {

	return ((unsigned)AT91F_PMC_GetMasterClock(
		(AT91PS_PMC)AT91C_BASE_PMC,
		(AT91PS_CKGR)AT91C_BASE_CKGR,
		32768));
}
