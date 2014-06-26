/*
 * Copyright (C) 2005 Texas Instruments.
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
#include <asm/arch/omap2420.h>
#include <asm/arch/bits.h>

#include <asm/arch/mem.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>

static void wait_for_command_complete(unsigned int wd_base);
static void watchdog_init(void);
static void peripheral_enable(void);
static void muxSetupUART1(void);
static u32  get_cpu_rev(void);


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

#ifdef CFG_SDRAM_DDR
void
config_sdram_ddr(u32 rev)
{
	/* ball D11, mode 0 */
	__raw_writeb(0x08, 0x48000032);

	/* SDRC_CS0 Configuration */
	if (rev == CPU_2420_2422_ES1) {
		__raw_writel(H4_2422_SDRC_MDCFG_0_DDR, SDRC_MCFG_0);
		__raw_writel(H4_2422_SDRC_SHARING, SDRC_SHARING);
	} else {
		__raw_writel(H4_2420_SDRC_MDCFG_0_DDR, SDRC_MCFG_0);
		__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);
	}

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
	if (rev == CPU_2420_2422_ES1) {
		/* Enable DLL, Load counter with 115 (middle of range) */ 
		__raw_writel(0x00000002, SDRC_DLLA_CTRL);
		/* Enable DLL, Load counter with 128 (middle of range) */ 
		__raw_writel(0x00000002, SDRC_DLLB_CTRL);
	} else {
		/* Enable DLL, Load counter with 115 (middle of range) */ 
		__raw_writel(0x00000008, SDRC_DLLA_CTRL);	// ES2.x
		/* Enable DLL, Load counter with 128 (middle of range) */ 
		__raw_writel(0x00000008, SDRC_DLLB_CTRL);	// ES2.x
	}

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
        delay(200000);
        __raw_writel(0x00000010, SDRC_SYSCONFIG);

	/* SDRCTriState: no Tris */
        /* CS0MuxCfg: 000 (32-bit SDRAM on D31..0) */
	__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);


        /* CS0 SDRC Memory Configuration, */
        /* DDR-SDRAM, External SDRAM is x32bit, */
        /* Configure to MUX9: 1x8Mbx32  */
        __raw_writel(H4_2420_COMBO_MDCFG_0_DDR, SDRC_MCFG_0);
        __raw_writel(H4_2420_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
        __raw_writel(H4_2420_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);
	__raw_writel(H4_242x_SDRC_RFR_CTRL_ES1, SDRC_RFR_CTRL);

        /* Manual Command sequence */
        __raw_writel(CMD_NOP, SDRC_MANUAL_0);
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
	__raw_writel(H4_242x_SDRC_RFR_CTRL_ES1, 0x680090d4);

        /* Manual Command sequence */
        __raw_writel(CMD_NOP, SDRC_MANUAL_1);
        __raw_writel(CMD_PRECHARGE, SDRC_MANUAL_1);
        __raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_1);
        __raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_1);

        /* CS1 SDRC Mode Register */
	/* Burst length = 4 - DDR memory */
        /* Serial mode */
        /* CAS latency = 3 */
        __raw_writel(H4_2422_SDRC_MR_0_DDR, SDRC_MR_1);

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
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL & ~(LOADDLL|dllctrl), SDRC_DLLA_CTRL); // ES2.x
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL, SDRC_DLLB_CTRL);   // ES2.x ?
                __raw_writel(H4_2420_SDRC_DLLAB_CTRL & ~(LOADDLL|dllctrl), SDRC_DLLB_CTRL); // ES2.x
        }
}

#endif // CFG_SDRAM_COMBO

