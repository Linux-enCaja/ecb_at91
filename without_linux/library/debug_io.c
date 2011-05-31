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


/*
 * .KB_C_FN_DEFINITION_START
 * void DebugPrintValue(int value)
 *  This global function displays the decimal value specified.
 * .KB_C_FN_DEFINITION_END
 */
void DebugPrintValue(int value) {

	char	dValue[16], *cPtr;
	int	modValue, nextDigit, leading;

	cPtr = dValue;

	if (value < 0) {
		*cPtr++ = '-';
		value = -value;
	}

	modValue = 1000000000;
	leading = 1;

	while (modValue) {
		nextDigit = value / modValue;
		if (nextDigit) {
			*cPtr++ = ToASCII(nextDigit);
			leading = 0;
		} else if (!leading) {
			*cPtr++ = '0';
		}
		value %= modValue;
		modValue /= 10;
	}

	*cPtr = 0;

	DebugPrint(dValue);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void ReadMem(unsigned address, unsigned length, unsigned size)
 *  This global function displays a block of memory in size increments.
 * .KB_C_FN_DEFINITION_END
 */
void ReadMem(unsigned address, unsigned length, unsigned size) {

	unsigned	i, perLine;
	char		charValue;
	unsigned short	shortValue;
	unsigned	longValue;
	

	perLine = 16 / size;

	while (length) {
		i = perLine;

		if (length < 16) {
			i = (length/size);
			length = 0;
		} else {
			length -= 16;
		}

		DebugPrint("\n\r(R)");
		DebugPrintHex(8, address);
		DebugPrint(" : ");

		while (i--) {
			switch (size) {
				case 1:
					charValue = *(unsigned char*)address;
					DebugPrintHex(2, charValue);
					address += 1;
					break;
				case 2:
					shortValue = *(unsigned short*)address;
					DebugPrintHex(4, shortValue);
					address += 2;
					break;
				case 4:
					longValue = *(unsigned*)address;
					DebugPrintHex(8, longValue);
					address += 4;
					break;
				default:
					return ;
			}

			DebugPutc(' ');
		}
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * void WriteMem(unsigned address, unsigned size, unsigned value)
 *  This global function writes value at address using size width.
 * .KB_C_FN_DEFINITION_END
 */
void WriteMem(unsigned address, unsigned size, unsigned value) {

	switch(size) {
	case 1:
		*(char*)address = (char)(value & 0xFF);
		break;
	case 2:
		*(unsigned short*)address = (unsigned short)(value & 0xFFFF);
		break;
	case 4:
		*(unsigned*)address = value;
		break;
	}
}
