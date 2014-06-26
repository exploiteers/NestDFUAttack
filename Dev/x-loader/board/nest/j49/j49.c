/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *    (C) Copyright 2006
 *    Texas Instruments, <www.ti.com>
 *    Jian Zhang <jzhang@ti.com>
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
 *      This file is the board-specific set-up for the Nest Learning
 *      Thermostat board, based on the TI OMAP3 AM3703ACUS, focusing
 *      primarily on GPIO, RAM and flash initialization.
 *
 *      This is inherited from the OMAP3 EVM equivalent file.
 *
 *      Initialization function order is roughly:
 *
 *        1) s_init
 *        2) board_init
 *        3) misc_init_r
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>
#include <asm/arch/mem.h>

#include "platform.h"

#define MAX_J49_BOOT_DEVICES (3)

void
udelay(unsigned long usecs) {
	delay(usecs);
}

static inline u32
get_cpu_id(void)
{
	u32 cpuid = 0;

	__asm__ __volatile__("mrc p15, 0, %0, c0, c0, 0":"=r" (cpuid));

	return (cpuid);
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32
get_cpu_rev(void)
{
	const u32 cpuid = get_cpu_id();

	/* On ES1.0 the IDCODE register is not exposed on L4
	 * so using CPU ID to differentiate
	 * between ES2.0 and ES1.0.
	 */
	if ((cpuid  & 0xf) == 0x0) {
		return (CPU_3430_ES1);

	} else {
		return (CPU_3430_ES2);

	}

}

u32
is_cpu_family(void)
{
	const u32 cpuid = get_cpu_id();
	u32 cpu_family = 0, omap34xx_id = 0;
	u16 hawkeye;

	if ((cpuid & 0xf) == 0x0) {
		cpu_family = CPU_OMAP34XX;

	} else {
		omap34xx_id = __raw_readl(OMAP34XX_CONTROL_ID);
		hawkeye  = (omap34xx_id >> HAWKEYE_SHIFT) & 0xffff;

		switch (hawkeye) {

		case HAWKEYE_AM35XX:
			cpu_family = CPU_AM35XX;
			break;

		case HAWKEYE_OMAP36XX:
			cpu_family = CPU_OMAP36XX;
			break;

		case HAWKEYE_OMAP34XX:
		default:
			cpu_family = CPU_OMAP34XX;
			break;

		}
	}

	return (cpu_family);
}
/******************************************
 * cpu_is_3410(void) - returns true for 3410
 ******************************************/
u32
cpu_is_3410(void)
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

/*
 *  void sr32()
 *
 *  Description:
 *    This routine clears and sets a value in a bit extent for a
 *    32-bit value at the specified address.
 *
 *  Input(s):
 *    addr      - The address at which to clear and set the specified
 *                specified 32-bit value.
 *    start_bit - The first bit of the 32-bit data to clear and set.
 *    num_bits  - The range of bits of the 32-bit data to clear and set.
 *    value     - The value to set.
 *
 *  Output(s):
 *    addr      - The address with the specified bit extent cleared and
 *                set to the specified value.
 *
 *  Returns:
 *    N/A
 *
 */
void
sr32(u32 addr, u32 start_bit, u32 num_bits, u32 value)
{
	u32 tmp, msk = 0;

	msk = (1 << num_bits);
	--msk;
	tmp = __raw_readl(addr) & ~(msk << start_bit);
	tmp |=  value << start_bit;
	__raw_writel(tmp, addr);
}

/*
 *  u32 wait_on_value()
 *
 *  Description:
 *    This routine reads the register at the specified address and
 *    busy waits until the specified match value is read or until the
 *    bounded number of loops have been reached.
 *
 *  Input(s):
 *    read_bit_mask - The bit mask to apply after reading the register.
 *    match_value   - The value to match on after reading and masking.
 *    read_addr     - The register address to read.
 *    bound         - The maximum number of times to read before giving
 *                    up.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    True (1) if the match_value was reached; otherwise, false (0).
 *
 */
u32
wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
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

/*****************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access
 * (GP Device only)
 *****************************************/
static void
secure_unlock(void)
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
static void
try_unlock_memory(void)
{
	const int type = get_device_type();

	/*
	 * If the processor is a GP device, unlock the device SRAM for
	 * general use.
	 */

	if (type == CONTROL_STATUS_DEVICETYPE_GP) {
		secure_unlock();
	}

	return;
}

#if defined(CFG_SDRAM_DEBUG)
static void
omap3_sdrc_register_dump(void)
{
    printf("\n  SDRC Register Dump:\n");

	DUMP_REGL(SDRC_REVISION);
	DUMP_REGL(SDRC_SYSCONFIG);
	DUMP_REGL(SDRC_SYSSTATUS);
	DUMP_REGL(SDRC_CS_CFG);
	DUMP_REGL(SDRC_SHARING);
	DUMP_REGL(SDRC_ERR_ADDR);
	DUMP_REGL(SDRC_ERR_TYPE);
	DUMP_REGL(SDRC_DLLA_CTRL);
	DUMP_REGL(SDRC_DLLA_STATUS);
	DUMP_REGL(SDRC_POWER_REG);
	DUMP_REGL(SDRC_MCFG_0);
	DUMP_REGL(SDRC_MR_0);
	DUMP_REGL(SDRC_EMR2_0);
	DUMP_REGL(SDRC_ACTIM_CTRLA_0);
	DUMP_REGL(SDRC_ACTIM_CTRLB_0);
	DUMP_REGL(SDRC_RFR_CTRL_0);
	DUMP_REGL(SDRC_MANUAL_0);
	DUMP_REGL(SDRC_MCFG_1);
	DUMP_REGL(SDRC_MR_1);
	DUMP_REGL(SDRC_EMR2_1);
	DUMP_REGL(SDRC_ACTIM_CTRLA_1);
	DUMP_REGL(SDRC_ACTIM_CTRLB_1);
	DUMP_REGL(SDRC_RFR_CTRL_1);
	DUMP_REGL(SDRC_MANUAL_1);
}
#else
# define omap3_sdrc_register_dump()	do { } while (0)
#endif /* defined(CFG_SDRAM_DEBUG) */

#if defined(CFG_GPMC_DEBUG)
static void
omap3_gpmc_register_dump(void)
{
    printf("\n  GPMC Register Dump:\n");

	DUMP_REGL(GPMC_REVISION);
	DUMP_REGL(GPMC_SYSCONFIG);
	DUMP_REGL(GPMC_SYSSTATUS);
	DUMP_REGL(GPMC_IRQSTATUS);
	DUMP_REGL(GPMC_IRQENABLE);
	DUMP_REGL(GPMC_TIMEOUT_CONTROL);
	DUMP_REGL(GPMC_ERR_ADDRESS);
	DUMP_REGL(GPMC_ERR_TYPE );
	DUMP_REGL(GPMC_CONFIG );
	DUMP_REGL(GPMC_STATUS );

	DUMP_REGL(GPMC_CONFIG1_0);
	DUMP_REGL(GPMC_CONFIG2_0);
	DUMP_REGL(GPMC_CONFIG3_0);
	DUMP_REGL(GPMC_CONFIG4_0);
	DUMP_REGL(GPMC_CONFIG5_0);
	DUMP_REGL(GPMC_CONFIG6_0);
	DUMP_REGL(GPMC_CONFIG7_0);

	DUMP_REGL(GPMC_CONFIG1_1);
	DUMP_REGL(GPMC_CONFIG2_1);
	DUMP_REGL(GPMC_CONFIG3_1);
	DUMP_REGL(GPMC_CONFIG4_1);
	DUMP_REGL(GPMC_CONFIG5_1);
	DUMP_REGL(GPMC_CONFIG6_1);
	DUMP_REGL(GPMC_CONFIG7_1);

	DUMP_REGL(GPMC_CONFIG1_2);
	DUMP_REGL(GPMC_CONFIG2_2);
	DUMP_REGL(GPMC_CONFIG3_2);
	DUMP_REGL(GPMC_CONFIG5_2);
	DUMP_REGL(GPMC_CONFIG4_2);
	DUMP_REGL(GPMC_CONFIG6_2);
	DUMP_REGL(GPMC_CONFIG7_2);

	DUMP_REGL(GPMC_CONFIG1_3);
	DUMP_REGL(GPMC_CONFIG2_3);
	DUMP_REGL(GPMC_CONFIG3_3);
	DUMP_REGL(GPMC_CONFIG4_3);
	DUMP_REGL(GPMC_CONFIG5_3);
	DUMP_REGL(GPMC_CONFIG6_3);
	DUMP_REGL(GPMC_CONFIG7_3);
			               
	DUMP_REGL(GPMC_CONFIG1_4);
	DUMP_REGL(GPMC_CONFIG2_4);
	DUMP_REGL(GPMC_CONFIG3_4);
	DUMP_REGL(GPMC_CONFIG4_4);
	DUMP_REGL(GPMC_CONFIG5_4);
	DUMP_REGL(GPMC_CONFIG6_4);
	DUMP_REGL(GPMC_CONFIG7_4);
			               
	DUMP_REGL(GPMC_CONFIG1_5);
	DUMP_REGL(GPMC_CONFIG2_5);
	DUMP_REGL(GPMC_CONFIG3_5);
	DUMP_REGL(GPMC_CONFIG4_5);
	DUMP_REGL(GPMC_CONFIG5_5);
	DUMP_REGL(GPMC_CONFIG6_5);
	DUMP_REGL(GPMC_CONFIG7_5);
			               
	DUMP_REGL(GPMC_CONFIG1_6);
	DUMP_REGL(GPMC_CONFIG2_6);
	DUMP_REGL(GPMC_CONFIG3_6);
	DUMP_REGL(GPMC_CONFIG4_6);
	DUMP_REGL(GPMC_CONFIG5_6);
	DUMP_REGL(GPMC_CONFIG6_6);
	DUMP_REGL(GPMC_CONFIG7_6);
			               
	DUMP_REGL(GPMC_CONFIG1_7);
	DUMP_REGL(GPMC_CONFIG2_7);
	DUMP_REGL(GPMC_CONFIG3_7);
	DUMP_REGL(GPMC_CONFIG4_7);
	DUMP_REGL(GPMC_CONFIG5_7);
	DUMP_REGL(GPMC_CONFIG6_7);
	DUMP_REGL(GPMC_CONFIG7_7);

#if 0
	DUMP_REGL(GPMC_NAND_COMMAND_0);
	DUMP_REGL(GPMC_NAND_COMMAND_1);
	DUMP_REGL(GPMC_NAND_COMMAND_2);
	DUMP_REGL(GPMC_NAND_COMMAND_3);
	DUMP_REGL(GPMC_NAND_COMMAND_4);
	DUMP_REGL(GPMC_NAND_COMMAND_5);
	DUMP_REGL(GPMC_NAND_COMMAND_6);
	DUMP_REGL(GPMC_NAND_COMMAND_7);

	DUMP_REGL(GPMC_NAND_ADDRESS_0);
	DUMP_REGL(GPMC_NAND_ADDRESS_1);
	DUMP_REGL(GPMC_NAND_ADDRESS_2);
	DUMP_REGL(GPMC_NAND_ADDRESS_3);
	DUMP_REGL(GPMC_NAND_ADDRESS_4);
	DUMP_REGL(GPMC_NAND_ADDRESS_5);
	DUMP_REGL(GPMC_NAND_ADDRESS_6);
	DUMP_REGL(GPMC_NAND_ADDRESS_7);

	DUMP_REGL(GPMC_NAND_DATA_0);
	DUMP_REGL(GPMC_NAND_DATA_1);
	DUMP_REGL(GPMC_NAND_DATA_2);
	DUMP_REGL(GPMC_NAND_DATA_3);
	DUMP_REGL(GPMC_NAND_DATA_4);
	DUMP_REGL(GPMC_NAND_DATA_5);
	DUMP_REGL(GPMC_NAND_DATA_6);
	DUMP_REGL(GPMC_NAND_DATA_7);

	DUMP_REGL(GPMC_PREFETCH_CONFIG1);
	DUMP_REGL(GPMC_PREFETCH_CONFIG2);
	DUMP_REGL(GPMC_PREFETCH_CONTROL);
	DUMP_REGL(GPMC_PREFETCH_STATUS);

	DUMP_REGL(GPMC_ECC_CONFIG);
	DUMP_REGL(GPMC_ECC_CONTROL);
	DUMP_REGL(GPMC_ECC_SIZE_CONFIG);

	DUMP_REGL(GPMC_ECC1_RESULT);
	DUMP_REGL(GPMC_ECC2_RESULT);
	DUMP_REGL(GPMC_ECC3_RESULT);
	DUMP_REGL(GPMC_ECC4_RESULT);
	DUMP_REGL(GPMC_ECC5_RESULT);
	DUMP_REGL(GPMC_ECC6_RESULT);
	DUMP_REGL(GPMC_ECC7_RESULT);
	DUMP_REGL(GPMC_ECC8_RESULT);
	DUMP_REGL(GPMC_ECC9_RESULT);

	DUMP_REGL(GPMC_BCH_RESULT0_0);
	DUMP_REGL(GPMC_BCH_RESULT0_1);
	DUMP_REGL(GPMC_BCH_RESULT0_2);
	DUMP_REGL(GPMC_BCH_RESULT0_3);
	DUMP_REGL(GPMC_BCH_RESULT0_4);
	DUMP_REGL(GPMC_BCH_RESULT0_5);
	DUMP_REGL(GPMC_BCH_RESULT0_6);
	DUMP_REGL(GPMC_BCH_RESULT0_7);

	DUMP_REGL(GPMC_BCH_RESULT1_0);
	DUMP_REGL(GPMC_BCH_RESULT1_1);
	DUMP_REGL(GPMC_BCH_RESULT1_2);
	DUMP_REGL(GPMC_BCH_RESULT1_3);
	DUMP_REGL(GPMC_BCH_RESULT1_4);
	DUMP_REGL(GPMC_BCH_RESULT1_5);
	DUMP_REGL(GPMC_BCH_RESULT1_6);
	DUMP_REGL(GPMC_BCH_RESULT1_7);

	DUMP_REGL(GPMC_BCH_RESULT2_0);
	DUMP_REGL(GPMC_BCH_RESULT2_1);
	DUMP_REGL(GPMC_BCH_RESULT2_2);
	DUMP_REGL(GPMC_BCH_RESULT2_3);
	DUMP_REGL(GPMC_BCH_RESULT2_4);
	DUMP_REGL(GPMC_BCH_RESULT2_5);
	DUMP_REGL(GPMC_BCH_RESULT2_6);
	DUMP_REGL(GPMC_BCH_RESULT2_7);

	DUMP_REGL(GPMC_BCH_RESULT3_0);
	DUMP_REGL(GPMC_BCH_RESULT3_1);
	DUMP_REGL(GPMC_BCH_RESULT3_2);
	DUMP_REGL(GPMC_BCH_RESULT3_3);
	DUMP_REGL(GPMC_BCH_RESULT3_4);
	DUMP_REGL(GPMC_BCH_RESULT3_5);
	DUMP_REGL(GPMC_BCH_RESULT3_6);
	DUMP_REGL(GPMC_BCH_RESULT3_7);

	DUMP_REGL(GPMC_BCH_SWDATA);
#endif
}
#else
# define omap3_gpmc_register_dump() do { } while (0)
#endif /* defined(CFG_NAND_DEBUG) */

#if defined(CFG_PRCM_DEBUG)
static void
omap3_prcm_register_dump(void)
{
    printf("\n  PRCM Register Dump:\n");

	DUMP_REGL(PRCM_PRM_CCR_CLKSEL);
	DUMP_REGL(PRCM_PRM_GR_CLKSRC_CTRL);
	DUMP_REGL(CM_CLKSEL1_PLL);
	DUMP_REGL(CM_CLKSEL2_PLL);
	DUMP_REGL(CM_CLKSEL3_PLL);
	DUMP_REGL(CM_CLKEN_PLL);
	DUMP_REGL(CM_CLKSEL1_PLL_MPU);
	DUMP_REGL(CM_CLKSEL2_PLL_MPU);
	DUMP_REGL(CM_CLKEN_PLL_MPU);
	DUMP_REGL(CM_CLKSEL1_PLL_IVA2);
	DUMP_REGL(CM_CLKSEL2_PLL_IVA2);
	DUMP_REGL(CM_CLKEN_PLL_IVA2);
	DUMP_REGL(CM_CLKSEL_CAM);
	DUMP_REGL(CM_CLKSEL_CORE);
	DUMP_REGL(CM_CLKSEL_DSS);
	DUMP_REGL(CM_CLKSEL1_EMU);
}
#else
#define omap3_prcm_register_dump()	do { } while (0)
#endif /* CFG_PRCM_DEBUG */

/*
 *  void config_diamond_sdram()
 *
 *  Description:
 *    This routine initializes the processor's SDRAM Controller (SDRC)
 *    as appropriate for the Samsung K4X51163PI-FCG6 32 Mb x 16 b (64
 *    MiB) DDR SDRAM on the Nest Learning Thermostat board.
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
config_diamond_sdram(void)
{
	u32 value, actim_ctrla, actim_ctrlb;

	/*
	 * Step 1: Reset the SDRC controller and then wait for the reset
	 * event to clear.
	 */

    __raw_writel(SDRC_SYSCONFIG_SOFTRESET_SET, SDRC_SYSCONFIG);
    wait_on_value(SDRC_SYSSTATUS_RESETDONE,
				  SDRC_SYSSTATUS_RESETDONE,
				  SDRC_STATUS,
				  12000000);
    __raw_writel(SDRC_SYSCONFIG_SOFTRESET_CLEAR, SDRC_SYSCONFIG);

	/*
	 * Step 2: Setup the position and geometry of the SDRAM device on
	 * the controller's bus.
	 */

	value =
		(SDRC_SHARING_LOCK_OFF 												|
		 SDRC_SHARING_CS1MUXCFG_ENCODE(SDRC_SHARING_CS1MUXCFG_16_BIT_16_0)	|
		 SDRC_SHARING_CS0MUXCFG_ENCODE(SDRC_SHARING_CS0MUXCFG_16_BIT_16_0)	|
		 SDRC_SHARING_SDRCTRISTATE_OFF);
	__raw_writel(value, SDRC_SHARING);

	/*
	 * Step 3: Setup the memory configuration, including RAS width,
	 * CAS width, address multiplexing, size, bank mapping, bus width,
	 * power mode, DDR type and memory type.
	 */

	value =
		(SDRC_MCFG_LOCKSTATUS_RW										|
		 SDRC_MCFG_RASWIDTH_ENCODE(SDRC_MCFG_RASWIDTH_14_BITS)			|
		 SDRC_MCFG_CASWIDTH_ENCODE(SDRC_MCFG_CASWIDTH_10_BITS)			|
		 SDRC_MCFG_ADDRMUXLEGACY_FLEXIBLE								|
		 SDRC_MCFG_RAMSIZE_ENCODE(CFG_SDRAM_SIZE_MB)					|
		 SDRC_MCFG_BANKALLOCATION_ENCODE(SDRC_MCFG_BANKALLOCATION_R_B_C)|
		 SDRC_MCFG_B32NOT16_OFF											|
		 SDRC_MCFG_DEEPPD_SUPPORTED										|
		 SDRC_MCFG_DDRTYPE_ENCODE(SDRC_MCFG_DDRTYPE_MOBILE_DDR)			|
		 SDRC_MCFG_RAMTYPE_ENCODE(SDRC_MCFG_RAMTYPE_DDR));
	__raw_writel(value, SDRC_MCFG_0);

	/*
	 * Step 4: Establish the AC fine tuning timing characteristics.
	 */

	actim_ctrla
		= (SDRC_ACTIM_CTRLA_TRFC_ENCODE(CFG_SDRC_ACTIM_CTRLA_TRFC)	|
		   SDRC_ACTIM_CTRLA_TRC_ENCODE(CFG_SDRC_ACTIM_CTRLA_TRC)	|
		   SDRC_ACTIM_CTRLA_TRAS_ENCODE(CFG_SDRC_ACTIM_CTRLA_TRAS)	|
		   SDRC_ACTIM_CTRLA_TRP_ENCODE(CFG_SDRC_ACTIM_CTRLA_TRP)	|
		   SDRC_ACTIM_CTRLA_TRCD_ENCODE(CFG_SDRC_ACTIM_CTRLA_TRCD)	|
		   SDRC_ACTIM_CTRLA_TRRD_ENCODE(CFG_SDRC_ACTIM_CTRLA_TRRD)	|
		   SDRC_ACTIM_CTRLA_TDPL_ENCODE(CFG_SDRC_ACTIM_CTRLA_TDPL)	|
		   SDRC_ACTIM_CTRLA_TDAL_ENCODE(CFG_SDRC_ACTIM_CTRLA_TDAL));

	actim_ctrlb
		= (SDRC_ACTIM_CTRLB_TWTR_ENCODE(CFG_SDRC_ACTIM_CTRLB_TWTR)	|
		   SDRC_ACTIM_CTRLB_TCKE_ENCODE(CFG_SDRC_ACTIM_CTRLB_TCKE)	|
		   SDRC_ACTIM_CTRLB_TXP_ENCODE(CFG_SDRC_ACTIM_CTRLB_TXP)	|
		   SDRC_ACTIM_CTRLB_TXSR_ENCODE(CFG_SDRC_ACTIM_CTRLB_TXSR));

	__raw_writel(actim_ctrla, SDRC_ACTIM_CTRLA_0);
	__raw_writel(actim_ctrlb, SDRC_ACTIM_CTRLB_0);

	/*
	 * Step 5: Establish the memory autorefresh control
	 */

	value =
		(SDRC_RFR_CTRL_ARCV_ENCODE(1244)						|
		 SDRC_RFR_CTRL_ARE_ENCODE(SDRC_RFR_CTRL_ARE_1_ARCV));
	__raw_writel(value, SDRC_RFR_CTRL_0);

	value =
		(SDRC_POWER_REG_WAKEUP_DELAYED										|
		 SDRC_POWER_REG_AUTOCOUNT_ENCODE(0)									|
		 SDRC_POWER_REG_SRFR_ON_RST_ENABLE									|
		 SDRC_POWER_REG_SRFR_ON_IDLE_DISABLE								|
		 SDRC_POWER_REG_CLKCTRL_ENCODE(SDRC_POWER_REG_CLKCTRL_NONE)			|
		 SDRC_POWER_REG_PAGEPOLICY_HPHB);
	__raw_writel(value, SDRC_POWER_REG);

	/*
	 * Step 6: Establish the JEDEC-defined mode and DLL parameters.
	 */

	__raw_writel(SDRC_MANUAL_CMDCODE_ENCODE(SDRC_MANUAL_CMDCODE_AUTOREFRESH),
				 SDRC_MANUAL_0);

	delay(5000);	                 

	__raw_writel(SDRC_MANUAL_CMDCODE_ENCODE(SDRC_MANUAL_CMDCODE_PRECHARGE_ALL),
				 SDRC_MANUAL_0);
	__raw_writel(SDRC_MANUAL_CMDCODE_ENCODE(SDRC_MANUAL_CMDCODE_AUTOREFRESH),
				 SDRC_MANUAL_0);
	__raw_writel(SDRC_MANUAL_CMDCODE_ENCODE(SDRC_MANUAL_CMDCODE_AUTOREFRESH),
				 SDRC_MANUAL_0);

	value =
		(SDRC_MR_ZERO_1							|
		 SDRC_MR_WBST_ENABLE					|
		 SDRC_MR_ZERO_0							|
		 SDRC_MR_CASL_ENCODE(SDRC_MR_CASL_3)	|
		 SDRC_MR_SIL_SERIAL						|
		 SDRC_MR_BL_ENCODE(SDRC_MR_BL_4));
	__raw_writel(value, SDRC_MR_0);

	value =
		(SDRC_DLLA_CTRL_FIXED_DELAY_ENCODE(0)								 |
		 SDRC_DLLA_CTRL_INIT_LAT_ENCODE(0)									 |
		 SDRC_DLLA_CTRL_MODE_ON_IDLE_ENCODE(SDRC_DLLA_CTRL_MODE_ON_IDLE_PWD) | 
		 SDRC_DLLA_CTRL_DLL_ENABLE											 |
		 SDRC_DLLA_CTRL_LOCK_TRACKINGDELAY);
	__raw_writel(value, SDRC_DLLA_CTRL);

	/*
	 * Delay for a "reasonably long" period of time to allow the
	 * requested changes to take effect.
	 */

 	delay(0x20000);
}

