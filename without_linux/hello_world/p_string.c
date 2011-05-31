/*******************************************************************************
 *
 * Filename: p_string.c
 *
 * Instantiation of basic string operations to prevent inclusion of full
 * string library.  These are simple implementations not necessarily optimized
 * for speed, but rather to show intent.
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


/*
 * .KB_C_FN_DEFINITION_START
 * int p_IsWhiteSpace(char)
 *  This global function returns true if the character is not considered
 * a non-space character.
 * .KB_C_FN_DEFINITION_END
 */
int p_IsWhiteSpace(char cValue) {
	return ((cValue == ' ') ||
		(cValue == '\t') ||
		(cValue == 0) ||
		(cValue == '\r') ||
		(cValue == '\n'));
}


/*
 * .KB_C_FN_DEFINITION_START
 * unsigned p_HexCharValue(char)
 *  This global function returns the decimal value of the validated hex char.
 * .KB_C_FN_DEFINITION_END
 */
unsigned p_HexCharValue(char cValue) {
	if (cValue < ('9' + 1))
		return (cValue - '0');
	if (cValue < ('F' + 1))
		return (cValue - 'A' + 10);
	return (cValue - 'a' + 10);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void p_bzero(char *, int)
 *  This global function clears memory at the pointer for the specified
 * number of bytes.
 * .KB_C_FN_DEFINITION_END
 */
void p_bzero(char *buffer, int size) {

	while (size--)
		*buffer++ = 0;
}


/*
 * .KB_C_FN_DEFINITION_START
 * int p_strlen(char *)
 *  This global function returns the number of bytes starting at the pointer
 * before (not including) the string termination character ('/0').
 * .KB_C_FN_DEFINITION_END
 */
int p_strlen(char *buffer) {
	int	len = 0;
	if (buffer) {
		while (buffer[len]) len++;
	}
	return (len);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void p_ASCIIToHex(char *, unsigned *)
 *  This global function set the unsigned value equal to the converted
 * hex number passed as a string.  No error checking is performed; the
 * string must be valid hex value, point at the start of string, and be
 * NULL-terminated.
 * .KB_C_FN_DEFINITION_END
 */
void p_ASCIIToHex(char *buf, unsigned *value) {

	unsigned	lValue = 0;

	if ((*buf == '0') && ((buf[1] == 'x') || (buf[1] == 'X')))
		buf += 2;

	while (*buf) {

		lValue <<= 4;
		lValue += p_HexCharValue(*buf++);
	}

	*value = lValue;
}


/*
 * .KB_C_FN_DEFINITION_START
 * void p_memcpy(char *, char *, unsigned)
 *  This global function copies data from the first pointer to the second
 * pointer for the specified number of bytes.
 * .KB_C_FN_DEFINITION_END
 */
void p_memcpy(char *to, char *from, unsigned size) {
	while (size--)
		*to++ = *from++;
}
