/*
 *    Copyright (c) 2010-2011 Nest, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program. If not, see
 *    <http://www.gnu.org/licenses/>.
 *
 *    Description:
 *      This file is the LCD panel driver for the Tianma TM025ZDZ01
 *      320 x 320 TFT LCD display panel using the Samsung S6D05A1
 *      interface driver.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/s6d05a1.h>
#include <linux/spi/spi.h>
#include <linux/fb.h>
#include <linux/slab.h>

#include <plat/display.h>

/* Preprocessor Definitions */

/*
 * Driver Strings
 */
#define TM025ZDZ01_DRIVER_NAME			"Tianma TM025ZDZ01 LCD Driver"
#define TM025ZDZ01_DRIVER_VERSION		"2011-05-09"

/*
 * 32- and 8-bit encode and decode macros
 */

#define U32_ENCODE(b3, b2, b1, b0)		((((b3) & 0xFF) << 24) |	\
										 (((b2) & 0xFF) << 16) |	\
										 (((b1) & 0xFF) <<  8) |	\
										 (((b0) & 0xFF) <<  0))
#define U32_B3_DECODE(u32)				(((u32) >> 24) & 0xFF)
#define U32_B2_DECODE(u32)				(((u32) >> 16) & 0xFF)
#define U32_B1_DECODE(u32)				(((u32) >>  8) & 0xFF)
#define U32_B0_DECODE(u32)				(((u32) >>  0) & 0xFF)

/*
 * Samsung S6D05A1 Slave SPI Protocol Definitions
 */
#define S6D05A1_SPI_XSCK_MAX			15151515
#define S6D05A1_SPI_MODE				SPI_MODE_3
#define S6D05A1_SPI_BITS_PER_WORD		9
#define S6D05A1_SPI_BYTES_PER_WORD		sizeof (s6d05a1_word)

/*
 * Samsung S6D05A1 Reset Timings
 */
#define S6D05A1_TRES_LOW_MS_MIN			10   //data sheet requires 5, double for margin
#define S6D05A1_TRES_HIGH_MS_MIN		120

/*
 * Samsung S6D05A1 Maximum Resolutions
 */
#define S6D05A1_H_RES_MAX				360
#define	S6D05A1_V_RES_MAX				480

/*
 * Samsung S6D05A1 Commands
 */

#define	S6D05A1_REG_VAL(bit, value)					\
	((value) << (bit))
#define S6D05A1_REG_VAL_ENCODE(shift, mask, value)	\
	(((value) << (shift)) & (mask))

/*
 * Level 1 Commands
 *
 * From Section 5.1, Table 111, Page 249 of "S6D05A1 Ref. Specification, P.03"
 */

/* Control Commands (Zero Parameters) */

#define S6D05A1_OP_CMD_NOP								0x00
#define S6D05A1_OP_CMD_SOFT_RESET						0x01
#define S6D05A1_OP_CMD_ENTER_SLEEP_MODE					0x10
#define S6D05A1_OP_CMD_EXIT_SLEEP_MODE					0x11
#define S6D05A1_OP_CMD_ENTER_PARTIAL_MODE				0x12
#define S6D05A1_OP_CMD_ENTER_NORMAL_MODE				0x13
#define S6D05A1_OP_CMD_EXIT_INVERT_MODE					0x20
#define S6D05A1_OP_CMD_ENTER_INVERT_MODE				0x21
#define S6D05A1_OP_CMD_DISPLAY_OFF						0x28
#define S6D05A1_OP_CMD_DISPLAY_ON						0x29
#define S6D05A1_OP_CMD_TEAR_OFF							0x34
#define S6D05A1_OP_CMD_EXIT_IDLE_MODE					0x38
#define S6D05A1_OP_CMD_ENTER_IDLE_MODE					0x39

/* Read / Get Commands (One or More Read Parameters) */

#define S6D05A1_OP_GET_RED_CHANNEL						0x06
#define S6D05A1_OP_GET_GREEN_CHANNEL					0x07
#define S6D05A1_OP_GET_BLUE_CHANNEL						0x08
#define	S6D05A1_OP_GET_DISPLAY_STATUS					0x09
#define S6D05A1_OP_GET_POWER_MODE						0x0A
#define S6D05A1_OP_GET_ADDRESS_MODE						0x0B
#define S6D05A1_OP_GET_MADCTL							S6D05A1_OP_GET_ADDRESS_MODE
#define S6D05A1_OP_GET_PIXEL_FORMAT						0x0C
#define S6D05A1_OP_GET_DISPLAY_MODE						0x0D
#define S6D05A1_OP_GET_SIGNAL_MODE						0x0E
#define S6D05A1_OP_GET_DIAGNOSTIC_RESULT				0x0F
#define S6D05A1_OP_GET_MEMORY_READ						0x2E
#define S6D05A1_OP_GET_MEMORY_CONTINUE					0x3E
#define S6D05A1_OP_GET_TEARING_SCANLINE					0x45
#define S6D05A1_OP_GET_BACKLIGHT_CONTROL				0x54
#define S6D05A1_OP_GET_DISPLAY_BRIGHTNESS				0x52
#define S6D05A1_OP_GET_MIE_MODE							0x56
#define S6D05A1_OP_GET_MINIMUM_BRIGHTNESS				0x5F
#define S6D05A1_OP_GET_DDB_START						0xA1
#define S6D05A1_OP_GET_DDB_CONTINUE						0xA8
#define S6D05A1_OP_GET_ID1								0xDA
#define S6D05A1_OP_GET_ID2								0xDB
#define S6D05A1_OP_GET_ID3								0xDC

/* Write / Set Commands (One or More Write Parameters) */

#define S6D05A1_OP_SET_COLUMN_ADDRESS					0x2A
#define S6D05A1_OP_SET_PAGE_ADDRESS						0x2B
#define S6D05A1_OP_SET_MEMORY_WRITE						0x2C
#define S6D05A1_OP_SET_PARTIAL_AREA						0x30
#define S6D05A1_OP_SET_TEAR_ON							0x35
#define S6D05A1_OP_SET_ADDRESS_MODE						0x36
#define	S6D05A1_OP_SET_MADCTL							S6D05A1_OP_SET_ADDRESS_MODE
#define S6D05A1_OP_SET_MEMORY_DATA_ACCESS_CONTROL		S6D05A1_OP_SET_ADDRESS_MODE
#define   S6D05A1_ADDRESS_MODE_PAGE_ADDR_ORDER_TTOB			S6D05A1_REG_VAL(7, 0)
#define   S6D05A1_ADDRESS_MODE_PAGE_ADDR_ORDER_BTOT			S6D05A1_REG_VAL(7, 1)
#define   S6D05A1_ADDRESS_MODE_COL_ADDR_ORDER_TTOB			S6D05A1_REG_VAL(6, 0)
#define   S6D05A1_ADDRESS_MODE_COL_ADDR_ORDER_BTOT			S6D05A1_REG_VAL(6, 1)
#define   S6D05A1_ADDRESS_MODE_PAGE_COL_NORMAL				S6D05A1_REG_VAL(5, 0)
#define   S6D05A1_ADDRESS_MODE_PAGE_COL_REVERSE				S6D05A1_REG_VAL(5, 1)
#define   S6D05A1_ADDRESS_MODE_VERT_REFR_ORDER_TTOB			S6D05A1_REG_VAL(4, 0)
#define   S6D05A1_ADDRESS_MODE_VERT_REFR_ORDER_BTOT			S6D05A1_REG_VAL(4, 1)
#define   S6D05A1_ADDRESS_MODE_PIXEL_ORDER_RGB				S6D05A1_REG_VAL(3, 0)
#define   S6D05A1_ADDRESS_MODE_PIXEL_ORDER_BGR				S6D05A1_REG_VAL(3, 1)
#define S6D05A1_OP_SET_PIXEL_FORMAT						0x3A
#define   S6D05A1_PIXEL_FORMAT_16BPP						S6D05A1_REG_VAL(0, 0x55)
#define   S6D05A1_PIXEL_FORMAT_18BPP						S6D05A1_REG_VAL(0, 0x66)
#define   S6D05A1_PIXEL_FORMAT_24BPP						S6D05A1_REG_VAL(0, 0x77)
#define S6D05A1_OP_SET_MEMORY_CONTINUE					0x3C
#define S6D05A1_OP_SET_TEARING_SCANLINE					0x44
#define S6D05A1_OP_SET_MANUAL_BRIGHTNESS				0x51
#define S6D05A1_OP_SET_BACKLIGHT_CONTROL				0x53
#define S6D05A1_OP_SET_MIE_MODE							0x55
#define   S6D05_MIE_MODE_OFF								S6D05A1_REG_VAL(0, 0)
#define   S6D05_MIE_MODE_UI									S6D05A1_REG_VAL(0, 1)
#define   S6D05_MIE_MODE_STILL								S6D05A1_REG_VAL(0, 2)
#define   S6D05_MIE_MODE_MOVING								S6D05A1_REG_VAL(0, 3)
#define S6D05A1_OP_SET_MINIMUM_BRIGHTNESS				0x5E

/* Transfer Commands (One or More Read and Write Parameters) */

/*
 * Level 2 Commands
 *
 * From Section 5.2, Table 114, Page 333 of "S6D05A1 Ref. Specification, P.03"
 */

/* Control Commands (Zero Parameters) */

/* Read / Get Commands (One or More Read Parameters) */

#define S6D05A1_OP_GET_MTPRD							0xD3

/* Write / Set Commands (One or More Write Parameters) */

#define S6D05A1_OP_SET_MIECTL1							0xC0
#define   S6D05A1_MIECTL1_RRC_ENCODE(n)						S6D05A1_REG_VAL_ENCODE(0, 0xFF, n)
#define   S6D05A1_MIECTL1_IERC_ENCODE(n)					S6D05A1_REG_VAL_ENCODE(0, 0xFF, n)
#define   S6D05A1_MIECTL1_SERC_ENCODE(n)					S6D05A1_REG_VAL_ENCODE(0, 0x1F, n)
#define   S6D05A1_MIECTL1_DIMMING_DISABLE					S6D05A1_REG_VAL(5, 0)
#define	  S6D05A1_MIECTL1_DIMMING_ENABLE					S6D05A1_REG_VAL(5, 1)
#define S6D05A1_OP_SET_BCMODE							0xC1
#define S6D05A1_OP_SET_MIECTL2							0xC2
#define S6D05A1_OP_SET_WRBLCTL							0xC3
#define S6D05A1_OP_SET_MTPCTL							0xD0
#define S6D05A1_OP_SET_MTPWR							0xD2
#define S6D05A1_OP_SET_DSTB								0xDF
#define S6D05A1_OP_SET_MDDICTL							0xEA
#define S6D05A1_OP_SET_MDDILIK							0xEB
#define S6D05A1_OP_SET_PASSWD1							0xF0
#define   S6D05A1_PASSWD1_L2_ENABLE							0x5A5A
#define   S6D05A1_PASSWD1_L2_DISABLE						0xA5A5
#define S6D05A1_OP_SET_PASSWD2							0xF1
#define   S6D05A1_PASSWD2_L2_ENABLE							0x5A5A
#define   S6D05A1_PASSWD2_L2_DISABLE						0xA5A5
#define S6D05A1_OP_SET_DISCTL							0xF2
#define   S6D05A1_DISCTL_NL_ENCODE(n)						S6D05A1_REG_VAL(0, (((n) - 1) >> 3))
#define   S6D05A1_DISCTL_NINV_FRAME							S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_DISCTL_NINV_LINE							S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_DISCTL_PINV_FRAME							S6D05A1_REG_VAL(1, 0)
#define   S6D05A1_DISCTL_PINV_LINE							S6D05A1_REG_VAL(1, 1)
#define   S6D05A1_DISCTL_IINV_FRAME							S6D05A1_REG_VAL(2, 0)
#define   S6D05A1_DISCTL_IINV_LINE							S6D05A1_REG_VAL(2, 1)
#define   S6D05A1_DISCTL_PIINV_FRAME						S6D05A1_REG_VAL(3, 0)
#define   S6D05A1_DISCTL_PIINV_LINE							S6D05A1_REG_VAL(3, 1)
#define   S6D05A1_DISCTL_REV_BLACK							S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_DISCTL_REV_WHITE							S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_DISCTL_GS_LO_TO_HI						S6D05A1_REG_VAL(1, 0)
#define   S6D05A1_DISCTL_GS_HI_TO_LO						S6D05A1_REG_VAL(1, 1)
#define   S6D05A1_DISCTL_SM_EVEN_ODD						S6D05A1_REG_VAL(2, 0)
#define   S6D05A1_DISCTL_SM_UPPER_LOWER					S6D05A1_REG_VAL(2, 1)

