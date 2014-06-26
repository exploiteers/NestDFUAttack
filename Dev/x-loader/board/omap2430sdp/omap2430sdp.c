/*
 * (C) Copyright 2004-2005
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
#include <asm/arch/omap2430.h>
#include <asm/arch/bits.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>

static void wait_for_command_complete(unsigned int wd_base);
static void watchdog_init(void);
static void peripheral_enable(void);
static void muxSetupAll(void);
static u32  get_cpu_rev(void);
static u32  get_device_type(void);
static void prcm_init(void);


/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
					  "subs %0, %1, #1\n"
					  "bne 1b":"=r" (loops):"0" (loops));
}

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init (void)
{
	return 0;
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
        u32 v;
        v = __raw_readl(TAP_IDCODE_REG);
        v = v >> 28;
        return(v+1);  /* currently 2422 and 2420 match up */
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

/*************************************************************
 *  Helper function to wait for the status of a register
 *************************************************************/
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
{
        u32 i = 0, val;
        do {
                ++i;
                val = __raw_readl(read_addr) & read_bit_mask;
                if (val == match_value)
                        return(1);
                if (i==bound)
                        return(0);
        } while (1);
}

/*************************************************************
 *  Support for multiple type of memory types
 *************************************************************/
#ifdef CFG_SDRAM_DDR
void
config_sdram_ddr(u32 rev)
{
	/* ball D11, mode 0 */
	__raw_writeb(0x08, 0x48000032);

	/* SDRC_CS0 Configuration */
	__raw_writel(H4_2420_SDRC_MDCFG_0_DDR, SDRC_MCFG_0);
	__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);

	__raw_writel(H4_242x_SDRC_RFR_CTRL_ES1, SDRC_RFR_CTRL);
	__raw_writel(H4_242x_SDRC_ACTIM_CTRLA_0_ES1, SDRC_ACTIM_CTRLA_0);
	__raw_writel(H4_242x_SDRC_ACTIM_CTRLB_0_ES1, SDRC_ACTIM_CTRLB_0);

	/* Manual Command sequence */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0);
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);


	/* 
	 * CS0 SDRC Mode Register
	 * Burst length = 4 - DDR memory
	 * Serial mode
	 * CAS latency = 3 
	 */
	__raw_writel(0x00000032, SDRC_MR_0);

 	/* SDRC DLLA control register */
	/* Delay is 90 degrees */
	/* Enable DLL, Load counter with 115 (middle of range) */ 
	__raw_writel(0x00000008, SDRC_DLLA_CTRL);	// ES2.x
	/* Enable DLL, Load counter with 128 (middle of range) */ 
	__raw_writel(0x00000008, SDRC_DLLB_CTRL);	// ES2.x

}
#endif // CFG_SDRAM_DDR

