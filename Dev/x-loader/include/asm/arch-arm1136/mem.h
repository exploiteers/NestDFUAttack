
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

#ifndef _OMAP24XX_MEM_H_
#define _OMAP24XX_MEM_H_

#define SDRC_CS0_OSET    0x0
#define SDRC_CS1_OSET    0x30  /* mirror CS1 regs appear offset 0x30 from CS0 */

#ifndef __ASSEMBLY__
/* struct's for holding data tables for current boards, they are getting used
   early in init when NO global access are there */
struct sdrc_data_s {
	u32    sdrc_sharing;
	u32    sdrc_mdcfg_0_ddr;
	u32    sdrc_mdcfg_0_sdr;
	u32    sdrc_actim_ctrla_0;
	u32    sdrc_actim_ctrlb_0;
	u32    sdrc_rfr_ctrl;
	u32    sdrc_mr_0_ddr;
	u32    sdrc_mr_0_sdr;
	u32    sdrc_dllab_ctrl;
} /*__attribute__ ((packed))*/;
typedef struct sdrc_data_s sdrc_data_t;

typedef enum {
	STACKED		= 0,
	IP_DDR		= 1,
	COMBO_DDR	= 2,
	IP_SDR	 	= 3,
} mem_t;

#endif

/* set the 243x-SDRC incoming address convention */
#if defined(SDRC_B_R_C)
#define B_ALL	(0 << 6)	/* bank-row-column */
#elif defined(SDRC_B1_R_B0_C)
#define B_ALL	(1 << 6)	/* bank1-row-bank0-column */
#elif defined(SDRC_R_B_C)
#define B_ALL	(2 << 6)	/* row-bank-column */
#endif

/* Slower full frequency range default timings for x32 operation*/
#define H4_2420_SDRC_SHARING		0x00000100
#define H4_2420_SDRC_MDCFG_0_SDR	0x00D04010 /* discrete sdr module */
#define H4_2420_SDRC_MR_0_SDR		0x00000031
#define H4_2420_SDRC_MDCFG_0_DDR	0x01702011 /* descrite ddr module */
#define SDP_2430_SDRC_MDCFG_0_DDR	(0x02584019|B_ALL) /* Infin ddr module */
#define H4_2420_COMBO_MDCFG_0_DDR	0x00801011 /* combo module */
#define H4_2420_SDRC_MR_0_DDR		0x00000032

#define H4_2422_SDRC_SHARING		0x00004b00
#define H4_2422_SDRC_MDCFG_MONO_DDR	0x01A02011 /* stacked mono die ddr on 2422 */
#define H4_2422_SDRC_MDCFG_0_DDR	0x00801011 /* stacked dual die ddr on 2422 */
#define H4_2422_SDRC_MR_0_DDR		0x00000032

#define H4_2423_SDRC_SHARING		0x00004900 /* 2420POP board cke1 not connected */
#define H4_2423_SDRC_MDCFG_0_DDR	0x01A02011 /* stacked dual die ddr on 2423 */
#define H4_2423_SDRC_MDCFG_1_DDR	0x00801011 /* stacked dual die ddr on 2423 */

/* ES1 work around timings */
#define H4_242x_SDRC_ACTIM_CTRLA_0_ES1	0x9bead909  /* 165Mhz for use with 100/133 */
#define H4_242x_SDRC_ACTIM_CTRLB_0_ES1  0x00000020
#define H4_242x_SDRC_RFR_CTRL_ES1	0x00002401  /* use over refresh for ES1 */

/* optimized timings good for current shipping parts */
#define H4_242X_SDRC_ACTIM_CTRLA_0_100MHz   0x5A59B485
#define H4_242X_SDRC_ACTIM_CTRLB_0_100MHz   0x0000000e
#define H4_242X_SDRC_ACTIM_CTRLA_0_133MHz   0x8BA6E6C8 /* temp warn 0 settings */
#define H4_242X_SDRC_ACTIM_CTRLB_0_133MHz   0x00000010 /* temp warn 0 settings */
#define H4_242X_SDRC_RFR_CTRL_100MHz	    0x0002da01
#define H4_242X_SDRC_RFR_CTRL_133MHz	    0x0003de01 /* 7.8us/7.5ns - 50 = 0x3de */
#define SDP_24XX_SDRC_RFR_CTRL_165MHz	    0x0004e201 /* 7.8us/6ns - 50 = 0x4e2 */
#define H4_242X_SDRC_DLLAB_CTRL_100MHz      0x0000980E /* 90deg, allow DPLLout*1 to work (combo)*/
#define H4_242X_SDRC_DLLAB_CTRL_133MHz      0x0000690E /* 90deg, for ES2 */
#define SDP_24XX_SDRC_DLLAB_CTRL_165MHz     0x0000170C /* 72deg, code will recalc dll load */