/*
 *  void s_init()
 *
 *  Description:
 *    This routine performs very early system initialization of chip
 *    pin multiplexing and clocks and is called when ONLY an
 *    SRAM-based stack is available (i.e. no SDRAM).
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
s_init(void)
{
	watchdog_init();
	try_unlock_memory();
	set_muxconf_regs();

	delay(100);

	prcm_init();
	per_clocks_enable();
	config_diamond_sdram();
}

/*
 *  int board_init()
 *
 *  Description:
 *    This routine performs any early, board-specific initialization
 *    following core CPU initialization but prior to serial and NAND
 *    initialization.
 *
 *    At present, there is nothing board-specific to do.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    0 if OK; otherwise, non-zero on error.
 *
 */
int
board_init(void)
{
	return (0);
}

/*
 *  u32 get_device_type()
 *
 *  Description:
 *    This routine returns the decoded value from the CPU's
 *    CONTROL_STATUS register DEVICETYPE field, indicating whether the
 *    device is of TST, EMU, HS or GP type.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    The decoded CONTROL_STATUS register DEVICETYPE field.
 *
 */
u32
get_device_type(void)
{
	const int value = __raw_readl(CONTROL_STATUS);

	return (CONTROL_STATUS_DEVICETYPE_DECODE(value));
}

