/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
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
 *      This file is the board-specific set-up for the Nest Learning
 *      Thermostat board, based on the TI OMAP3 AM3703CUS, focusing
 *      primarily clock set-up for the processor's Power, Reset and
 *      Clock Manager (PRCM)
 *
 *      This is originally inherited and split from the OMAP3 EVM
 *      board file.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>

#include "platform.h"

/*
 *  u32 get_osc_clk_speed()
 *
 *  Description:
 *    This routine determines the reference oscillator speed based on
 *    a known 32 kHz clock and general purpose timer.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    The reference oscillator clock speed.
 *
 */
static u32
get_osc_clk_speed(void)
{
	u32 start, cstart, cend, cdiff, cdiv, val;

	val = __raw_readl(PRM_CLKSRC_CTRL);

	if (val & BIT7)
		cdiv = 2;
	else if (val & BIT6)
		cdiv = 1;
	else
		/*
		 * Should never reach here!
		 * TBD: Add a WARN()/BUG()
		 *      For now, assume divider as 1.
		 */
		cdiv = 1;

	/* enable timer2 */
	val = __raw_readl(CM_CLKSEL_WKUP) | BIT0;
	__raw_writel(val, CM_CLKSEL_WKUP);	/* select sys_clk for GPT1 */

	/* Enable I and F Clocks for GPT1 */
	val = __raw_readl(CM_ICLKEN_WKUP) | BIT0 | BIT2;
	__raw_writel(val, CM_ICLKEN_WKUP);
	val = __raw_readl(CM_FCLKEN_WKUP) | BIT0;
	__raw_writel(val, CM_FCLKEN_WKUP);

	__raw_writel(0, OMAP34XX_GPT1 + TLDR);	/* start counting at 0 */
	__raw_writel(GPT_EN, OMAP34XX_GPT1 + TCLR);     /* enable clock */
	/* enable 32kHz source *//* enabled out of reset */
	/* determine sys_clk via gauging */

	start = 20 + __raw_readl(S32K_CR);	/* start time in 20 cycles */
	while (__raw_readl(S32K_CR) < start);	/* dead loop till start time */
	cstart = __raw_readl(OMAP34XX_GPT1 + TCRR);	/* get start sys_clk count */
	while (__raw_readl(S32K_CR) < (start + 20));	/* wait for 40 cycles */
	cend = __raw_readl(OMAP34XX_GPT1 + TCRR);	/* get end sys_clk count */
	cdiff = cend - cstart;				/* get elapsed ticks */

	if (cdiv == 2)
	{
		cdiff *= 2;
	}

	/* based on number of ticks assign speed */
	if (cdiff > 19000)
		return (S38_4M);
	else if (cdiff > 15200)
		return (S26M);
	else if (cdiff > 13000)
		return (S24M);
	else if (cdiff > 9000)
		return (S19_2M);
	else if (cdiff > 7600)
		return (S13M);
	else
		return (S12M);
}

/*
 *  void get_sys_clkin_sel()
 *
 *  Description:
 *    This routine sets the value for the PRCM PRM system clock
 *    frequency selector, which should be written to PRM_CLKSEL.
 *
 *  Input(s):
 *    hertz  - The speed, in MHz, that the system clock is believed
 *               to be running at.
 *    sys_clkin_sel - A pointer to storage for the index used to access the
 *               digital PLL settings.
 *
 *  Output(s):
 *    sys_clkin_sel - A pointer to the index for the digital PLL settings
 *               appropriate for the specified system clock rate.
 *
 *  Returns:
 *    N/A
 *
 */
static void
get_sys_clkin_sel(u32 hertz, u32 *sys_clkin_sel)
{
	switch (hertz) {

	case S38_4M:
		*sys_clkin_sel = PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_38_4_MHZ;
		break;

	case S26M:
		*sys_clkin_sel = PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_26_0_MHZ;
		break;

	case S19_2M:
		*sys_clkin_sel = PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_19_2_MHZ;
		break;

	case S16_8M:
		*sys_clkin_sel = PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_16_8_MHZ;
		break;

	case S13M:
		*sys_clkin_sel = PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_13_0_MHZ;
		break;

	case S12M:
	default:
		*sys_clkin_sel = PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_12_0_MHZ;
		break;

	}
}

