/*
 * Board specific setup info
 *
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Jian Zhang <jzhang@ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
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
#if defined(CONFIG_OMAP1710)
#include <./configs/omap1510.h>
#endif


_TEXT_BASE:
	.word	TEXT_BASE	/* sdram load addr from config.mk */

.globl platformsetup
platformsetup:


	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT1)                   *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT1
	ldr	r1,	VAL_ARM_IDLECT1
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT2)  		        *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT2
	ldr	r1,	VAL_ARM_IDLECT2
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT3)		        *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT3
	ldr	r1,	VAL_ARM_IDLECT3
	str	r1,	[r0]


	mov	r1,	#0x05		/* PER_EN bit */
	ldr	r0,	REG_ARM_RSTCT2
	strh	r1,	[r0]		/* CLKM; Peripheral reset. */

	/* Set CLKM to Sync-Scalable	*/
	/* I supposedly need to enable the dsp clock before switching */
	ldr	r1,	VAL_ARM_SYSST
	ldr	r0,	REG_ARM_SYSST
	strh	r1,	[r0]
	mov	r0,	#0x400
1:
	subs	r0,	r0,	#0x1	/* wait for any bubbles to finish */
	bne	1b
	ldr	r1,	VAL_ARM_CKCTL
	ldr	r0,	REG_ARM_CKCTL
	strh	r1,	[r0]

	/* a few nops to let settle */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	/* setup DPLL 1 */
	/* Ramp up the clock to 96Mhz */
	ldr	r1,	VAL_DPLL1_CTL
	ldr	r0,	REG_DPLL1_CTL
	strh	r1,	[r0]
	ands	r1,	r1,	#0x10	/* Check if PLL is enabled. */
	beq	lock_end	/* Do not look for lock if BYPASS selected */
2:
	ldrh	r1,	[r0]
	ands	r1,	r1,	#0x01	/*	Check the LOCK bit.*/
	beq 2b			/*	loop until bit goes hi. */
lock_end:


	/*------------------------------------------------------*
	 * Turn off the watchdog during init...			*
 	 *------------------------------------------------------*/
	ldr	r0,	REG_WATCHDOG
	ldr	r1,	WATCHDOG_VAL1
	str	r1,	[r0]
	ldr	r1,	WATCHDOG_VAL2
	str	r1,	[r0]
	ldr	r0,	REG_WSPRDOG
	ldr	r1,	WSPRDOG_VAL1
	str	r1,	[r0]
	ldr	r0,	REG_WWPSDOG

watch1Wait:
	ldr	r1,	[r0]
	tst	r1,	#0x10
	bne	watch1Wait

	ldr	r0,	REG_WSPRDOG
	ldr	r1,	WSPRDOG_VAL2
	str	r1,	[r0]
	ldr	r0,	REG_WWPSDOG
watch2Wait:
	ldr	r1,	[r0]
	tst	r1,	#0x10
	bne	watch2Wait


	/* Set memory timings corresponding to the new clock speed */
 	 
	/*
	* Delay for SDRAM initialization.
	*/
	mov	r3,	#0x1800		/* value should be checked */
3:
	subs	r3,	r3,	#0x1	/* Decrement count */
	bne 	3b


	/*
	 * Set SDRAM control values. Disable refresh before MRS command.
	 */

	/* mobile ddr operation */
	ldr	r0,	REG_SDRAM_OPERATION
	mov	r2,	#07
	str	r2,	[r0]

	/* config register */
	ldr	r0,	REG_SDRAM_CONFIG
	ldr	r1,	SDRAM_CONFIG_VAL
	str	r1,	[r0]

	/* manual command register */
	ldr	r0,	REG_SDRAM_MANUAL_CMD
	/* issue set cke high */
	mov	r1,	#CMD_SDRAM_CKE_SET_HIGH
	str	r1,	[r0]
	/* issue nop */
	mov	r1,	#CMD_SDRAM_NOP
	str	r1,	[r0]

	mov	r2,	#0x0100