#ifdef CFG_SDRAM_COMBO 
void
config_sdram_combo(u32 rev)
{

	u32 dllctrl=0;

        /* ball C12, mode 0 */
        __raw_writeb(0x00, 0x480000a1);
        /* ball D11, mode 0 */
        __raw_writeb(0x00, 0x48000032);
        /* ball B13, mode 0 - for CKE1 (not needed rkw for combo) */
        __raw_writeb(0x00, 0x480000a3);

        /*configure sdrc 32 bit for COMBO ddr sdram. Issue soft reset */
        __raw_writel(0x00000012, SDRC_SYSCONFIG);
        wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000); /* wait till reset done set */
        __raw_writel(0x00000000, SDRC_SYSCONFIG);

        /* SDRCTriState: no Tris */
        /* CS0MuxCfg: 000 (32-bit SDRAM on D31..0) */
	if (rev == CPU_2420_2422_ES1)
	        __raw_writel(H4_2422_SDRC_SHARING, SDRC_SHARING);
	else
       		__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);


        /* CS0 SDRC Memory Configuration, */
        /* DDR-SDRAM, External SDRAM is x32bit, */
        /* Configure to MUX9: 1x8Mbx32  */
        __raw_writel(H4_2420_COMBO_MDCFG_0_DDR, SDRC_MCFG_0);
        __raw_writel(H4_2420_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
        __raw_writel(H4_2420_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);
	
	/* This is reqd only for ES1 */
	if (rev == CPU_242X_ES1)
        	__raw_writel(H4_242x_SDRC_RFR_CTRL_ES1, SDRC_RFR_CTRL);

        /* Manual Command sequence */
        __raw_writel(CMD_NOP, SDRC_MANUAL_0);
	delay(5000);
        __raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0);
        __raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
        __raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);

        /* CS0 SDRC Mode Register */
        /* Burst length = 4 - DDR memory */
        /* Serial mode */
        /* CAS latency = 3  */
        __raw_writel(H4_2422_SDRC_MR_0_DDR, SDRC_MR_0);

        /* CS1 SDRC Memory Configuration, */
        /* DDR-SDRAM, External SDRAM is x32bit, */
        /* Configure to MUX9: 1x8Mbx32 */
        __raw_writel(H4_2420_COMBO_MDCFG_0_DDR, SDRC_MCFG_1);
        __raw_writel(H4_242X_SDRC_ACTIM_CTRLA_0_100MHz, SDRC_ACTIM_CTRLA_1);
        __raw_writel(H4_2420_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_1);
	/* This is reqd only for ES1 */
	if (rev == CPU_242X_ES1)
        	__raw_writel(H4_242x_SDRC_RFR_CTRL_ES1, 0x680090d4);

        /* Manual Command sequence */
        __raw_writel(CMD_NOP, 0x680090d8);
        __raw_writel(CMD_PRECHARGE, 0x680090d8);
        __raw_writel(CMD_AUTOREFRESH, 0x680090d8);
        __raw_writel(CMD_AUTOREFRESH, 0x680090d8);

        /* CS1 SDRC Mode Register */
	/* Burst length = 4 - DDR memory */
        /* Serial mode */
        /* CAS latency = 3 */
        __raw_writel(H4_2422_SDRC_MR_0_DDR, 0x680090b4);

        /* SDRC DLLA control register */
        /* Delay is 90 degrees */

	if (rev == CPU_242X_ES1)
		dllctrl = (BIT0|BIT3);
	else
		dllctrl = BIT0;		

        if (rev == CPU_2420_2422_ES1) {
                /* Enable DLL, Load counter with 115 (middle of range) */
                __raw_writel(0x00007306, SDRC_DLLA_CTRL);
                __raw_writel(0x00007302, SDRC_DLLA_CTRL);
                /* Enable DLL, Load counter with 128 (middle of range)  */
                __raw_writel(0x00007306, SDRC_DLLB_CTRL); /* load ctr value */
                __raw_writel(0x00007302, SDRC_DLLB_CTRL); /* lock and go */
        }
        else {
                /* Enable DLL, Load counter with 115 (middle of range) */
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL, SDRC_DLLA_CTRL);   // ES2.x
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL & ~(LOADDLL|dllctrl), SDRC_DLLA_CTRL);   // ES2.x
               // __raw_writel(0x00009808, SDRC_DLLA_CTRL);   // ES2.x
                /* Enable DLL, Load counter with 128 (middle of range) */
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL, SDRC_DLLB_CTRL);   // ES2.x ?
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL & ~(LOADDLL|dllctrl), SDRC_DLLB_CTRL);   // ES2.x
                //__raw_writel(0x00009808, SDRC_DLLB_CTRL);   // ES2.x
        }
}

#endif // CFG_SDRAM_COMBO

