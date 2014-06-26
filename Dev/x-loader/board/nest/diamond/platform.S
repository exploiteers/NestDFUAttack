/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *    (C) Copyright 2004-2006
 *    Texas Instruments, <www.ti.com>
 *    Richard Woodruff <r-woodruff2@ti.com>
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *      This file is the board-specific setup for the Nest Learning
 *      Thermostat board.
 *
 *      It inherits entirely from the equivalent TI OMAP3 EVM
 *	file.
 */

#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>
#include <asm/arch/clocks.h>

_TEXT_BASE:
	.word	TEXT_BASE	/* sdram load addr from config.mk */

#if !defined(CFG_NAND_BOOT) && !defined(CFG_NAND_BOOT)
/**************************************************************************
 * cpy_clk_code: relocates clock code into SRAM where its safer to execute
 * R1 = SRAM destination address.
 *************************************************************************/
.global cpy_clk_code
 cpy_clk_code:
        /* Copy DPLL code into SRAM */
        adr     r0, go_to_speed         /* get addr of clock setting code */
        mov     r2, #384                /* r2 size to copy (div by 32 bytes) */
        mov     r1, r1                  /* r1 <- dest address (passed in) */
        add     r2, r2, r0              /* r2 <- source end address */
next2:
        ldmia   r0!, {r3-r10}           /* copy from source address [r0]    */
        stmia   r1!, {r3-r10}           /* copy to   target address [r1]    */
        cmp     r0, r2                  /* until source end address [r2]    */
        bne     next2
	mov	pc, lr                  /* back to caller */

/* ****************************************************************************
 * NOTE: 3430 X-loader currently does not use this code.
*   It could be removed its is kept for compatabily with u-boot.
 *
 *  go_to_speed: -Moves to bypass, -Commits clock dividers, -puts dpll at speed
 *               -executed from SRAM.
 *  R0 = CM_CLKEN_PLL-bypass value
 *  R1 = CM_CLKSEL1_PLL-m, n, and divider values
 *  R2 = CM_CLKSEL_CORE-divider values
 *  R3 = CM_IDLEST_CKGEN - addr dpll lock wait
 *
 *  Note: If core unlocks/relocks and SDRAM is running fast already it gets
 *        confused.  A reset of the controller gets it back.  Taking away its
 *        L3 when its not in self refresh seems bad for it.  Normally, this code
 *        runs from flash before SDR is init so that should be ok.
 ******************************************************************************/
.global go_to_speed
 go_to_speed:
        stmfd sp!, {r4-r6}

        /* move into fast relock bypass */
        ldr     r4, pll_ctl_add
        str     r0, [r4]
wait1:
        ldr     r5, [r3]       /* get status */
        and     r5, r5, #0x1   /* isolate core status */
        cmp     r5, #0x1       /* still locked? */
        beq     wait1          /* if lock, loop */

	/* set new dpll dividers _after_ in bypass */
	ldr     r5, pll_div_add1
        str     r1, [r5]          /* set m, n, m2 */
        ldr     r5, pll_div_add2
        str     r2, [r5]          /* set l3/l4/.. dividers*/
        ldr     r5, pll_div_add3  /* wkup */
        ldr     r2, pll_div_val3  /* rsm val */
        str     r2, [r5]
        ldr     r5, pll_div_add4  /* gfx */
        ldr     r2, pll_div_val4
        str     r2, [r5]
        ldr     r5, pll_div_add5  /* emu */
        ldr     r2, pll_div_val5
        str     r2, [r5]

        /* now prepare GPMC (flash) for new dpll speed */
	/* flash needs to be stable when we jump back to it */
        ldr     r5, flash_cfg3_addr
        ldr     r2, flash_cfg3_val
        str     r2, [r5]
        ldr     r5, flash_cfg4_addr
        ldr     r2, flash_cfg4_val
        str     r2, [r5]
        ldr     r5, flash_cfg5_addr
        ldr     r2, flash_cfg5_val
        str     r2, [r5]
        ldr     r5, flash_cfg1_addr
        ldr     r2, [r5]
        orr     r2, r2, #0x3     /* up gpmc divider */
        str     r2, [r5]

        /* lock DPLL3 and wait a bit */
        orr     r0, r0, #0x7   /* set up for lock mode */
        str     r0, [r4]       /* lock */
        nop                    /* ARM slow at this point working at sys_clk */
        nop
        nop
        nop
