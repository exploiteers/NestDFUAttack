/*
 * (C) Copyright 2006
 * Texas Instruments, <www.ti.com>
 * Jian Zhang <jzhang@ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
#include <common.h>
#include <command.h>
#include <part.h>
#include <fat.h>
#include <asm/arch/cpu.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>
#include <asm/arch/mem.h>

/* Used to index into DPLL parameter tables */
struct dpll_param {
        unsigned int m;
        unsigned int n;
        unsigned int fsel;
        unsigned int m2;
};

struct dpll_per_36x_param {
	unsigned int sys_clk;
	unsigned int m;
	unsigned int n;
	unsigned int m2;
	unsigned int m3;
	unsigned int m4;
	unsigned int m5;
	unsigned int m6;
	unsigned int m2div;
};

typedef struct dpll_param dpll_param;

#define MAX_SIL_INDEX	3

/* Following functions are exported from lowlevel_init.S */
extern dpll_param *get_mpu_dpll_param(void);
extern dpll_param *get_iva_dpll_param(void);
extern dpll_param *get_core_dpll_param(void);
extern dpll_param *get_per_dpll_param(void);

extern dpll_param *get_36x_mpu_dpll_param(void);
extern dpll_param *get_36x_iva_dpll_param(void);
extern dpll_param *get_36x_core_dpll_param(void);
extern dpll_param *get_36x_per_dpll_param(void);

extern int mmc_init(int verbose);
extern block_dev_desc_t *mmc_get_dev(int dev);

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

void udelay (unsigned long usecs) {
	delay(usecs);
}

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init (void)
{
	return 0;
}

/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
        int mode;
        mode = __raw_readl(CONTROL_STATUS) & (DEVICE_MASK);
        return(mode >>= 8);
}

/************************************************
 * get_sysboot_value(void) - return SYS_BOOT[4:0]
 ************************************************/
u32 get_sysboot_value(void)
{
        int mode;
        mode = __raw_readl(CONTROL_STATUS) & (SYSBOOT_MASK);
        return mode;
}
/*************************************************************
 * Routine: get_mem_type(void) - returns the kind of memory connected
 * to GPMC that we are trying to boot form. Uses SYS BOOT settings.
 *************************************************************/
u32 get_mem_type(void)
{
        u32   mem_type = get_sysboot_value();
        switch (mem_type){
            case 0:
            case 2:
            case 4:
            case 16:
            case 22:    return GPMC_ONENAND;

            case 1:
            case 12:
            case 15:
            case 21:
            case 27:    return GPMC_NAND;

            case 3:
            case 6:     return MMC_ONENAND;

            case 8:
            case 11:
            case 14:
            case 20:
            case 26:    return GPMC_MDOC;

            case 17:
            case 18:
            case 24:	return MMC_NAND;

            case 7:
            case 10:
            case 13:
            case 19:
            case 25:
            default:    return GPMC_NOR;
        }
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
	u32 cpuid=0;
	/* On ES1.0 the IDCODE register is not exposed on L4
	 * so using CPU ID to differentiate
	 * between ES2.0 and ES1.0.
	 */
	__asm__ __volatile__("mrc p15, 0, %0, c0, c0, 0":"=r" (cpuid));
	if((cpuid  & 0xf) == 0x0)
		return CPU_3430_ES1;
	else
		return CPU_3430_ES2;

}

u32 is_cpu_family(void)
{
	u32 cpuid = 0, cpu_family = 0;
	u16 hawkeye;

	__asm__ __volatile__("mrc p15, 0, %0, c0, c0, 0":"=r"(cpuid));
	if ((cpuid & 0xf) == 0x0) {
		cpu_family = CPU_OMAP34XX;
	} else {
		cpuid = __raw_readl(OMAP34XX_CONTROL_ID);
		hawkeye  = (cpuid >> HAWKEYE_SHIFT) & 0xffff;

		switch (hawkeye) {
			case HAWKEYE_OMAP34XX:
				cpu_family = CPU_OMAP34XX;
				break;
			case HAWKEYE_AM35XX:
				cpu_family = CPU_AM35XX;
				break;
			case HAWKEYE_OMAP36XX:
				cpu_family = CPU_OMAP36XX;
				break;
			default:
				cpu_family = CPU_OMAP34XX;
				break;
		}
	}
	return cpu_family;
}
/******************************************
 * cpu_is_3410(void) - returns true for 3410
 ******************************************/
u32 cpu_is_3410(void)
{
	int status;
	if(get_cpu_rev() < CPU_3430_ES2) {
		return 0;
	} else {
		/* read scalability status and return 1 for 3410*/
		status = __raw_readl(CONTROL_SCALABLE_OMAP_STATUS);
		/* Check whether MPU frequency is set to 266 MHz which
		 * is nominal for 3410. If yes return true else false
		 */
		if (((status >> 8) & 0x3) == 0x2)
			return 1;
		else
			return 0;
	}
}