#define S6D05A1_OP_SET_MANPWRSEQ						0xF3
#define	  S6D05A1_MANPWRSEQ_APON_DISABLE					S6D05A1_REG_VAL(0, 0)
#define	  S6D05A1_MANPWRSEQ_APON_ENABLE						S6D05A1_REG_VAL(0, 1)
#define S6D05A1_OP_SET_PWRCTL							0xF4
#define   S6D05A1_PWRCTL_VCI1_2v10							S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_PWRCTL_VCI1_2v16							S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_PWRCTL_VCI1_2v22							S6D05A1_REG_VAL(0, 2)
#define   S6D05A1_PWRCTL_VCI1_2v28							S6D05A1_REG_VAL(0, 3)
#define   S6D05A1_PWRCTL_VCI1_2v34							S6D05A1_REG_VAL(0, 4)
#define   S6D05A1_PWRCTL_VCI1_2v40							S6D05A1_REG_VAL(0, 5)
#define   S6D05A1_PWRCTL_VCI1_2v46							S6D05A1_REG_VAL(0, 6)
#define   S6D05A1_PWRCTL_VCI1_2v52							S6D05A1_REG_VAL(0, 7)
#define   S6D05A1_PWRCTL_VCI1_2v58							S6D05A1_REG_VAL(0, 8)
#define   S6D05A1_PWRCTL_VCI1_2v64							S6D05A1_REG_VAL(0, 9)
#define   S6D05A1_PWRCTL_VCI1_2v70							S6D05A1_REG_VAL(0, 10)
#define   S6D05A1_PWRCTL_VCI1_2v76							S6D05A1_REG_VAL(0, 11)
#define   S6D05A1_PWRCTL_VCI1_2v82							S6D05A1_REG_VAL(0, 12)
#define   S6D05A1_PWRCTL_VCI1_2v88							S6D05A1_REG_VAL(0, 13)
#define   S6D05A1_PWRCTL_VCI1_2v94							S6D05A1_REG_VAL(0, 14)
#define   S6D05A1_PWRCTL_VCI1_3v00							S6D05A1_REG_VAL(0, 15)
#define   S6D05A1_PWRCTL_BOOST_CLK_SEL_EXT					S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_PWRCTL_BOOST_CLK_SEL_INT					S6D05A1_REG_VAL(0, 1)
#define	  S6D05A1_PWRCTL_DC_ENCODE(i, n)					S6D05A1_REG_VAL_ENCODE((((i) - 1) * 2), 0x3 << (((i) - 1) * 2), n)
#define	    S6D05A1_PWRCTL_DC_1_2							0x0
#define	    S6D05A1_PWRCTL_DC_1_1							0x1
#define	    S6D05A1_PWRCTL_DC_1_0p5							0x2
#define	    S6D05A1_PWRCTL_DC_1_0p25						0x3
#define   S6D05A1_PWRCTL_NDC_ENCODE(i, n)					S6D05A1_PWRCTL_DC_ENCODE(i, n)
#define   S6D05A1_PWRCTL_PIDC_ENCODE(i, n)					S6D05A1_PWRCTL_DC_ENCODE(i, n)
#define   S6D05A1_PWRCTL_GVD_2v46							0x00
#define   S6D05A1_PWRCTL_GVD_2v48							0x01
#define   S6D05A1_PWRCTL_GVD_2v50							0x02
#define   S6D05A1_PWRCTL_GVD_2v52							0x03
#define   S6D05A1_PWRCTL_GVD_2v54							0x04
#define   S6D05A1_PWRCTL_GVD_2v56							0x05
#define   S6D05A1_PWRCTL_GVD_2v58							0x06
#define   S6D05A1_PWRCTL_GVD_2v60							0x07
#define   S6D05A1_PWRCTL_GVD_2v62							0x08
#define   S6D05A1_PWRCTL_GVD_2v64							0x09
#define   S6D05A1_PWRCTL_GVD_2v66							0x0a
#define   S6D05A1_PWRCTL_GVD_2v68							0x0b
#define   S6D05A1_PWRCTL_GVD_2v70							0x0c
#define   S6D05A1_PWRCTL_GVD_2v72							0x0d
#define   S6D05A1_PWRCTL_GVD_2v74							0x0e
#define   S6D05A1_PWRCTL_GVD_2v76							0x0f
#define   S6D05A1_PWRCTL_GVD_2v78							0x10
#define   S6D05A1_PWRCTL_GVD_2v80							0x11
#define   S6D05A1_PWRCTL_GVD_2v82							0x12
#define   S6D05A1_PWRCTL_GVD_2v84							0x13
#define   S6D05A1_PWRCTL_GVD_2v86							0x14
#define   S6D05A1_PWRCTL_GVD_2v88							0x15
#define   S6D05A1_PWRCTL_GVD_2v90							0x16
#define   S6D05A1_PWRCTL_GVD_2v92							0x17
#define   S6D05A1_PWRCTL_GVD_2v94							0x18
#define   S6D05A1_PWRCTL_GVD_2v96							0x19
#define   S6D05A1_PWRCTL_GVD_2v98							0x1a
#define   S6D05A1_PWRCTL_GVD_3v00							0x1b
#define   S6D05A1_PWRCTL_GVD_3v02							0x1c
#define   S6D05A1_PWRCTL_GVD_3v04							0x1d
#define   S6D05A1_PWRCTL_GVD_3v06							0x1e
#define   S6D05A1_PWRCTL_GVD_3v08							0x1f
#define   S6D05A1_PWRCTL_GVD_3v10							0x20
#define   S6D05A1_PWRCTL_GVD_3v12							0x21
#define   S6D05A1_PWRCTL_GVD_3v14							0x22
#define   S6D05A1_PWRCTL_GVD_3v16							0x23
#define   S6D05A1_PWRCTL_GVD_3v18							0x24
#define   S6D05A1_PWRCTL_GVD_3v20							0x25
#define   S6D05A1_PWRCTL_GVD_3v22							0x26
#define   S6D05A1_PWRCTL_GVD_3v24							0x27
#define   S6D05A1_PWRCTL_GVD_3v26							0x28
#define   S6D05A1_PWRCTL_GVD_3v28							0x29
#define   S6D05A1_PWRCTL_GVD_3v30							0x2a
#define   S6D05A1_PWRCTL_GVD_3v32							0x2b
#define   S6D05A1_PWRCTL_GVD_3v34							0x2c
#define   S6D05A1_PWRCTL_GVD_3v36							0x2d
#define   S6D05A1_PWRCTL_GVD_3v38							0x2e
#define   S6D05A1_PWRCTL_GVD_3v40							0x2f
#define   S6D05A1_PWRCTL_GVD_3v42							0x30
#define   S6D05A1_PWRCTL_GVD_3v44							0x31
#define   S6D05A1_PWRCTL_GVD_3v46							0x32
#define   S6D05A1_PWRCTL_GVD_3v48							0x33
#define   S6D05A1_PWRCTL_GVD_3v50							0x34
#define   S6D05A1_PWRCTL_GVD_3v52							0x35
#define   S6D05A1_PWRCTL_GVD_3v54							0x36
#define   S6D05A1_PWRCTL_GVD_3v56							0x37
#define   S6D05A1_PWRCTL_GVD_3v58							0x38
#define   S6D05A1_PWRCTL_GVD_3v60							0x39
#define   S6D05A1_PWRCTL_GVD_3v62							0x3a
#define   S6D05A1_PWRCTL_GVD_3v64							0x3b
#define   S6D05A1_PWRCTL_GVD_3v66							0x3c
#define   S6D05A1_PWRCTL_GVD_3v68							0x3d
#define   S6D05A1_PWRCTL_GVD_3v70							0x3e
#define   S6D05A1_PWRCTL_GVD_3v72							0x3f
#define   S6D05A1_PWRCTL_GVD_3v74							0x40
#define   S6D05A1_PWRCTL_GVD_3v76							0x41
#define   S6D05A1_PWRCTL_GVD_3v78							0x42
#define   S6D05A1_PWRCTL_GVD_3v80							0x43
#define   S6D05A1_PWRCTL_GVD_3v82							0x44
#define   S6D05A1_PWRCTL_GVD_3v84							0x45
#define   S6D05A1_PWRCTL_GVD_3v86							0x46
#define   S6D05A1_PWRCTL_GVD_3v88							0x47
#define   S6D05A1_PWRCTL_GVD_3v90							0x48
#define   S6D05A1_PWRCTL_GVD_3v92							0x49
#define   S6D05A1_PWRCTL_GVD_3v94							0x4a
#define   S6D05A1_PWRCTL_GVD_3v96							0x4b
#define   S6D05A1_PWRCTL_GVD_3v98							0x4c
#define   S6D05A1_PWRCTL_GVD_4v00							0x4d
#define   S6D05A1_PWRCTL_GVD_4v02							0x4e
#define   S6D05A1_PWRCTL_GVD_4v04							0x4f
#define   S6D05A1_PWRCTL_GVD_4v06							0x50
#define   S6D05A1_PWRCTL_GVD_4v08							0x51
#define   S6D05A1_PWRCTL_GVD_4v10							0x52
#define   S6D05A1_PWRCTL_GVD_4v12							0x53
#define   S6D05A1_PWRCTL_GVD_4v14							0x54
#define   S6D05A1_PWRCTL_GVD_4v16							0x55
#define   S6D05A1_PWRCTL_GVD_4v18							0x56
#define   S6D05A1_PWRCTL_GVD_4v20							0x57
#define   S6D05A1_PWRCTL_GVD_4v22							0x58
#define   S6D05A1_PWRCTL_GVD_4v24							0x59
#define   S6D05A1_PWRCTL_GVD_4v26							0x5a
#define   S6D05A1_PWRCTL_GVD_4v28							0x5b
#define   S6D05A1_PWRCTL_GVD_4v30							0x5c
#define   S6D05A1_PWRCTL_GVD_4v32							0x5d
#define   S6D05A1_PWRCTL_GVD_4v34							0x5e
#define   S6D05A1_PWRCTL_GVD_4v36							0x5f
#define   S6D05A1_PWRCTL_GVD_4v38							0x60
#define   S6D05A1_PWRCTL_GVD_4v40							0x61
#define   S6D05A1_PWRCTL_GVD_4v42							0x62
#define   S6D05A1_PWRCTL_GVD_4v44							0x63
#define   S6D05A1_PWRCTL_GVD_4v46							0x64
#define   S6D05A1_PWRCTL_GVD_4v48							0x65
#define   S6D05A1_PWRCTL_GVD_4v50							0x66
#define   S6D05A1_PWRCTL_GVD_4v52							0x67
#define   S6D05A1_PWRCTL_GVD_4v54							0x68
#define   S6D05A1_PWRCTL_GVD_4v56							0x69
#define   S6D05A1_PWRCTL_GVD_4v58							0x6a
#define   S6D05A1_PWRCTL_GVD_4v60							0x6b
#define   S6D05A1_PWRCTL_GVD_4v62							0x6c
#define   S6D05A1_PWRCTL_GVD_4v64							0x6d
#define   S6D05A1_PWRCTL_GVD_4v66							0x6e
#define   S6D05A1_PWRCTL_GVD_4v68							0x6f
#define   S6D05A1_PWRCTL_GVD_4v70							0x70
#define   S6D05A1_PWRCTL_GVD_4v72							0x71
#define   S6D05A1_PWRCTL_GVD_4v74							0x72
#define   S6D05A1_PWRCTL_GVD_4v76							0x73
#define   S6D05A1_PWRCTL_GVD_4v78							0x74
#define   S6D05A1_PWRCTL_GVD_4v80							0x75
#define   S6D05A1_PWRCTL_GVD_4v82							0x76
#define   S6D05A1_PWRCTL_GVD_4v84							0x77
#define   S6D05A1_PWRCTL_GVD_4v86							0x78
#define   S6D05A1_PWRCTL_GVD_4v88							0x79
#define   S6D05A1_PWRCTL_GVD_4v90							0x7a
#define   S6D05A1_PWRCTL_GVD_4v92							0x7b
#define   S6D05A1_PWRCTL_GVD_4v94							0x7c
#define   S6D05A1_PWRCTL_GVD_4v96							0x7d
#define   S6D05A1_PWRCTL_GVD_4v98							0x7e
#define   S6D05A1_PWRCTL_GVD_5v00							0x7f
#define	  S6D05A1_PWRCTL_BT_13v75_NEG_8v25					0x0
#define	  S6D05A1_PWRCTL_BT_13v75_NEG_11v00					0x1
#define	  S6D05A1_PWRCTL_BT_16v50_NEG_8v25					0x2
#define	  S6D05A1_PWRCTL_BT_16v50_NEG_11v00					0x3
#define	  S6D05A1_PWRCTL_BT_16v50_NEG_13v75					0x4
#define	  S6D05A1_PWRCTL_BT_19v25_NEG_11v00					0x5
#define S6D05A1_OP_SET_VCMCTL							0xF5
#define   S6D05A1_VCMCTL_VCMH_2v46							0x00
#define   S6D05A1_VCMCTL_VCMH_2v48							0x01
#define   S6D05A1_VCMCTL_VCMH_2v50							0x02
#define   S6D05A1_VCMCTL_VCMH_2v52							0x03
#define   S6D05A1_VCMCTL_VCMH_2v54							0x04
#define   S6D05A1_VCMCTL_VCMH_2v56							0x05
#define   S6D05A1_VCMCTL_VCMH_2v58							0x06
#define   S6D05A1_VCMCTL_VCMH_2v60							0x07
#define   S6D05A1_VCMCTL_VCMH_2v62							0x08
#define   S6D05A1_VCMCTL_VCMH_2v64							0x09
#define   S6D05A1_VCMCTL_VCMH_2v66							0x0a
#define   S6D05A1_VCMCTL_VCMH_2v68							0x0b
#define   S6D05A1_VCMCTL_VCMH_2v70							0x0c
#define   S6D05A1_VCMCTL_VCMH_2v72							0x0d
#define   S6D05A1_VCMCTL_VCMH_2v74							0x0e
#define   S6D05A1_VCMCTL_VCMH_2v76							0x0f
#define   S6D05A1_VCMCTL_VCMH_2v78							0x10
#define   S6D05A1_VCMCTL_VCMH_2v80							0x11
#define   S6D05A1_VCMCTL_VCMH_2v82							0x12
#define   S6D05A1_VCMCTL_VCMH_2v84							0x13
#define   S6D05A1_VCMCTL_VCMH_2v86							0x14
#define   S6D05A1_VCMCTL_VCMH_2v88							0x15
#define   S6D05A1_VCMCTL_VCMH_2v90							0x16
#define   S6D05A1_VCMCTL_VCMH_2v92							0x17
#define   S6D05A1_VCMCTL_VCMH_2v94							0x18
#define   S6D05A1_VCMCTL_VCMH_2v96							0x19
#define   S6D05A1_VCMCTL_VCMH_2v98							0x1a
#define   S6D05A1_VCMCTL_VCMH_3v00							0x1b
#define   S6D05A1_VCMCTL_VCMH_3v02							0x1c
#define   S6D05A1_VCMCTL_VCMH_3v04							0x1d
#define   S6D05A1_VCMCTL_VCMH_3v06							0x1e
#define   S6D05A1_VCMCTL_VCMH_3v08							0x1f
#define   S6D05A1_VCMCTL_VCMH_3v10							0x20
#define   S6D05A1_VCMCTL_VCMH_3v12							0x21
#define   S6D05A1_VCMCTL_VCMH_3v14							0x22
#define   S6D05A1_VCMCTL_VCMH_3v16							0x23
#define   S6D05A1_VCMCTL_VCMH_3v18							0x24
#define   S6D05A1_VCMCTL_VCMH_3v20							0x25
#define   S6D05A1_VCMCTL_VCMH_3v22							0x26
#define   S6D05A1_VCMCTL_VCMH_3v24							0x27
#define   S6D05A1_VCMCTL_VCMH_3v26							0x28
#define   S6D05A1_VCMCTL_VCMH_3v28							0x29
#define   S6D05A1_VCMCTL_VCMH_3v30							0x2a
#define   S6D05A1_VCMCTL_VCMH_3v32							0x2b
#define   S6D05A1_VCMCTL_VCMH_3v34							0x2c
#define   S6D05A1_VCMCTL_VCMH_3v36							0x2d
#define   S6D05A1_VCMCTL_VCMH_3v38							0x2e
#define   S6D05A1_VCMCTL_VCMH_3v40							0x2f
#define   S6D05A1_VCMCTL_VCMH_3v42							0x30
#define   S6D05A1_VCMCTL_VCMH_3v44							0x31
#define   S6D05A1_VCMCTL_VCMH_3v46							0x32
#define   S6D05A1_VCMCTL_VCMH_3v48							0x33
#define   S6D05A1_VCMCTL_VCMH_3v50							0x34
#define   S6D05A1_VCMCTL_VCMH_3v52							0x35
#define   S6D05A1_VCMCTL_VCMH_3v54							0x36
#define   S6D05A1_VCMCTL_VCMH_3v56							0x37
#define   S6D05A1_VCMCTL_VCMH_3v58							0x38
#define   S6D05A1_VCMCTL_VCMH_3v60							0x39
#define   S6D05A1_VCMCTL_VCMH_3v62							0x3a
#define   S6D05A1_VCMCTL_VCMH_3v64							0x3b
#define   S6D05A1_VCMCTL_VCMH_3v66							0x3c
#define   S6D05A1_VCMCTL_VCMH_3v68							0x3d
#define   S6D05A1_VCMCTL_VCMH_3v70							0x3e
#define   S6D05A1_VCMCTL_VCMH_3v72							0x3f
#define   S6D05A1_VCMCTL_VCMH_3v74							0x40
#define   S6D05A1_VCMCTL_VCMH_3v76							0x41
#define   S6D05A1_VCMCTL_VCMH_3v78							0x42
#define   S6D05A1_VCMCTL_VCMH_3v80							0x43
#define   S6D05A1_VCMCTL_VCMH_3v82							0x44
#define   S6D05A1_VCMCTL_VCMH_3v84							0x45
#define   S6D05A1_VCMCTL_VCMH_3v86							0x46
#define   S6D05A1_VCMCTL_VCMH_3v88							0x47
#define   S6D05A1_VCMCTL_VCMH_3v90							0x48
#define   S6D05A1_VCMCTL_VCMH_3v92							0x49
#define   S6D05A1_VCMCTL_VCMH_3v94							0x4a
#define   S6D05A1_VCMCTL_VCMH_3v96							0x4b
#define   S6D05A1_VCMCTL_VCMH_3v98							0x4c
#define   S6D05A1_VCMCTL_VCMH_4v00							0x4d
#define   S6D05A1_VCMCTL_VCMH_4v02							0x4e
#define   S6D05A1_VCMCTL_VCMH_4v04							0x4f
#define   S6D05A1_VCMCTL_VCMH_4v06							0x50
#define   S6D05A1_VCMCTL_VCMH_4v08							0x51
#define   S6D05A1_VCMCTL_VCMH_4v10							0x52
#define   S6D05A1_VCMCTL_VCMH_4v12							0x53
#define   S6D05A1_VCMCTL_VCMH_4v14							0x54
#define   S6D05A1_VCMCTL_VCMH_4v16							0x55
#define   S6D05A1_VCMCTL_VCMH_4v18							0x56
#define   S6D05A1_VCMCTL_VCMH_4v20							0x57
#define   S6D05A1_VCMCTL_VCMH_4v22							0x58
#define   S6D05A1_VCMCTL_VCMH_4v24							0x59
#define   S6D05A1_VCMCTL_VCMH_4v26							0x5a
#define   S6D05A1_VCMCTL_VCMH_4v28							0x5b
#define   S6D05A1_VCMCTL_VCMH_4v30							0x5c
#define   S6D05A1_VCMCTL_VCMH_4v32							0x5d
#define   S6D05A1_VCMCTL_VCMH_4v34							0x5e
#define   S6D05A1_VCMCTL_VCMH_4v36							0x5f
#define   S6D05A1_VCMCTL_VCMH_4v38							0x60
#define   S6D05A1_VCMCTL_VCMH_4v40							0x61
#define   S6D05A1_VCMCTL_VCMH_4v42							0x62
#define   S6D05A1_VCMCTL_VCMH_4v44							0x63
#define   S6D05A1_VCMCTL_VCMH_4v46							0x64
#define   S6D05A1_VCMCTL_VCMH_4v48							0x65
#define   S6D05A1_VCMCTL_VCMH_4v50							0x66
#define   S6D05A1_VCMCTL_VCMH_4v52							0x67
#define   S6D05A1_VCMCTL_VCMH_4v54							0x68
#define   S6D05A1_VCMCTL_VCMH_4v56							0x69
#define   S6D05A1_VCMCTL_VCMH_4v58							0x6a
#define   S6D05A1_VCMCTL_VCMH_4v60							0x6b
#define   S6D05A1_VCMCTL_VCMH_4v62							0x6c
#define   S6D05A1_VCMCTL_VCMH_4v64							0x6d
#define   S6D05A1_VCMCTL_VCMH_4v66							0x6e
#define   S6D05A1_VCMCTL_VCMH_4v68							0x6f
#define   S6D05A1_VCMCTL_VCMH_4v70							0x70
#define   S6D05A1_VCMCTL_VCMH_4v72							0x71
#define   S6D05A1_VCMCTL_VCMH_4v74							0x72
#define   S6D05A1_VCMCTL_VCMH_4v76							0x73
#define   S6D05A1_VCMCTL_VCMH_4v78							0x74
#define   S6D05A1_VCMCTL_VCMH_4v80							0x75
#define   S6D05A1_VCMCTL_VCMH_4v82							0x76
#define   S6D05A1_VCMCTL_VCMH_4v84							0x77
#define   S6D05A1_VCMCTL_VCMH_4v86							0x78
#define   S6D05A1_VCMCTL_VCMH_4v88							0x79
#define   S6D05A1_VCMCTL_VCMH_4v90							0x7a
#define   S6D05A1_VCMCTL_VCMH_4v92							0x7b
#define   S6D05A1_VCMCTL_VCMH_4v94							0x7c
#define   S6D05A1_VCMCTL_VCMH_4v96							0x7d
#define   S6D05A1_VCMCTL_VCMH_4v98							0x7e
#define   S6D05A1_VCMCTL_VCMH_5v00							0x7f
#define   S6D05A1_VCMCTL_VML_3v075							0x00
#define   S6D05A1_VCMCTL_VML_3v100							0x01
#define   S6D05A1_VCMCTL_VML_3v125							0x02
#define   S6D05A1_VCMCTL_VML_3v150							0x03
#define   S6D05A1_VCMCTL_VML_3v175							0x04
#define   S6D05A1_VCMCTL_VML_3v200							0x05
#define   S6D05A1_VCMCTL_VML_3v225							0x06
#define   S6D05A1_VCMCTL_VML_3v250							0x07
#define   S6D05A1_VCMCTL_VML_3v275							0x08
#define   S6D05A1_VCMCTL_VML_3v300							0x09
#define   S6D05A1_VCMCTL_VML_3v325							0x0a
#define   S6D05A1_VCMCTL_VML_3v350							0x0b
#define   S6D05A1_VCMCTL_VML_3v375							0x0c
#define   S6D05A1_VCMCTL_VML_3v400							0x0d
#define   S6D05A1_VCMCTL_VML_3v425							0x0e
#define   S6D05A1_VCMCTL_VML_3v450							0x0f
#define   S6D05A1_VCMCTL_VML_3v475							0x10
#define   S6D05A1_VCMCTL_VML_3v500							0x11
#define   S6D05A1_VCMCTL_VML_3v525							0x12
#define   S6D05A1_VCMCTL_VML_3v550							0x13
#define   S6D05A1_VCMCTL_VML_3v575							0x14
#define   S6D05A1_VCMCTL_VML_3v600							0x15
#define   S6D05A1_VCMCTL_VML_3v625							0x16
#define   S6D05A1_VCMCTL_VML_3v650							0x17
#define   S6D05A1_VCMCTL_VML_3v675							0x18
#define   S6D05A1_VCMCTL_VML_3v700							0x19
#define   S6D05A1_VCMCTL_VML_3v725							0x1a
#define   S6D05A1_VCMCTL_VML_3v750							0x1b
#define   S6D05A1_VCMCTL_VML_3v775							0x1c
#define   S6D05A1_VCMCTL_VML_3v800							0x1d
#define   S6D05A1_VCMCTL_VML_3v825							0x1e
#define   S6D05A1_VCMCTL_VML_3v850							0x1f
#define   S6D05A1_VCMCTL_VML_3v875							0x20
#define   S6D05A1_VCMCTL_VML_3v900							0x21
#define   S6D05A1_VCMCTL_VML_3v925							0x22
#define   S6D05A1_VCMCTL_VML_3v950							0x23
#define   S6D05A1_VCMCTL_VML_3v975							0x24
#define   S6D05A1_VCMCTL_VML_4v000							0x25
#define   S6D05A1_VCMCTL_VML_4v025							0x26
#define   S6D05A1_VCMCTL_VML_4v050							0x27
#define   S6D05A1_VCMCTL_VML_4v075							0x28
#define   S6D05A1_VCMCTL_VML_4v100							0x29
#define   S6D05A1_VCMCTL_VML_4v125							0x2a
#define   S6D05A1_VCMCTL_VML_4v150							0x2b
#define   S6D05A1_VCMCTL_VML_4v175							0x2c
#define   S6D05A1_VCMCTL_VML_4v200							0x2d
#define   S6D05A1_VCMCTL_VML_4v225							0x2e
#define   S6D05A1_VCMCTL_VML_4v250							0x2f
#define   S6D05A1_VCMCTL_VML_4v275							0x30
#define   S6D05A1_VCMCTL_VML_4v300							0x31
#define   S6D05A1_VCMCTL_VML_4v325							0x32
#define   S6D05A1_VCMCTL_VML_4v350							0x33
#define   S6D05A1_VCMCTL_VML_4v375							0x34
#define   S6D05A1_VCMCTL_VML_4v400							0x35
#define   S6D05A1_VCMCTL_VML_4v425							0x36
#define   S6D05A1_VCMCTL_VML_4v450							0x37
#define   S6D05A1_VCMCTL_VML_4v475							0x38
#define   S6D05A1_VCMCTL_VML_4v500							0x39
#define   S6D05A1_VCMCTL_VML_4v525							0x3a
#define   S6D05A1_VCMCTL_VML_4v550							0x3b
#define   S6D05A1_VCMCTL_VML_4v575							0x3c
#define   S6D05A1_VCMCTL_VML_4v600							0x3d
#define   S6D05A1_VCMCTL_VML_4v625							0x3e
#define   S6D05A1_VCMCTL_VML_4v650							0x3f
#define   S6D05A1_VCMCTL_VML_4v675							0x40
#define   S6D05A1_VCMCTL_VML_4v700							0x41
#define   S6D05A1_VCMCTL_VML_4v725							0x42
#define   S6D05A1_VCMCTL_VML_4v750							0x43
#define   S6D05A1_VCMCTL_VML_4v775							0x44
#define   S6D05A1_VCMCTL_VML_4v800							0x45
#define   S6D05A1_VCMCTL_VML_4v825							0x46
#define   S6D05A1_VCMCTL_VML_4v850							0x47
#define   S6D05A1_VCMCTL_VML_4v875							0x48
#define   S6D05A1_VCMCTL_VML_4v900							0x49
#define   S6D05A1_VCMCTL_VML_4v925							0x4a
#define   S6D05A1_VCMCTL_VML_4v950							0x4b
#define   S6D05A1_VCMCTL_VML_4v975							0x4c
#define   S6D05A1_VCMCTL_VML_5v000							0x4d
#define   S6D05A1_VCMCTL_VML_5v025							0x4e
#define   S6D05A1_VCMCTL_VML_5v050							0x4f
#define   S6D05A1_VCMCTL_VML_5v075							0x50
#define   S6D05A1_VCMCTL_VML_5v100							0x51
#define   S6D05A1_VCMCTL_VML_5v125							0x52
#define   S6D05A1_VCMCTL_VML_5v150							0x53
#define   S6D05A1_VCMCTL_VML_5v175							0x54
#define   S6D05A1_VCMCTL_VML_5v200							0x55
#define   S6D05A1_VCMCTL_VML_5v225							0x56
#define   S6D05A1_VCMCTL_VML_5v250							0x57
#define   S6D05A1_VCMCTL_VML_5v275							0x58
#define   S6D05A1_VCMCTL_VML_5v300							0x59
#define   S6D05A1_VCMCTL_VML_5v325							0x5a
#define   S6D05A1_VCMCTL_VML_5v350							0x5b
#define   S6D05A1_VCMCTL_VML_5v375							0x5c
#define   S6D05A1_VCMCTL_VML_5v400							0x5d
#define   S6D05A1_VCMCTL_VML_5v425							0x5e
#define   S6D05A1_VCMCTL_VML_5v450							0x5f
#define   S6D05A1_VCMCTL_VML_5v475							0x60
#define   S6D05A1_VCMCTL_VML_5v500							0x61
#define   S6D05A1_VCMCTL_VML_5v525							0x62
#define   S6D05A1_VCMCTL_VML_5v550							0x63
#define   S6D05A1_VCMCTL_VML_5v575							0x64
#define   S6D05A1_VCMCTL_VML_5v600							0x65
#define   S6D05A1_VCMCTL_VML_5v625							0x66
#define   S6D05A1_VCMCTL_VML_5v650							0x67
#define   S6D05A1_VCMCTL_VML_5v675							0x68
#define   S6D05A1_VCMCTL_VML_5v700							0x69
#define   S6D05A1_VCMCTL_VML_5v725							0x6a
#define   S6D05A1_VCMCTL_VML_5v750							0x6b
#define   S6D05A1_VCMCTL_VML_5v775							0x6c
#define   S6D05A1_VCMCTL_VML_5v800							0x6d
#define   S6D05A1_VCMCTL_VML_5v825							0x6e
#define   S6D05A1_VCMCTL_VML_5v850							0x6f
#define   S6D05A1_VCMCTL_VML_5v875							0x70
#define   S6D05A1_VCMCTL_VML_5v900							0x71
#define   S6D05A1_VCMCTL_VML_5v925							0x72
#define   S6D05A1_VCMCTL_VML_5v950							0x73
#define   S6D05A1_VCMCTL_VML_5v975							0x74
#define   S6D05A1_VCMCTL_VML_6v000							0x75
#define S6D05A1_OP_SET_SRCCTL							0xF6
#define   S6D05A1_SRCCTL_SVCIR_ENCODE(n)					S6D05A1_REG_VAL_ENCODE(0, 0x7, n)
#define   S6D05A1_SRCCTL_SG_X_AXIS_SYMMETRY					S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_SRCCTL_SG_Y_AXIS_SYMMETRY					S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_SRCCTL_SEL_360_61_1020					S6D05A1_REG_VAL(4, 0)
#define   S6D05A1_SRCCTL_SEL_360_1_1080						S6D05A1_REG_VAL(4, 1)
#define   S6D05A1_SRCCTL_OCM_2_LINE_4_FRAME					S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_SRCCTL_OCM_1_LINE_4_FRAME					S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_SRCCTL_OCM_4_FRAME						S6D05A1_REG_VAL(0, 2)
#define   S6D05A1_SRCCTL_OCM_HALT							S6D05A1_REG_VAL(0, 3)
#define   S6D05A1_SRCCTL_SR_BLK_ENCODE(n)					S6D05A1_REG_VAL_ENCODE(2, 0xC, n)
#define	  S6D05A1_SRCCTL_SR_BLK_AMPLIFIER_DRIVE					0
#define	  S6D05A1_SRCCTL_SR_BLK_BINARY_DRIVE					1
#define	  S6D05A1_SRCCTL_SR_BLK_GND								2
#define	  S6D05A1_SRCCTL_SR_BLK_HI_Z							3
#define   S6D05A1_SRCCTL_NSR_BLK_ENCODE(n)					S6D05A1_SRCCTL_SR_BLK_ENCODE(n)
#define   S6D05A1_SRCCTL_PISR_BLK_ENCODE(n)					S6D05A1_SRCCTL_SR_BLK_ENCODE(n)
#define   S6D05A1_SRCCTL_SR_ND_AMPLIFIER_DRIVE				S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_SRCCTL_SR_ND_BINARY_DRIVE					S6D05A1_REG_VAL(0, 1)
#define S6D05A1_OP_SET_IFCTL							0xF7
#define   S6D05A1_IFCTL_MY_EOR(n)							S6D05A1_REG_VAL(7, (n) & 0x1)
#define   S6D05A1_IFCTL_MX_EOR(n)							S6D05A1_REG_VAL(6, (n) & 0x1)
#define   S6D05A1_IFCTL_MV_EOR(n)							S6D05A1_REG_VAL(5, (n) & 0x1)
#define   S6D05A1_IFCTL_ML_EOR(n)							S6D05A1_REG_VAL(4, (n) & 0x1)
#define   S6D05A1_IFCTL_BGR_EOR(n)							S6D05A1_REG_VAL(3, (n) & 0x1)
#define	  S6D05A1_IFCTL_IPM_ENCODE(n)						S6D05A1_REG_VAL(5, (n) & 0x7)
#define	  S6D05A1_IFCTL_MDT_ENCODE(n)						S6D05A1_REG_VAL(3, (n) & 0x3)
#define	  S6D05A1_IFCTL_SELF_REF_DISABLE					S6D05A1_REG_VAL(2, 0)
#define	  S6D05A1_IFCTL_SELF_REF_ENABLE						S6D05A1_REG_VAL(2, 1)
#define	  S6D05A1_IFCTL_DM_ENCODE(n)						S6D05A1_REG_VAL(0, (n) & 0x3)
#define	  S6D05A1_IFCTL_DM_CPU_MODE							0x0
#define	  S6D05A1_IFCTL_DM_RGB_MODE							0x1
#define	  S6D05A1_IFCTL_DM_MIPI_CMD_MODE					S6D05A1_IFCTL_DM_CPU_MODE
#define	  S6D05A1_IFCTL_DM_MIPI_VIDEO_MODE					S6D05A1_IFCTL_DM_RGB_MODE
#define	  S6D05A1_IFCTL_DM_VSYNC_MODE						0x2
#define	  S6D05A1_IFCTL_DM_DISABLE							0x3
#define   S6D05A1_IFCTL_VPL_LO								S6D05A1_REG_VAL(7, 0)
#define   S6D05A1_IFCTL_VPL_HI								S6D05A1_REG_VAL(7, 1)
#define   S6D05A1_IFCTL_HPL_LO								S6D05A1_REG_VAL(6, 0)
#define   S6D05A1_IFCTL_HPL_HI								S6D05A1_REG_VAL(6, 1)
#define   S6D05A1_IFCTL_DPL_LO								S6D05A1_REG_VAL(5, 0)
#define   S6D05A1_IFCTL_DPL_HI								S6D05A1_REG_VAL(5, 1)
#define   S6D05A1_IFCTL_EPL_LO								S6D05A1_REG_VAL(4, 0)
#define   S6D05A1_IFCTL_EPL_HI								S6D05A1_REG_VAL(4, 1)
#define   S6D05A1_IFCTL_ENDIAN_BIG							S6D05A1_REG_VAL(3, 0)
#define   S6D05A1_IFCTL_ENDIAN_LITTLE						S6D05A1_REG_VAL(3, 1)
#define   S6D05A1_IFCTL_RIM_WIDE							S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_IFCTL_RIM_NARROW							S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_IFCTL_SPR_DISABLE							S6D05A1_REG_VAL(4, 0)
#define   S6D05A1_IFCTL_SPR_ENABLE							S6D05A1_REG_VAL(4, 1)
#define   S6D05A1_IFCTL_RGB_DIV_ENCODE(n)					S6D05A1_REG_VAL(0, (((n) - 2) & 0x7))
#define   S6D05A1_IFCTL_SDO_DISABLE							S6D05A1_REG_VAL(0, 0)
#define   S6D05A1_IFCTL_SDO_ENABLE							S6D05A1_REG_VAL(0, 1)
#define S6D05A1_OP_SET_PANELCTL							0xF8
#define   S6D05A1_PANELCTL_NON_OVERLAP_DISABLE				0
#define   S6D05A1_PANELCTL_NON_OVERLAP_4					1
#define   S6D05A1_PANELCTL_NON_OVERLAP_8					2
#define   S6D05A1_PANELCTL_NON_OVERLAP_12					3
#define   S6D05A1_PANELCTL_NON_OVERLAP_16					4
#define   S6D05A1_PANELCTL_NON_OVERLAP_20					5
#define   S6D05A1_PANELCTL_NON_OVERLAP_24					6
#define   S6D05A1_PANELCTL_NON_OVERLAP_28					7
#define   S6D05A1_PANELCTL_NNO_ENCODE(n)					S6D05A1_REG_VAL(0, n)
#define   S6D05A1_PANELCTL_PNO_ENCODE(n)					S6D05A1_REG_VAL(4, n)
#define   S6D05A1_PANELCTL_SCN_ENCODE(start)				S6D05A1_REG_VAL(0, (((start) - 1) >> 3))
#define S6D05A1_OP_SET_GAMMASEL							0xF9
#define   S6D05A1_GAMMASEL_NGF_POSITIVE						S6D05A1_REG_VAL(4, 0)
#define   S6D05A1_GAMMASEL_NGF_NEGATIVE						S6D05A1_REG_VAL(4, 1)
#define   S6D05A1_GAMMASEL_NGF_USER							S6D05A1_REG_VAL(4, 2)
#define   S6D05A1_GAMMASEL_NGF_DISABLE						S6D05A1_REG_VAL(4, 3)
#define   S6D05A1_GAMMASEL_R_GMA_SELECT						S6D05A1_REG_VAL(0, 4)
#define   S6D05A1_GAMMASEL_G_GMA_SELECT						S6D05A1_REG_VAL(0, 2)
#define   S6D05A1_GAMMASEL_B_GMA_SELECT						S6D05A1_REG_VAL(0, 1)
#define   S6D05A1_GAMMASEL_RGB_GMA_SELECT					S6D05A1_REG_VAL(0, 7)
#define S6D05A1_OP_SET_PGAMMACTL						0xFA
#define S6D05A1_OP_SET_NGAMMACTL						0xFB