wait2:
        ldr     r5, [r3]       /* get status */
        and     r5, r5, #0x1   /* isolate core status */
        cmp     r5, #0x1       /* still locked? */
        bne     wait2          /* if lock, loop */
        nop
        nop
        nop
        nop
        ldmfd sp!, {r4-r6}
        mov     pc, lr           /* back to caller, locked */

_go_to_speed: .word go_to_speed

/* these constants need to be close for PIC code */
/* The Nor has to be in the Flash Base CS0 for this condition to happen */
flash_cfg1_addr:
    .word (GPMC_CONFIG1_0)
flash_cfg3_addr:
    .word  (GPMC_CONFIG3_0)
flash_cfg3_val:
    .word  STNOR_GPMC_CONFIG3
flash_cfg4_addr:
    .word (GPMC_CONFIG4_0)
flash_cfg4_val:
    .word  STNOR_GPMC_CONFIG4
flash_cfg5_val:
    .word  STNOR_GPMC_CONFIG5
flash_cfg5_addr:
    .word (GPMC_CONFIG5_0)
pll_ctl_add:
    .word CM_CLKEN_PLL
pll_div_add1:
    .word CM_CLKSEL1_PLL
pll_div_add2:
    .word CM_CLKSEL_CORE
pll_div_add3:
    .word CM_CLKSEL_WKUP
pll_div_val3:
    .word (WKUP_RSM << 1)
pll_div_add4:
    .word CM_CLKSEL_GFX
pll_div_val4:
    .word (GFX_DIV << 0)
pll_div_add5:
    .word CM_CLKSEL1_EMU
pll_div_val5:
    .word CLSEL1_EMU_VAL
#endif /* !defined(CFG_NAND_BOOT) && !defined(CFG_NAND_BOOT) */

.globl lowlevel_init
lowlevel_init:
	ldr	sp,	SRAM_STACK
        str     ip,	[sp]    /* stash old link register */
	mov	ip,	lr	/* save link reg across call */
        bl      s_init          /* go setup pll,mux,memory */
        ldr     ip,	[sp]    /* restore save ip */
	mov	lr,	ip	/* restore link reg */

	/* back to arch calling code */
	mov	pc,	lr

	/* the literal pools origin */
	.ltorg

REG_CONTROL_STATUS:
	.word CONTROL_STATUS
SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK


/* DPLL(1-4) PARAM TABLES */
/* Each of the tables has M, N, FREQSEL, M2 values defined for nominal
 * OPP (1.2V). The fields are defined according to dpll_param struct(clock.c).
 * The values are defined for all possible sysclk and for ES1 and ES2.
 */

mpu_dpll_param:
/* 12MHz */
/* ES1 */
.word 0x0FE
.word 0x07
.word 0x05
.word 0x01
/* ES2 */
.word 0x0FA
.word 0x05
.word 0x07
.word 0x01

/* 13MHz */
/* ES1 */
.word 0x17D
.word 0x0C
.word 0x03
.word 0x01
/* ES2 */
.word 0x1F4
.word 0x0C
.word 0x03
.word 0x01

/* 19.2MHz */
/* ES1 */
.word 0x179
.word 0x12
.word 0x04
.word 0x01
/* ES2 */
.word 0x271
.word 0x17
.word 0x03
.word 0x01

/* 26MHz */
/* ES1 */
.word 0x17D
.word 0x19
.word 0x03
.word 0x01
/* ES2 */
.word 0x0FA
.word 0x0C
.word 0x07
.word 0x01

/* 38.4MHz */
/* ES1 */
.word 0x1FA
.word 0x32
.word 0x03
.word 0x01
/* ES2 */
.word 0x271
.word 0x2F
.word 0x03
.word 0x01


.globl get_mpu_dpll_param
get_mpu_dpll_param:
	adr r0, mpu_dpll_param
	mov pc, lr

iva_dpll_param:
/* 12MHz */
/* ES1 */
.word 0x07D
.word 0x05
.word 0x07
.word 0x01
/* ES2 */
.word 0x0B4
.word 0x05
.word 0x07
.word 0x01

/* 13MHz */
/* ES1 */
.word 0x0FA
.word 0x0C
.word 0x03
.word 0x01
/* ES2 */
.word 0x168
.word 0x0C
.word 0x03
.word 0x01