/*
 *  u32 get_sysboot_value()
 *
 *  Description:
 *    This routine returns the decoded value from the CPU's
 *    CONTROL_STATUS register SYSBOOT order subfield, indicating the
 *    order of memory interfaces from which the processor will attempt
 *    to boot itself.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    The decoded CONTROL_STATUS register SYSBOOT order subfield.
 *
 */
u32
get_sysboot_value(void)
{
#if 0
	const u32 value = __raw_readl(CONTROL_STATUS);
	const u32 peripheral_type = CONTROL_STATUS_SYSBOOT_TYPE_PERIPHERAL;

	/*
	 * This routine and callers of it are implicitly expecting the
	 * MEMORY interface (SYSBOOT[5] == 0) not the PERHIPHERAL
	 * interface (SYSBOOT[5] == 1) boot order, so check that is
	 * actually the case.
	 */

	if (CONTROL_STATUS_SYSBOOT_TYPE_DECODE(value) == peripheral_type) {
		hang();
	}

	return (CONTROL_STATUS_SYSBOOT_ORDER_DECODE(value));
#else
        return __raw_readl(CONTROL_STATUS) & 0x3f;
#endif
}

/*
 *  int get_boot_device_list(const u32** device_list)
 *
 *  Description:
 *    This routine returns a constant list indicating the preferred
 *    device boot list.
 *    It gets called from start_armboot in board.c
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    The preferred first processor memory boot interface.
 *
 */