/* Transfer Commands (One or More Read and Write Parameters) */

/*
 * All commands and read and write parameters are 9 bits. There are
 * 8-bits of lower-order data and high-order bit, DCX. For commands,
 * DCX is set to 0, for parameters, it is set to 1. To accomodate
 * this, we need 16-bits for each parameter.
 */

#define	S6D05A1_COMMAND_MASK				0xff
#define S6D05A1_PARAM_MASK					0xff

#define S6D05A1_COMMAND_DCX					0x00
#define S6D05A1_PARAM_DCX					0x01

#define S6D05A1_WORD_ENCODE(dcx, mask, x)	(((dcx) << 8) | ((x) & (mask)))
#define S6D05A1_WORD_DECODE(x, mask)		((x) & (mask))

#define S6D05A1_COMMAND_ENCODE(x)			S6D05A1_WORD_ENCODE(S6D05A1_COMMAND_DCX, S6D05A1_COMMAND_MASK, x)
#define S6D05A1_PARAM_ENCODE(x)				S6D05A1_WORD_ENCODE(S6D05A1_PARAM_DCX, S6D05A1_PARAM_MASK, x)
#define S6D05A1_PARAM_DECODE(x)				S6D05A1_WORD_DECODE(x, S6D05A1_PARAM_MASK)

