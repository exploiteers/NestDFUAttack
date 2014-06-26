/*
/* linux/arch/arm/mach-omap2/sleep3517.S
 *
 * AM3505/3517 Sleep Code.
 * Ranjith Lohithakshan <ranjithl@ti.com>
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <plat/sram.h>
#include <mach/io.h>

#include "cm2xxx_3xxx.h"
#include "prm2xxx_3xxx.h"
#include "sdrc.h"
#include "control.h"

#define CM_IDLEST1_CORE_V OMAP34XX_CM_REGADDR(CORE_MOD, CM_IDLEST1)
#define CM_CLKST_CORE_V   OMAP34XX_CM_REGADDR(CORE_MOD, OMAP3430_CM_CLKSTST)
#define CM_ICLKEN1_CORE_V OMAP34XX_CM_REGADDR(CORE_MOD, CM_ICLKEN1)

#define EMIF_PM_CTR_V           OMAP2_L3_IO_ADDRESS(0x6D000038)
#define OMAP3517_CONF1_REG_V    OMAP2_L4_IO_ADDRESS(0x48002584)

/*
 * Forces OMAP into idle state
 *
 * omap34xx_suspend() - This bit of code just executes the WFI
 * for normal idles.
 *
 * Note: This code get's copied to internal SRAM at boot. When the OMAP
 *	 wakes up it continues execution at the point it went to sleep.
 */
ENTRY(omap3517_cpu_suspend)
	stmfd	sp!, {r0-r12, lr}		@ save registers on stack
loop:
	/*b	loop*/	@Enable to debug by stepping through code

	/* Put EMIF in self-refresh */
	ldr     r4, emif_pm_ctrl
	ldr     r5, [r4]
	orr     r5, r5, #0x200
	str     r5, [r4]

	/* Disable SDRC and Control Module */
	ldr     r4, cm_iclken1_core
	ldr     r5, [r4]
	str     r5, iclk_core_enable
	ldr     r4, cm_iclken1_core
	ldr     r5, clk_core_disable
	str     r5, [r4]
wait_sdrc_ok:
	ldr     r4, cm_idlest1_core
	ldr     r5, [r4]
	and     r5, r5, #0x2
	cmp     r5, #0x2
	bne     wait_sdrc_ok

	/* Gate DDR Phy clock */
	ldr     r4, omap3517_conf1
	ldr     r5, emif_phy_gate
	str     r5, [r4]

	/* Data memory barrier and Data sync barrier */
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4
	mcr	p15, 0, r1, c7, c10, 5

	wfi

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

	/* Enable SDRC and Control Module */
	ldr     r4, cm_iclken1_core
	ldr     r5, iclk_core_enable
	str     r5, [r4]

	/* Enable DDR Phy Clock */
	ldr     r4, omap3517_conf1
	ldr     r5, emif_phy_enable
	str     r5, [r4]

	/* Take EMIF out of self-refresh */
	ldr     r4, emif_pm_ctrl
	ldr     r5, [r4]
	bic     r5, r5, #0x200
	str     r5, [r4]

	ldmfd	sp!, {r0-r12, pc}		@ restore regs and return

clk_core_disable:
	.word   0x0
iclk_core_enable:
	.word   0x0
emif_phy_gate:
	.word   0x2620
emif_phy_enable:
	.word   0x8620
cm_idlest1_core:
	.word   CM_IDLEST1_CORE_V
cm_clkst_core:
	.word   CM_CLKST_CORE_V
emif_pm_ctrl:
	.word   EMIF_PM_CTR_V
cm_iclken1_core:
	.word   CM_ICLKEN1_CORE_V
omap3517_conf1:
	.word   OMAP3517_CONF1_REG_V
ENTRY(omap3517_cpu_suspend_sz)
	.word	. - omap3517_cpu_suspend

/* Function to call rom code to save secure ram context */
ENTRY(omap3517_save_secure_ram_context)
	stmfd   sp!, {r1-r12, lr}   @ save registers on stack
save_secure_ram_debug:
	/* b save_secure_ram_debug */   @ enable to debug save code
	ldmfd   sp!, {r1-r12, pc}
ENTRY(omap3517_save_secure_ram_context_sz)
	.word   . - omap3517_save_secure_ram_context

/* Function call to get the restore pointer for resume from OFF */
ENTRY(omap3517_get_restore_pointer)
	stmfd   sp!, {lr}     @ save registers on stack
	ldmfd   sp!, {pc}     @ restore regs and return
ENTRY(omap3517_get_restore_pointer_sz)
	.word   . - omap3517_get_restore_pointer