/* 19.2MHz */
/* ES1 */
.word 0x082
.word 0x09
.word 0x07
.word 0x01
/* ES2 */
.word 0x0E1
.word 0x0B
.word 0x06
.word 0x01

/* 26MHz */
/* ES1 */
.word 0x07D
.word 0x0C
.word 0x07
.word 0x01
/* ES2 */
.word 0x0B4
.word 0x0C
.word 0x07
.word 0x01

/* 38.4MHz */
/* ES1 */
.word 0x13F
.word 0x30
.word 0x03
.word 0x01
/* ES2 */
.word 0x0E1
.word 0x17
.word 0x06
.word 0x01


.globl get_iva_dpll_param
get_iva_dpll_param:
	adr r0, iva_dpll_param
	mov pc, lr

core_dpll_param:
/* 12MHz */
/* ES1 */
.word 0x19F
.word 0x0E
.word 0x03
.word 0x01
/* ES2 */
.word 0x0A6
.word 0x05
.word 0x07
.word 0x01

/* 13MHz */
/* ES1 */
.word 0x1B2
.word 0x10
.word 0x03
.word 0x01
/* ES2 */
.word 0x14C
.word 0x0C
.word 0x03
.word 0x01

/* 19.2MHz */
/* ES1 */
.word 0x19F
.word 0x17
.word 0x03
.word 0x01
/* ES2 */
.word 0x19F
.word 0x17
.word 0x03
.word 0x01

/* 26MHz */
/* ES1 */
.word 0x1B2
.word 0x21
.word 0x03
.word 0x01
/* ES2 */
.word 0x0A6
.word 0x0C
.word 0x07
.word 0x01

/* 38.4MHz */
/* ES1 */
.word 0x19F
.word 0x2F
.word 0x03
.word 0x01
/* ES2 */
.word 0x19F
.word 0x2F
.word 0x03
.word 0x01

.globl get_core_dpll_param
get_core_dpll_param:
	adr r0, core_dpll_param
	mov pc, lr

/* PER DPLL values are same for both ES1 and ES2 */
per_dpll_param:
/* 12MHz */
.word 0xD8
.word 0x05
.word 0x07
.word 0x09

/* 13MHz */
.word 0x1B0
.word 0x0C
.word 0x03
.word 0x09

/* 19.2MHz */
.word 0xE1
.word 0x09
.word 0x07
.word 0x09

/* 26MHz */
.word 0xD8
.word 0x0C
.word 0x07
.word 0x09

/* 38.4MHz */
.word 0xE1
.word 0x13
.word 0x07
.word 0x09

.globl get_per_dpll_param
get_per_dpll_param:
	adr r0, per_dpll_param
	mov pc, lr

/*
 * Tables for 36x/37x devices
 *
 * (Populated for 13MHz only)
 */
mpu_36x_dpll_param:
//    M    N   FS M2
#if defined(CONFIG_SYS_MPU_DPLL_300MHZ)
.word 150,  5, 0, 1 	// 12   MHz
.word 300, 12, 0, 1 	// 13   MHz
.word 125,  7, 0, 1 	// 19.2 MHz
.word 150, 12, 0, 1 	// 26   MHz
.word 125, 15, 0, 1	// 38.4 MHz
.word   0,  0, 0, 1	// 16.8 MHz - TBD
#elif defined(CONFIG_SYS_MPU_DPLL_600MHZ)
.word 300,  5, 0, 1 	// 12   MHz
.word 600, 12, 0, 1 	// 13   MHz
.word 125,  3, 0, 1 	// 19.2 MHz
.word 300, 12, 0, 1 	// 26   MHz
.word 125,  7, 0, 1 	// 38.4 MHz
.word   0,  0, 0, 1	// 16.8 MHz - TBD
#else
# error "MPU DPLL settings are not defined!"
#endif /* defined(CONFIG_SYS_MPU_DPLL_600MHZ) */

iva_36x_dpll_param:
//     M   N   FS M2
.word  10,  0, 0, 1	// 12   MHz
.word  10,  0, 0, 1	// 13   MHz
.word  10,  0, 0, 1	// 19.2 MHz
.word  10,  0, 0, 1	// 26   MHz
.word  10,  0, 0, 1	// 38.4 MHz
.word  10,  0, 0, 1	// 16.8 MHz