int
get_boot_devices_list(const u32** devices_list)
{
	static u32 list[MAX_J49_BOOT_DEVICES];
	const u32 mem_order = get_sysboot_value();

	// set defaults
	int number_of_devices = 0;
	*devices_list = &list[0];

	switch (mem_order) {
	case 15:
		list[0] = GPMC_NAND;
		list[1] = USB_PERIPHERAL;
		list[2] = MMC_NAND;
		number_of_devices = MAX_J49_BOOT_DEVICES;
		break;
	case 47:
		list[0] = MMC_NAND;
		list[1] = GPMC_NAND;
		list[2] = USB_PERIPHERAL;
		number_of_devices = MAX_J49_BOOT_DEVICES;
		break;
	default:
		break;
	}
	return number_of_devices;
}

/*
 *  int misc_init_r()
 *
 *  Description:
 *    This routine performs any miscellaneous, board-specific
 *    initialization following CPU, early board, serial and NAND
 *    initialization but prior to loading the secondary program loader
 *    to RAM.
 *
 *    At present, there is nothing board-specific to do.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    0 if OK; otherwise, non-zero on error.
 *
 */
int
misc_init_r(void)
{
	return (0);
}

/******************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 ******************************************************/
static void
wait_for_command_complete(unsigned int wd_base)
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
void
watchdog_init(void)
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

