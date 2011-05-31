/*******************************************************************************
 *
 * Filename: debug_io.c
 *
 * Instantiation of routines for basic debug uart support.
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
#include "p_string.h"

/*
 * .KB_C_FN_DEFINITION_START
 * void DebugPutc(char)
 *  This global function writes a character to the debug uart port as soon
 * as it is ready to send another character.
 * .KB_C_FN_DEFINITION_END
 */
void DebugPutc(char cValue) {

	AT91PS_USART pUSART = (AT91PS_USART)AT91C_BASE_DBGU;

	while (!(pUSART->US_CSR & AT91C_US_TXRDY)) ;

	pUSART->US_THR = (cValue & 0x1FF);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void DebugPrint(char *)
 *  This global function writes a string to the debug uart port.
 * .KB_C_FN_DEFINITION_END
 */
void DebugPrint(char *buffer) {

	if (!buffer) return;

	while(*buffer != '\0') {
		DebugPutc(*buffer++);
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * int DebugGetchar(char *)
 *  This global function return true if a character was received from the
 * debug uart port and sets the character in the pointer passed.  Otherwise,
 * the function returns 0 if not character was available.
 * .KB_C_FN_DEFINITION_END
 */
int DebugGetchar(char *retChar) {

	AT91PS_USART pUSART = (AT91PS_USART)AT91C_BASE_DBGU;

	if ((pUSART->US_CSR & AT91C_US_RXRDY)) {
		*retChar = (char)((pUSART->US_RHR) & 0x1FF);
		return (1);
	}
	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * int WaitForChar(char *, int)
 *  This global function waits at least the specified number of seconds for
 * a character and returns non-zero if a character was received and stored in
 * the pointer.  Otherwise, the function returns 0.
 * .KB_C_FN_DEFINITION_END
 */
int WaitForChar(char *cPtr, int seconds) {

	unsigned	thisSecond;

	++seconds;
	thisSecond = GetSeconds();

	while (seconds) {
		if (DebugGetchar(cPtr)) {
			return (1);
		}

		if (GetSeconds() != thisSecond) {
			--seconds;
			thisSecond = GetSeconds();
		}
	}

	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void DebugPrintHex(int, int)
 *  This global function displays the value with the number of digits specified.
 * .KB_C_FN_DEFINITION_END
 */
void DebugPrintHex(int digits, int value) {

	char	dValue[11], *cPtr;
	int	nextDigit;

	if ((digits < 1) || (digits > 8)) return ;

	cPtr = &dValue[10];
	*cPtr-- = 0;
	while (digits--) {
		nextDigit = 0xF & value;
		*cPtr-- = ToASCII(nextDigit);
		value >>= 4;
	}
	*cPtr-- = 'x';
	*cPtr = '0';
	DebugPrint(cPtr);
}
