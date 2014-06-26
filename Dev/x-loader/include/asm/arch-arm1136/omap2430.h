/* 
 * (C) Copyright 2004-2005
 * Texas Instruments, <www.ti.com>
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

#ifndef _OMAP2430_SYS_H_
#define _OMAP2430_SYS_H_

#include <asm/arch/sizes.h>

/* device type */
#define DEVICE_MASK          (BIT8|BIT9|BIT10)
#define TST_DEVICE           0x0
#define EMU_DEVICE           0x1
#define HS_DEVICE            0x2
#define GP_DEVICE            0x3

/*
 * 2430 specific Section
 */
#define OMAP243X_CORE_L4_IO_BASE        0x48000000
#define OMAP243X_WAKEUP_L4_IO_BASE      0x49000000
#define OMAP24XX_L4_IO_BASE     OMAP243X_CORE_L4_IO_BASE


/* CONTROL */
#define OMAP24XX_CTRL_BASE    (OMAP243X_WAKEUP_L4_IO_BASE+0x2000)
#define CONTROL_STATUS        (OMAP24XX_CTRL_BASE + 0x2F8)

/* TAP information */
#define OMAP24XX_TAP_BASE     (OMAP243X_WAKEUP_L4_IO_BASE+0xA000)
#define TAP_IDCODE_REG        (OMAP24XX_TAP_BASE+0x204)

/* 
	GPMC : In 2430 NOR and NAND can coexist.
	During NAND booting , NAND is at CS0 and NOR at CS1
	and Debug FPGA is GPMC_CS5 
*/
#define OMAP24XX_GPMC_BASE    (0x6E000000)

#define GPMC_SYSCONFIG        (OMAP24XX_GPMC_BASE+0x10)
#define GPMC_SYSSTATUS	      (OMAP24XX_GPMC_BASE+0x14)
#define GPMC_IRQENABLE        (OMAP24XX_GPMC_BASE+0x1C)
#define GPMC_TIMEOUT_CONTROL  (OMAP24XX_GPMC_BASE+0x40)
#define GPMC_CONFIG           (OMAP24XX_GPMC_BASE+0x50)
#define GPMC_CONFIG1_0        (OMAP24XX_GPMC_BASE+0x60)
#define GPMC_CONFIG2_0        (OMAP24XX_GPMC_BASE+0x64)
#define GPMC_CONFIG3_0        (OMAP24XX_GPMC_BASE+0x68)
#define GPMC_CONFIG4_0        (OMAP24XX_GPMC_BASE+0x6C)
#define GPMC_CONFIG5_0        (OMAP24XX_GPMC_BASE+0x70)
#define GPMC_CONFIG6_0        (OMAP24XX_GPMC_BASE+0x74)
#define GPMC_CONFIG7_0	      (OMAP24XX_GPMC_BASE+0x78)
#define GPMC_CONFIG1_1        (OMAP24XX_GPMC_BASE+0x90)
#define GPMC_CONFIG2_1        (OMAP24XX_GPMC_BASE+0x94)
#define GPMC_CONFIG3_1        (OMAP24XX_GPMC_BASE+0x98)
#define GPMC_CONFIG4_1        (OMAP24XX_GPMC_BASE+0x9C)
#define GPMC_CONFIG5_1        (OMAP24XX_GPMC_BASE+0xA0)
#define GPMC_CONFIG6_1        (OMAP24XX_GPMC_BASE+0xA4)
#define GPMC_CONFIG7_1	      (OMAP24XX_GPMC_BASE+0xA8)
#define GPMC_CONFIG1_5        (OMAP24XX_GPMC_BASE+0x150)
#define GPMC_CONFIG2_5        (OMAP24XX_GPMC_BASE+0x154)
#define GPMC_CONFIG3_5        (OMAP24XX_GPMC_BASE+0x158)
#define GPMC_CONFIG4_5        (OMAP24XX_GPMC_BASE+0x15C)
#define GPMC_CONFIG5_5        (OMAP24XX_GPMC_BASE+0x160)
#define GPMC_CONFIG6_5        (OMAP24XX_GPMC_BASE+0x164)
#define GPMC_CONFIG7_5        (OMAP24XX_GPMC_BASE+0x168)


/* SMS */
#define OMAP24XX_SMS_BASE 0x6C000000
#define SMS_SYSCONFIG     (OMAP24XX_SMS_BASE+0x10)