/*****************************************************************
 * Routine: peripheral_enable
 * Description: Enable the clks & power for perifs (GPT2, UART1,...)
 ******************************************************************/
/*
 *  void foobar()
 *
 *  Description:
 *    This routine...
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
per_clocks_enable(void)
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

static int
gpmc_config_0(void)
{
#if defined(CFG_GPMC_CONFIG1_0)
	/*
	 * First, disable the interface and wait.
	 */
	__raw_writel(GPMC_CONFIG7_CSVALID_DISABLED, GPMC_CONFIG7_0);

	delay(1000);

	/*
	 * Next, program the configuration parameters.
	 */
	__raw_writel(CFG_GPMC_CONFIG1_0, GPMC_CONFIG1_0);
	__raw_writel(CFG_GPMC_CONFIG2_0, GPMC_CONFIG2_0);
	__raw_writel(CFG_GPMC_CONFIG3_0, GPMC_CONFIG3_0);
	__raw_writel(CFG_GPMC_CONFIG4_0, GPMC_CONFIG4_0);
	__raw_writel(CFG_GPMC_CONFIG5_0, GPMC_CONFIG5_0);
	__raw_writel(CFG_GPMC_CONFIG6_0, GPMC_CONFIG6_0);

	/*
	 * Finally, enable the GPMC mapping and spin for the parameters to
	 * become active.
	 */
	__raw_writel((CFG_GPMC_CONFIG7_0 | GPMC_CONFIG7_CSVALID_ENABLED),
				 GPMC_CONFIG7_0);

	delay(2000);

	if (nand_chip()){
		return (1);
	}
