/*******************************************************************************
 *
 * Filename: xmodem.h
 *
 * Definition of simple x-modem support using debug uart channel.
 *
 * Revision information:
 *
 * 22AUG2004	kb_admin	initial creation
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

#ifndef _XMODEM_H_
#define _XMODEM_H_

#define	LOCAL_BUFFER_SIZE	(4 * 1024 * 1024)
#define LOCAL_BUFFER_TEXT	"4MB"

extern int xmodem_rx(char *dest, int report);

#endif