#define S6D05A1_PARAMS_TO_BYTES(n)			((n) * S6D05A1_SPI_BYTES_PER_WORD)
#define S6D05A1_BYTES_TO_PARAMS(b)			((b) / S6D05A1_SPI_BYTES_PER_WORD)

#define FFRAMEHZ							60
#define	TFRAMEMS							(1000 / FFRAMEHZ)

#define fdelay(frames)						mdelay(frames * TFRAMEMS)

/* Type Definitions */

struct s6d05a1_device {
	int 						enabled:1,
								suspended:1;
	struct spi_device *			spi;
	struct omap_dss_device *	dssdev;
	struct regulator *			vcc_reg;
};

typedef u16 s6d05a1_word;

/* Function Prototypes */

static int	tm025zdz01_dss_probe(struct omap_dss_device *dssdev);
static void	tm025zdz01_dss_remove(struct omap_dss_device *dssdev);
static int	tm025zdz01_dss_enable(struct omap_dss_device *dssdev);
static int	tm025zdz01_dss_power_on(struct omap_dss_device *dssdev);
static void	tm025zdz01_dss_disable(struct omap_dss_device *dssdev);
static void	tm025zdz01_dss_power_off(struct omap_dss_device *dssdev);
static int	tm025zdz01_dss_suspend(struct omap_dss_device *dssdev);
static int	tm025zdz01_dss_resume(struct omap_dss_device *dssdev);