/* Infineon part of 2430SDP (133MHz optimized) ~ 7.5ns
 *	TDAL = Twr/Tck + Trp/tck = 15/7.5 + 22.5/7.5 = 2 + 3 = 5
 *	TDPL = 15/7.5	= 2
 *	TRRD = 15/2.5	= 2
 *	TRCD = 22.5/7.5	= 3
 *	TRP = 22.5/7.5	= 3
 *	TRAS = 45/7.5	= 6
 *	TRC = 65/7.5	= 8.6->9
 *	TRFC = 75/7.5	= 10
 *   ACTIMB
 *	TCKE = 2	<new in 2430>
 *	XSR = 120/7.5 = 16
 */
#define TDAL_133   5
#define TDPL_133   2
#define TRRD_133   2
#define TRCD_133   3
#define TRP_133    3
#define TRAS_133   6
#define TRC_133    9
#define TRFC_133  10
#define V_ACTIMA_133 ((TRFC_133 << 27) | (TRC_133 << 22) | (TRAS_133 << 18) |(TRP_133 << 15) | \
		(TRCD_133 << 12) |(TRRD_133 << 9) |(TDPL_133 << 6) | (TDAL_133))

#define TCKE_133   2
#define XSR_133   16
#define V_ACTIMB_133 ((TCKE_133 << 12) | (XSR_133 << 0))

/* Infineon part of 2430SDP (165MHz optimized) 6.06ns
 *   ACTIMA
 *	TDAL = Twr/Tck + Trp/tck = 15/6 + 18/6 = 2.5 + 3 = 5.5 -> 6
 *	TDPL (Twr) = 15/6	= 2.5 -> 3
 *	TRRD = 12/6	= 2
 *	TRCD = 18/6	= 3
 *	TRP = 18/6	= 3
 *	TRAS = 42/6	= 7
 *	TRC = 60/6	= 10
 *	TRFC = 72/6	= 12
 *   ACTIMB
 *	TCKE = 2	<new in 2430>
 *	XSR = 120/6 = 20
 */
#define TDAL_165   6
#define TDPL_165   3
#define TRRD_165   2
#define TRCD_165   3
#define TRP_165    3
#define TRAS_165   7
#define TRC_165   10
#define TRFC_165  12
#define V_ACTIMA_165 ((TRFC_165 << 27) | (TRC_165 << 22) | (TRAS_165 << 18) |(TRP_165 << 15) | \
		(TRCD_165 << 12) |(TRRD_165 << 9) |(TDPL_165 << 6) | (TDAL_165))

#define TCKE_165   2
#define XSR_165   20
#define V_ACTIMB_165 ((TCKE_165 << 12) | (XSR_165 << 0))