#ifdef CFG_2430SDRAM_DDR
void
config_2430sdram_ddr(u32 rev)
{
	u32 dllstat, dllctrl;

        __raw_writel(0x00000012, SDRC_SYSCONFIG);
        wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000); /* wait till reset done set */
        __raw_writel(0x00000000, SDRC_SYSCONFIG);

	/* Chip-level shared interface management */
	/* SDRCTriState: no Tris */
	/* CS0MuxCfg: 000 (32-bit SDRAM on D31..0) */
	/* CS1MuxCfg: 000 (32-bit SDRAM on D31..0) */
	__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);

	/* CS0 SDRC Memory Configuration, */
	/* DDR-SDRAM, External SDRAM is x32bit, */
	/* Configure to MUX14: 32Mbx32 */
	__raw_writel(SDP_2430_SDRC_MDCFG_0_DDR, SDRC_MCFG_0);
	__raw_writel(SDP_2430_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
	__raw_writel(H4_2420_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);

	__raw_writel(H4_2420_SDRC_RFR_CTRL, SDRC_RFR_CTRL);

	/* Manual Command sequence */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0);
	delay(5000);
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
 
	/* CS0 SDRC Mode Register */
	/* Burst length = 2 - SDR memory */
	/* Serial mode */
	/* CAS latency = 3  */
	__raw_writel(H4_2420_SDRC_MR_0_DDR, SDRC_MR_0);

	/* Set up SDRC DLL values for 2430 DDR */
	dllctrl = (SDP_2430_SDRC_DLLAB_CTRL & ~BIT2); /* set target ctrl val */
	__raw_writel(dllctrl, SDRC_DLLA_CTRL);	/* set lock mode */
	__raw_writel(dllctrl, SDRC_DLLB_CTRL);	/* set lock mode */
	delay(0x1000); /* time to track to center */	
	dllstat = __raw_readl(SDRC_DLLA_STATUS) & 0xFF00; /* get status */
	dllctrl = (dllctrl & 0x00FF) | dllstat | BIT2; /* build unlock value */
	__raw_writel(dllctrl, SDRC_DLLA_CTRL);	/* set unlock mode */
	__raw_writel(dllctrl, SDRC_DLLB_CTRL);	/* set unlock mode */
}
#endif // CFG_2430SDRAM_DDR

#ifdef CFG_SDRAM_STACKED 
void
config_sdram_stacked(u32 rev)
{

	/* Pin Muxing for SDRC */
	__raw_writeb(0x00, 0x480000a1);	/* mux mode 0 (CS1) */
	__raw_writeb(0x00, 0x480000a3);	/* mux mode 0 (CKE1) */
	__raw_writeb(0x00, 0x48000032);	/* connect sdrc_a12 */
	__raw_writeb(0x00, 0x48000031);	/* connect sdrc_a13 */

	/* configure sdrc 32 bit for COMBO ddr sdram */
	__raw_writel(0x00000010, SDRC_SYSCONFIG);	/* no idle ack and RESET enable */
	delay(200000);
	__raw_writel(0x00000010, SDRC_SYSCONFIG);	/* smart idle mode */

	/* SDRC_SHARING */
	/* U-boot is writing 0x00000100 though (H4_2420_SDRC_SHARING ) */
	//__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);

	__raw_writel(0x00004900, SDRC_SHARING);

	/* SDRC_CS0 Configuration */
	/* None for ES2.1 */

	/*  SDRC_CS1 Configuration */
	__raw_writel(0x00000000, SDRC_CS_CFG);	/* Remap CS1 to 0x80000000 */

	/* Disable power down of CKE */
	__raw_writel(0x00000085, SDRC_POWER);

	__raw_writel(0x01A02019, SDRC_MCFG_1);	/* SDRC_MCFG1 */
	__raw_writel(0x0003DD03, SDRC_RFR_CTRL1);	/* SDRC_RFR_CTRL1 */
	__raw_writel(0x92DDC485, SDRC_ACTIM_CTRLA_1);	/* SDRC_ACTIM_CTRLA0 */
	__raw_writel(0x00000014, SDRC_ACTIM_CTRLB_1);	/* SDRC_ACTIM_CTRLB0 */

	/*Manual Command sequence */
	__raw_writel(0x00000000, 0x680090D8);
	__raw_writel(0x00000001, 0x680090D8);
	__raw_writel(0x00000002, 0x680090D8);
	__raw_writel(0x00000002, 0x680090D8);

	/* CS0 SDRC Mode Register */
	/* Burst length = 4 - DDR memory */
	/* Serial mode */
	/* CAS latency = 3  */
	__raw_writel(0x00000032, 0x680090B4);
	__raw_writel(0x00000020, 0x680090BC);	/* weak-strength driver */

 	/* SDRC DLLA control register */
	/* Delay is 90 degrees */
	if (rev == CPU_2420_2422_ES1) {
		/* Enable DLL, Load counter with 115 (middle of range) */ 
		__raw_writel(0x00007302, SDRC_DLLA_CTRL);
		/* Enable DLL, Load counter with 128 (middle of range) */ 
		__raw_writel(0x00007302, SDRC_DLLB_CTRL);
	}
	else {
		/* Enable DLL, Load counter with 115 (middle of range) */ 
		__raw_writel(0x00003108, SDRC_DLLA_CTRL);	// ES2.x
		/* Enable DLL, Load counter with 128 (middle of range) */ 
		__raw_writel(0x00003108, SDRC_DLLB_CTRL);	// ES2.x
	}
}
#endif // CFG_SDRAM_STACKED

