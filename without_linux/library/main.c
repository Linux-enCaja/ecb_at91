/*******************************************************************************
 *
 * Filename: main.c
 *
 * Basic entry points for top-level functions
 *
 * Revision information:
 *
 * 20AUG2004	kb_admin	initial creation
 * 17APR2005	kb_admin	1.3.0:
 *				  added version tracking and several packages
 * 29APR2005	kb_admin	1.3.1:
 *				  added ROM-RAM copy operation
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

#include "debug_io.h"
#include "at91rm9200_lowlevel.h"
#include "prompt.h"
#include "flash.h"
#include "spi_flash.h"
#include "led.h"


int main(void) {

	DebugPrint("\n\rEntry: RAM Monitor v1.3.0\n\r");
	InitPrompt();
	SPI_InitFlash();
	RAM_Monitor(0);

	return (1);
}