#if defined(PRCM_CONFIG_II) || defined(PRCM_CONFIG_5B)
# define H4_2420_SDRC_ACTIM_CTRLA_0  H4_242X_SDRC_ACTIM_CTRLA_0_100MHz
# define SDP_2430_SDRC_ACTIM_CTRLA_0 V_ACTIMA_133
# define H4_2420_SDRC_ACTIM_CTRLB_0  H4_242X_SDRC_ACTIM_CTRLB_0_100MHz
# define H4_2420_SDRC_RFR_CTRL       H4_242X_SDRC_RFR_CTRL_100MHz
# define H4_2420_SDRC_DLLAB_CTRL     H4_242X_SDRC_DLLAB_CTRL_100MHz
# define SDP_2430_SDRC_DLLAB_CTRL    0x0000730E
# define H4_2422_SDRC_ACTIM_CTRLA_0  H4_242X_SDRC_ACTIM_CTRLA_0_100MHz
# define H4_2422_SDRC_ACTIM_CTRLB_0  H4_242X_SDRC_ACTIM_CTRLB_0_100MHz
# define H4_2422_SDRC_RFR_CTRL       H4_242X_SDRC_RFR_CTRL_100MHz
# define H4_2422_SDRC_DLLAB_CTRL     H4_242X_SDRC_DLLAB_CTRL_100MHz
#elif defined(PRCM_CONFIG_III) || defined(PRCM_CONFIG_5A) || defined(PRCM_CONFIG_3)
# define H4_2420_SDRC_ACTIM_CTRLA_0  H4_242X_SDRC_ACTIM_CTRLA_0_133MHz
# define SDP_2430_SDRC_ACTIM_CTRLA_0 V_ACTIMA_133
# define H4_2420_SDRC_ACTIM_CTRLB_0  H4_242X_SDRC_ACTIM_CTRLB_0_133MHz
# define H4_2420_SDRC_RFR_CTRL       H4_242X_SDRC_RFR_CTRL_133MHz
# define H4_2420_SDRC_DLLAB_CTRL     H4_242X_SDRC_DLLAB_CTRL_133MHz
# define SDP_2430_SDRC_DLLAB_CTRL    0x0000730E
# define H4_2422_SDRC_ACTIM_CTRLA_0  H4_242X_SDRC_ACTIM_CTRLA_0_133MHz
# define H4_2422_SDRC_ACTIM_CTRLB_0  H4_242X_SDRC_ACTIM_CTRLB_0_133MHz
# define H4_2422_SDRC_RFR_CTRL       H4_242X_SDRC_RFR_CTRL_133MHz
# define H4_2422_SDRC_DLLAB_CTRL     H4_242X_SDRC_DLLAB_CTRL_133MHz
#elif defined(PRCM_CONFIG_I) || defined(PRCM_CONFIG_2)
# define H4_2420_SDRC_ACTIM_CTRLA_0  V_ACTIMA_165 
# define SDP_2430_SDRC_ACTIM_CTRLA_0 V_ACTIMA_165
# define H4_2420_SDRC_ACTIM_CTRLB_0  V_ACTIMB_165 
# define H4_2420_SDRC_RFR_CTRL       SDP_24XX_SDRC_RFR_CTRL_165MHz
# define H4_2420_SDRC_DLLAB_CTRL     SDP_24XX_SDRC_DLLAB_CTRL_165MHz
# define SDP_2430_SDRC_DLLAB_CTRL    SDP_24XX_SDRC_DLLAB_CTRL_165MHz
# define H4_2422_SDRC_ACTIM_CTRLA_0  V_ACTIMA_165 
# define H4_2422_SDRC_ACTIM_CTRLB_0  V_ACTIMB_165 
# define H4_2422_SDRC_RFR_CTRL       SDP_24XX_SDRC_RFR_CTRL_165MHz
# define H4_2422_SDRC_DLLAB_CTRL     SDP_24XX_SDRC_DLLAB_CTRL_165MHz
#endif

/*
 * GPMC settings -
 * Definitions is as per the following format
 * # define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of the same.
 * if values are independent of the board, this definition will be present in this file
 * if values are dependent on the board, then this should go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * STNOR - Intel Strata Flash
 * SMNAND - Samsung NAND
 * MPDB - H4 MPDB board
 * SBNOR - Sibley NOR
 * ONNAND - Samsung One NAND
 *
 * include/configs/file.h contains the following defn - for all CS we are interested
 * #define OMAP24XX_GPMC_CSx PART
 * #define OMAP24XX_GPMC_CSx_SIZE Size
 * #define OMAP24XX_GPMC_CSx_MAP Map
 * Where:
 * x - CS number
 * PART - Part Name as defined above
 * SIZE - how big is the mapping to be
 *   GPMC_SIZE_128M - 0x8
 *   GPMC_SIZE_64M  - 0xC
 *   GPMC_SIZE_32M  - 0xE
 *   GPMC_SIZE_16M  - 0xF
 * MAP  - Map this CS to which address(GPMC address space)- Absolute address
 *   >>24 before being used.
 */