waitMDDR1:
	subs	r2,	r2,	 #1
	bne	waitMDDR1	/* delay loop */

	/* issue precharge */
	mov	r1,	#CMD_SDRAM_PRECHARGE
	str	r1,	[r0]


	/* issue autorefresh x 2 */
	mov	r1,	#CMD_SDRAM_AUTOREFRESH
	str	r1,	[r0]
	str	r1,	[r0]

	/* mrs register ddr mobile */
	ldr	r0,	REG_SDRAM_MRS
	mov	r1,	#0x33
	str	r1,	[r0]

	/* emrs1 low-power register */
	ldr	r0,	REG_SDRAM_EMRS1
	/* self refresh on all banks */
	mov	r1,	#0
	str	r1,	[r0]

	ldr	r0,	REG_DLL_URD_CONTROL
	ldr	r1,	DLL_URD_CONTROL_VAL
	str	r1,	[r0]

	ldr	r0,	REG_DLL_LRD_CONTROL
	ldr	r1,	DLL_LRD_CONTROL_VAL
	str	r1,	[r0]

	ldr	r0,	REG_DLL_WRT_CONTROL
	ldr	r1,	DLL_WRT_CONTROL_VAL
	str	r1,	[r0]

	/* delay loop */
	mov	r2,	#0x0100
waitMDDR2:
	subs	r2,	r2,	#1
	bne	waitMDDR2

	/*
	 * Delay for SDRAM initialization.
	 */
	mov	r3,	#0x1800
4:
	subs	r3,	r3,	#1	/* Decrement count. */
	bne	4b
	b	common_tc

skip_sdram:

	ldr	r0,	REG_SDRAM_CONFIG
	ldr	r1,	SDRAM_CONFIG_VAL
	str	r1,	[r0]

	/* Enable EMIFF TC Doubler in OMAP1710 */
	ldr	r0,	REG_EMIFF_DOUBLER
	mov	r0,	#0x1;

common_tc:
	/* slow interface */
	ldr	r1,	VAL_TC_EMIFS_CONFIG
	ldr	r0,	REG_TC_EMIFS_CONFIG
	str	r1,	[r0]

#ifdef CFG_BOOT_CS0
	/* Chip Select 3 for NAND*/
	ldr	r1,	VAL_TC_EMIFS_CS3_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS3_CONFIG
	str	r1,	[r0] 
#else
	/* Chip Select 2 for NAND*/
	ldr	r1,	VAL_TC_EMIFS_CS2_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS2_CONFIG
	str	r1,	[r0] 
#endif
  
        /* Start MPU Timer 1 */
        ldr     r0,     REG_MPU_LOAD_TIMER
        ldr     r1,     VAL_MPU_LOAD_TIMER
        str     r1,     [r0]

        ldr     r0,     REG_MPU_CNTL_TIMER
        ldr     r1,     VAL_MPU_CNTL_TIMER
        str     r1,     [r0]

	/* back to arch calling code */
	mov	pc,	lr

	/* the literal pools origin */
	.ltorg


REG_TC_EMIFS_CONFIG:		/* 32 bits */
	.word 0xfffecc0c
#ifdef CFG_BOOT_CS0
REG_TC_EMIFS_CS3_CONFIG:	/* 32 bits */
	.word 0xfffecc1c
#else
REG_TC_EMIFS_CS2_CONFIG:	/* 32 bits */
	.word 0xfffecc18
#endif
 
/* MPU clock/reset/power mode control registers */
REG_ARM_CKCTL:			/* 16 bits */
	.word 0xfffece00

REG_ARM_IDLECT3:		/* 16 bits */
	.word 0xfffece24
REG_ARM_IDLECT2:		/* 16 bits */
	.word 0xfffece08
REG_ARM_IDLECT1:		/* 16 bits */
	.word 0xfffece04