/*
 * OMAP34x/35x specific functions
 */
static void
dpll3_init_34xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	/* Getting the base address of Core DPLL param table*/
	ptr = (dpll_param *)get_core_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	ptr = ptr + 2*clk_index + sil_index;

	/* CORE DPLL */
	/* Select relock bypass: CM_CLKEN_PLL[0:2] */
	sr32(CM_CLKEN_PLL, 0, 3, PLL_FAST_RELOCK_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_CKGEN, LDELAY);

	/* CM_CLKSEL1_EMU[DIV_DPLL3] */
	sr32(CM_CLKSEL1_EMU, 16, 5, CORE_M3X2);

	/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
	sr32(CM_CLKSEL1_PLL, 27, 5, ptr->m2);

	/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
	sr32(CM_CLKSEL1_PLL, 16, 11, ptr->m);

	/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
	sr32(CM_CLKSEL1_PLL, 8, 7, ptr->n);

	/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
	sr32(CM_CLKSEL1_PLL, 6, 1, 0);

	sr32(CM_CLKSEL_CORE, 8, 4, CORE_SSI_DIV);	/* ssi */
	sr32(CM_CLKSEL_CORE, 4, 2, CORE_FUSB_DIV);	/* fsusb */
	sr32(CM_CLKSEL_CORE, 2, 2, CORE_L4_DIV);	/* l4 */
	sr32(CM_CLKSEL_CORE, 0, 2, CORE_L3_DIV);	/* l3 */

	sr32(CM_CLKSEL_GFX,  0, 3, GFX_DIV_34X);	/* gfx */
	sr32(CM_CLKSEL_WKUP, 1, 2, WKUP_RSM);		/* reset mgr */

	/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
	sr32(CM_CLKEN_PLL,   4, 4, ptr->fsel);
	sr32(CM_CLKEN_PLL,   0, 3, PLL_LOCK);		/* lock mode */

	wait_on_value(BIT0, 1, CM_IDLEST_CKGEN, LDELAY);
}

static void
dpll4_init_34xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	ptr = (dpll_param *)get_per_dpll_param();

	/* Moving it to the right sysclk base */
	ptr = ptr + clk_index;

	/* EN_PERIPH_DPLL: CM_CLKEN_PLL[16:18] */
	sr32(CM_CLKEN_PLL, 16, 3, PLL_STOP);
	wait_on_value(BIT1, 0, CM_IDLEST_CKGEN, LDELAY);

	sr32(CM_CLKSEL1_EMU, 24, 5, PER_M6X2);		/* set M6 */
	sr32(CM_CLKSEL_CAM, 0, 5, PER_M5X2);		/* set M5 */
	sr32(CM_CLKSEL_DSS, 0, 5, PER_M4X2);		/* set M4 */
	sr32(CM_CLKSEL_DSS, 8, 5, PER_M3X2);		/* set M3 */

	/* M2 (DIV_96M): CM_CLKSEL3_PLL[0:4] */
	sr32(CM_CLKSEL3_PLL, 0, 5, ptr->m2);

	/* M (PERIPH_DPLL_MULT): CM_CLKSEL2_PLL[8:18] */
	sr32(CM_CLKSEL2_PLL, 8, 11, ptr->m);

	/* N (PERIPH_DPLL_DIV): CM_CLKSEL2_PLL[0:6] */
	sr32(CM_CLKSEL2_PLL, 0, 7, ptr->n);

	/* FREQSEL (PERIPH_DPLL_FREQSEL): CM_CLKEN_PLL[20:23] */
	sr32(CM_CLKEN_PLL, 20, 4, ptr->fsel);

	/* LOCK MODE (EN_PERIPH_DPLL) : CM_CLKEN_PLL[16:18] */
	sr32(CM_CLKEN_PLL, 16, 3, PLL_LOCK);
	wait_on_value(BIT1, 2, CM_IDLEST_CKGEN, LDELAY);
}