/* SDRC */
#define OMAP24XX_SDRC_BASE 0x6D000000
#define OMAP24XX_SDRC_CS0  0x80000000
#define OMAP24XX_SDRC_CS1  0xA0000000
#define SDRC_SYSCONFIG     (OMAP24XX_SDRC_BASE+0x10)
#define SDRC_STATUS        (OMAP24XX_SDRC_BASE+0x14)
#define SDRC_SHARING       (OMAP24XX_SDRC_BASE+0x44)
#define SDRC_DLLA_CTRL     (OMAP24XX_SDRC_BASE+0x60)
#define SDRC_DLLA_STATUS   (OMAP24XX_SDRC_BASE+0x64)
#define SDRC_DLLB_CTRL     (OMAP24XX_SDRC_BASE+0x68)
#define SDRC_POWER         (OMAP24XX_SDRC_BASE+0x70)
#define SDRC_MCFG_0        (OMAP24XX_SDRC_BASE+0x80)
#define SDRC_MR_0          (OMAP24XX_SDRC_BASE+0x84)
#define SDRC_ACTIM_CTRLA_0 (OMAP24XX_SDRC_BASE+0x9C)
#define SDRC_ACTIM_CTRLB_0 (OMAP24XX_SDRC_BASE+0xA0)
#define SDRC_MCFG_1        (OMAP24XX_SDRC_BASE+0xB0)
#define SDRC_ACTIM_CTRLA_1 (OMAP24XX_SDRC_BASE+0xC4)
#define SDRC_ACTIM_CTRLB_1 (OMAP24XX_SDRC_BASE+0xC8)
#define SDRC_RFR_CTRL      (OMAP24XX_SDRC_BASE+0xA4)
#define SDRC_MANUAL_0      (OMAP24XX_SDRC_BASE+0xA8)
#define SDRC_RFR_CTRL1	   (OMAP24XX_SDRC_BASE+0xD4)

#define LOADDLL            BIT2
#define CMD_NOP            0x0
#define CMD_PRECHARGE      0x1
#define CMD_AUTOREFRESH    0x2
#define CMD_ENTR_PWRDOWN   0x3
#define CMD_EXIT_PWRDOWN   0x4
#define CMD_ENTR_SRFRSH    0x5
#define CMD_CKE_HIGH       0x6
#define CMD_CKE_LOW        0x7
#define SOFTRESET          BIT1
#define SMART_IDLE         (0x2 << 3)
#define REF_ON_IDLE        (0x1 << 6)


/* UART */
#define OMAP2430_UART1	      0x4806A000
#define OMAP2430_UART2	      0x4806C000
#define OMAP2430_UART3        0x4806E000

/* General Purpose Timers */
#define OMAP24XX_GPT1         (OMAP243X_WAKEUP_L4_IO_BASE+0x18000)
#define OMAP24XX_GPT2         (OMAP24XX_L4_IO_BASE+0x2A000)
#define OMAP24XX_GPT3         (OMAP24XX_L4_IO_BASE+0x78000)
#define OMAP24XX_GPT4         (OMAP24XX_L4_IO_BASE+0x7A000)
#define OMAP24XX_GPT5         (OMAP24XX_L4_IO_BASE+0x7C000)
#define OMAP24XX_GPT6         (OMAP24XX_L4_IO_BASE+0x7E000)
#define OMAP24XX_GPT7         (OMAP24XX_L4_IO_BASE+0x80000)
#define OMAP24XX_GPT8         (OMAP24XX_L4_IO_BASE+0x82000)
#define OMAP24XX_GPT9         (OMAP24XX_L4_IO_BASE+0x84000)
#define OMAP24XX_GPT10        (OMAP24XX_L4_IO_BASE+0x86000)
#define OMAP24XX_GPT11        (OMAP24XX_L4_IO_BASE+0x88000)
#define OMAP24XX_GPT12        (OMAP24XX_L4_IO_BASE+0x8A000

/* timer regs offsets (32 bit regs) */
#define TIDR       0x0      /* r */
#define TIOCP_CFG  0x10     /* rw */
#define TISTAT     0x14     /* r */
#define TISR       0x18     /* rw */
#define TIER       0x1C     /* rw */
#define TWER       0x20     /* rw */
#define TCLR       0x24     /* rw */
#define TCRR       0x28     /* rw */
#define TLDR       0x2C     /* rw */
#define TTGR       0x30     /* rw */
#define TWPS       0x34     /* r */
#define TMAR       0x38     /* rw */
#define TCAR1      0x3c     /* r */
#define TSICR      0x40     /* rw */
#define TCAR2      0x44     /* r */

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE              (OMAP243X_WAKEUP_L4_IO_BASE+0x14000)
#define WD2_BASE              (OMAP243X_WAKEUP_L4_IO_BASE+0x16000)
#define WD3_BASE              (OMAP24XX_L4_IO_BASE+0x24000) /* not present */
#define WD4_BASE              (OMAP24XX_L4_IO_BASE+0x26000)