core_36x_dpll_param:
//    M    N   FS M2
#if defined(CONFIG_SYS_CORE_DPLL_200MHZ)
.word 100,  5, 0, 1	// 12   MHz
.word 200, 12, 0, 1	// 13   MHz
.word 375, 35, 0, 1	// 19.2 MHz
.word 100, 12, 0, 1	// 26   MHz
.word 375, 71, 0, 1	// 38.4 MHz
.word   0,  0, 0, 1	// 16.8 MHz - TBD
#elif defined(CONFIG_SYS_CORE_DPLL_332MHZ)
.word 166,  5, 0, 1	// 12   MHz
.word 332, 12, 0, 1	// 13   MHz
.word 415, 23, 0, 1	// 19.2 MHz
.word 166, 12, 0, 1	// 26   MHz
.word 415, 47, 0, 1	// 38.4 MHz
.word   0,  0, 0, 1	// 16.8 MHz - TBD
#elif defined(CONFIG_SYS_CORE_DPLL_400MHZ)
.word 200,  5, 0, 1	// 12   MHz
.word 400, 12, 0, 1	// 13   MHz
.word 375, 17, 0, 1	// 19.2 MHz
.word 200, 12, 0, 1	// 26   MHz
.word 375, 35, 0, 1	// 38.4 MHz
.word   0,  0, 0, 1	// 16.8 MHz - TBD
#else
# error "Core DPLL settings are not defined!"
#endif /* defined(CONFIG_SYS_CORE_DPLL_400MHZ) */

/*
 * For the peripheral (PER) (aka DPLL4) clock settings, there are only
 * effectively two clock choices, 96 MHz or 192 MHz. However, the only
 * time 192 MHz is apt to be used in an application is if both SGX and
 * TV output are used.
 *
 *   For any given system clock, the dividers are configured thus (per
 *   Section 3.5.3.3.3.2 "Type B DPLL (Low-Jitter)" of the OMAP3 TRM):
 *
 *     PER[clkout] = (SYS[clkout] * M) / (N + 1)
 *
 *       M2[clkout]: PER[clkout] / M2           used by UART, MMC, I2C, etc.
 *       M3[clkout]: PER[clkout] / M3           used by TV out.
 *       M4[clkout]: PER[clkout] / M4           used by DSS.
 *       M5[clkout]: PER[clkout] / M5           used by camera.
 *       M6[clkout]: PER[clkout] / M6           used by emulation.
 *
 *  So, for a 19.2 MHz system clock and the scalar entries below:
 *
 *       PER[clkout] = (19.2 * 540) / (11 + 1) = 864
 *
 *       M2[clkout]: 864 /  9 =  96   MHz
 *       M3[clkout]: 864 / 16 =  54   MHz
 *       M4[clkout]: 864 /  5 = 172.8 MHz
 *       M5[clkout]: 864 /  4 = 216   MHz
 *       M6[clkout]: 864 /  3 = 288   MHz
 */
per_36x_dpll_param:
//    SYS_CLK    M      N      M2      M3      M4     M5      M6      m2DIV
.word 12000,    432,    5,     9,      16,     9,     4,      3,      1		// 12   MHz
.word 13000,    864,   12,     9,      16,     9,     4,      3,      1		// 13   MHz
.word 19200,    540,   11,     9,      16,     9,     4,      3,      1		// 19.2 MHz
.word 26000,    432,   12,     9,      16,     9,     4,      3,      1		// 26   MHz
.word 38400,    270,   11,     9,      16,     9,     4,      3,      1		// 38.4 MHz
.word 16800,    360,    6,     9,      16,     9,     4,      3,      1		// 16.8 MHz

.globl get_36x_mpu_dpll_param
get_36x_mpu_dpll_param:
	adr	r0, mpu_36x_dpll_param
	mov	pc, lr

.globl get_36x_iva_dpll_param
get_36x_iva_dpll_param:
	adr	r0, iva_36x_dpll_param
	mov	pc, lr

.globl get_36x_core_dpll_param
get_36x_core_dpll_param:
	adr	r0, core_36x_dpll_param
	mov	pc, lr

.globl get_36x_per_dpll_param
get_36x_per_dpll_param:
	adr	r0, per_36x_dpll_param
	mov	pc, lr