static void
mpu_init_34xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	/* Getting the base address to MPU DPLL param table*/
	ptr = (dpll_param *)get_mpu_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	ptr = ptr + 2*clk_index + sil_index;

	/* MPU DPLL (unlocked already) */
	/* M2 (MPU_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_MPU[0:4] */
	sr32(CM_CLKSEL2_PLL_MPU, 0, 5, ptr->m2);

	/* M (MPU_DPLL_MULT) : CM_CLKSEL2_PLL_MPU[8:18] */
	sr32(CM_CLKSEL1_PLL_MPU, 8, 11, ptr->m);

	/* N (MPU_DPLL_DIV) : CM_CLKSEL2_PLL_MPU[0:6] */
	sr32(CM_CLKSEL1_PLL_MPU, 0, 7, ptr->n);

	/* FREQSEL (MPU_DPLL_FREQSEL) : CM_CLKEN_PLL_MPU[4:7] */
	sr32(CM_CLKEN_PLL_MPU, 4, 4, ptr->fsel);
}

static void
iva_init_34xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	/* Getting the base address to IVA DPLL param table*/
	ptr = (dpll_param *)get_iva_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	ptr = ptr + 2*clk_index + sil_index;

	/* IVA DPLL */
	/* EN_IVA2_DPLL : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(CM_CLKEN_PLL_IVA2, 0, 3, PLL_STOP);
	wait_on_value(BIT0, 0, CM_IDLEST_PLL_IVA2, LDELAY);

	/* M2 (IVA2_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_IVA2[0:4] */
	sr32(CM_CLKSEL2_PLL_IVA2, 0, 5, ptr->m2);

	/* M (IVA2_DPLL_MULT) : CM_CLKSEL1_PLL_IVA2[8:18] */
	sr32(CM_CLKSEL1_PLL_IVA2, 8, 11, ptr->m);

	/* N (IVA2_DPLL_DIV) : CM_CLKSEL1_PLL_IVA2[0:6] */
	sr32(CM_CLKSEL1_PLL_IVA2, 0, 7, ptr->n);

	/* FREQSEL (IVA2_DPLL_FREQSEL) : CM_CLKEN_PLL_IVA2[4:7] */
	sr32(CM_CLKEN_PLL_IVA2, 4, 4, ptr->fsel);

	/* LOCK MODE (EN_IVA2_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(CM_CLKEN_PLL_IVA2, 0, 3, PLL_LOCK);

	wait_on_value(BIT0, 1, CM_IDLEST_PLL_IVA2, LDELAY);
}

/*
 * OMAP3630 specific functions
 */
static void
dpll3_init_36xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	/* Getting the base address of Core DPLL param table*/
	ptr = (dpll_param *)get_36x_core_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	ptr += clk_index;

	/* CORE DPLL */
	/* Select relock bypass: CM_CLKEN_PLL[0:2] */
	sr32(CM_CLKEN_PLL, 0, 3, PLL_FAST_RELOCK_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_CKGEN, LDELAY);

	/* CM_CLKSEL1_EMU[DIV_DPLL3] */
	sr32(CM_CLKSEL1_EMU, 16, 5, CORE_M3X2);

	/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
	sr32(CM_CLKSEL1_PLL, 27, 5, ptr->m2);

	/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
	sr32(CM_CLKSEL1_PLL, 16, 11, ptr->m);

	/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
	sr32(CM_CLKSEL1_PLL, 8, 7, ptr->n);

	/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
	sr32(CM_CLKSEL1_PLL, 6, 1, 0);

	sr32(CM_CLKSEL_CORE, 8, 4, CORE_SSI_DIV);	/* ssi */
	sr32(CM_CLKSEL_CORE, 4, 2, CORE_FUSB_DIV);	/* fsusb */
	sr32(CM_CLKSEL_CORE, 2, 2, CORE_L4_DIV);	/* l4 */
	sr32(CM_CLKSEL_CORE, 0, 2, CORE_L3_DIV);	/* l3 */

	sr32(CM_CLKSEL_GFX,  0, 3, GFX_DIV_36X);		/* gfx */
	sr32(CM_CLKSEL_WKUP, 1, 2, WKUP_RSM);		/* reset mgr */

	/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
	sr32(CM_CLKEN_PLL,   4, 4, ptr->fsel);
	sr32(CM_CLKEN_PLL,   0, 3, PLL_LOCK);		/* lock mode */

	wait_on_value(BIT0, 1, CM_IDLEST_CKGEN, LDELAY);
}

