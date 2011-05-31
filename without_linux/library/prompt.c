/*******************************************************************************
 *
 * Filename: prompt.c
 *
 * Instantiation of the interactive loader functions.
 *
 * Revision information:
 *
 * 20AUG2004	kb_admin	initial creation
 * 20SEP2004	kb_admin	adapted for RAM monitor functionality
 * 12JAN2005	kb_admin	updated for string support, added functions
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
#include "p_string.h"
#include "prompt.h"


/* ****************************** GLOBALS *************************************/


/* ********************** PRIVATE FUNCTIONS/DATA ******************************/

static parse_function_t	help1_function, help2_function, help3_function;
static char		inputBuffer[MAX_INPUT_SIZE];
static int		buffCount;

static parse_function_t	*parse_list_head, *parse_list_tail;
static int nested;

// argv pointer are either NULL or point to locations in inputBuffer
static char		*argv[MAX_COMMAND_PARAMS];

const static char	backspaceString[] = {0x8, ' ', 0x8, 0};



/*
 * .KB_C_FN_DEFINITION_START
 * int BreakCommand(char *)
 *  This private function splits the buffer into separate strings as pointed
 * by argv and returns the number of parameters (< 0 on failure).
 * .KB_C_FN_DEFINITION_END
 */
static int BreakCommand(char *buffer) {
	int	pCount, cCount, state;

	state = pCount = 0;
	p_bzero((char*)argv, sizeof(argv));

	for (cCount = 0; cCount < MAX_INPUT_SIZE; ++cCount) {

		if (!state) {
			/* look for next command */
			if (!p_IsWhiteSpace(buffer[cCount])) {
				argv[pCount++] = &buffer[cCount];
				state = 1;
			} else {
				buffer[cCount] = 0;
			}
		} else {
			/* in command, find next white space */
			if (p_IsWhiteSpace(buffer[cCount])) {
				buffer[cCount] = 0;
				state = 0;
			}
		}

		if (pCount >= MAX_COMMAND_PARAMS) {
			return (-1);
		}
	}

	return (pCount);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void ParseCommand(char *)
 *  This private function executes matching functions.
 * .KB_C_FN_DEFINITION_END
 */
static void ParseCommand(char *buffer) {
	int			argc;
	parse_function_t	*this_parse_entry = parse_list_head;

	if ((argc = BreakCommand(buffer)) < 1)
		return ;

	while ((this_parse_entry) &&
		(p_strcmp(argv[0], this_parse_entry->f_string))) {
		this_parse_entry = this_parse_entry->next;
	}

	if (!this_parse_entry)
		return ;

	(this_parse_entry->parse_function)(argc, argv);

	DebugPrint("\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void ServicePrompt(char)
 *  This private function process each character checking for valid commands.
 * This function is only executed if the character is considered valid.
 * Each command is terminated with NULL (0) or '\r'.
 * .KB_C_FN_DEFINITION_END
 */
static void ServicePrompt(char p_char) {

	if (p_char == '\r') {
		p_char = 0;

	}

	if (p_char != 0x8) {
		if (buffCount < MAX_INPUT_SIZE-1) {

			inputBuffer[buffCount] = p_char;
			++buffCount;
			DebugPutc(p_char);
		}

	} else if (buffCount) {

		/* handle backspace BS */
		--buffCount;
		inputBuffer[buffCount] = 0;
		DebugPrint((char*)backspaceString);
		return ;
	}

	if (!p_char) {
		DebugPrint("\n\r");
		ParseCommand(inputBuffer);
		p_bzero(inputBuffer, MAX_INPUT_SIZE);
		buffCount = 0;
		DebugPrint("\n\r>");
	}
}


/* ************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * void InitPrompt(void)
 *  This global initializes the prompt-service system.
 * .KB_C_FN_DEFINITION_END
 */
void InitPrompt(void) {
	parse_list_head = 0;
	parse_list_tail = 0;
}


/*
 * .KB_C_FN_DEFINITION_START
 * int StringToCommand(char *cPtr)
 *  This private function converts a command string to a command code.
 * .KB_C_FN_DEFINITION_END
 */
void RegisterFunction(parse_function_t *newFunction) {

	newFunction->next = 0;

	if (!parse_list_head) {
		newFunction->prev = 0;
		parse_list_head = newFunction;
		parse_list_tail = newFunction;
	} else {
		newFunction->prev = parse_list_tail;
		parse_list_tail->next = newFunction;
		parse_list_tail = newFunction;
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * int help_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int help_function_parse(int argc, char *argv[]) {

	parse_function_t	*this_parse_entry = parse_list_head;

	if (nested)
		return(0);

	++nested;

	while (this_parse_entry) {
		if (this_parse_entry->help_function)
			(this_parse_entry->help_function)();
		this_parse_entry = this_parse_entry->next;
	}

	--nested;

	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void help_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void help_function_help(void) {
	DebugPrint("\tDisplay function usage information/help\n\r");
	DebugPrint("\t\t<h | help | ?>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void RAM_Monitor(void *inputFunction)
 *  This global function is the entry point for the RAM monitor.  If the
 * inputFunction pointer is NULL, the monitor input will be serviced from
 * the uart.  Otherwise, inputFunction is called to get characters which
 * the monitor will parse.
 * .KB_C_FN_DEFINITION_END
 */
void RAM_Monitor(int(*inputFunction)(char*)) {

	char	l_char;
	int	returnValue = 0;

	p_bzero((void*)inputBuffer, sizeof(inputBuffer));

	nested = 0;

	help1_function.f_string = "help";
	help1_function.parse_function = help_function_parse;
	help1_function.help_function = help_function_help;
	RegisterFunction(&help1_function);

	help2_function.f_string = "h";
	help2_function.parse_function = help_function_parse;
	help2_function.help_function = 0;
	RegisterFunction(&help2_function);

	help3_function.f_string = "?";
	help3_function.parse_function = help_function_parse;
	help3_function.help_function = 0;
	RegisterFunction(&help3_function);

	buffCount = 0;
	if (!inputFunction) {
		inputFunction = DebugGetchar;
	}

	DebugPrint("\n\r>");

	while (returnValue >= 0)
		if ((returnValue = ((*inputFunction)(&l_char))) > 0)
			ServicePrompt(l_char);
}