/* 32KTIMER */
#define SYNC_32KTIMER_BASE    (OMAP243X_WAKEUP_L4_IO_BASE+0x20000)
#define S32K_CR               (SYNC_32KTIMER_BASE+0x10)

#define WWPS       0x34     /* r */
#define WSPR       0x48     /* rw */
#define WD_UNLOCK1 0xAAAA
#define WD_UNLOCK2 0x5555

/* PRCM */
#define OMAP24XX_CM_BASE (OMAP243X_WAKEUP_L4_IO_BASE+0x06000)

#define PRCM_CLKSRC_CTRL (OMAP24XX_CM_BASE+0x060)
#define PRCM_CLKOUT_CTRL (OMAP24XX_CM_BASE+0x070)
#define PRCM_CLKEMUL_CTRL (OMAP24XX_CM_BASE+0x078)
#define PRCM_CLKCFG_CTRL (OMAP24XX_CM_BASE+0x080)
#define PRCM_CLKCFG_STATUS (OMAP24XX_CM_BASE+0x084)
#define CM_CLKSEL_MPU    (OMAP24XX_CM_BASE+0x140)
#define CM_FCLKEN1_CORE  (OMAP24XX_CM_BASE+0x200)
#define CM_FCLKEN2_CORE  (OMAP24XX_CM_BASE+0x204)
#define CM_ICLKEN1_CORE  (OMAP24XX_CM_BASE+0x210)
#define CM_ICLKEN2_CORE  (OMAP24XX_CM_BASE+0x214)
#define CM_CLKSEL1_CORE  (OMAP24XX_CM_BASE+0x240)
#define CM_CLKSEL_WKUP   (OMAP24XX_CM_BASE+0x440)
#define CM_CLKSEL2_CORE  (OMAP24XX_CM_BASE+0x244)
#define CM_FCLKEN_GFX    (OMAP24XX_CM_BASE+0x300)
#define CM_ICLKEN_GFX    (OMAP24XX_CM_BASE+0x310)
#define CM_CLKSEL_GFX    (OMAP24XX_CM_BASE+0x340)
#define RM_RSTCTRL_GFX    (OMAP24XX_CM_BASE+0x350)
#define CM_FCLKEN_WKUP    (OMAP24XX_CM_BASE+0x400)
#define CM_ICLKEN_WKUP    (OMAP24XX_CM_BASE+0x410)
#define PM_RSTCTRL_WKUP  (OMAP24XX_CM_BASE+0x450)
#define CM_CLKEN_PLL     (OMAP24XX_CM_BASE+0x500)
#define CM_IDLEST_CKGEN  (OMAP24XX_CM_BASE+0x520)
#define CM_CLKSEL1_PLL   (OMAP24XX_CM_BASE+0x540)
#define CM_CLKSEL2_PLL   (OMAP24XX_CM_BASE+0x544)
#define CM_CLKSEL_DSP    (OMAP24XX_CM_BASE+0x840)
#define CM_CLKSEL_MDM    (OMAP24XX_CM_BASE+0xC40)

/* SMX-APE */
#define SMX_APE_BASE 0x68000000
#define PM_RT_APE_BASE_ADDR_ARM  (SMX_APE_BASE + 0x10000)
#define PM_GPMC_BASE_ADDR_ARM    (SMX_APE_BASE + 0x12400)
#define PM_OCM_RAM_BASE_ADDR_ARM (SMX_APE_BASE + 0x12800)
#define PM_OCM_ROM_BASE_ADDR_ARM (SMX_APE_BASE + 0x12C00)

/* IVA2 */
#define PM_IVA2_BASE_ADDR_ARM    (SMX_APE_BASE + 0x14000)

/*
 *  The 2430's chip selects are programmable.  The mask ROM
 *  does configure CS0 to 0x08000000 before dispatch.  So, if
 *  you want your code to live below that address, you have to
 *  be prepared to jump though hoops, to reset the base address.
 */
#if defined(CONFIG_OMAP243X)

/* GPMC */
/* This is being used by the macros in mem.h. PHYS_FLASH_1 is defined to H4_CS0_BASE */
# define H4_CS1_BASE           0x09000000  /* flash (64 Meg aligned) */
#define CFG_FLASH_BASE H4_CS1_BASE
#define DEBUG_BASE	0x08000000

/* base address for indirect vectors (internal boot mode) */
#define SRAM_OFFSET0          0x40000000
#define SRAM_OFFSET1          0x00200000
#define SRAM_OFFSET2          0x0000F800
#define SRAM_VECT_CODE       (SRAM_OFFSET0|SRAM_OFFSET1|SRAM_OFFSET2)

#define LOW_LEVEL_SRAM_STACK  0x4020FFFC

#define PERIFERAL_PORT_BASE   0x480FE003

#endif  /* endif CONFIG_2430SDP */

#endif