/*************************************************************
 * get_sys_clk_speed - determine reference oscillator speed
 *  based on known 32kHz clock and gptimer.
 *************************************************************/
u32 get_osc_clk_speed(u32 *shift)
{
#define GPT_EN  ((0<<2)|BIT1|BIT0)      /* enable sys_clk NO-prescale /1 */
#define GPT_CTR OMAP24XX_GPT2+TCRR      /* read counter address */
        u32 start, cstart, cend, cdiff, val;
	unsigned int v, if_clks=0, func_clks=0 ;



        if(__raw_readl(PRCM_CLKSRC_CTRL) & BIT7){    /* if currently /2 */
                *shift = 1;
        }else{
                *shift = 0;
        }

	/* enable timer2 */
	val = __raw_readl(CM_CLKSEL2_CORE) | 0x4;      /* mask for sys_clk use */
	__raw_writel(val, CM_CLKSEL2_CORE);            /* timer2 source to sys_clk */
	__raw_writel(BIT4, CM_ICLKEN1_CORE);           /* timer2 interface clock on */
	__raw_writel(BIT4, CM_FCLKEN1_CORE);           /* timer2 function clock on */
	/* Enable GP2 timer.*/
	if_clks |= BIT4;
	func_clks |= BIT4;
	v = __raw_readl(CM_ICLKEN1_CORE) | if_clks;	/* Interface clocks on */
	__raw_writel(v,CM_ICLKEN1_CORE );
	v = __raw_readl(CM_FCLKEN1_CORE) | func_clks; /* Functional Clocks on */
	__raw_writel(v, CM_FCLKEN1_CORE);
        __raw_writel(0, OMAP24XX_GPT2+TLDR);           /* start counting at 0 */
        __raw_writel(GPT_EN, OMAP24XX_GPT2+TCLR);      /* enable clock */
        /* enable 32kHz source */                      /* enabled out of reset */
        /* determine sys_clk via gauging */
        start = 20 + __raw_readl(S32K_CR);             /* start time in 20 cycles*/
        while(__raw_readl(S32K_CR) < start);           /* dead loop till start time */
        cstart = __raw_readl(GPT_CTR);                 /* get start sys_clk count */
        while(__raw_readl(S32K_CR) < (start+20));      /* wait for 40 cycles */
        cend = __raw_readl(GPT_CTR);                   /* get end sys_clk count */
        cdiff = cend - cstart;                         /* get elapsed ticks */
        /* based on number of ticks assign speed */
        if(cdiff > (19000 >> *shift))
                return(S38_4M);
        else if (cdiff > (15200 >> *shift))
                return(S26M);
        else if (cdiff > (13000 >> *shift))
                return(S24M);
        else if (cdiff > (9000 >> *shift))
                return(S19_2M);
        else if (cdiff > (7600 >> *shift))
                return(S13M);
        else
                return(S12M);
}

