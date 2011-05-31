/*******************************************************************************
 *
 * Filename: flash.c
 *
 * Instantiation of flash control routines supporting Intel/Micron StrataFlash
 *
 * Revision information:
 *
 * 20SEP2004	kb_admin	initial creation
 * 01MAY2005	kb_admin	changed to support KB9202 flash device
 * 14JUN2005	kb_admin	updated this block to reflect previous source change
 * 05JUL2005	kb_admin	added block unlock for flash chip erase
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

#include "debug_io.h"
#include "flash.h"
#include "prompt.h"

/* ********************** PRIVATE FUNCTIONS/DATA ******************************/


parse_function_t	parallel_flash_function, parallel2_flash_function;


/*
 * .KB_C_FN_DEFINITION_START
 * void EraseFlashSector(unsigned startAddr, unsigned endAddr)
 *  Private function to erase a group of blocks from the expansion flash.
 * .KB_C_FN_DEFINITION_END
 */
void EraseFlashSector(unsigned startAddr, unsigned endAddr) {

	char		cValue;

	if ((startAddr < FLASH_BASE) || (startAddr >= FLASH_MAX) ||
		(endAddr < FLASH_BASE) || (endAddr >= FLASH_MAX) ||
		(startAddr > endAddr))
		return ;

	while (startAddr <= endAddr) {

		DebugPrint(" Erasing block: ");
		DebugPrintHex(8, startAddr);
		DebugPrint("\n\r");

		*(char*)startAddr = 0x20;
		*(char*)startAddr = 0xd0;
		while (!((cValue = *(char*)startAddr) & 0x80)) ;

		startAddr += FLASH_SECTOR_SIZE;
	}

	*(char*)endAddr = 0xff;
}


/*
 * .KB_C_FN_DEFINITION_START
 * void EraseFlashSector(unsigned startAddr, unsigned endAddr)
 *  Private function to program flash starting at target using data supplied
 * at source for size bytes.
 * .KB_C_FN_DEFINITION_END
 */
void ProgramFlash(unsigned startAddr, unsigned srcAddr, unsigned size) {

	char	cValue;

	while (size--) {
		*(char*)startAddr = 0x40;
		*(char*)startAddr = *(char*)srcAddr;
		while (!((cValue = *(char*)startAddr) & 0x80)) ;
		++startAddr;
		++srcAddr;
		if (!(size & 0x1FFF))
			DebugPrint(".");
	}

	DebugPrint("\n\r");

	*(char*)startAddr = 0xFF;
}


/*
 * .KB_C_FN_DEFINITION_START
 * void UnlockBits(void)
 *  Private function to unlock flash sectors - intended for use before chip
 * erase operation.
 * .KB_C_FN_DEFINITION_END
 */
void UnlockBits(void) {
	char	*cPtr, cValue;

	cPtr = (char*)0x10000000;

	*cPtr = 0x60;
	*cPtr = 0xd0;

	while (!((cValue = *cPtr) & 0x80)) ;

	*cPtr = 0xFF;
}


/* ************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * void EraseFlashChip(void)
 *  Global function to erase the entire flash chip and verify results.
 * .KB_C_FN_DEFINITION_END
 */
void EraseFlashChip(void) {

	EraseFlashSector(FLASH_BASE, FLASH_MAX - 1);
}


/*
 * .KB_C_FN_DEFINITION_START
 * int p_flash_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int p_flash_parse(int argc, char *argv[]) {

	char		opChar;
	unsigned	startAddr, endAddr, srcAddr, size;

	if (argc < 2)
		return (1);
	opChar = *argv[1];
	switch (opChar) {
		case 'e': /* erase flash chip */
		case 'E':
			if (argc < 3) {
				UnlockBits();
				EraseFlashChip();
				break;
			} else {
				p_ASCIIToHex(argv[2], &startAddr);

				endAddr = startAddr + 1;
				if (argc > 3)
					p_ASCIIToHex(argv[3],
						&endAddr);

				EraseFlashSector(startAddr, endAddr);
			}
			break;

		case 'p': /* program flash */
		case 'P':
			if (argc < 5) {
				DebugPrint("Missing parameter");
				break;
			}

			p_ASCIIToHex(argv[2], &startAddr);
			p_ASCIIToHex(argv[3], &srcAddr);
			p_ASCIIToHex(argv[4], &size);
			ProgramFlash(startAddr, srcAddr, size);
			break;

		default:
			DebugPrint("Operation not recognized");
	}
	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void p_flash_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void p_flash_help(void) {
	DebugPrint("\terase flash chip\n\r");
	DebugPrint("\t\t<f | flash> e\n\r");
	DebugPrint("\terase sectors defined by address range\n\r");
	DebugPrint("\t\t<f | flash> e <from> | <to>\n\r");
	DebugPrint("\tprogram flash using memory range\n\r");
	DebugPrint("\t\t<f | flash> p <flash_addr> <data_src_addr> <size>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void InitFlash(void)
 *  Global function to initialize flash utilities.
 * .KB_C_FN_DEFINITION_END
 */
void InitFlash(void) {

	// setup expansion bus 16MB flash
	// 18 wait states, 1 setup, 1 hold, 1 float for 8-bit device
	AT91C_BASE_SMC2->SMC2_CSR[3] =
		AT91C_SMC2_WSEN |
		(18 & AT91C_SMC2_NWS) |
		((1 << 8) & AT91C_SMC2_TDF) |
		AT91C_SMC2_DBW_8 |
		((1 << 24) & AT91C_SMC2_RWSETUP) |
		((1 << 29) & AT91C_SMC2_RWHOLD);

	parallel_flash_function.f_string = "flash";
	parallel_flash_function.parse_function = p_flash_parse;
	parallel_flash_function.help_function = p_flash_help;
	RegisterFunction(&parallel_flash_function);

	parallel2_flash_function.f_string = "f";
	parallel2_flash_function.parse_function = p_flash_parse;
	parallel2_flash_function.help_function = 0;
	RegisterFunction(&parallel2_flash_function);
}