static int	s6d05a1_spi_probe(struct spi_device *spi);
static int	s6d05a1_spi_remove(struct spi_device *spi);
static ssize_t  s6d05a1_spi_test_pixel_read(struct device *unused, struct device_attribute *attr, char *buf);
static void     s6d05a1_spi_display_id_read_fields(struct s6d05a1_device *id, uint8_t *buf);
static ssize_t  s6d05a1_spi_display_id_read(struct device *unused, struct device_attribute *attr, char *buf);

static DEVICE_ATTR(test_pixel, S_IRUGO, s6d05a1_spi_test_pixel_read, NULL);
static DEVICE_ATTR(display_id, S_IRUGO, s6d05a1_spi_display_id_read, NULL);
 
static struct attribute *s6d05a1_attributes[] = {
	&dev_attr_test_pixel.attr,
    &dev_attr_display_id.attr,
	NULL
};

static const struct attribute_group s6d05a1_attr_group = {
	.attrs = s6d05a1_attributes,
};

/* Global Variables */

static struct omap_video_timings tianma_tm025zdz01_timings = {
	.x_res			= 320,		// Horizontal resolution, pixels
	.y_res			= 320,		// Vertical resolution, pixels

	.pixel_clock	= 7180,		// Pixel clock, kHz

	// Per Section 3.3.2.2, Figure 73, Page 111 of "S6D05A1
	// Ref. Specification, P.03", the number of DOTCLKs for one
	// horizontal / line period must be greater than the number of
	// horizontal pixels + 30.
	//
	// For now, we divide an extra 32 clocks evenly between the front
	// and back porch.

	.hsw			= 2,		// Horizontal synchronization pulse width
	.hfp			= 16,		// Horizontal front porch, pixels clocks
	.hbp			= 16,		// Horizontal back porch, pixel clocks

	.vsw			= 2,		// Vertical synchronization pulse width
	.vfp			= 8,		// Vertical front porch, line clocks
	.vbp			= 8,		// Vertical back porch, line clocks
};

static struct omap_dss_driver tianma_tm025zdz01_driver = {
	.probe			= tm025zdz01_dss_probe,
	.remove			= tm025zdz01_dss_remove,

	.enable			= tm025zdz01_dss_enable,
	.disable		= tm025zdz01_dss_disable,
	.suspend		= tm025zdz01_dss_suspend,
	.resume			= tm025zdz01_dss_resume,

	.driver         = {
		.name   		= "tianma_tm025zdz01",
		.owner  		= THIS_MODULE,
	}
};