/*****************************************************************
 * sr32 - clear & set a value in a bit range for a 32 bit address
 *****************************************************************/
void sr32(u32 addr, u32 start_bit, u32 num_bits, u32 value)
{
	u32 tmp, msk = 0;
	msk = 1 << num_bits;
	--msk;
	tmp = __raw_readl(addr) & ~(msk << start_bit);
	tmp |=  value << start_bit;
	__raw_writel(tmp, addr);
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
{
	u32 i = 0, val;
	do {
		++i;
		val = __raw_readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return (1);
		if (i == bound)
			return (0);
	} while (1);
}

#ifdef CFG_OMAPEVM_DDR
#ifdef CONFIG_DDR_256MB_STACKED
/**************************************************************************
 * make_cs1_contiguous() - for es2 and above remap cs1 behind cs0 to allow
 *  command line mem=xyz use all memory with out discontinuous support
 *  compiled in.  Could do it at the ATAG, but there really is two banks...
 * Called as part of 2nd phase DDR init.
 **************************************************************************/
void make_cs1_contiguous(void)
{
	u32 size, a_add_low, a_add_high;

	size = get_sdr_cs_size(SDRC_CS0_OSET);
	size /= SZ_32M;         /* find size to offset CS1 */
	a_add_high = (size & 3) << 8;   /* set up low field */
	a_add_low = (size & 0x3C) >> 2; /* set up high field */
	__raw_writel((a_add_high | a_add_low), SDRC_CS_CFG);
}

/***********************************************************************
 * get_cs0_size() - get size of chip select 0/1
 ************************************************************************/
u32 get_sdr_cs_size(u32 offset)
{
	u32 size;

	/* get ram size field */
	size = __raw_readl(SDRC_MCFG_0 + offset) >> 8;
	size &= 0x3FF;          /* remove unwanted bits */
	size *= SZ_2M;          /* find size in MB */
	return size;
}
#endif

/*********************************************************************
 * config_3430sdram_ddr() - Init DDR on 3430SDP dev board.
 *********************************************************************/
void config_3430sdram_ddr(void)
{

#ifndef CONFIG_DDR_256MB_STACKED
	/* reset sdrc controller */
	__raw_writel(SOFTRESET, SDRC_SYSCONFIG);
	wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000);
	__raw_writel(0, SDRC_SYSCONFIG);

	/* setup sdrc to ball mux */
	__raw_writel(SDP_SDRC_SHARING, SDRC_SHARING);

	/* set mdcfg */
	__raw_writel(SDP_SDRC_MDCFG_0_DDR, SDRC_MCFG_0);

	/* set timing */
	if (is_cpu_family() == CPU_OMAP36XX) {
		__raw_writel(HYNIX_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
		__raw_writel(HYNIX_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);

	} else {
		if ((get_mem_type() == GPMC_ONENAND) || (get_mem_type() == MMC_ONENAND)){
			__raw_writel(INFINEON_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
			__raw_writel(INFINEON_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);
		}
		if ((get_mem_type() == GPMC_NAND) ||(get_mem_type() == MMC_NAND)){
			__raw_writel(MICRON_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
			__raw_writel(MICRON_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);
		}
	 }

	__raw_writel(SDP_SDRC_RFR_CTRL, SDRC_RFR_CTRL);
	__raw_writel(SDP_SDRC_POWER_POP, SDRC_POWER);

	/* init sequence for mDDR/mSDR using manual commands (DDR is different) */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0);
	delay(5000);
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);

	/* set mr0 */
	__raw_writel(SDP_SDRC_MR_0_DDR, SDRC_MR_0);

	/* set up dll */
	__raw_writel(SDP_SDRC_DLLAB_CTRL, SDRC_DLLA_CTRL);
	delay(0x2000);	/* give time to lock */
#else
       /* reset sdrc controller */
         __raw_writel(SOFTRESET, SDRC_SYSCONFIG);
         wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000);
         __raw_writel(0, SDRC_SYSCONFIG);

         /* setup sdrc to ball mux */
         __raw_writel(SDP_SDRC_SHARING, SDRC_SHARING);

         /* SDRC_MCFG0 register */
         (*(unsigned int*)0x6D000080) = 0x02584099;//from Micron

	 if (is_cpu_family() == CPU_OMAP36XX) {
		 /* SDRC_ACTIM_CTRLA0 register */
		 (*(unsigned int*)0x6D00009c) = 0x92e1c4c6;// for 200M
		 /* SDRC_ACTIM_CTRLB0 register */
		 (*(unsigned int*)0x6D0000a0) = 0x0002111c;
	 } else {
		 /* SDRC_ACTIM_CTRLA0 register */
		 (*(unsigned int*)0x6D00009c) = 0xaa9db4c6;// for 166M from rkw
		 /* SDRC_ACTIM_CTRLB0 register */
		 (*(unsigned int*)0x6D0000a0) = 0x00011517;
	 }


         (*(unsigned int*)0x6D0000a4) =0x0004DC01;

         /* Disble Power Down of CKE cuz of 1 CKE on combo part */
         (*(unsigned int*)0x6D000070) = 0x00000081;

         /* SDRC_Manual command register */
         (*(unsigned int*)0x6D0000a8) = 0x00000000; // NOP command
         delay(5000);
         (*(unsigned int*)0x6D0000a8) = 0x00000001; // Precharge command
         (*(unsigned int*)0x6D0000a8) = 0x00000002; // Auto-refresh command
         (*(unsigned int*)0x6D0000a8) = 0x00000002; // Auto-refresh command

         /* SDRC MR0 register */
         (*(int*)0x6D000084) = 0x00000032; // Burst length =4
         // CAS latency = 3
         // Write Burst = Read Burst
         // Serial Mode

         /* SDRC DLLA control register */
         (*(unsigned int*)0x6D000060) = 0x0000A;
         delay(0x20000); // some delay

#endif

#ifdef CONFIG_DDR_256MB_STACKED
	make_cs1_contiguous();

	__raw_writel(SDP_SDRC_MDCFG_0_DDR, SDRC_MCFG_0 + SDRC_CS1_OSET);
	__raw_writel(MICRON_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_1);
	__raw_writel(MICRON_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_1);

	__raw_writel(SDP_SDRC_RFR_CTRL, SDRC_RFR_CTRL + SDRC_CS1_OSET);
	/* init sequence for mDDR/mSDR using manual commands */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0 + SDRC_CS1_OSET);
	delay(5000);   /* supposed to be 100us per design spec for mddr/msdr */
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0 + SDRC_CS1_OSET);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0 + SDRC_CS1_OSET);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0 + SDRC_CS1_OSET);
	__raw_writel(SDP_SDRC_MR_0_DDR, SDRC_MR_0 + SDRC_CS1_OSET);
