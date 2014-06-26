/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (c) 2004	Jian Zhang <jzhang@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <config.h>
#if defined(CONFIG_OMAP1610)
#include <./configs/omap1510.h>
#endif

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start:
	b	reset
 	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang

_hang:
 	.word do_hang
  
	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

 
/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0
  
	/*
	 * turn off the watchdog, unlock/diable sequence
	 */
	mov	r1, #0xF5
	ldr	r0, =WDTIM_MODE
	strh	r1, [r0]
	mov	r1, #0xA0
	strh	r1, [r0]


	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */

	mov	r1, #0xffffffff
	ldr	r0, =REG_IHL1_MIR
	str	r1, [r0]
	ldr	r0, =REG_IHL2_MIR
	str	r1, [r0]
 
	/*
	 * we do sys-critical inits at reboot,
 	 */
 	bl	cpu_init_crit 
  
	/* 
	 * relocate exception vectors to SRAM where ROM code expects
	 */
#ifdef CFG_BOOT_CS0

	adr	r0, _start		/* r0 <- current position of code   */
 	add     r0, r0, #4	/* skip reset vector			*/
	mov	r2, #36		/* r2 <- size of data (8+1 words)            */
	add	r2, r0, r2		/* r2 <- source end address         */
	mov r1, #0x20000000
next:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	ble	next

#endif
 
relocate:				/* relocate X-Loader to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	/*beq     stack_setup*/

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	ble	copy_loop

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated X-Loader   */
  	sub	sp, r0, #128		/* leave 32 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	add	r0, r0, #4		/* start at first byte of bss       */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

 	ldr	pc, _start_armboot

_start_armboot:
	.word  start_armboot


/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */


cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	/* clear bits 13, 9:8 (--V- --RS) */
	bic	r0, r0, #0x00000087	/* clear bits 7, 2:0 (B--- -CAM) */
	orr	r0, r0, #0x00000002	/* set bit 2 (A) Align */
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	mov	ip, lr		/* perserve link reg across call */
	bl	platformsetup	/* go setup pll,mux,memory */
	mov	lr, ip		/* restore link */
	mov	pc, lr		/* back to my caller */
  
/*
 * exception handler
 */
 	.align  5
do_hang:
	ldr	sp, _TEXT_BASE		/* use 32 words abort stack */
   	bl	hang				/* hang and never return */