static struct spi_driver samsung_s6d05a1_spi_driver = {
	.driver			= {
		.name			= "s6d05a1",
		.bus			= &spi_bus_type,
		.owner			= THIS_MODULE,
	},
	.probe			= s6d05a1_spi_probe,
	.remove			= __devexit_p(s6d05a1_spi_remove),
};

static struct s6d05a1_device s6d05a1_dev;

static void s6d05a1_transfer(struct s6d05a1_device * id, u8 operation,
							 const u8 *wbuf, unsigned int wlen,
							 u8 *rbuf, unsigned int rlen)
{
	struct spi_message m;
	struct spi_transfer	*x, xfer[4];
	s6d05a1_word	command;
	s6d05a1_word	word;
	int				status;

	BUG_ON(id->spi == NULL);

	dev_dbg(&id->spi->dev, "%s transfer %#02x %u byte%s out @ %p, "
			"%u byte%s in @ %p.\n",
			((wbuf != NULL) ?
			 ((rbuf != NULL) ? "Bi-directional" : "Write") :
			 ((rbuf != NULL) ? "Read" : "Command")),
			operation,
			wlen, ((wlen == 1) ? "" : "s"), wbuf,
			rlen, ((rlen == 1) ? "" : "s"), rbuf);

	spi_message_init(&m);

	memset(xfer, 0, sizeof(xfer));
	x = &xfer[0];

	// At minimum, we have a command, read, write or bi-directional
	// transfer operation to handle that requires a single send
	// phase. Handle that phase.

	command = S6D05A1_COMMAND_ENCODE(operation);

	x->tx_buf					= &command;
	x->bits_per_word			= S6D05A1_SPI_BITS_PER_WORD;
	x->len						= S6D05A1_SPI_BYTES_PER_WORD;

	spi_message_add_tail(x, &m);

	// If we have a write or bi-directional transfer operation, handle
	// the send phase for it.

	if ((wlen > 0) && (wbuf != NULL)) {
		x++;

		x->tx_buf				= wbuf;
		x->bits_per_word		= S6D05A1_SPI_BITS_PER_WORD;
		x->len					= wlen;

		spi_message_add_tail(x, &m);
	}

	// If we have a read or bi-directional transfer operation, handle
	// the receive phase for it.

	if ((rlen > 0) && (rbuf != NULL)) {
		x++;
		x->rx_buf				= &word;
		x->len					= 1;

		spi_message_add_tail(x, &m);

		// Arrange for the extra clock before the first
		// data bit.

		if (rlen > 1) {
			x->bits_per_word	= S6D05A1_SPI_BITS_PER_WORD;
			x->len				= S6D05A1_SPI_BYTES_PER_WORD;

			x++;
			x->rx_buf			= &rbuf[1];
			x->len				= rlen - 1;

			spi_message_add_tail(x, &m);
		}
	}

    status = spi_sync(id->spi, &m);

	if (status < 0) {
		dev_warn(&id->spi->dev, "S6D05A1 SPI transfer failed with status %d\n",
				 status);
		goto done;
	}

	if (rlen) {
		rbuf[0] = S6D05A1_PARAM_DECODE(word);
	}

 done:
	return;
}

static void s6d05a1_command(struct s6d05a1_device * id, u8 operation)
{
	s6d05a1_transfer(id, operation, NULL, 0, NULL, 0);
}

static void __used s6d05a1_read(struct s6d05a1_device * id, u8 operation,
								u8 *buffer, unsigned int length)
{
	s6d05a1_transfer(id, operation, NULL, 0, buffer, length);
}

static ssize_t s6d05a1_spi_test_pixel_read(struct device *unused, 
                                  struct device_attribute *attr, char *buf)
{
    struct s6d05a1_device * id;

    id = &s6d05a1_dev;    

    s6d05a1_read(id, S6D05A1_OP_GET_RED_CHANNEL, &buf[0], 1);
    s6d05a1_read(id, S6D05A1_OP_GET_GREEN_CHANNEL, &buf[1], 1);
    s6d05a1_read(id, S6D05A1_OP_GET_BLUE_CHANNEL, &buf[2], 1);

    return 3;
}

static void s6d05a1_spi_display_id_read_fields(struct s6d05a1_device *id, uint8_t *buf)
{
    s6d05a1_read(id, S6D05A1_OP_GET_ID1, &buf[0], 1);
    s6d05a1_read(id, S6D05A1_OP_GET_ID2, &buf[1], 1);
    s6d05a1_read(id, S6D05A1_OP_GET_ID3, &buf[2], 1);
}

static ssize_t s6d05a1_spi_display_id_read(struct device *unused, 
                                  struct device_attribute *attr, char *buf)
{
    struct s6d05a1_device * id;
    uint8_t displayId[3];

    id = &s6d05a1_dev;    

    s6d05a1_spi_display_id_read_fields(id, displayId);

    return sprintf(buf, "%02x %02x %02x\n", displayId[0], displayId[1], displayId[2]);
}

static void s6d05a1_write(struct s6d05a1_device * id, u8 operation,
						  const u8 *buffer, unsigned int length)
{
	s6d05a1_transfer(id, operation, buffer, length, NULL, 0);
}

static void s6d05a1_reset(unsigned long gpio, bool inverted, bool sleeping)
{
	const bool asserted = inverted;
	
	// First, ensure the reset line is deasserted for a "sufficiently
	// long" time as we don't know it's initial condition. Then,
	// assert it for at least the required time. Finally, deassert it
	// again for at least the required time.

	gpio_set_value(gpio, !asserted);
	mdelay(1 * 2);

	gpio_set_value(gpio, asserted);
	mdelay(S6D05A1_TRES_LOW_MS_MIN);

	gpio_set_value(gpio, !asserted);
    
	if (sleeping)
    {
        mdelay(S6D05A1_TRES_LOW_MS_MIN);
    }
    else
    {
        mdelay(S6D05A1_TRES_HIGH_MS_MIN);
    }
}

// Per Section 4.1.7, Figure 136, Page 171 of "S6D05A1 Ref. Specification,
// P.03".