#define GPMC_SIZE_256M  0x0
#define GPMC_SIZE_128M  0x8
#define GPMC_SIZE_64M   0xC
#define GPMC_SIZE_32M   0xE
#define GPMC_SIZE_16M   0xF

#if defined(PRCM_CONFIG_II) || defined(PRCM_CONFIG_5B) /* L3 at 100MHz */
# define SMNAND_GPMC_CONFIG1 0x0
# define SMNAND_GPMC_CONFIG2 0x00141400
# define SMNAND_GPMC_CONFIG3 0x00141400
# define SMNAND_GPMC_CONFIG4 0x0F010F01
# define SMNAND_GPMC_CONFIG5 0x010C1414
# define SMNAND_GPMC_CONFIG6 0x00000A80
# define STNOR_GPMC_CONFIG1  0x3
# define STNOR_GPMC_CONFIG2  0x000f0f01
# define STNOR_GPMC_CONFIG3  0x00050502
# define STNOR_GPMC_CONFIG4  0x0C060C06
# define STNOR_GPMC_CONFIG5  0x01131F1F
# define STNOR_GPMC_CONFIG6  0x0  /* 0? Not defined so far... this value is reset val as per gpmc doc */
# define MPDB_GPMC_CONFIG1   0x00011000
# define MPDB_GPMC_CONFIG2   0x001F1F00
# define MPDB_GPMC_CONFIG3   0x00080802
# define MPDB_GPMC_CONFIG4   0x1C091C09
# define MPDB_GPMC_CONFIG5   0x031A1F1F
# define MPDB_GPMC_CONFIG6   0x000003C2
#endif

#if defined(PRCM_CONFIG_III) || defined(PRCM_CONFIG_5A) || defined(PRCM_CONFIG_3) /* L3 at 133MHz */
# define SMNAND_GPMC_CONFIG1 0x00001800
# define SMNAND_GPMC_CONFIG2 0x00141400
# define SMNAND_GPMC_CONFIG3 0x00141400
# define SMNAND_GPMC_CONFIG4 0x0F010F01
# define SMNAND_GPMC_CONFIG5 0x010C1414
# define SMNAND_GPMC_CONFIG6 0x00000A80
# define SMNAND_GPMC_CONFIG7 0x00000C44

# define STNOR_GPMC_CONFIG1  0x3
# define STNOR_GPMC_CONFIG2  0x00151501
# define STNOR_GPMC_CONFIG3  0x00060602
# define STNOR_GPMC_CONFIG4  0x10081008
# define STNOR_GPMC_CONFIG5  0x01131F1F
# define STNOR_GPMC_CONFIG6  0x000004c4

# define MPDB_GPMC_CONFIG1  0x00011000
# define MPDB_GPMC_CONFIG2  0x001f1f01
# define MPDB_GPMC_CONFIG3  0x00080803
# define MPDB_GPMC_CONFIG4  0x1C091C09
# define MPDB_GPMC_CONFIG5  0x041f1F1F
# define MPDB_GPMC_CONFIG6  0x000004C4

# define SIBNOR_GPMC_CONFIG1  0x3
# define SIBNOR_GPMC_CONFIG2  0x00151501
# define SIBNOR_GPMC_CONFIG3  0x00060602
# define SIBNOR_GPMC_CONFIG4  0x10081008
# define SIBNOR_GPMC_CONFIG5  0x01131F1F
# define SIBNOR_GPMC_CONFIG6  0x00000000

# define ONENAND_GPMC_CONFIG1 0x00001200
# define ONENAND_GPMC_CONFIG2 0x000c0c01
# define ONENAND_GPMC_CONFIG3 0x00030301
# define ONENAND_GPMC_CONFIG4 0x0c040c04
# define ONENAND_GPMC_CONFIG5 0x010C1010
# define ONENAND_GPMC_CONFIG6 0x00000000

