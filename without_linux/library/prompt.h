/*******************************************************************************
 *
 * Filename: prompt.h
 *
 * Definition of the interactive functions.
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

#ifndef _PROMPT_H_
#define _PROMPT_H_

#define MAX_INPUT_SIZE		256
#define MAX_COMMAND_PARAMS	10

typedef struct parse_function_ {
	char			*f_string;
	int			(*parse_function)(int argc, char *argv[]);
	void			(*help_function)(void);
	struct parse_function_	*prev;
	struct parse_function_	*next;
} parse_function_t;

extern void RAM_Monitor(int(*inputFunction)(char*));
extern void RegisterFunction(parse_function_t*);
extern void InitPrompt(void);

#endif