#endif
}
#endif /* CFG_OMAPEVM_DDR */

/*************************************************************
 * get_sys_clk_speed - determine reference oscillator speed
 *  based on known 32kHz clock and gptimer.
 *************************************************************/
u32 get_osc_clk_speed(void)
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

/******************************************************************************
 * get_sys_clkin_sel() - returns the sys_clkin_sel field value based on
 *   -- input oscillator clock frequency.
 *
 *****************************************************************************/
void get_sys_clkin_sel(u32 osc_clk, u32 *sys_clkin_sel)
{
	if(osc_clk == S38_4M)
		*sys_clkin_sel=  4;
	else if(osc_clk == S26M)
		*sys_clkin_sel = 3;
	else if(osc_clk == S19_2M)
		*sys_clkin_sel = 2;
	else if(osc_clk == S13M)
		*sys_clkin_sel = 1;
	else if(osc_clk == S12M)
		*sys_clkin_sel = 0;
}

/*
 * OMAP34x/35x specific functions
 */
static void dpll3_init_34xx(u32 sil_index, u32 clk_index)
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

static void dpll4_init_34xx(u32 sil_index, u32 clk_index)
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

static void mpu_init_34xx(u32 sil_index, u32 clk_index)
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

static void iva_init_34xx(u32 sil_index, u32 clk_index)
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
static void dpll3_init_36xx(u32 sil_index, u32 clk_index)
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

static void dpll4_init_36xx(u32 sil_index, u32 clk_index)
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

static void mpu_init_36xx(u32 sil_index, u32 clk_index)
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

static void iva_init_36xx(u32 sil_index, u32 clk_index)
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


/******************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 *   -- called from SRAM, or Flash (using temp SRAM stack).
 *****************************************************************************/