/*********************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h (config II default).
 *   -- called from SRAM
 *********************************************************************************/
void
prcm_init()
{
	u32 div, speed, val, div_by_2;

	val = __raw_readl(PRCM_CLKSRC_CTRL) & ~(BIT1 | BIT0);
#if defined(OMAP2430_SQUARE_CLOCK_INPUT)
	__raw_writel(val, PRCM_CLKSRC_CTRL);
#else
	__raw_writel((val | BIT0), PRCM_CLKSRC_CTRL);
#endif
	speed = get_osc_clk_speed(&div_by_2);
        if((speed > S19_2M) && (!div_by_2)){ /* if fast && /2 off, enable it */
                val = ~(BIT6|BIT7) & __raw_readl(PRCM_CLKSRC_CTRL);
                val |= (0x2 << 6); /* divide by 2 if (24,26,38.4) -> (12/13/19.2)  */
                __raw_writel(val, PRCM_CLKSRC_CTRL);
        }

	__raw_writel(0, CM_FCLKEN1_CORE);	/* stop all clocks to reduce ringing */
	__raw_writel(0, CM_FCLKEN2_CORE);	/* may not be necessary */
	__raw_writel(0, CM_ICLKEN1_CORE);
	__raw_writel(0, CM_ICLKEN2_CORE);

	/*DPLL into low power bypass (others off) */
	__raw_writel(0x00000001, CM_CLKEN_PLL);

	__raw_writel(DPLL_OUT, CM_CLKSEL2_PLL); /* set DPLL out */
        __raw_writel(MPU_DIV, CM_CLKSEL_MPU);   /* set MPU divider */
        __raw_writel(DSP_DIV, CM_CLKSEL_DSP);   /* set dsp and iva dividers */
        __raw_writel(GFX_DIV, CM_CLKSEL_GFX);   /* set gfx dividers */
        __raw_writel(MDM_DIV, CM_CLKSEL_MDM);   /* set mdm dividers */

	div = BUS_DIV;
	__raw_writel(div, CM_CLKSEL1_CORE);/* set L3/L4/USB/Display/SSi dividers */
	delay(1000);

	/*13MHz apll src, PRCM 'x' DPLL rate */
	__raw_writel(DPLL_VAL, CM_CLKSEL1_PLL);

	/*Valid the configuration */
	__raw_writel(0x00000001, PRCM_CLKCFG_CTRL);
	delay(1000);

	/* set up APLLS_CLKIN per crystal */
	if (speed > S19_2M)
                speed >>= 1;    /* if fast shift to /2 range */
        val = (0x2 << 23);      /* default to 13Mhz for 2430c */
        if (speed == S12M)
                val = (0x3 << 23);
        else if (speed == S19_2M)
                val = (0x0 << 23);
        val |= (~(BIT23|BIT24|BIT25) & __raw_readl(CM_CLKSEL1_PLL));
        __raw_writel(val, CM_CLKSEL1_PLL);

        __raw_writel(DPLL_LOCK|APLL_LOCK, CM_CLKEN_PLL);   /* enable apll */
        wait_on_value(BIT8, BIT8, CM_IDLEST_CKGEN, LDELAY);/* wait for apll lock */

        delay(200000);
}