REG_ARM_RSTCT2:			/* 16 bits */
	.word 0xfffece14
REG_ARM_SYSST:			/* 16 bits */
	.word 0xfffece18
/* DPLL control registers */
REG_DPLL1_CTL:			/* 16 bits */
	.word 0xfffecf00

/* Watch Dog register */
/* secure watchdog stop */
REG_WSPRDOG:
	.word 0xfffeb048
/* watchdog write pending */
REG_WWPSDOG:
	.word 0xfffeb034

WSPRDOG_VAL1:
	.word 0x0000aaaa
WSPRDOG_VAL2:
	.word 0x00005555

/* SDRAM config is: auto refresh enabled, 16 bit 4 bank,
 counter @8192 rows, 10 ns, 8 burst */
REG_SDRAM_CONFIG:
	.word 0xfffecc20

/* Operation register */
REG_SDRAM_OPERATION:
	.word 0xfffecc80

REG_EMIFF_DOUBLER:
	.word 0xfffecc60

/* Manual command register */
REG_SDRAM_MANUAL_CMD:
	.word 0xfffecc84

/* SDRAM MRS (New) config is: CAS latency is 2, burst length 8 */
REG_SDRAM_MRS:
	.word 0xfffecc70

/* SDRAM MRS (New) config is: CAS latency is 2, burst length 8 */
REG_SDRAM_EMRS1:
	.word 0xfffecc78

/* WRT DLL register */
REG_DLL_WRT_CONTROL:
	.word 0xfffecc64
DLL_WRT_CONTROL_VAL:
	.word 0x03500002

/* URD DLL register */
REG_DLL_URD_CONTROL:
	.word 0xfffeccc0
DLL_URD_CONTROL_VAL:
	.word 0x00000006

/* LRD DLL register */
REG_DLL_LRD_CONTROL:
	.word 0xfffecccc

REG_WATCHDOG:
	.word 0xfffec808

REG_MPU_LOAD_TIMER:
        .word 0xfffec600
REG_MPU_CNTL_TIMER:
        .word 0xfffec500

/* 96 MHz Samsung Mobile DDR */
SDRAM_CONFIG_VAL:
	.word 0x0c028af4

DLL_LRD_CONTROL_VAL:
	.word 0x00000006

VAL_ARM_CKCTL:
	.word 0x350e
VAL_ARM_SYSST:
	.word 0x1001

VAL_DPLL1_CTL:
	.word 0x2810

#ifdef CFG_BOOT_CS0
VAL_TC_EMIFS_CONFIG:
	.word 0x00000010 
VAL_TC_EMIFS_CS3_CONFIG:	
 	.word 0xff80fff3   
#else
VAL_TC_EMIFS_CONFIG:
	.word 0x00000012   /*swap CS0/CS3 addressing */
VAL_TC_EMIFS_CS2_CONFIG:	
 	.word 0xff80fff3   
#endif

 
VAL_TC_EMIFF_SDRAM_CONFIG:
	.word 0x010290fc
VAL_TC_EMIFF_MRS:
	.word 0x00000027

VAL_ARM_IDLECT1:
	.word 0x000014c6

VAL_ARM_IDLECT2:
	.word 0x000009ff
VAL_ARM_IDLECT3:
	.word 0x0000003f

WATCHDOG_VAL1:
	.word 0x000000f5
WATCHDOG_VAL2:
	.word 0x000000a0

VAL_MPU_LOAD_TIMER:
        .word 0xffffffff
VAL_MPU_CNTL_TIMER:
        .word 0xffffffa1

/* command values */
.equ CMD_SDRAM_NOP,		0x00000000
.equ CMD_SDRAM_PRECHARGE,	0x00000001
.equ CMD_SDRAM_AUTOREFRESH,	0x00000002
.equ CMD_SDRAM_CKE_SET_HIGH,	0x00000007