void prcm_init(void)
{
	u32 osc_clk=0, sys_clkin_sel;
	u32 clk_index, sil_index;

	/* Gauge the input clock speed and find out the sys_clkin_sel
	 * value corresponding to the input clock.
	 */
	osc_clk = get_osc_clk_speed();
	get_sys_clkin_sel(osc_clk, &sys_clkin_sel);

	sr32(PRM_CLKSEL, 0, 3, sys_clkin_sel); /* set input crystal speed */

	/* If the input clock is greater than 19.2M always divide/2 */
	/*
	 * On OMAP3630, DDR data corruption has been observed on OFF mode
	 * exit if the sys clock was lower than 26M. As a work around,
	 * OMAP3630 is operated at 26M sys clock and this internal division
	 * is not performed.
	 */
	if((is_cpu_family() != CPU_OMAP36XX) && (sys_clkin_sel > 2)) {
		sr32(PRM_CLKSRC_CTRL, 6, 2, 2);/* input clock divider */
		clk_index = sys_clkin_sel/2;
	} else {
		sr32(PRM_CLKSRC_CTRL, 6, 2, 1);/* input clock divider */
		clk_index = sys_clkin_sel;
	}

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

/*****************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access
 * (GP Device only)
 *****************************************/
void secure_unlock(void)
{
	/* Permission values for registers -Full fledged permissions to all */
	#define UNLOCK_1 0xFFFFFFFF
	#define UNLOCK_2 0x00000000
	#define UNLOCK_3 0x0000FFFF
	/* Protection Module Register Target APE (PM_RT)*/
	__raw_writel(UNLOCK_1, RT_REQ_INFO_PERMISSION_1);
	__raw_writel(UNLOCK_1, RT_READ_PERMISSION_0);
	__raw_writel(UNLOCK_1, RT_WRITE_PERMISSION_0);
	__raw_writel(UNLOCK_2, RT_ADDR_MATCH_1);

	__raw_writel(UNLOCK_3, GPMC_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, GPMC_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, GPMC_WRITE_PERMISSION_0);

	__raw_writel(UNLOCK_3, OCM_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, OCM_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, OCM_WRITE_PERMISSION_0);
	__raw_writel(UNLOCK_2, OCM_ADDR_MATCH_2);

	/* IVA Changes */
	__raw_writel(UNLOCK_3, IVA2_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, IVA2_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, IVA2_WRITE_PERMISSION_0);

	__raw_writel(UNLOCK_1, SMS_RG_ATT0); /* SDRC region 0 public */
}

/**********************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP type, unlock the SRAM for
 *  general use.
 ***********************************************************/
void try_unlock_memory(void)
{
	int mode;

	/* if GP device unlock device SRAM for general use */
	/* secure code breaks for Secure/Emulation device - HS/E/T*/
	mode = get_device_type();
	if (mode == GP_DEVICE) {
		secure_unlock();
	}
	return;
}

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called at time when only stack is available.
 **********************************************************/

void s_init(void)
{
	watchdog_init();
#ifdef CONFIG_3430_AS_3410
	/* setup the scalability control register for
	 * 3430 to work in 3410 mode
	 */
	__raw_writel(0x5ABF,CONTROL_SCALABLE_OMAP_OCP);
#endif
	try_unlock_memory();
	set_muxconf_regs();
	delay(100);
	prcm_init();
	per_clocks_enable();
	config_3430sdram_ddr();
}

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r (void)
{
	return(0);
}

/******************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 ******************************************************/
void wait_for_command_complete(unsigned int wd_base)
{
	int pending = 1;
	do {
		pending = __raw_readl(wd_base + WWPS);
	} while (pending);
}

/****************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************/
void watchdog_init(void)
{
	/* There are 3 watch dogs WD1=Secure, WD2=MPU, WD3=IVA. WD1 is
	 * either taken care of by ROM (HS/EMU) or not accessible (GP).
	 * We need to take care of WD2-MPU or take a PRCM reset.  WD3
	 * should not be running and does not generate a PRCM reset.
	 */
	sr32(CM_FCLKEN_WKUP, 5, 1, 1);
	sr32(CM_ICLKEN_WKUP, 5, 1, 1);
	wait_on_value(BIT5, 0x20, CM_IDLEST_WKUP, 5); /* some issue here */

	__raw_writel(WD_UNLOCK1, WD2_BASE + WSPR);
	wait_for_command_complete(WD2_BASE);
	__raw_writel(WD_UNLOCK2, WD2_BASE + WSPR);
}

/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init (void)
{
	return 0;
}

/*****************************************************************
 * Routine: peripheral_enable
 * Description: Enable the clks & power for perifs (GPT2, UART1,...)
 ******************************************************************/
void per_clocks_enable(void)
{
	/* Enable GP2 timer. */
	sr32(CM_CLKSEL_PER, 0, 1, 0x1); /* GPT2 = sys clk */
	sr32(CM_ICLKEN_PER, 3, 1, 0x1); /* ICKen GPT2 */
	sr32(CM_FCLKEN_PER, 3, 1, 0x1); /* FCKen GPT2 */

#ifdef CFG_NS16550
	/* Enable UART1 clocks */
	sr32(CM_FCLKEN1_CORE, 13, 1, 0x1);
	sr32(CM_ICLKEN1_CORE, 13, 1, 0x1);
#endif

#ifdef CONFIG_MMC
	/* Enable MMC1 clocks */
	sr32(CM_FCLKEN1_CORE, 24, 1, 0x1);
	sr32(CM_ICLKEN1_CORE, 24, 1, 0x1);
#endif
	delay(1000);
}

/* Set MUX for UART, GPMC, SDRC, GPIO */

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
#define MUX_DEFAULT()\
	MUX_VAL(CP(SDRC_D0),        (IEN  | PTD | DIS | M0)) /*SDRC_D0*/\
	MUX_VAL(CP(SDRC_D1),        (IEN  | PTD | DIS | M0)) /*SDRC_D1*/\
	MUX_VAL(CP(SDRC_D2),        (IEN  | PTD | DIS | M0)) /*SDRC_D2*/\
	MUX_VAL(CP(SDRC_D3),        (IEN  | PTD | DIS | M0)) /*SDRC_D3*/\
	MUX_VAL(CP(SDRC_D4),        (IEN  | PTD | DIS | M0)) /*SDRC_D4*/\
	MUX_VAL(CP(SDRC_D5),        (IEN  | PTD | DIS | M0)) /*SDRC_D5*/\
	MUX_VAL(CP(SDRC_D6),        (IEN  | PTD | DIS | M0)) /*SDRC_D6*/\
	MUX_VAL(CP(SDRC_D7),        (IEN  | PTD | DIS | M0)) /*SDRC_D7*/\
	MUX_VAL(CP(SDRC_D8),        (IEN  | PTD | DIS | M0)) /*SDRC_D8*/\
	MUX_VAL(CP(SDRC_D9),        (IEN  | PTD | DIS | M0)) /*SDRC_D9*/\
	MUX_VAL(CP(SDRC_D10),       (IEN  | PTD | DIS | M0)) /*SDRC_D10*/\
	MUX_VAL(CP(SDRC_D11),       (IEN  | PTD | DIS | M0)) /*SDRC_D11*/\
	MUX_VAL(CP(SDRC_D12),       (IEN  | PTD | DIS | M0)) /*SDRC_D12*/\
	MUX_VAL(CP(SDRC_D13),       (IEN  | PTD | DIS | M0)) /*SDRC_D13*/\
	MUX_VAL(CP(SDRC_D14),       (IEN  | PTD | DIS | M0)) /*SDRC_D14*/\
	MUX_VAL(CP(SDRC_D15),       (IEN  | PTD | DIS | M0)) /*SDRC_D15*/\
	MUX_VAL(CP(SDRC_D16),       (IEN  | PTD | DIS | M0)) /*SDRC_D16*/\
	MUX_VAL(CP(SDRC_D17),       (IEN  | PTD | DIS | M0)) /*SDRC_D17*/\
	MUX_VAL(CP(SDRC_D18),       (IEN  | PTD | DIS | M0)) /*SDRC_D18*/\
	MUX_VAL(CP(SDRC_D19),       (IEN  | PTD | DIS | M0)) /*SDRC_D19*/\
	MUX_VAL(CP(SDRC_D20),       (IEN  | PTD | DIS | M0)) /*SDRC_D20*/\
	MUX_VAL(CP(SDRC_D21),       (IEN  | PTD | DIS | M0)) /*SDRC_D21*/\
	MUX_VAL(CP(SDRC_D22),       (IEN  | PTD | DIS | M0)) /*SDRC_D22*/\
	MUX_VAL(CP(SDRC_D23),       (IEN  | PTD | DIS | M0)) /*SDRC_D23*/\
	MUX_VAL(CP(SDRC_D24),       (IEN  | PTD | DIS | M0)) /*SDRC_D24*/\
	MUX_VAL(CP(SDRC_D25),       (IEN  | PTD | DIS | M0)) /*SDRC_D25*/\
	MUX_VAL(CP(SDRC_D26),       (IEN  | PTD | DIS | M0)) /*SDRC_D26*/\
	MUX_VAL(CP(SDRC_D27),       (IEN  | PTD | DIS | M0)) /*SDRC_D27*/\
	MUX_VAL(CP(SDRC_D28),       (IEN  | PTD | DIS | M0)) /*SDRC_D28*/\
	MUX_VAL(CP(SDRC_D29),       (IEN  | PTD | DIS | M0)) /*SDRC_D29*/\
	MUX_VAL(CP(SDRC_D30),       (IEN  | PTD | DIS | M0)) /*SDRC_D30*/\
	MUX_VAL(CP(SDRC_D31),       (IEN  | PTD | DIS | M0)) /*SDRC_D31*/\
	MUX_VAL(CP(SDRC_CLK),       (IEN  | PTD | DIS | M0)) /*SDRC_CLK*/\
	MUX_VAL(CP(SDRC_DQS0),      (IEN  | PTD | DIS | M0)) /*SDRC_DQS0*/\
	MUX_VAL(CP(SDRC_DQS1),      (IEN  | PTD | DIS | M0)) /*SDRC_DQS1*/\
	MUX_VAL(CP(SDRC_DQS2),      (IEN  | PTD | DIS | M0)) /*SDRC_DQS2*/\
	MUX_VAL(CP(SDRC_DQS3),      (IEN  | PTD | DIS | M0)) /*SDRC_DQS3*/\
	MUX_VAL(CP(GPMC_A1),        (IDIS | PTD | DIS | M0)) /*GPMC_A1*/\
	MUX_VAL(CP(GPMC_A2),        (IDIS | PTD | DIS | M0)) /*GPMC_A2*/\
	MUX_VAL(CP(GPMC_A3),        (IDIS | PTD | DIS | M0)) /*GPMC_A3*/\
	MUX_VAL(CP(GPMC_A4),        (IDIS | PTD | DIS | M0)) /*GPMC_A4*/\
	MUX_VAL(CP(GPMC_A5),        (IDIS | PTD | DIS | M0)) /*GPMC_A5*/\
	MUX_VAL(CP(GPMC_A6),        (IDIS | PTD | DIS | M0)) /*GPMC_A6*/\
	MUX_VAL(CP(GPMC_A7),        (IDIS | PTD | DIS | M0)) /*GPMC_A7*/\
	MUX_VAL(CP(GPMC_A8),        (IDIS | PTD | DIS | M0)) /*GPMC_A8*/\
	MUX_VAL(CP(GPMC_A9),        (IDIS | PTD | DIS | M0)) /*GPMC_A9*/\
	MUX_VAL(CP(GPMC_A10),       (IDIS | PTD | DIS | M0)) /*GPMC_A10*/\
	MUX_VAL(CP(GPMC_D0),        (IEN  | PTD | DIS | M0)) /*GPMC_D0*/\
	MUX_VAL(CP(GPMC_D1),        (IEN  | PTD | DIS | M0)) /*GPMC_D1*/\
	MUX_VAL(CP(GPMC_D2),        (IEN  | PTD | DIS | M0)) /*GPMC_D2*/\
	MUX_VAL(CP(GPMC_D3),        (IEN  | PTD | DIS | M0)) /*GPMC_D3*/\
	MUX_VAL(CP(GPMC_D4),        (IEN  | PTD | DIS | M0)) /*GPMC_D4*/\
	MUX_VAL(CP(GPMC_D5),        (IEN  | PTD | DIS | M0)) /*GPMC_D5*/\
	MUX_VAL(CP(GPMC_D6),        (IEN  | PTD | DIS | M0)) /*GPMC_D6*/\
	MUX_VAL(CP(GPMC_D7),        (IEN  | PTD | DIS | M0)) /*GPMC_D7*/\
	MUX_VAL(CP(GPMC_D8),        (IEN  | PTD | DIS | M0)) /*GPMC_D8*/\
	MUX_VAL(CP(GPMC_D9),        (IEN  | PTD | DIS | M0)) /*GPMC_D9*/\
	MUX_VAL(CP(GPMC_D10),       (IEN  | PTD | DIS | M0)) /*GPMC_D10*/\
	MUX_VAL(CP(GPMC_D11),       (IEN  | PTD | DIS | M0)) /*GPMC_D11*/\
	MUX_VAL(CP(GPMC_D12),       (IEN  | PTD | DIS | M0)) /*GPMC_D12*/\
	MUX_VAL(CP(GPMC_D13),       (IEN  | PTD | DIS | M0)) /*GPMC_D13*/\
	MUX_VAL(CP(GPMC_D14),       (IEN  | PTD | DIS | M0)) /*GPMC_D14*/\
	MUX_VAL(CP(GPMC_D15),       (IEN  | PTD | DIS | M0)) /*GPMC_D15*/\
	MUX_VAL(CP(GPMC_NCS0),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS0*/\
	MUX_VAL(CP(GPMC_NCS1),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS1*/\
	MUX_VAL(CP(GPMC_NCS2),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS2*/\
	MUX_VAL(CP(GPMC_NCS3),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS3*/\
	MUX_VAL(CP(GPMC_NCS4),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS4*/\
	MUX_VAL(CP(GPMC_NCS5),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS5*/\
	MUX_VAL(CP(GPMC_NCS6),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS6*/\
	MUX_VAL(CP(GPMC_NCS7),      (IDIS | PTU | EN  | M0)) /*GPMC_NCS7*/\
	MUX_VAL(CP(GPMC_CLK),       (IDIS | PTD | DIS | M0)) /*GPMC_CLK*/\
	MUX_VAL(CP(GPMC_NADV_ALE),  (IDIS | PTD | DIS | M0)) /*GPMC_NADV_ALE*/\
	MUX_VAL(CP(GPMC_NOE),       (IDIS | PTD | DIS | M0)) /*GPMC_NOE*/\
	MUX_VAL(CP(GPMC_NWE),       (IDIS | PTD | DIS | M0)) /*GPMC_NWE*/\
	MUX_VAL(CP(GPMC_NBE0_CLE),  (IDIS | PTD | DIS | M0)) /*GPMC_NBE0_CLE*/\
	MUX_VAL(CP(GPMC_NBE1),      (IDIS | PTD | DIS | M4)) /*GPIO_61*/\
	MUX_VAL(CP(GPMC_NWP),       (IEN  | PTD | DIS | M0)) /*GPMC_NWP*/\
	MUX_VAL(CP(GPMC_WAIT0),     (IEN  | PTU | EN  | M0)) /*GPMC_WAIT0*/\
	MUX_VAL(CP(GPMC_WAIT1),     (IEN  | PTU | EN  | M0)) /*GPMC_WAIT1*/\
	MUX_VAL(CP(GPMC_WAIT2),     (IEN  | PTU | EN  | M4)) /*GPIO_64*/\
	MUX_VAL(CP(GPMC_WAIT3),     (IEN  | PTU | EN  | M4)) /*GPIO_65*/\
	MUX_VAL(CP(DSS_DATA18),     (IEN  | PTD | DIS | M4)) /*GPIO_88*/\
	MUX_VAL(CP(DSS_DATA19),     (IEN  | PTD | DIS | M4)) /*GPIO_89*/\
	MUX_VAL(CP(DSS_DATA20),     (IEN  | PTD | DIS | M4)) /*GPIO_90*/\
	MUX_VAL(CP(DSS_DATA21),     (IEN  | PTD | DIS | M4)) /*GPIO_91*/\
	MUX_VAL(CP(CAM_WEN),        (IEN  | PTD | DIS | M4)) /*GPIO_167*/\
	MUX_VAL(CP(UART1_TX),       (IDIS | PTD | DIS | M0)) /*UART1_TX*/\
	MUX_VAL(CP(UART1_RTS),      (IDIS | PTD | DIS | M0)) /*UART1_RTS*/\
	MUX_VAL(CP(UART1_CTS),      (IEN | PTU | DIS | M0)) /*UART1_CTS*/\
	MUX_VAL(CP(UART1_RX),       (IEN | PTD | DIS | M0)) /*UART1_RX*/\
	MUX_VAL(CP(MCBSP1_DX),      (IEN  | PTD | DIS | M4)) /*GPIO_158*/\
	MUX_VAL(CP(SYS_32K),        (IEN  | PTD | DIS | M0)) /*SYS_32K*/\
	MUX_VAL(CP(SYS_BOOT0),      (IEN  | PTD | DIS | M4)) /*GPIO_2 */\
	MUX_VAL(CP(SYS_BOOT1),      (IEN  | PTD | DIS | M4)) /*GPIO_3 */\
	MUX_VAL(CP(SYS_BOOT2),      (IEN  | PTD | DIS | M4)) /*GPIO_4 */\
	MUX_VAL(CP(SYS_BOOT3),      (IEN  | PTD | DIS | M4)) /*GPIO_5 */\
	MUX_VAL(CP(SYS_BOOT4),      (IEN  | PTD | DIS | M4)) /*GPIO_6 */\
	MUX_VAL(CP(SYS_BOOT5),      (IEN  | PTD | DIS | M4)) /*GPIO_7 */\
	MUX_VAL(CP(SYS_BOOT6),      (IEN  | PTD | DIS | M4)) /*GPIO_8 */\
	MUX_VAL(CP(SYS_CLKOUT2),    (IEN  | PTU | EN  | M4)) /*GPIO_186*/\
	MUX_VAL(CP(JTAG_nTRST),     (IEN  | PTD | DIS | M0)) /*JTAG_nTRST*/\
	MUX_VAL(CP(JTAG_TCK),       (IEN  | PTD | DIS | M0)) /*JTAG_TCK*/\
	MUX_VAL(CP(JTAG_TMS),       (IEN  | PTD | DIS | M0)) /*JTAG_TMS*/\
	MUX_VAL(CP(JTAG_TDI),       (IEN  | PTD | DIS | M0)) /*JTAG_TDI*/\
	MUX_VAL(CP(JTAG_EMU0),      (IEN  | PTD | DIS | M0)) /*JTAG_EMU0*/\
	MUX_VAL(CP(JTAG_EMU1),      (IEN  | PTD | DIS | M0)) /*JTAG_EMU1*/\
	MUX_VAL(CP(ETK_CLK),        (IEN  | PTD | DIS | M4)) /*GPIO_12*/\
	MUX_VAL(CP(ETK_CTL),        (IEN  | PTD | DIS | M4)) /*GPIO_13*/\
	MUX_VAL(CP(ETK_D0 ),        (IEN  | PTD | DIS | M4)) /*GPIO_14*/\
	MUX_VAL(CP(ETK_D1 ),        (IEN  | PTD | DIS | M4)) /*GPIO_15*/\
	MUX_VAL(CP(ETK_D2 ),        (IEN  | PTD | DIS | M4)) /*GPIO_16*/\
	MUX_VAL(CP(ETK_D10),        (IEN  | PTD | DIS | M4)) /*GPIO_24*/\
	MUX_VAL(CP(ETK_D11),        (IEN  | PTD | DIS | M4)) /*GPIO_25*/\
	MUX_VAL(CP(ETK_D12),        (IEN  | PTD | DIS | M4)) /*GPIO_26*/\
	MUX_VAL(CP(ETK_D13),        (IEN  | PTD | DIS | M4)) /*GPIO_27*/\
	MUX_VAL(CP(ETK_D14),        (IEN  | PTD | DIS | M4)) /*GPIO_28*/\
	MUX_VAL(CP(ETK_D15),        (IEN  | PTD | DIS | M4)) /*GPIO_29*/

/**********************************************************
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware. Many pins need
 *              to be moved from protect to primary mode.
 *********************************************************/
void set_muxconf_regs(void)
{
	MUX_DEFAULT();
}

/**********************************************************
 * Routine: nand+_init
 * Description: Set up nand for nand and jffs2 commands
 *********************************************************/

int nand_init(void)
{
	/* global settings */
	__raw_writel(0x10, GPMC_SYSCONFIG);	/* smart idle */
	__raw_writel(0x0, GPMC_IRQENABLE);	/* isr's sources masked */
	__raw_writel(0, GPMC_TIMEOUT_CONTROL);/* timeout disable */

	/* Set the GPMC Vals . For NAND boot on 3430SDP, NAND is mapped at CS0
         *  , NOR at CS1 and MPDB at CS3. And oneNAND boot, we map oneNAND at CS0.
	 *  We configure only GPMC CS0 with required values. Configiring other devices
	 *  at other CS in done in u-boot anyway. So we don't have to bother doing it here.
         */
	__raw_writel(0 , GPMC_CONFIG7 + GPMC_CONFIG_CS0);
	delay(1000);

	if ((get_mem_type() == GPMC_NAND) || (get_mem_type() == MMC_NAND)){
        	__raw_writel( M_NAND_GPMC_CONFIG1, GPMC_CONFIG1 + GPMC_CONFIG_CS0);
        	__raw_writel( M_NAND_GPMC_CONFIG2, GPMC_CONFIG2 + GPMC_CONFIG_CS0);
        	__raw_writel( M_NAND_GPMC_CONFIG3, GPMC_CONFIG3 + GPMC_CONFIG_CS0);
        	__raw_writel( M_NAND_GPMC_CONFIG4, GPMC_CONFIG4 + GPMC_CONFIG_CS0);
        	__raw_writel( M_NAND_GPMC_CONFIG5, GPMC_CONFIG5 + GPMC_CONFIG_CS0);
        	__raw_writel( M_NAND_GPMC_CONFIG6, GPMC_CONFIG6 + GPMC_CONFIG_CS0);

        	/* Enable the GPMC Mapping */
        	__raw_writel(( ((OMAP34XX_GPMC_CS0_SIZE & 0xF)<<8) |
        		     ((NAND_BASE_ADR>>24) & 0x3F) |
        		     (1<<6) ),  (GPMC_CONFIG7 + GPMC_CONFIG_CS0));
        	delay(2000);

         	if (nand_chip()){
#ifdef CFG_PRINTF
        		printf("Unsupported Chip!\n");
#endif
        		return 1;
        	}

	}

	if ((get_mem_type() == GPMC_ONENAND) || (get_mem_type() == MMC_ONENAND)){
        	__raw_writel( ONENAND_GPMC_CONFIG1, GPMC_CONFIG1 + GPMC_CONFIG_CS0);
        	__raw_writel( ONENAND_GPMC_CONFIG2, GPMC_CONFIG2 + GPMC_CONFIG_CS0);
        	__raw_writel( ONENAND_GPMC_CONFIG3, GPMC_CONFIG3 + GPMC_CONFIG_CS0);
        	__raw_writel( ONENAND_GPMC_CONFIG4, GPMC_CONFIG4 + GPMC_CONFIG_CS0);
        	__raw_writel( ONENAND_GPMC_CONFIG5, GPMC_CONFIG5 + GPMC_CONFIG_CS0);
        	__raw_writel( ONENAND_GPMC_CONFIG6, GPMC_CONFIG6 + GPMC_CONFIG_CS0);

        	/* Enable the GPMC Mapping */
        	__raw_writel(( ((OMAP34XX_GPMC_CS0_SIZE & 0xF)<<8) |
        		     ((ONENAND_BASE>>24) & 0x3F) |
        		     (1<<6) ),  (GPMC_CONFIG7 + GPMC_CONFIG_CS0));
        	delay(2000);

        	if (onenand_chip()){
#ifdef CFG_PRINTF
        		printf("OneNAND Unsupported !\n");
#endif
        		return 1;
        	}
	}
	return 0;
}


typedef int (mmc_boot_addr) (void);
int mmc_boot(unsigned char *buf)
{

       long size = 0;
#ifdef CFG_CMD_FAT
       block_dev_desc_t *dev_desc = NULL;
       unsigned char ret = 0;

       printf("Starting X-loader on MMC\n");

       ret = mmc_init(1);
       if(ret == 0){
               printf("\n MMC init failed\n");
               return 0;
       }

       dev_desc = mmc_get_dev(0);
       fat_register_device(dev_desc, 1);
       size = file_fat_read("u-boot.bin", buf, 0);
       if (size == -1) {
               return 0;
       }
       printf("\n%ld Bytes Read from MMC\n", size);

       printf("Starting OS Bootloader from MMC...\n");
#endif
       return size;
}

/* optionally do something like blinking LED */
void board_hang (void)
{ while (0) {};}
