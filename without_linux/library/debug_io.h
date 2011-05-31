/*******************************************************************************
 *
 * Filename: debug_io.h
 *
 * Definitions for basic debug uart support.
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

#ifndef _DEBUG_IO_H_
#define _DEBUG_IO_H_

extern void DebugPutc(char cValue);
extern void DebugPrint(char *buffer);
extern int DebugGetchar(char *retChar);
extern int WaitForChar(char *cPtr, int seconds);
extern void DebugPrintHex(int digits, int value);
extern void DebugPrintValue(int value);
extern void ReadMem(unsigned address, unsigned length, unsigned size);
extern void WriteMem(unsigned address, unsigned size, unsigned value);

#endif