#ifdef CFG_SDRAM_SDR 
void
config_sdram_sdr(u32 rev)
{
	u32 dllctrl=0;

	/* ball D11, mode 0 */
	__raw_writeb(0x00, 0x48000032);

	__raw_writel(0x00000012, SDRC_SYSCONFIG);
	delay(200000);
	__raw_writel(0x00000010, SDRC_SYSCONFIG);

	/* Chip-level shared interface management */
	/* SDRCTriState: no Tris */
	/* CS0MuxCfg: 000 (32-bit SDRAM on D31..0) */
	/* CS1MuxCfg: 000 (32-bit SDRAM on D31..0) */
	if (rev == CPU_2420_2422_ES1)
		__raw_writel(H4_2422_SDRC_SHARING, SDRC_SHARING);
	else
		__raw_writel(H4_2420_SDRC_SHARING, SDRC_SHARING);

	/* CS0 SDRC Memory Configuration, */
	/* DDR-SDRAM, External SDRAM is x32bit, */
	/* Configure to MUX14: 32Mbx32 */
	__raw_writel(H4_2420_SDRC_MDCFG_0_SDR, SDRC_MCFG_0); /* diff from combo case */
	__raw_writel(H4_2420_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
	__raw_writel(H4_2420_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);
	__raw_writel(H4_242x_SDRC_RFR_CTRL_ES1, SDRC_RFR_CTRL);

	/* Manual Command sequence */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0);
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0);
 
	/* CS0 SDRC Mode Register */
	/* Burst length = 2 - SDR memory */
	/* Serial mode */
	/* CAS latency = 3  */
	__raw_writel(H4_2420_SDRC_MR_0_SDR, SDRC_MR_0);  /* diff from combo case */

	/* SDRC DLLA control register */
	/* Enable DLL, Load counter with 115 (middle of range) */ 
	/* Delay is 90 degrees */

	if (rev == CPU_242X_ES1)
                dllctrl = (BIT0|BIT3);
        else
                dllctrl = BIT0;

	if (rev == CPU_2420_2422_ES1) {
		__raw_writel(0x00007306, SDRC_DLLA_CTRL);
		__raw_writel(0x00007302, SDRC_DLLA_CTRL);
		/* Enable DLL, Load counter with 128 (middle of range) */ 
		__raw_writel(0x00007306, SDRC_DLLB_CTRL); /* load ctr value */
		__raw_writel(0x00007302, SDRC_DLLB_CTRL); /* lock and go */
	}
	else {
		__raw_writel(H4_2420_SDRC_DLLAB_CTRL, SDRC_DLLA_CTRL);	// ES2.x
		__raw_writel(H4_2420_SDRC_DLLAB_CTRL & ~(LOADDLL|dllctrl), SDRC_DLLA_CTRL);	// ES2.x
		/* Enable DLL, Load counter with 128 (middle of range) */ 
		__raw_writel(H4_2420_SDRC_DLLAB_CTRL, SDRC_DLLB_CTRL);	// ES2.x
		__raw_writel(H4_2420_SDRC_DLLAB_CTRL & ~(LOADDLL|dllctrl), SDRC_DLLB_CTRL);	// ES2.x
	}
   
}
#endif // CFG_SDRAM_SDR

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
	__raw_writel(CMD_NOP, SDRC_MANUAL_1);
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_1);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_1);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_1);

	/* CS0 SDRC Mode Register */
	/* Burst length = 4 - DDR memory */
	/* Serial mode */
	/* CAS latency = 3  */
	__raw_writel(0x00000032, SDRC_MR_1);
	__raw_writel(0x00000020, SDRC_EMR2_1);	/* weak-strength driver */

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
	muxSetupUART1();
	delay(100);

	/*DPLL out = 2x DPLL --> core clock */
	__raw_writel(DPLL_OUT, CM_CLKSEL2_PLL);

	/*DPLL into low power bypass (others off) */
	__raw_writel(0x00000001, CM_CLKEN_PLL);

	/*MPU core clock = Core /2 = 300 */
	__raw_writel(MPU_DIV, CM_CLKSEL_MPU);

	/*DSPif=200, DSPif=100, IVA=200 */
	__raw_writel(DSP_DIV, CM_CLKSEL_DSP);

	/*GFX clock (L3/2) 50MHz */
	__raw_writel(GFX_DIV, CM_CLKSEL_GFX);

	/*L3=100, L4=100, DisplaySS=50 Vlync=96Mhz,ssi=100, usb=50 */
	__raw_writel(BUS_DIV, CM_CLKSEL1_CORE);

	/*12MHz apll src, 12/(1+1)*50=300 */
	__raw_writel(DPLL_VAL, CM_CLKSEL1_PLL);

	/*Valid the configuration */
	__raw_writel(0x00000001, PRCM_CLKCFG_CTRL);
	delay(1000);

	/*Enable DPLL=300, 96MHz APLL locked. */
	__raw_writel(0x0000000F, CM_CLKEN_PLL);
	delay(200000);

#ifdef CFG_SDRAM_DDR
	config_sdram_ddr(rev);
#elif defined(CFG_SDRAM_COMBO)
	config_sdram_combo(rev);
#elif defined(CFG_SDRAM_SDR)
	config_sdram_sdr(rev);
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

#if MPU_WD_CLOCKED /* value 0x10 stick on aptix, BIT4 polarity seems oppsite*/
	__raw_writel(WD_UNLOCK1 ,WD3_BASE+WSPR);
	wait_for_command_complete(WD3_BASE);
	__raw_writel(WD_UNLOCK2 ,WD3_BASE+WSPR);

	__raw_writel(WD_UNLOCK1 ,WD4_BASE+WSPR);
	wait_for_command_complete(WD4_BASE);
	__raw_writel(WD_UNLOCK2 ,WD4_BASE+WSPR);
