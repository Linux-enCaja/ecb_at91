/*******************************************************************************
 *
 * Filename: processor.c
 *
 * Instantiation of processor/generic support routines
 *
 * Revision information:
 *
 * 20JAN2005	kb_admin	initial creation
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

#include "processor.h"
#include "AT91RM9200.h"
#include "prompt.h"

/* ****************************** GLOBALS *************************************/


/* ********************** PRIVATE FUNCTIONS/DATA ******************************/


parse_function_t	compare_function;
parse_function_t	busspeed_function;
parse_function_t	copy_function, copy2_function;
parse_function_t	execute_function, execute2_function;
parse_function_t	procspeed_function;
parse_function_t	memread_function;
parse_function_t	memwrite_function;

/* ************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * int compare_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int compare_parse(int argc, char *argv[]) {

	char		*to, *from;
	unsigned	size;

	if (argc > 3) {
		p_ASCIIToHex(argv[1], (unsigned*)&to);
		p_ASCIIToHex(argv[2], (unsigned*)&from);
		p_ASCIIToHex(argv[3], (unsigned*)&size);

		while (size--) {
			if (*to != *from) {
				DebugPrint(" Memory mismatch at: ");
				DebugPrintHex(8, to);
				DebugPrint(" expected: ");
				DebugPrintHex(2, *to);
				DebugPrint(" read:" );
				DebugPrintHex(2, *from);
				DebugPrint("\n\r");
			}
			to++;
			from++;
			if (!(size & 0x1FFF))
				DebugPrint(".");
		}
	}
	DebugPrint("\n\r\tCompare operation complete\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void compare_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void compare_help(void) {
	DebugPrint("\tcompare memory sections\n\r");
	DebugPrint("\t\tcompare <start_addr1> <start_addr2> <size>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int busspeed_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int busspeed_parse(int argc, char *argv[]) {

	DebugPrintValue(GetMasterClockSpeed());
	DebugPrint(" Hz");
	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void busspeed_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void busspeed_help(void) {
	DebugPrint("\tdisplay memory bus frequency\n\r");
	DebugPrint("\t\tbus\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int copy_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int copy_function_parse(int argc, char *argv[]) {

	char		*to, *from;
	unsigned	size;

	if (argc > 3) {
		p_ASCIIToHex(argv[1], (unsigned*)&to);
		p_ASCIIToHex(argv[2], (unsigned*)&from);
		p_ASCIIToHex(argv[3], (unsigned*)&size);
		p_memcpy(to, from, size);
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * void copy_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void copy_function_help(void) {
	DebugPrint("\tcopy memory\n\r");
	DebugPrint("\t\t<c | copy> <to> <from> <size>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int exec_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int exec_function_parse(int argc, char *argv[]) {

	void (*execAddr)(void);

	if (argc > 1) {
		p_ASCIIToHex(argv[1], (unsigned*)&execAddr);
		(*execAddr)();
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * void exec_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void exec_function_help(void) {
	DebugPrint("\texecute at address\n\r");
	DebugPrint("\t\t<e | exec> <address>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int proc_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int proc_function_parse(int argc, char *argv[]) {

	DebugPrintValue(GetProcessorClockSpeed());
	DebugPrint(" Hz");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void proc_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void proc_function_help(void) {
	DebugPrint("\tdisplay cpu frequency\n\r");
	DebugPrint("\t\tproc\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int read_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int read_function_parse(int argc, char *argv[]) {

	unsigned	size = 4, address, length;

	if (argc < 2) return (1);

	if (argc > 3) {
		p_ASCIIToHex(argv[1], &size);
		p_ASCIIToHex(argv[2], &address);
		p_ASCIIToHex(argv[3], &length);
	} else if (argc == 3) {
		p_ASCIIToHex(argv[1], &size);
		p_ASCIIToHex(argv[2], &address);
		length = size;
	} else {
		p_ASCIIToHex(argv[1], &address);
		length = size;
	}

	if (length < size) length = size;

	length -= length % size;

	ReadMem(address, length, size);
	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void read_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void read_function_help(void) {
	DebugPrint("\tDisplay memory\n\r");
	DebugPrint("\t\tread <size> <addr> <bytes>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int write_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int write_function_parse(int argc, char *argv[]) {

	unsigned	size, address, value;

	if (argc < 3) return (1);

	if (argc > 3) {
		p_ASCIIToHex(argv[1], &size);
		p_ASCIIToHex(argv[2], &address);
		p_ASCIIToHex(argv[3], &value);
	} else if (argc == 3) {
		size = 4;
		p_ASCIIToHex(argv[1], &address);
		p_ASCIIToHex(argv[2], &value);
	}

	WriteMem(address, size, value);
	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void write_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void write_function_help(void) {
	DebugPrint("\tModify memory\n\r");
	DebugPrint("\t\twrite <size> <addr> <value>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void InitProcessor(void)
 *  This global function initializes the generic support functions.
 * .KB_C_FN_DEFINITION_END
 */
void InitProcessor(void) {

	compare_function.f_string = "compare";
	compare_function.parse_function = compare_parse;
	compare_function.help_function = compare_help;
	RegisterFunction(&compare_function);

	busspeed_function.f_string = "bus";
	busspeed_function.parse_function = busspeed_parse;
	busspeed_function.help_function = busspeed_help;
	RegisterFunction(&busspeed_function);

	copy_function.f_string = "copy";
	copy_function.parse_function = copy_function_parse;
	copy_function.help_function = copy_function_help;
	RegisterFunction(&copy_function);

	copy2_function.f_string = "c";
	copy2_function.parse_function = copy_function_parse;
	copy2_function.help_function = 0;
	RegisterFunction(&copy2_function);

	execute_function.f_string = "exec";
	execute_function.parse_function = exec_function_parse;
	execute_function.help_function = exec_function_help;
	RegisterFunction(&execute_function);

	execute2_function.f_string = "e";
	execute2_function.parse_function = exec_function_parse;
	execute2_function.help_function = 0;
	RegisterFunction(&execute2_function);

	procspeed_function.f_string = "proc";
	procspeed_function.parse_function = proc_function_parse;
	procspeed_function.help_function = proc_function_help;
	RegisterFunction(&procspeed_function);

	memread_function.f_string = "read";
	memread_function.parse_function = read_function_parse;
	memread_function.help_function = read_function_help;
	RegisterFunction(&memread_function);

	memwrite_function.f_string = "write";
	memwrite_function.parse_function = write_function_parse;
	memwrite_function.help_function = write_function_help;
	RegisterFunction(&memwrite_function);
}


