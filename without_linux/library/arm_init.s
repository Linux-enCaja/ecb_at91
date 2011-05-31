/*******************************************************************************
 *
 * Filename: arm_init.s
 *
 * Initialization for C-environment and basic operation.  Adapted from
 *  ATMEL cstartup.s.
 *
 * Revision information:
 *
 * 20AUG2004	kb_admin	initial creation
 * 20SEP2004	kb_admin	adapted for RAM monitor
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

	.equ	SVC_STACK_USE,		0x21C00000

	.extern	_edata

_begin_exec:
	b	_skip_data

_app_start:
	.word	_stext

edata:
	.word	_edata

_skip_data:

	adr	r0, _begin_exec
	ldr	r1, _app_start
	cmp     r0, r1
	beq     _skip_copy

	ldr	r2, _app_start
	ldr	r3, edata
	sub	r2, r3, r2
	add	r2, r0, r2

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop

	ldr	r1, _app_start
	mov	pc, r1

_skip_copy:

/* Start execution at main					*/

	ldr	r1, = SVC_STACK_USE
	mov	sp, r1		@ ; Move the stack to SDRAM

	.extern	main
_main:
__main:
	bl	main

/* main should not return.  If it does, try to jump back	*/

infiniteLoop:
	b	_main