static void
dpll4_init_36xx(u32 sil_index, u32 clk_index)
{
	struct dpll_per_36x_param *ptr;

	ptr = (struct dpll_per_36x_param *)get_36x_per_dpll_param();

	/* Moving it to the right sysclk base */
	ptr += clk_index;

	/* EN_PERIPH_DPLL: CM_CLKEN_PLL[16:18] */
	sr32(CM_CLKEN_PLL, 16, 3, PLL_STOP);
	wait_on_value(BIT1, 0, CM_IDLEST_CKGEN, LDELAY);

	/* M6 (DIV_DPLL4): CM_CLKSEL1_EMU[24:29] */
	sr32(CM_CLKSEL1_EMU, 24, 6, ptr->m6);

	/* M5 (CLKSEL_CAM): CM_CLKSEL1_EMU[0:5] */
	sr32(CM_CLKSEL_CAM, 0, 6, ptr->m5);

	/* M4 (CLKSEL_DSS1): CM_CLKSEL_DSS[0:5] */
	sr32(CM_CLKSEL_DSS, 0, 6, ptr->m4);

	/* M3 (CLKSEL_DSS1): CM_CLKSEL_DSS[8:13] */
	sr32(CM_CLKSEL_DSS, 8, 6, ptr->m3);

	/* M2 (DIV_96M): CM_CLKSEL3_PLL[0:4] */
	sr32(CM_CLKSEL3_PLL, 0, 5, ptr->m2);

	/* M (PERIPH_DPLL_MULT): CM_CLKSEL2_PLL[8:19] */
	sr32(CM_CLKSEL2_PLL, 8, 12, ptr->m);

	/* N (PERIPH_DPLL_DIV): CM_CLKSEL2_PLL[0:6] */
	sr32(CM_CLKSEL2_PLL, 0, 7, ptr->n);

	/* M2DIV (CLKSEL_96M): CM_CLKSEL_CORE[12:13] */
	sr32(CM_CLKSEL_CORE, 12, 2, ptr->m2div);

	/* LOCK MODE (EN_PERIPH_DPLL): CM_CLKEN_PLL[16:18] */
	sr32(CM_CLKEN_PLL, 16, 3, PLL_LOCK);
	wait_on_value(BIT1, 2, CM_IDLEST_CKGEN, LDELAY);
}

static void
mpu_init_36xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	/* Getting the base address to MPU DPLL param table*/
	ptr = (dpll_param *)get_36x_mpu_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	ptr = ptr + (2*clk_index) + sil_index;

	/* MPU DPLL (unlocked already) */
	/* M2 (MPU_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_MPU[0:4] */
	sr32(CM_CLKSEL2_PLL_MPU, 0, 5, ptr->m2);

	/* M (MPU_DPLL_MULT) : CM_CLKSEL2_PLL_MPU[8:18] */
	sr32(CM_CLKSEL1_PLL_MPU, 8, 11, ptr->m);

	/* N (MPU_DPLL_DIV) : CM_CLKSEL2_PLL_MPU[0:6] */
	sr32(CM_CLKSEL1_PLL_MPU, 0, 7, ptr->n);

	/* LOCK MODE (EN_MPU_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(CM_CLKEN_PLL_MPU, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_PLL_MPU, LDELAY);
}

static void
iva_init_36xx(u32 sil_index, u32 clk_index)
{
	dpll_param *ptr;

	/* Getting the base address to IVA DPLL param table*/
	ptr = (dpll_param *)get_36x_iva_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	ptr = ptr + (2*clk_index) + sil_index;

	/* IVA DPLL */
	/* EN_IVA2_DPLL : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(CM_CLKEN_PLL_IVA2, 0, 3, PLL_STOP);
	wait_on_value(BIT0, 0, CM_IDLEST_PLL_IVA2, LDELAY);

	/* M2 (IVA2_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_IVA2[0:4] */
	sr32(CM_CLKSEL2_PLL_IVA2, 0, 5, ptr->m2);

	/* M (IVA2_DPLL_MULT) : CM_CLKSEL1_PLL_IVA2[8:18] */
	sr32(CM_CLKSEL1_PLL_IVA2, 8, 11, ptr->m);

	/* N (IVA2_DPLL_DIV) : CM_CLKSEL1_PLL_IVA2[0:6] */
	sr32(CM_CLKSEL1_PLL_IVA2, 0, 7, ptr->n);

	/* LOCK MODE (EN_IVA2_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(CM_CLKEN_PLL_IVA2, 0, 3, PLL_LOCK);

	wait_on_value(BIT0, 1, CM_IDLEST_PLL_IVA2, LDELAY);
}

/*
 *  void prcm_init()
 *
 *  Description:
 *    This routine initializes clocks in a board-specific manner for
 *    the processor's Power, Reset and Clock Manager (PRCM) and is
 *    called only when an SRAM-based stack is available (i.e. no
 *    SDRAM).
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    N/A
 *
 */