# define PCMCIA_GPMC_CONFIG1 0x01E91200
# define PCMCIA_GPMC_CONFIG2 0x001E1E01
# define PCMCIA_GPMC_CONFIG3 0x00020203
# define PCMCIA_GPMC_CONFIG4 0x1D041D04
# define PCMCIA_GPMC_CONFIG5 0x031D1F1F
# define PCMCIA_GPMC_CONFIG6 0x000004C4
#endif /* endif CFG_PRCM_III */

#if defined (PRCM_CONFIG_I) || defined(PRCM_CONFIG_2) /* L3 at 165MHz */
# define SMNAND_GPMC_CONFIG1 0x00001800
# define SMNAND_GPMC_CONFIG2 0x00141400
# define SMNAND_GPMC_CONFIG3 0x00141400
# define SMNAND_GPMC_CONFIG4 0x0F010F01
# define SMNAND_GPMC_CONFIG5 0x010C1414
# define SMNAND_GPMC_CONFIG6 0x00000A80
# define SMNAND_GPMC_CONFIG7 0x00000C44

# define STNOR_GPMC_CONFIG1  0x3
# define STNOR_GPMC_CONFIG2  0x00151501
# define STNOR_GPMC_CONFIG3  0x00060602
# define STNOR_GPMC_CONFIG4  0x11091109
# define STNOR_GPMC_CONFIG5  0x01141F1F
# define STNOR_GPMC_CONFIG6  0x000004c4

# define MPDB_GPMC_CONFIG1  0x00011000
# define MPDB_GPMC_CONFIG2  0x001f1f01
# define MPDB_GPMC_CONFIG3  0x00080803
# define MPDB_GPMC_CONFIG4  0x1c0b1c0a
# define MPDB_GPMC_CONFIG5  0x041f1F1F
# define MPDB_GPMC_CONFIG6  0x000004C4

# define SIBNOR_GPMC_CONFIG1  0x3
# define SIBNOR_GPMC_CONFIG2  0x00151501
# define SIBNOR_GPMC_CONFIG3  0x00060602
# define SIBNOR_GPMC_CONFIG4  0x11091109
# define SIBNOR_GPMC_CONFIG5  0x01141F1F
# define SIBNOR_GPMC_CONFIG6  0x00000000

# define ONENAND_GPMC_CONFIG1 0x00001200
# define ONENAND_GPMC_CONFIG2 0x000F0F01
# define ONENAND_GPMC_CONFIG3 0x00030301
# define ONENAND_GPMC_CONFIG4 0x0F040F04
# define ONENAND_GPMC_CONFIG5 0x010F1010
# define ONENAND_GPMC_CONFIG6 0x00000000

# define PCMCIA_GPMC_CONFIG1 0x01E91200
# define PCMCIA_GPMC_CONFIG2 0x001E1E01
# define PCMCIA_GPMC_CONFIG3 0x00020203
# define PCMCIA_GPMC_CONFIG4 0x1D041D04
# define PCMCIA_GPMC_CONFIG5 0x031D1F1F
# define PCMCIA_GPMC_CONFIG6 0x000004C4

#endif

#if 0
/* Board Specific Settings for each of the configurations for chips
 * whose values change as per platform. - None currently
 */
#if CONFIG_OMAP24XXH4
#include <asm/arch/mem-h4.h>
#endif

#if CONFIG_2430SDP
#include <asm/arch/mem-sdp2430.h>
#endif

#endif /* if 0 */

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS    8
/* max number of GPMC regs */
#define GPMC_MAX_REG   7

#define PROC_NOR       1
#define PROC_NAND      2
#define PISMO_SIBLEY0  3
#define PISMO_SIBLEY1  4
#define PISMO_ONENAND  5
#define DBG_MPDB       6
#define PISMO_PCMCIA   7

/* make it readable for the gpmc_init */
#define PROC_NOR_BASE   	FLASH_BASE
#define PROC_NAND_BASE  	NAND_BASE
#define PISMO_SIB0_BASE 	SIBLEY_MAP1
#define PISMO_SIB1_BASE 	SIBLEY_MAP2
#define PISMO_ONEN_BASE 	ONENAND_MAP
#define DBG_MPDB_BASE   	DEBUG_BASE
#define PISMO_PCMCIA_BASE  	PCMCIA_BASE

#endif /* endif _OMAP24XX_MEM_H_ */