#endif

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
	unsigned int v, if_clks=0, func_clks=0;

	/* Enable GP2 timer.*/
	if_clks |= BIT4;
	func_clks |= BIT4;
	v = __raw_readl(CM_CLKSEL2_CORE) | 0x4;	/* Sys_clk input OMAP2420_GPT2 */
	__raw_writel(v, CM_CLKSEL2_CORE);
	__raw_writel(0x1, CM_CLKSEL_WKUP);

#ifdef CFG_NS16550
	/* Enable UART1 clock */
	func_clks |= BIT21;
	if_clks |= BIT21;
#endif
	v = __raw_readl(CM_ICLKEN1_CORE) | if_clks;	/* Interface clocks on */
	__raw_writel(v,CM_ICLKEN1_CORE );
	v = __raw_readl(CM_FCLKEN1_CORE) | func_clks; /* Functional Clocks on */
	__raw_writel(v, CM_FCLKEN1_CORE);
	delay(1000);

#ifndef KERNEL_UPDATED
	{
#define V1 0xffffffff
#define V2 0x00000007

		__raw_writel(V1, CM_FCLKEN1_CORE);
		__raw_writel(V2, CM_FCLKEN2_CORE);
		__raw_writel(V1, CM_ICLKEN1_CORE);
		__raw_writel(V1, CM_ICLKEN2_CORE);
	}
#endif

}

/* Pin Muxing registers used for UART1 */
#define CONTROL_PADCONF_UART1_CTS       ((volatile unsigned char *)0x480000C5)
#define CONTROL_PADCONF_UART1_RTS       ((volatile unsigned char *)0x480000C6)
#define CONTROL_PADCONF_UART1_TX        ((volatile unsigned char *)0x480000C7)
#define CONTROL_PADCONF_UART1_RX        ((volatile unsigned char *)0x480000C8)
/****************************************
 * Routine: muxSetupUART1  (ostboot)
 * Description: Set up uart1 muxing
 *****************************************/
static void muxSetupUART1(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* UART1_CTS pin configuration, PIN = D21 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_CTS;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* UART1_RTS pin configuration, PIN = H21 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_RTS;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* UART1_TX pin configuration, PIN = L20 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_TX;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* UART1_RX pin configuration, PIN = T21 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_RX;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
}

int nand_init(void)
{
	u32	rev;

	rev = get_cpu_rev();


	/* GPMC pin muxing */ 
	(*(volatile int*)0x48000070) &= 0x000000FF;
	(*(volatile int*)0x48000074) &= 0x00000000;
	(*(volatile int*)0x48000078) &= 0x00000000;
	(*(volatile int*)0x4800007C) &= 0x00000000;
	(*(volatile int*)0x48000080) &= 0xFF000000;

	/* GPMC_IO_DIR */
	(*(volatile int*)0x4800008C) = 0x19000000;

	/* GPMC Configuration */
	(*(volatile int*)0x6800A010) = 0x0000000A;
	while (((*(volatile int *)0x6800A014) & 0x00000001) == 0);

	(*(volatile int*)0x6800A050) = 0x00000001;	
	(*(volatile int*)0x6800A060) = 0x00001800;	
	(*(volatile int*)0x6800A064) = 0x00141400;	
	(*(volatile int*)0x6800A068) = 0x00141400;	
	(*(volatile int*)0x6800A06C) = 0x0F010F01;	
	(*(volatile int*)0x6800A070) = 0x010C1414;
	(*(volatile int*)0x6800A074) = 0x00000A80;
	(*(volatile int*)0x6800A078) = 0x00000C44; 	//base 0x04000000

	(*(volatile int*)0x6800A0A8) = 0x00000000;
	delay(1000);
#ifdef CFG_SDRAM_STACKED
	(*(volatile int*)0x6800A090) = 0x00011000;
#else
	(*(volatile int*)0x6800A090) = 0x00011200;
#endif
	(*(volatile int*)0x6800A094) = 0x001f1f00;
	(*(volatile int*)0x6800A098) = 0x00080802;
	(*(volatile int*)0x6800A09C) = 0x1C091C09;
	(*(volatile int*)0x6800A0A0) = 0x031A1F1F;
	(*(volatile int*)0x6800A0A4) = 0x000003C2;
	(*(volatile int*)0x6800A0A8) = 0x00000F48;

	if (rev != CPU_2420_2422_ES1)
		(*(volatile int*)0x6800A040) = 0x1FF0;	// es2.x

 	if (nand_chip()){
		printf("Unsupported Chip!\n");
		return 1;
	}
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

/* optionally do something like blinking LED */
void board_hang (void)
{}



