/*******************************************************************************
 *
 * Filename: flash.h
 *
 * Instantiation of flash control routines supporting Intel/Micron StrataFlash
 *
 * Revision information:
 *
 * 20SEP2004	kb_admin	initial creation
 * 01MAY2005	kb_admin	changed to support KB9202 flash device
 * 14JUN2005	kb_admin	updated this block to reflect previous source change
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

#ifndef _FLASH_H_
#define _FLASH_H_

#define FLASH_BASE		0x10000000
// #define FLASH_SIZE		(4 * 1024 * 1024)
#define FLASH_SIZE		(16 * 1024 * 1024)
#define FLASH_MAX		(FLASH_BASE + FLASH_SIZE)
#define FLASH_SECTOR_SIZE	(128 * 1024)
#define FLASH_SECTORS		(FLASH_SIZE/FLASH_SECTOR_SIZE)

extern void EraseFlashSector(unsigned start, unsigned end);
extern void EraseFlashChip(void);
extern void ProgramFlash(unsigned target, unsigned source, unsigned size);
extern void InitFlash(void);

#endif