void
prcm_init(void)
{
	u32 sys_clk_rate = 0, sys_clkin_sel, sys_clk_div;
	u32 clk_index, sil_index;

	/* Gauge the input clock speed and find out the sys_clkin_sel
	 * value corresponding to the input clock.
	 */
	sys_clk_rate = get_osc_clk_speed();
	get_sys_clkin_sel(sys_clk_rate, &sys_clkin_sel);

	/* Set the PRM_CLKSEL_SYS_CLKIN_SEL value in the processor. */

	sr32(PRCM_PRM_CCR_CLKSEL,
		 PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_START,
		 PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_BITS,
		 sys_clkin_sel);

	/* If the input clock is greater than 19.2M always divide by two.
	 *
	 * On OMAP3630, DDR data corruption has been observed on OFF mode
	 * exit if the sys clock was lower than 26M. As a work around,
	 * OMAP3630 is operated at 26M sys clock and this internal division
	 * is not performed.
	 */

	if((is_cpu_family() != CPU_OMAP36XX) &&
	   (sys_clkin_sel > PRCM_PRM_CCR_CLKSEL_SYS_CLKIN_SEL_19_2_MHZ)) {
		sys_clk_div = PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BY_2;
		clk_index = sys_clkin_sel / 2;

	} else {
		sys_clk_div = PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BY_1;
		clk_index = sys_clkin_sel / 1;

	}

	sr32(PRCM_PRM_GR_CLKSRC_CTRL,
		 PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_START,
		 PRCM_PRM_GR_CLKSRC_CTRL_SYSCLKDIV_BITS,
		 sys_clk_div);

	if (is_cpu_family() == CPU_OMAP36XX) {
		dpll3_init_36xx(0, clk_index);
		dpll4_init_36xx(0, clk_index);
		mpu_init_36xx(0, clk_index);
		iva_init_36xx(0, clk_index);

	} else {
		sil_index = get_cpu_rev() - 1;

		/* The DPLL tables are defined according to sysclk value and
		 * silicon revision. The clk_index value will be used to get
		 * the values for that input sysclk from the DPLL param table
		 * and sil_index will get the values for that SysClk for the
		 * appropriate silicon rev.
		 */

		/* Unlock MPU DPLL (slows things down, and needed later) */
		sr32(CM_CLKEN_PLL_MPU, 0, 3, PLL_LOW_POWER_BYPASS);
		wait_on_value(BIT0, 0, CM_IDLEST_PLL_MPU, LDELAY);

		dpll3_init_34xx(sil_index, clk_index);
		dpll4_init_34xx(sil_index, clk_index);
		iva_init_34xx(sil_index, clk_index);
		mpu_init_34xx(sil_index, clk_index);

		/* Lock MPU DPLL to set frequency */
		sr32(CM_CLKEN_PLL_MPU, 0, 3, PLL_LOCK);
		wait_on_value(BIT0, 1, CM_IDLEST_PLL_MPU, LDELAY);
	}

	/* Set up GPTimers to sys_clk source only */
 	sr32(CM_CLKSEL_PER, 0, 8, 0xff);
	sr32(CM_CLKSEL_WKUP, 0, 1, 1);

	delay(5000);
}