#endif /* defined(CFG_GPMC_CONFIG1_0) */

	return (0);
}

/*
 *  int nand_init()
 *
 *  Description:
 *    This routine initializes the General Purpose Memory Controller
 *    (GPMC) such that on-board NAND devices may be accessed for
 *    second-stage boot.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    0 if NAND was successfully initialized; otherwise, 1.
 *
 */
int
nand_init(void)
{
	int status = 1;

	/*
	 * Establish GPMC global settings.
	 */

	__raw_writel(GPMC_SYSCONFIG_IDLEMODE_SMART, GPMC_SYSCONFIG);

	__raw_writel(GPMC_IRQENABLE_ALL_DISABLE, GPMC_IRQENABLE);

	__raw_writel(GPMC_TIMEOUTENABLE_OFF, GPMC_TIMEOUT_CONTROL);

	if (gpmc_config_0()) {
		puts("Unsupported NAND device @ GPMC0!\n");
		goto done;
	}

	status = 0;

 done:
	/* Dump GPMC and SDRC registers if so configured. */

	omap3_sdrc_register_dump();
	omap3_gpmc_register_dump();
	omap3_prcm_register_dump();

	return (status);
}

/*
 *  void board_hang()
 *
 *  Description:
 *    This routine performs any board-specific actions when the system
 *    hangs by executing hang(). At present, there is nothing to do;
 *    however, we might later drive the piezo or a LED.
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
board_hang (void)
{
	return;
}