void SEC_generic(void)
{
/* Permission values for registers -Full fledged permissions to all */
#define UNLOCK_1 0xFFFFFFFF
#define UNLOCK_2 0x00000000
#define UNLOCK_3 0x0000FFFF
	/* Protection Module Register Target APE (PM_RT)*/
	__raw_writel(UNLOCK_1, PM_RT_APE_BASE_ADDR_ARM + 0x68); /* REQ_INFO_PERMISSION_1 L*/
	__raw_writel(UNLOCK_1, PM_RT_APE_BASE_ADDR_ARM + 0x50);  /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_1, PM_RT_APE_BASE_ADDR_ARM + 0x58);  /* WRITE_PERMISSION_0 L*/
	__raw_writel(UNLOCK_2, PM_RT_APE_BASE_ADDR_ARM + 0x60); /* ADDR_MATCH_1 L*/


	__raw_writel(UNLOCK_3, PM_GPMC_BASE_ADDR_ARM + 0x48); /* REQ_INFO_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_GPMC_BASE_ADDR_ARM + 0x50); /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_GPMC_BASE_ADDR_ARM + 0x58); /* WRITE_PERMISSION_0 L*/

	__raw_writel(UNLOCK_3, PM_OCM_RAM_BASE_ADDR_ARM + 0x48); /* REQ_INFO_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_OCM_RAM_BASE_ADDR_ARM + 0x50); /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_OCM_RAM_BASE_ADDR_ARM + 0x58); /* WRITE_PERMISSION_0 L*/
	__raw_writel(UNLOCK_2, PM_OCM_RAM_BASE_ADDR_ARM + 0x80);  /* ADDR_MATCH_2 L*/

	/* IVA Changes */
	__raw_writel(UNLOCK_3, PM_IVA2_BASE_ADDR_ARM + 0x48); /* REQ_INFO_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_IVA2_BASE_ADDR_ARM + 0x50); /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_IVA2_BASE_ADDR_ARM + 0x58); /* WRITE_PERMISSION_0 L*/
}

/**********************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP type, unlock the SRAM for general use.
 ***********************************************************/
void try_unlock_sram(void)
{
        int mode;

        /* if GP device unlock device SRAM for general use */
        mode = get_device_type();

        if ((mode == GP_DEVICE) || (mode == HS_DEVICE) || (mode == EMU_DEVICE)
            || (mode == TST_DEVICE)) {
                /* Secure or Emulation device - HS/E/T */
                SEC_generic();
        }
        return;
}

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called at time when only stack is available.
 **********************************************************/

int s_init(int skip)
{
        u32   rev;

        rev = get_cpu_rev();

        watchdog_init();
	try_unlock_sram();   
        muxSetupAll();
        delay(100);
	prcm_init();

#ifdef CFG_SDRAM_DDR
        config_sdram_ddr(rev);
#elif defined(CFG_SDRAM_COMBO)
        config_sdram_combo(rev);
#elif defined(CFG_2430SDRAM_DDR)
        config_2430sdram_ddr(rev);
#elif defined(CFG_SDRAM_STACKED)
        config_sdram_stacked(rev);
#else
#error SDRAM type not supported
#endif

        delay(20000);
        peripheral_enable();
        return(0);
}

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r (void)
{
	return(0);
}

/****************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************/
static void watchdog_init(void)
{
#define GP (BIT8|BIT9)

	/* There are 4 watch dogs.  1 secure, and 3 general purpose.
	 * I would expect that the ROM takes care of the secure one,
	 * but we will try also.  Of the 3 GP ones, 1 can reset us
	 * directly, the other 2 only generate MPU interrupts.
	 */
	__raw_writel(WD_UNLOCK1 ,WD2_BASE+WSPR);
	wait_for_command_complete(WD2_BASE);
	__raw_writel(WD_UNLOCK2 ,WD2_BASE+WSPR);

}