static void s6d05a1_power_on(struct s6d05a1_device * id)
{
	int status = 0;
	const unsigned int maxparam = 20;
	s6d05a1_word params[maxparam];
	unsigned int nparams;
	u8 operation;
	struct omap_dss_device *dss;
	struct s6d05a1_platform_data *pdata;
	u32 width, height;
	uint8_t display_id[3];

	dss = id->dssdev;
	pdata = id->spi->dev.platform_data;

	width = dss->panel.timings.x_res;
	height = dss->panel.timings.y_res;

	// First, allow the platform to do any necessary steps (turn on
	// rails, etc.).

	if (dss->platform_enable) {
		status = dss->platform_enable(dss);

		if (status) {
			dev_err(&dss->dev, "The platform failed to enable the display.\n");
			goto done;
		}
	}

	// Then, wait at least 1 ms.

	mdelay(1 * 2);

	// Next, issue the power-on reset.

	s6d05a1_reset(pdata->reset.gpio, 
                  pdata->reset.inverted,
				  (!id->enabled || id->suspended));

	// Establish the memory address control (MADCTL) mode as dictated
	// by platform data in accordance with Section 4.5.1, Figure 149,
	// Page 220 of "S6D05A1 Ref. Specification, P.03".

	operation = S6D05A1_OP_SET_ADDRESS_MODE;

	params[0] = S6D05A1_PARAM_ENCODE(
		(pdata->orientation.y_mirror ?
		 S6D05A1_ADDRESS_MODE_PAGE_ADDR_ORDER_BTOT :
		 S6D05A1_ADDRESS_MODE_PAGE_ADDR_ORDER_TTOB)		|
		(pdata->orientation.x_mirror ?
		 S6D05A1_ADDRESS_MODE_COL_ADDR_ORDER_BTOT :
		 S6D05A1_ADDRESS_MODE_COL_ADDR_ORDER_TTOB)		|
		(pdata->orientation.x_y_exchange ?
		 S6D05A1_ADDRESS_MODE_PAGE_COL_REVERSE :
		 S6D05A1_ADDRESS_MODE_PAGE_COL_NORMAL)			|
		(pdata->orientation.y_mirror ?
		 S6D05A1_ADDRESS_MODE_VERT_REFR_ORDER_BTOT :
		 S6D05A1_ADDRESS_MODE_VERT_REFR_ORDER_TTOB)		|
		S6D05A1_ADDRESS_MODE_PIXEL_ORDER_RGB);
	nparams = 1;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Enter the "passwords" to enable level two command input.

	operation = S6D05A1_OP_SET_PASSWD1;

	params[0] = S6D05A1_PARAM_ENCODE(U32_B1_DECODE(S6D05A1_PASSWD1_L2_ENABLE));
	params[1] = S6D05A1_PARAM_ENCODE(U32_B0_DECODE(S6D05A1_PASSWD1_L2_ENABLE));
	nparams = 2;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	operation = S6D05A1_OP_SET_PASSWD2;

	params[0] = S6D05A1_PARAM_ENCODE(U32_B1_DECODE(S6D05A1_PASSWD2_L2_ENABLE));
	params[1] = S6D05A1_PARAM_ENCODE(U32_B0_DECODE(S6D05A1_PASSWD2_L2_ENABLE));
	nparams = 2;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the display control settings

	operation = S6D05A1_OP_SET_DISCTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(S6D05A1_DISCTL_NL_ENCODE(height));
    params[ 1] = S6D05A1_PARAM_ENCODE(0x30);					// NHW
    params[ 2] = S6D05A1_PARAM_ENCODE(S6D05A1_DISCTL_NINV_LINE |
									  S6D05A1_DISCTL_PINV_LINE |
									  S6D05A1_DISCTL_IINV_FRAME |
									  S6D05A1_DISCTL_PIINV_FRAME);
    params[ 3] = S6D05A1_PARAM_ENCODE(dss->panel.timings.vbp +
									  dss->panel.timings.vsw);	// NVBP
    params[ 4] = S6D05A1_PARAM_ENCODE(dss->panel.timings.vfp);	// NVFP
    params[ 5] = S6D05A1_PARAM_ENCODE(0x08);
    params[ 6] = S6D05A1_PARAM_ENCODE(0x08);
    params[ 7] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 8] = S6D05A1_PARAM_ENCODE(0x08);
    params[ 9] = S6D05A1_PARAM_ENCODE(0x08);
    params[10] = S6D05A1_PARAM_ENCODE(0x00);
    params[11] = S6D05A1_PARAM_ENCODE(S6D05A1_DISCTL_REV_WHITE |
									  S6D05A1_DISCTL_GS_LO_TO_HI |
									  S6D05A1_DISCTL_SM_EVEN_ODD);
    params[12] = S6D05A1_PARAM_ENCODE(0x00);
    params[13] = S6D05A1_PARAM_ENCODE(0x00);
    params[14] = S6D05A1_PARAM_ENCODE(0x54);					// PIHW
    params[15] = S6D05A1_PARAM_ENCODE(dss->panel.timings.vbp +
									  dss->panel.timings.vsw);	// PIVBP
    params[16] = S6D05A1_PARAM_ENCODE(dss->panel.timings.vfp);	// PIVFP
    params[17] = S6D05A1_PARAM_ENCODE(dss->panel.timings.vbp +
									  dss->panel.timings.vsw);	// RGB_NVBP
    params[18] = S6D05A1_PARAM_ENCODE(dss->panel.timings.vfp);	// RGB_NVFP
	nparams = 19;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the power control settings.

	operation = S6D05A1_OP_SET_PWRCTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_VCI1_2v58);
    params[ 1] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_BOOST_CLK_SEL_EXT);
    params[ 2] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 3] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 4] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 5] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 6] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 7] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 8] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_NDC_ENCODE(3, S6D05A1_PWRCTL_DC_1_2) |
									  S6D05A1_PWRCTL_NDC_ENCODE(2, S6D05A1_PWRCTL_DC_1_2) |
									  S6D05A1_PWRCTL_NDC_ENCODE(1, S6D05A1_PWRCTL_DC_1_2));
    params[ 9] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_GVD_4v50);				// NGVD
    params[10] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_BT_16v50_NEG_8v25);	// NBT
    params[11] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_PIDC_ENCODE(3, S6D05A1_PWRCTL_DC_1_2) |
									  S6D05A1_PWRCTL_PIDC_ENCODE(2, S6D05A1_PWRCTL_DC_1_2) |
									  S6D05A1_PWRCTL_PIDC_ENCODE(1, S6D05A1_PWRCTL_DC_1_2));
    params[12] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_GVD_4v10);				// PIGVD
    params[13] = S6D05A1_PARAM_ENCODE(S6D05A1_PWRCTL_BT_16v50_NEG_8v25);	// PIBT
	nparams = 14;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the VCOM control settings.

	operation = S6D05A1_OP_SET_VCMCTL;

	params[ 0] = S6D05A1_PARAM_ENCODE(0x00);						// VCOMG
	params[ 1] = S6D05A1_PARAM_ENCODE(S6D05A1_VCMCTL_VCMH_4v30);	// NVCMH
	params[ 2] = S6D05A1_PARAM_ENCODE(S6D05A1_VCMCTL_VML_5v400);	// NVML
    params[ 3] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 4] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 5] = S6D05A1_PARAM_ENCODE(0x04);						// VCIR
    params[ 6] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 7] = S6D05A1_PARAM_ENCODE(0x00);
    params[ 8] = S6D05A1_PARAM_ENCODE(0x04);						// NVC_BLK
    params[ 9] = S6D05A1_PARAM_ENCODE(0x00);						// PIVC_BLK
    params[10] = S6D05A1_PARAM_ENCODE(S6D05A1_VCMCTL_VCMH_3v30);	// PIVCMH
    params[11] = S6D05A1_PARAM_ENCODE(S6D05A1_VCMCTL_VML_3v900);	// PIVML
	nparams = 12;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the source control settings.

	operation = S6D05A1_OP_SET_SRCCTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(S6D05A1_SRCCTL_SVCIR_ENCODE(1));
    params[ 1] = S6D05A1_PARAM_ENCODE(S6D05A1_SRCCTL_SG_X_AXIS_SYMMETRY |
									  S6D05A1_SRCCTL_SEL_360_61_1020);
    params[ 2] = S6D05A1_PARAM_ENCODE(0x08);	// SAP
    params[ 3] = S6D05A1_PARAM_ENCODE(S6D05A1_SRCCTL_OCM_HALT);
    params[ 4] = S6D05A1_PARAM_ENCODE(0x01);	// NSDT
    params[ 5] = S6D05A1_PARAM_ENCODE(S6D05A1_SRCCTL_NSR_BLK_ENCODE(S6D05A1_SRCCTL_SR_BLK_AMPLIFIER_DRIVE) |
									  S6D05A1_SRCCTL_SR_ND_BINARY_DRIVE);
    params[ 6] = S6D05A1_PARAM_ENCODE(0x01);	// PISDT
    params[ 7] = S6D05A1_PARAM_ENCODE(S6D05A1_SRCCTL_PISR_BLK_ENCODE(S6D05A1_SRCCTL_SR_BLK_AMPLIFIER_DRIVE));
	params[ 8] = S6D05A1_PARAM_ENCODE(0x00);
	nparams = 9;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the interface control settings.

	operation = S6D05A1_OP_SET_IFCTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(S6D05A1_IFCTL_MY_EOR(0) |
									  S6D05A1_IFCTL_MX_EOR(1) |
									  S6D05A1_IFCTL_MV_EOR(0) |
									  S6D05A1_IFCTL_ML_EOR(0) |
									  S6D05A1_IFCTL_BGR_EOR(1));
    params[ 1] = S6D05A1_PARAM_ENCODE(S6D05A1_IFCTL_IPM_ENCODE(4) |
									  S6D05A1_IFCTL_MDT_ENCODE(0) |
									  S6D05A1_IFCTL_SELF_REF_DISABLE |
									  S6D05A1_IFCTL_DM_ENCODE(S6D05A1_IFCTL_DM_RGB_MODE));
    params[ 2] = S6D05A1_PARAM_ENCODE(((dss->panel.config & OMAP_DSS_LCD_IVS) ?
									   S6D05A1_IFCTL_VPL_LO :
									   S6D05A1_IFCTL_VPL_HI) |
									  ((dss->panel.config & OMAP_DSS_LCD_IHS) ?
									   S6D05A1_IFCTL_HPL_LO :
									   S6D05A1_IFCTL_HPL_HI) |
									  ((dss->panel.config & OMAP_DSS_LCD_IPC) ?
									   S6D05A1_IFCTL_DPL_HI :
									   S6D05A1_IFCTL_DPL_LO) |
									  ((dss->panel.config & OMAP_DSS_LCD_IEO) ?
									   S6D05A1_IFCTL_EPL_LO :
									   S6D05A1_IFCTL_EPL_HI) |
									  S6D05A1_IFCTL_ENDIAN_BIG |
									  S6D05A1_IFCTL_RIM_WIDE);
    params[ 3] = S6D05A1_PARAM_ENCODE(S6D05A1_IFCTL_SPR_ENABLE |
									  S6D05A1_IFCTL_RGB_DIV_ENCODE(4));
    params[ 4] = S6D05A1_PARAM_ENCODE(S6D05A1_IFCTL_SDO_ENABLE);
	nparams = 5;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the panel control settings.

	operation = S6D05A1_OP_SET_PANELCTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(S6D05A1_PANELCTL_NNO_ENCODE(S6D05A1_PANELCTL_NON_OVERLAP_4) |
									  S6D05A1_PANELCTL_PNO_ENCODE(S6D05A1_PANELCTL_NON_OVERLAP_4));
    params[ 1] = S6D05A1_PARAM_ENCODE(S6D05A1_PANELCTL_SCN_ENCODE(1));
	nparams = 2;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the gamma selection settings.

	operation = S6D05A1_OP_SET_GAMMASEL;

    params[ 0] = S6D05A1_PARAM_ENCODE(S6D05A1_GAMMASEL_NGF_NEGATIVE |
									  S6D05A1_GAMMASEL_RGB_GMA_SELECT);
	nparams = 1;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the positive gamma control settings.

	operation = S6D05A1_OP_SET_PGAMMACTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(0x00);	// RFP
    params[ 1] = S6D05A1_PARAM_ENCODE(0x02);	// OSP
    params[ 2] = S6D05A1_PARAM_ENCODE(0x00);	// PKP[5:0]
    params[ 3] = S6D05A1_PARAM_ENCODE(0x21);	// PKP[15:10]
    params[ 4] = S6D05A1_PARAM_ENCODE(0x2A);	// PKP[25:20]
    params[ 5] = S6D05A1_PARAM_ENCODE(0x2D);	// PKP[35:30]
    params[ 6] = S6D05A1_PARAM_ENCODE(0x2E);	// PKP[45:40]
    params[ 7] = S6D05A1_PARAM_ENCODE(0x22);	// PKP[55:50]
    params[ 8] = S6D05A1_PARAM_ENCODE(0x28);	// PKP[65:60]
    params[ 9] = S6D05A1_PARAM_ENCODE(0x2F);	// PKP[75:70]
    params[10] = S6D05A1_PARAM_ENCODE(0x3C);	// PKP[85:80]
    params[11] = S6D05A1_PARAM_ENCODE(0x3F);	// PKP[95:90]
    params[12] = S6D05A1_PARAM_ENCODE(0x34);	// PKP[105:100]
    params[13] = S6D05A1_PARAM_ENCODE(0x00);
    params[14] = S6D05A1_PARAM_ENCODE(0x00);
    params[15] = S6D05A1_PARAM_ENCODE(0x00);	// GLP
	nparams = 16;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the negative gamma control settings.

	operation = S6D05A1_OP_SET_NGAMMACTL;

    params[ 0] = S6D05A1_PARAM_ENCODE(0x00);	// RFN
    params[ 1] = S6D05A1_PARAM_ENCODE(0x02);	// OSN
    params[ 2] = S6D05A1_PARAM_ENCODE(0x34);	// PKN[5:0]
    params[ 3] = S6D05A1_PARAM_ENCODE(0x3F);	// PKN[15:10]
    params[ 4] = S6D05A1_PARAM_ENCODE(0x3C);	// PKN[25:20]
    params[ 5] = S6D05A1_PARAM_ENCODE(0x2F);	// PKN[35:30]
    params[ 6] = S6D05A1_PARAM_ENCODE(0x28);	// PKN[45:40]
    params[ 7] = S6D05A1_PARAM_ENCODE(0x22);	// PKN[55:50]
    params[ 8] = S6D05A1_PARAM_ENCODE(0x2E);	// PKN[65:60]
    params[ 9] = S6D05A1_PARAM_ENCODE(0x2D);	// PKN[75:70]
    params[10] = S6D05A1_PARAM_ENCODE(0x2A);	// PKN[85:80]
    params[11] = S6D05A1_PARAM_ENCODE(0x21);	// PKN[95:90]
    params[12] = S6D05A1_PARAM_ENCODE(0x00);	// PKN[105:100]
    params[13] = S6D05A1_PARAM_ENCODE(0x00);
    params[14] = S6D05A1_PARAM_ENCODE(0x00);
    params[15] = S6D05A1_PARAM_ENCODE(0x00);	// GLN
	nparams = 16;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the pixel format settings.

	operation = S6D05A1_OP_SET_PIXEL_FORMAT;

	params[0] = S6D05A1_PARAM_ENCODE(S6D05A1_PIXEL_FORMAT_24BPP);
	nparams = 1;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, program the column and page addresses to the display
	// extents.

	operation = S6D05A1_OP_SET_COLUMN_ADDRESS;

	params[0] = S6D05A1_PARAM_ENCODE(U32_B3_DECODE(width - 1));
	params[1] = S6D05A1_PARAM_ENCODE(U32_B2_DECODE(width - 1));
	params[2] = S6D05A1_PARAM_ENCODE(U32_B1_DECODE(width - 1));
	params[3] = S6D05A1_PARAM_ENCODE(U32_B0_DECODE(width - 1));
	nparams = 4;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	operation = S6D05A1_OP_SET_PAGE_ADDRESS;

	params[0] = S6D05A1_PARAM_ENCODE(U32_B3_DECODE(height - 1));
	params[1] = S6D05A1_PARAM_ENCODE(U32_B2_DECODE(height - 1));
	params[2] = S6D05A1_PARAM_ENCODE(U32_B1_DECODE(height - 1));
	params[3] = S6D05A1_PARAM_ENCODE(U32_B0_DECODE(height - 1));
	nparams = 4;
	s6d05a1_write(id,
				  operation,
				  (const u8 *)params,
				  S6D05A1_PARAMS_TO_BYTES(nparams));

	// Then, send the exit sleep mode command and wait at least 120 ms.

	s6d05a1_command(id, S6D05A1_OP_CMD_EXIT_SLEEP_MODE);

	mdelay(120);

	// Then, send the exit idle mode command.

	s6d05a1_command(id, S6D05A1_OP_CMD_EXIT_IDLE_MODE);

	// Then, send the display on command.

	s6d05a1_command(id, S6D05A1_OP_CMD_DISPLAY_ON);

	// Finally, wait at least 10 ms before writing data to the
	// display.

	mdelay(10);

	// Read and log the display ID
	s6d05a1_spi_display_id_read_fields(id, display_id);
	dev_printk(KERN_INFO, &dss->dev, "display ID: %02x %02x %02x\n", 
				display_id[0], display_id[1], display_id[2]);

 done:
	return;
}

