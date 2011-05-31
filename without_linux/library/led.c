/*******************************************************************************
 *
 * Filename: LED.c
 *
 * Instantiation of LED support routines
 *
 * Revision information:
 *
 * 27APR2005	kb_admin	initial creation
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

#include "led.h"
#include "AT91RM9200.h"
#include "prompt.h"

/* ****************************** GLOBALS *************************************/


/* ********************** PRIVATE FUNCTIONS/DATA ******************************/


parse_function_t	led_function;


/*
 * .KB_C_FN_DEFINITION_START
 * void SetLED(unsigned)
 *  This private function sets/clears the user LEDs.
 * .KB_C_FN_DEFINITION_END
 */
void SetLED(unsigned ledValue) {

	if (ledValue & 0x1) {
		AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC18;
	} else {
		AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC18;
	}

	if (ledValue & 0x2) {
		AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC19;
	} else {
		AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC19;
	}

	if (ledValue & 0x4) {
		AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC20;
	} else {
		AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC20;
	}
}


/* ************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * int led_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int led_parse(int argc, char *argv[]) {

	unsigned	ledValue;

	if (argc < 2)
		return 1;

	p_ASCIIToHex(argv[1], &ledValue);

	SetLED(ledValue);

	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void led_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void led_help(void) {
	DebugPrint("\tSet/clear LED signals\n\r");
	DebugPrint("\t\tled <value>\n\r");
	DebugPrint("\t\t  where value is (0..7)\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void InitLED(void)
 *  This global function initializes the LED pins.
 * .KB_C_FN_DEFINITION_END
 */
void InitLED(void) {

	led_function.f_string = "led";
	led_function.parse_function = led_parse;
	led_function.help_function = led_help;
	RegisterFunction(&led_function);

	AT91C_BASE_PIOC->PIO_PER = AT91C_PIO_PC18;
	AT91C_BASE_PIOC->PIO_OER = AT91C_PIO_PC18;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC18;

	AT91C_BASE_PIOC->PIO_PER = AT91C_PIO_PC19;
	AT91C_BASE_PIOC->PIO_OER = AT91C_PIO_PC19;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC19;

	AT91C_BASE_PIOC->PIO_PER = AT91C_PIO_PC20;
	AT91C_BASE_PIOC->PIO_OER = AT91C_PIO_PC20;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC20;
}
