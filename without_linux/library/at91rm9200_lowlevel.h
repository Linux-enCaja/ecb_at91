/*******************************************************************************
 *
 * Filename: at91rm9200_lowlevel.h
 *
 * Definition of low-level routines to access the chip-specific 
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

#ifndef _AT91RM9200_LOWLEVEL_H_
#define _AT91RM9200_LOWLEVEL_H_

/* default system config parameters */

/* Define following for 80MHz processor/20MHz master */
/* If not defined, set for 180MHz processor/60MHz master */
// #ifdef USE_80P_20M_CLOCKS

#define SDRAM_BASE		0x20000000

/* The following divisor sets PLLA frequency: e.g. 10/5 * 90 = 180MHz */
#define OSC_MAIN_FREQ_DIV	9	/* for 10MHz osc */

/* Master clock frequency at power-up */

#define AT91C_MASTER_CLOCK 60000000




extern unsigned GetSeconds(void);
extern unsigned GetProcessorClockSpeed(void);
extern unsigned GetMasterClockSpeed(void);

#endif
