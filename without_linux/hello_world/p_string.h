/*******************************************************************************
 *
 * Filename: p_string.h
 *
 * Definition of basic, private, string operations to prevent inclusion of full
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

#ifndef _P_STRING_H_
#define _P_STRING_H_

#define ToASCII(x) ((x > 9) ? (x + 'A' - 0xa) : (x + '0'))

extern int p_IsWhiteSpace(char cValue);
extern unsigned p_HexCharValue(char cValue);
extern void p_bzero(char *buffer, int size);
extern int p_strlen(char *buffer);
extern void p_ASCIIToHex(char *buf, unsigned *value);
extern void p_memcpy(char *to, char *from, unsigned size);

#endif /* _P_STRING_H_ */