/******************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 ******************************************************/
static void wait_for_command_complete(unsigned int wd_base)
{
	int pending = 1;
	do {
		pending = __raw_readl(wd_base+WWPS);
	} while (pending);
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
static void peripheral_enable(void)
{
	unsigned int v, if_clks=0, if_clks2 = 0, func_clks=0, func_clks2 = 0;

	/* Enable GP2 timer.*/
	if_clks |= BIT4;
	func_clks |= BIT4;
	v = __raw_readl(CM_CLKSEL2_CORE) | 0x4;	/* Sys_clk input OMAP24XX_GPT2 */
	__raw_writel(v, CM_CLKSEL2_CORE);
	__raw_writel(0x1, CM_CLKSEL_WKUP);

#ifdef CFG_NS16550
	/* Enable UART1 clock */
	func_clks |= BIT21;
	if_clks |= BIT21;
#endif
	v = __raw_readl(CM_ICLKEN1_CORE) | if_clks;	/* Interface clocks on */
	__raw_writel(v,CM_ICLKEN1_CORE );
	v = __raw_readl(CM_ICLKEN2_CORE) | if_clks2;    /* Interface clocks on */
        __raw_writel(v, CM_ICLKEN2_CORE);
	v = __raw_readl(CM_FCLKEN1_CORE) | func_clks; /* Functional Clocks on */
	__raw_writel(v, CM_FCLKEN1_CORE);
	v = __raw_readl(CM_FCLKEN2_CORE) | func_clks2;  /* Functional Clocks on */
        __raw_writel(v, CM_FCLKEN2_CORE);
	delay(1000);

}

/* Do pin muxing for all the devices used in X-Loader */
#define MUX_VAL(OFFSET,VALUE)\
                __raw_writeb(VALUE, OMAP24XX_CTRL_BASE + OFFSET);
static void muxSetupAll(void)
{
	/* UART 1*/
	MUX_VAL(0x00B1, 0x1B)        /* uart1_cts- EN, HI, 3, ->gpio_32 */
	MUX_VAL(0x00B2, 0x1B)        /* uart1_rts- EN, HI, 3, ->gpio_8 */
	MUX_VAL(0x00B3, 0x1B)        /* uart1_tx- EN, HI, 3, ->gpio_9 */
	MUX_VAL(0x00B4, 0x1B)        /* uart1_rx- EN, HI, 3, ->gpio_10 */
	MUX_VAL(0x0107, 0x01)        /* ssi1_dat_tx- Dis, 1, ->uart1_tx */
	MUX_VAL(0x0108, 0x01)        /* ssi1_flag_tx- Dis, 1, ->uart1_rts */
	MUX_VAL(0x0109, 0x01)        /* ssi1_rdy_tx- Dis, 1, ->uart1_cts */
	MUX_VAL(0x010A, 0x01)        /* ssi1_dat_rx- Dis, 1, ->uart1_rx */

	/* Mux settings for SDRC */
	MUX_VAL(0x0054, 0x1B)        /* sdrc_a14 - EN, HI, 3, ->gpio_0 */
	MUX_VAL(0x0055, 0x1B)        /* sdrc_a13 - EN, HI, 3, ->gpio_1 */
	MUX_VAL(0x0056, 0x00)        /* sdrc_a12 - Dis, 0 */
	MUX_VAL(0x0046, 0x00)        /* sdrc_ncs1 - Dis, 0 */
	MUX_VAL(0x0048, 0x00)        /* sdrc_cke1 - Dis, 0 */
	/* GPMC */
	MUX_VAL(0x0030, 0x00)        /* gpmc_clk - Dis, 0 */
	MUX_VAL(0x0032, 0x00)        /* gpmc_ncs1- Dis, 0 */
	MUX_VAL(0x0033, 0x00)        /* gpmc_ncs2- Dis, 0 */
	MUX_VAL(0x0034, 0x03)        /* gpmc_ncs3- Dis, 3, ->gpio_24 */
	MUX_VAL(0x0035, 0x03)        /* gpmc_ncs4- Dis, 3, ->gpio_25 */
	MUX_VAL(0x0036, 0x00)        /* gpmc_ncs5- Dis, 0 */
	MUX_VAL(0x0037, 0x03)        /* gpmc_ncs6- Dis, 3, ->gpio_27 */
	MUX_VAL(0x0038, 0x00)        /* gpmc_ncs7- Dis, 0 */
	MUX_VAL(0x0040, 0x18)        /* gpmc_wait1- Dis, 0 */
	MUX_VAL(0x0041, 0x18)        /* gpmc_wait2- Dis, 0 */
	MUX_VAL(0x0042, 0x1B)        /* gpmc_wait3- EN, HI, 3, ->gpio_35 */
	MUX_VAL(0x0085, 0x1B)        /* gpmc_a10- EN, HI, 3, ->gpio_3 */
}

int nand_init(void)
{
	u32	rev;

	/* GPMC Configuration */
	rev  = get_cpu_rev();

	/* global settings */
	__raw_writel(0x10, GPMC_SYSCONFIG);	/* smart idle */
	__raw_writel(0x0, GPMC_IRQENABLE);	/* isr's sources masked */
	__raw_writel(0, GPMC_TIMEOUT_CONTROL);/* timeout disable */
#ifdef CFG_NAND
	__raw_writel(0x001, GPMC_CONFIG);	/* set nWP, disable limited addr */
#endif

	/* Set the GPMC Vals . For NAND boot on 2430SDP, NAND is mapped at CS0
         *  , NOR at CS1 and MPDB at CS5. And oneNAND boot, we map oneNAND at CS0.
	 *  We configure only GPMC CS0 with required values. Configiring other devices 
	 *  at other CS in done in u-boot anyway. So we don't have to bother doing it here.
         */
	__raw_writel(0, GPMC_CONFIG7_0);
	sdelay(1000);

#ifdef CFG_NAND
	__raw_writel( SMNAND_GPMC_CONFIG1, GPMC_CONFIG1_0);
	__raw_writel( SMNAND_GPMC_CONFIG2, GPMC_CONFIG2_0);
	__raw_writel( SMNAND_GPMC_CONFIG3, GPMC_CONFIG3_0);
	__raw_writel( SMNAND_GPMC_CONFIG4, GPMC_CONFIG4_0);
	__raw_writel( SMNAND_GPMC_CONFIG5, GPMC_CONFIG5_0);
	__raw_writel( SMNAND_GPMC_CONFIG6, GPMC_CONFIG6_0);

#else /* CFG_ONENAND */
	__raw_writel( ONENAND_GPMC_CONFIG1, GPMC_CONFIG1_0);
	__raw_writel( ONENAND_GPMC_CONFIG2, GPMC_CONFIG2_0);
	__raw_writel( ONENAND_GPMC_CONFIG3, GPMC_CONFIG3_0);
	__raw_writel( ONENAND_GPMC_CONFIG4, GPMC_CONFIG4_0);
	__raw_writel( ONENAND_GPMC_CONFIG5, GPMC_CONFIG5_0);
	__raw_writel( ONENAND_GPMC_CONFIG6, GPMC_CONFIG6_0);
#endif

	/* Enable the GPMC Mapping */
	__raw_writel(( ((OMAP24XX_GPMC_CS0_SIZE & 0xF)<<8) |
		     ((OMAP24XX_GPMC_CS0_MAP>>24) & 0x3F) |
		     (1<<6) ), GPMC_CONFIG7_0);
	sdelay(2000);
#ifdef CFG_NAND
 	if (nand_chip()){
#ifdef CFG_PRINTF
		printf("Unsupported Chip!\n");
#endif
		return 1;
	}
#else
	if (onenand_chip()){
#ifdef CFG_PRINTF
		printf("OneNAND Unsupported !\n");
#endif
		return 1;
	}

#endif
	return 0;
}

/* optionally do something like blinking LED */
void board_hang (void)
{ while (0) {};}