// Per Section 4.10.1, Figure 169, Page 240 of "S6D05A1
// Ref. Specification, P.03".
//
// XXX - Per Section 4.1.1, Page 162 of "S6D05A1 Ref. Specification,
// P.03" during power off, if the LCD is in sleep out mode, VCI and
// VDD3 must be powered down a minimum 120 ms after reset (RESX) has
// been deasserted.
//
// If the LCD is in sleep out mode, VCI and VDD3 must be powered down
// a minimum 0 ms after reset (RESX) has been deasserted.

static void s6d05a1_power_off(struct s6d05a1_device * id)
{
	// First, run the display off command

	s6d05a1_command(id, S6D05A1_OP_CMD_DISPLAY_OFF);

	// Next, enter sleep mode

	s6d05a1_command(id, S6D05A1_OP_CMD_ENTER_SLEEP_MODE);

	// Wait at least 2 frames

	fdelay(2 * 2);

	// Finally, allow the platform to do any necessary steps (turn off
	// rails, etc.)

	if (id->dssdev->platform_disable) {
		id->dssdev->platform_disable(id->dssdev);
	}
}

static int	tm025zdz01_dss_probe(struct omap_dss_device *dssdev)
{
	struct s6d05a1_device *id = NULL;
	struct spi_device *spi = NULL;
	struct s6d05a1_platform_data *pdata = NULL;
	struct regulator *reg;
	const char *supply;
	int status = 0;

	id = &s6d05a1_dev;

	// Otherwise, the detect succeeded. So, cache a reference to the
	// OMAP DSS device in our device-private data.

	id->dssdev = dssdev;

	spi = id->spi;
	pdata = spi->dev.platform_data;

	// Platform data is required. Without it, we cannot determine the
	// reset GPIO and reset polarity. And without those, we cannot
	// successfully initialize the display.

	if (pdata == NULL) {
		dev_err(&spi->dev, "Could not retrieve platform data for display. "
				"Unable to initialize display.\n");
		status = -EINVAL;
		goto done;
	}

	// If we were supplied platform-specific data, request and assign
	// the reset and lcd_id GPIOs.

	status = gpio_request(pdata->reset.gpio, "s6d05a1 reset");

	if (status) {
		dev_err(&spi->dev,
				"Couldn't reserve GPIO %ld for s6d05a1 reset.\n",
				pdata->reset.gpio);
		goto done;
	}

	status = gpio_direction_output(pdata->reset.gpio,
								   !pdata->reset.inverted);

	if (status) {
		dev_err(&spi->dev,
				"Couldn't set GPIO %ld output for s6d05a1 reset.\n",
				pdata->reset.gpio);
		goto free_reset_gpio;
	}

	status = gpio_request(pdata->lcd_id.gpio, "s6d05a1 lcd_id");

	if (status) {
		dev_err(&spi->dev,
				"Couldn't reserve GPIO %ld for s6d05a1 lcd_id.\n",
				pdata->lcd_id.gpio);
		goto free_reset_gpio;
	}

	status = gpio_direction_input(pdata->lcd_id.gpio);

	if (status) {
		dev_err(&spi->dev,
				"Couldn't set GPIO %ld output for s6d05a1 lcd_id.\n",
				pdata->lcd_id.gpio);
		goto free_lcd_id_gpio;
	}

	status = gpio_export(pdata->lcd_id.gpio, false);

	if (status) {
		dev_err(&spi->dev, "Could not export GPIO %ld: %d\n", pdata->lcd_id.gpio, status);
		goto free_lcd_id_gpio;
	}

	status = gpio_export_link(&spi->dev, "lcd_id", pdata->lcd_id.gpio);

	if (status) {
		dev_err(&spi->dev, "Could not export GPIO %ld as link lcd_id: %d\n", pdata->lcd_id.gpio, status);
		goto unexport_lcd_id_gpio;
	}

	supply = pdata->regulator.vcc;

	if (supply != NULL) {
		reg = regulator_get(&dssdev->dev, supply);

		if (IS_ERR(reg)) {
			status = PTR_ERR(reg);
			dev_err(&dssdev->dev, "Could not get requested regulator supply '%s': %d\n", supply, status);
			goto unexport_lcd_id_gpio;

		} else {
			id->vcc_reg = reg;

		}
	}

	// Configure the panel as a TFT LCD with both horizontal and
	// vertical sync inverted. Also, invert the pixel (aka dot) clock
	// to ensure than tENS and tENH are observed. Otherwise, the pixel
	// clock changes RIGHT as enable changes and, frequently, bad data
	// is latched. Finally, invert the sense of the enable signal
	// because when it is NOT inverted, the display FAILS to latch the
	// final column of data.

	dssdev->panel.config = (OMAP_DSS_LCD_TFT |
							OMAP_DSS_LCD_IPC |
							OMAP_DSS_LCD_IEO |
							OMAP_DSS_LCD_IVS |
							OMAP_DSS_LCD_IHS);

	// Copy the default timing parameters.

	dssdev->panel.timings = tianma_tm025zdz01_timings;

    goto done;

 unexport_lcd_id_gpio:
	gpio_unexport(pdata->lcd_id.gpio);

 free_lcd_id_gpio:
	gpio_free(pdata->lcd_id.gpio);

 free_reset_gpio:
	gpio_free(pdata->reset.gpio);

 done:
	return (status);
}

static void	tm025zdz01_dss_remove(struct omap_dss_device *dssdev)
{
	struct spi_device *spi;
	struct s6d05a1_platform_data *pdata;

	spi = s6d05a1_dev.spi;
	pdata = spi->dev.platform_data;

	if (s6d05a1_dev.vcc_reg) {
		regulator_put(s6d05a1_dev.vcc_reg);
	}

	if (pdata != NULL) {
            gpio_free(pdata->reset.gpio);
            gpio_unexport(pdata->lcd_id.gpio);
            gpio_free(pdata->lcd_id.gpio);
	}

	return;
}

static int tm025zdz01_dss_enable(struct omap_dss_device *dssdev)
{
    return tm025zdz01_dss_power_on(dssdev);
}

static int tm025zdz01_dss_power_on(struct omap_dss_device *dssdev)
{
    struct s6d05a1_device * id = &s6d05a1_dev;
    int status = 0;

    if (!id->enabled)
    {
        status = omapdss_dpi_display_enable(dssdev);

	 if (id->vcc_reg) {
            status = regulator_enable(id->vcc_reg);

            if (status) {
                goto done;
	    }
        }

        s6d05a1_power_on(id);

        id->enabled = true;
        dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
    }

    done:
	return (status);
}

static void	tm025zdz01_dss_disable(struct omap_dss_device *dssdev)
{
    tm025zdz01_dss_power_off(dssdev);
}

static void tm025zdz01_dss_power_off(struct omap_dss_device *dssdev)
{
    struct s6d05a1_device * id = &s6d05a1_dev;

    if (id->enabled)
    {
        s6d05a1_power_off(id);

        if (id->vcc_reg) {
            regulator_disable(id->vcc_reg);
        }

        omapdss_dpi_display_disable(dssdev);

        id->enabled = false;
        dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
    }

}
static int tm025zdz01_dss_suspend(struct omap_dss_device *dssdev)
{
    struct s6d05a1_device * id = &s6d05a1_dev;
    int status = 0;

    if (!id->suspended)
    {
        s6d05a1_power_off(id);

        if (id->vcc_reg) {
            regulator_disable(id->vcc_reg);
        }

        omapdss_dpi_display_disable(dssdev);

        id->suspended = true;
        dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;
    }

    return (status);
}

static int tm025zdz01_dss_resume(struct omap_dss_device *dssdev)
{
	struct s6d05a1_device * id = &s6d05a1_dev;
	int status = 0;

    if (id->suspended)
    {

        status = omapdss_dpi_display_enable(dssdev);

        if (id->vcc_reg) {
            status = regulator_enable(id->vcc_reg);

	    if (status) {
	        goto done;
	    }
        }

	s6d05a1_power_on(id);

	id->suspended = false;
	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
    }

 done:
	return (status);
}

static int s6d05a1_spi_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct s6d05a1_device *id = NULL;
	int status = 0;

	// Check to ensure the specified platform SPI clock doesn't exceed
	// the allowed maximum.

	if (spi->max_speed_hz > S6D05A1_SPI_XSCK_MAX) {
		dev_err(dev, "The SPI interface clock must be less than or "
				"equal to %d KHz\n", S6D05A1_SPI_XSCK_MAX / 1000);
		status = -EINVAL;
		goto done;
    }

	// Get the device private data and save a reference to the SPI
	// device pointer.

	id = &s6d05a1_dev;
	id->spi = spi;

  
	// The Samsung S6D05A1 SPI interface requires mode 3. Bits-per-word
	// is variable and is set on a per-transfer basis.

	spi->mode = S6D05A1_SPI_MODE;

	// Set up the SPI controller interface for the chip select channel
	// we'll be using for SPI transactions associated with this
	// device.

	status = spi_setup(spi);

	if (status < 0) {
		dev_err(dev, "Failed to setup SPI controller with error %d\n", status);
		goto done;
	}

	// Register our device private data with the SPI driver.

	spi_set_drvdata(spi, id);

	// Register our panel-/module-specific methods with the OMAP DSS
	// driver.

	omap_dss_register_driver(&tianma_tm025zdz01_driver);

    status = sysfs_create_group(&spi->dev.kobj, &s6d05a1_attr_group);

 done:
	return (status);
}

static int s6d05a1_spi_remove(struct spi_device *spi)
{
	int status = 0;

	sysfs_remove_group(&spi->dev.kobj, &s6d05a1_attr_group);
	omap_dss_unregister_driver(&tianma_tm025zdz01_driver);

	return (status);
}

static int __init tianma_tm025zdz01_init(void)
{
	int status = 0;

	pr_info("%s %s\n", TM025ZDZ01_DRIVER_NAME, TM025ZDZ01_DRIVER_VERSION);

	status = spi_register_driver(&samsung_s6d05a1_spi_driver);

	return (status);
}

static void __exit tianma_tm025zdz01_exit(void)
{
	spi_unregister_driver(&samsung_s6d05a1_spi_driver);
}

module_init(tianma_tm025zdz01_init);
module_exit(tianma_tm025zdz01_exit);

MODULE_AUTHOR("Nest, Inc.");
MODULE_DESCRIPTION(TM025ZDZ01_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
