/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *    (C) Copyright 2008
 *    Nishanth Menon <menon.nishanth@gmail.com>
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
 *      primarily I/O pad multiplexer configuration.
 *
 *      This is originally inherited and split from the OMAP3 EVM
 *      board file.
 */

#include <common.h>
#include <asm/arch/omap3.h>
#include <asm/io.h>
#include <asm/arch/mux.h>

/*
 *  void set_muxconf_regs()
 *
 *  Description:
 *    This routines sets the I/O pad multiplexers for UART, GPMC, SDRC,
 *    GPIO.
 *
 *    The commented string gives the final mux configuration for that
 *    pin.
 *
 *    NOTE: Only generic model product configuration should be set
 *    here. Per-revision configuration is handled in diamond.c.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    N/A
 */

void
set_muxconf_regs(void)
{
	/*
	 * The bit fields for these settings are as follows:
	 *
	 *   I{EN,DIS}	 	- Input Enable/Disable
	 *   PT[DU]		 	- Pull-down, -up
	 *   DIS,EN		  	- Pull-down, -up disabled/enabled
	 *   M[01234567]	- Configuration mode 0-7
	 */

	/*
	 * SDRAM Controller (SDRC)
	 *
	 * Most of the SDRC signals are used to drive the Samsung
	 * K4X51163PI-FCG6 512 Mb x 16-bit (64 MiB) DDR SDRAM. Because we
	 * interface in 16-bit rather than 32-bit mode, we place the
	 * unused pins in safe mode (Mode 7).
	 */

	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0));	// SDRC_D0
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0));	// SDRC_D1
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0));	// SDRC_D2
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0));	// SDRC_D3
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0));	// SDRC_D4
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0));	// SDRC_D5
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0));	// SDRC_D6
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0));	// SDRC_D7
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0));	// SDRC_D8
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0));	// SDRC_D9
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0));	// SDRC_D10
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0));	// SDRC_D11
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0));	// SDRC_D12
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0));	// SDRC_D13
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0));	// SDRC_D14
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0));	// SDRC_D15

	// Because we interface SDRAM in 16-bit mode, place
	// SDRC_DATA[31:16] in safe mode (Mode 7).

	MUX_VAL(CP(SDRC_D16),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D17),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D18),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D19),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D20),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D21),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D22),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D23),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D24),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D25),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D26),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D27),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D28),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D29),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D30),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_D31),		(IDIS | PTD | DIS | M7));	// Unused

	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0));	// SDRC_CLK
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0));	// SDRC_DQS0
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0));	// SDRC_DQS1

	// Place unused DQ pins in safe mode (Mode 7).

	MUX_VAL(CP(SDRC_DQS2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(SDRC_DQS3),		(IDIS | PTD | DIS | M7));	// Unused

	MUX_VAL(CP(SDRC_CKE0),		(IDIS | PTU | EN  | M0));	// SDRC_CKE0

	// Place the unused clock-enable 1 pin in safe mode (Mode 7).

	MUX_VAL(CP(SDRC_CKE1),		(IDIS | PTD | DIS | M7));	// Unused

	/*
	 * General Purpose Memory Controller (GPMC)
	 *
	 */

	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0));	// GPMC_A1
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0));	// GPMC_A2
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0));	// GPMC_A3
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0));	// GPMC_A4
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0));	// GPMC_A5
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTU | EN  | M0));	// GPMC_A6
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTU | EN  | M0));	// GPMC_A7

	// Place the unused GPMC_A8 pin in safe mode (Mode 7).

	MUX_VAL(CP(GPMC_A8),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(GPMC_A9),		(IEN  | PTD | DIS | M4));	// GPIO_42
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTD | DIS | M7));	// Unused

	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0));	// GPMC_D0
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0));	// GPMC_D1
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0));	// GPMC_D2
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0));	// GPMC_D3
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0));	// GPMC_D4
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0));	// GPMC_D5
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0));	// GPMC_D6
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0));	// GPMC_D7
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0));	// GPMC_D8
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0));	// GPMC_D9
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0));	// GPMC_D10
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0));	// GPMC_D11
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0));	// GPMC_D12
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0));	// GPMC_D13
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0));	// GPMC_D14
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0));	// GPMC_D15
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0));	// GPMC_nCS0
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTU | EN  | M0));	// GPMC_nCS1
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTU | EN  | M0));	// GPMC_nCS2
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(GPMC_NCS4),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTD | DIS | M7));	// Unused

	// Assign GPMC_NCS6 to GPT11_PWM_EVT used as a push-pull pulse width
	// modulated (PWM) signal for the piezo.

	MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTD | DIS | M3));	// GPT11_PWM_EVT

	// Assign GPMC_NCS7 to GPIO_58 used as a push-pull output
	// PIEZO_NENABLE to the TI SN74LVC2G240 Dual Buffer/Driver With
	// 3-State Outputs inputs.

	MUX_VAL(CP(GPMC_NCS7),		(IDIS | PTD | DIS | M4));	// GPIO_58

	MUX_VAL(CP(GPMC_CLK),		(IDIS | PTU | EN  | M0));	// GPMC_CLK
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0));	// GPMC_nADV_ALE
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0));	// GPMC_nOE
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0));	// GPMC_nWE
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0));	// GPMC_nBE0_CLE

	// Assign GPMC_NBE1 to GPIO_61 used as a push-pull output to the
	// ZigBee active-low reset.

	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTD | DIS | M4));	// GPIO_61

	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | DIS | M0));	// GPMC_nWP
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M0));	// GPMC_WAIT0
	MUX_VAL(CP(GPMC_WAIT1),		(IEN  | PTU | EN  | M0));	// GPMC_WAIT1
	MUX_VAL(CP(GPMC_WAIT2),		(IDIS | PTD | DIS | M7));	// Unavailable

	// Assign GPMC_WAIT3 to GPIO_65 used as an active-high, push-pull
	// backplate detect input.

	MUX_VAL(CP(GPMC_WAIT3),		(IEN  | PTD | DIS | M4));	// GPIO_65

	/*
	 * Display Subsystem (DSS)
	 *
	 * All DSS signals are used in their normal, default mode (Mode 0)
	 * to drive the Samsung LMS350DF0[13] LCD display panel.
	 */

	MUX_VAL(CP(DSS_PCLK),		(IDIS | PTD | DIS | M0));	// DSS_PCLK
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0));	// DSS_HSYNC
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M0));	// DSS_VSYNC
	MUX_VAL(CP(DSS_ACBIAS),		(IDIS | PTD | DIS | M0));	// DSS_ACBIAS
	MUX_VAL(CP(DSS_DATA0),		(IDIS | PTD | DIS | M0));	// DSS_DATA0
	MUX_VAL(CP(DSS_DATA1),		(IDIS | PTD | DIS | M0));	// DSS_DATA1
	MUX_VAL(CP(DSS_DATA2),		(IDIS | PTD | DIS | M0));	// DSS_DATA2
	MUX_VAL(CP(DSS_DATA3),		(IDIS | PTD | DIS | M0));	// DSS_DATA3
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTD | DIS | M0));	// DSS_DATA4
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M0));	// DSS_DATA5
	MUX_VAL(CP(DSS_DATA6),		(IDIS | PTD | DIS | M0));	// DSS_DATA6
	MUX_VAL(CP(DSS_DATA7),		(IDIS | PTD | DIS | M0));	// DSS_DATA7
	MUX_VAL(CP(DSS_DATA8),		(IDIS | PTD | DIS | M0));	// DSS_DATA8
	MUX_VAL(CP(DSS_DATA9),		(IDIS | PTD | DIS | M0));	// DSS_DATA9
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M0));	// DSS_DATA10
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTD | DIS | M0));	// DSS_DATA11
	MUX_VAL(CP(DSS_DATA12),		(IDIS | PTD | DIS | M0));	// DSS_DATA12
	MUX_VAL(CP(DSS_DATA13),		(IDIS | PTD | DIS | M0));	// DSS_DATA13
	MUX_VAL(CP(DSS_DATA14),		(IDIS | PTD | DIS | M0));	// DSS_DATA14
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | DIS | M0));	// DSS_DATA15
	MUX_VAL(CP(DSS_DATA16),		(IDIS | PTD | DIS | M0));	// DSS_DATA16
	MUX_VAL(CP(DSS_DATA17),		(IDIS | PTD | DIS | M0));	// DSS_DATA17
	MUX_VAL(CP(DSS_DATA18),		(IDIS | PTD | DIS | M0));	// DSS_DATA18
	MUX_VAL(CP(DSS_DATA19),		(IDIS | PTD | DIS | M0));	// DSS_DATA19
	MUX_VAL(CP(DSS_DATA20),		(IDIS | PTD | DIS | M0));	// DSS_DATA20
	MUX_VAL(CP(DSS_DATA21),		(IDIS | PTD | DIS | M0));	// DSS_DATA21
	MUX_VAL(CP(DSS_DATA22),		(IDIS | PTD | DIS | M0));	// DSS_DATA22
	MUX_VAL(CP(DSS_DATA23),		(IDIS | PTD | DIS | M0));	// DSS_DATA23

	/*
	 * Camera
	 *
	 */

	MUX_VAL(CP(CAM_HS),			(IEN  | PTU | EN  | M0));	// CAM_HS 
	MUX_VAL(CP(CAM_VS),			(IEN  | PTU | EN  | M0));	// CAM_VS 
	MUX_VAL(CP(CAM_XCLKA),		(IDIS | PTD | DIS | M0));	// CAM_XCLKA
	MUX_VAL(CP(CAM_PCLK),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_FLD),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_D0),			(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_D1),			(IEN  | PTD | DIS | M0));	// CAM_D1
	MUX_VAL(CP(CAM_D2),			(IDIS | PTD | DIS | M7));	// Unused

	// Assign CAM_D3 to GPIO_102 used as a push-pull, active high
	// WL_IRQ interrupt input from the TI WL1270 wireless Ethernet.

	MUX_VAL(CP(CAM_D3),			(IEN  | PTD | DIS | M4));	// GPIO_102

	// Assign CAM_D4 to GPIO_103 used as a push-pull, active high
	// WL_EN enable output to the TI WL1270 wireless Ethernet.

	MUX_VAL(CP(CAM_D4),			(IDIS | PTD | DIS | M4));	// GPIO_103

	// Assign CAM_D5 to GPIO_104 used as a push-pull, active high
	// enable output to the backplate 3.3v supply.

	MUX_VAL(CP(CAM_D5),			(IDIS | PTD | DIS | M4));	// GPIO_104
	MUX_VAL(CP(CAM_D7),			(IEN  | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_D8),			(IEN  | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_XCLKB),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_WEN),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(CAM_STROBE),		(IDIS | PTD | DIS | M7));	// Unused

	MUX_VAL(CP(CSI2_DX0),		(IEN  | PTD | DIS | M0));	// CSI2_DX0
	MUX_VAL(CP(CSI2_DY0),		(IEN  | PTD | DIS | M0));	// CSI2_DY0
	MUX_VAL(CP(CSI2_DX1),		(IEN  | PTD | DIS | M0));	// CSI2_DX1
	MUX_VAL(CP(CSI2_DY1),		(IEN  | PTD | DIS | M0));	// CSI2_DY1

	/*
	 * Multimedia Card (MMC)
	 *
	 *   Channel 1: Secure Digital (SD) Slot
	 *   Channel 2: Texas Instruments WL1270 802.11 b/g/n Wireless Ethernet
	 */

	MUX_VAL(CP(MMC1_CLK),		(IDIS | PTU | EN  | M0));	// MMC1_CLK
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | EN  | M0));	// MMC1_CMD
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | EN  | M0));	// MMC1_DAT0
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | EN  | M0));	// MMC1_DAT1
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | EN  | M0));	// MMC1_DAT2
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | EN  | M0));	// MMC1_DAT3
	MUX_VAL(CP(MMC1_DAT4),		(IEN  | PTU | EN  | M0));	// MMC1_DAT4
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTU | EN  | M0));	// MMC1_DAT5
	MUX_VAL(CP(MMC1_DAT6),		(IEN  | PTU | EN  | M0));	// MMC1_DAT6
	MUX_VAL(CP(MMC1_DAT7),		(IEN  | PTU | EN  | M0));	// MMC1_DAT7

	MUX_VAL(CP(MMC2_CLK),		(IEN  | PTD | DIS | M0));	// MMC2_CLK
	MUX_VAL(CP(MMC2_CMD),		(IEN  | PTU | EN  | M0));	// MMC2_CMD
	MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTU | EN  | M0));	// MMC2_DAT0
	MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTU | EN  | M0));	// MMC2_DAT1
	MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTU | EN  | M0));	// MMC2_DAT2
	MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTU | EN  | M0));	// MMC2_DAT3
	MUX_VAL(CP(MMC2_DAT4),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MMC2_DAT5),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MMC2_DAT6),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MMC2_DAT7),		(IDIS | PTD | DIS | M7));	// Unused

	/*
	 * Universal Asynchronous Receiver / Transmitter (UART)
	 *
	 */

	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0));	// UART1_TX
	/* The MSP430 does not use HW flow control */
	MUX_VAL(CP(UART1_RTS),		(IEN  | PTD | EN  | M7));	// Unused
	MUX_VAL(CP(UART1_CTS),		(IEN  | PTU | EN  | M7));	// Unused
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0));	// UART1_RX
	MUX_VAL(CP(UART2_CTS),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(UART2_TX),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(UART2_RX),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(UART3_CTS_RCTX),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(UART3_RTS_SD),	(IDIS | PTD | DIS | M0));	// UART3_RTS_SD 
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0));	// UART3_RX_IRRX
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0));	// UART3_TX_IRTX

	/*
	 * Multi-channel Buffered Serial Port (MCBSP)
	 *
	 */

	MUX_VAL(CP(MCBSP1_CLKR),	(IEN  | PTD | DIS | M0));	// MCBSP1_CLKR
	MUX_VAL(CP(MCBSP1_FSR),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTD | DIS | M0));	// MCBSP1_DR
	MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTD | DIS | M0));	// MCBSP_CLKS
	MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCBSP2_FSX),		(IEN  | PTD | DIS | M0));	// MCBSP2_FSX
	MUX_VAL(CP(MCBSP2_CLKX),	(IEN  | PTD | DIS | M0));	// MCBSP2_CLKX
	MUX_VAL(CP(MCBSP2_DR),		(IEN  | PTD | DIS | M0));	// MCBSP2_DR
	MUX_VAL(CP(MCBSP2_DX),		(IDIS | PTD | DIS | M0));	// MCBSP2_DX

	// Assign MCBSP3 as UART2 (Mode 1) for communication with the
	// ZigBee module.

	MUX_VAL(CP(MCBSP3_DX),		(IEN  | PTD | DIS | M1));	// UART2_CTS
	MUX_VAL(CP(MCBSP3_DR),		(IDIS | PTD | DIS | M1));	// UART2_RTS
	MUX_VAL(CP(MCBSP3_CLKX),	(IDIS | PTD | DIS | M1));	// UART2_TX
	MUX_VAL(CP(MCBSP3_FSX),		(IEN  | PTD | DIS | M1));	// UART2_RX

	// MCBSP4 is unavailable on the DM3730CUS package. Place all of
	// the associated pins in safe mode (Mode 7).

	MUX_VAL(CP(MCBSP4_CLKX),	(IDIS | PTD | DIS | M7));	// Unavailable
	MUX_VAL(CP(MCBSP4_DR),		(IDIS | PTD | DIS | M7));	// Unavailable
	MUX_VAL(CP(MCBSP4_DX),		(IDIS | PTD | DIS | M7));	// Unavailable
	MUX_VAL(CP(MCBSP4_FSX),		(IDIS | PTD | DIS | M7));	// Unavailable

	/*
	 * Universal Serial Bus (USB)
	 *
	 */

	MUX_VAL(CP(HSUSB0_CLK),		(IEN  | PTD | DIS | M0));	// HSUSB0_CLK
	MUX_VAL(CP(HSUSB0_STP),		(IDIS | PTU | EN  | M0));	// HSUSB0_STP
	MUX_VAL(CP(HSUSB0_DIR),		(IEN  | PTD | DIS | M0));	// HSUSB0_DIR
	MUX_VAL(CP(HSUSB0_NXT),		(IEN  | PTD | DIS | M0));	// HSUSB0_NXT
	MUX_VAL(CP(HSUSB0_DATA0),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA0
	MUX_VAL(CP(HSUSB0_DATA1),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA1
	MUX_VAL(CP(HSUSB0_DATA2),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA2
	MUX_VAL(CP(HSUSB0_DATA3),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA3
	MUX_VAL(CP(HSUSB0_DATA4),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA4
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA5
	MUX_VAL(CP(HSUSB0_DATA6),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA6
	MUX_VAL(CP(HSUSB0_DATA7),	(IEN  | PTD | DIS | M0));	// HSUSB0_DATA7

	/*
 	 * I2C
	 *
	 *   Channel 1:
	 *     - Texas Instruments TPS65921 Power Management Unit (PMU)
	 *
	 *   Channel 2:
	 *     - Silicon Labs 143 Proximity / Ambient Light Sensor
	 *
	 *   Channel 3:
	 *     - Unused
	 *
	 *   Channel 4:
	 *     - Texas Instruments TPS65921 Power Management Unit (PMU)
	 */

	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | EN  | M0));	// I2C1_SCL
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | EN  | M0));	// I2C1_SDA
	MUX_VAL(CP(I2C2_SCL),		(IEN  | PTU | EN  | M0));	// I2C2_SCL
	MUX_VAL(CP(I2C2_SDA),		(IEN  | PTU | EN  | M0));	// I2C2_SDA
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTU | EN  | M7));	// I2C3_SCL
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTU | EN  | M7));	// I2C3_SDA
	MUX_VAL(CP(I2C4_SCL),		(IEN  | PTU | EN  | M0));	// I2C4_SCL
	MUX_VAL(CP(I2C4_SDA),		(IEN  | PTU | EN  | M0));	// I2C4_SDA
	MUX_VAL(CP(HDQ_SIO),		(IEN  | PTU | EN  | M0));	// HDQ_SIO

	/*
	 * Multi-channel Serial Peripheral Interface (MCSPI)
	 *
	 *   Channel 1:
	 *     - Largely unused or unavailable on the DM3730CUS package,
	 *       aside from chip select 3, used as a backplate USB PHY
	 *       signal (Prototype + EVT).
	 *     - Used for input/output to/from the SPI interface of the
	 *       Tianma TM025ZDZ01 LCD display panel (DVT)
	 *
	 *   Channel 2:
	 *     - Used for output only to drive the SPI interface of the
	 *       Samsung LMS350DF0[13] LCD display panel (Prototype + EVT).
	 *     - The remaining signals are assigned as GPIOs.
	 */

	// MCSPI1_CS[12] are unavailable in the DM3730CUS package. Place the
	// associated pins in safe mode (Mode 7).

	MUX_VAL(CP(MCSPI1_CS1),		(IDIS | PTD | DIS | M7));	// Unavailable
	MUX_VAL(CP(MCSPI1_CS2),		(IDIS | PTD | DIS | M7));	// Unavailable

	// Assign MCSPI1_CS3 to MM2_TXDAT USB FS/LS Host (Mode 5).

	MUX_VAL(CP(MCSPI1_CS3),		(IEN  | PTD | EN  | M5));	// MM2_TXDAT

	// Assign MCSPI2_SOMI to GPT10_PWM_EVT used as a push-pull pulse width
	// modulated (PWM) signal for the Samsung LMS350DF0[13] backlight
	// control.

	MUX_VAL(CP(MCSPI2_SOMI),	(IDIS | PTD | DIS | M1));	// GPT10_PWM_EVT

	// Assign MCSPI2_CS2 to MM2_TXEN_N USB FS/LS Host (Mode 5).

	MUX_VAL(CP(MCSPI2_CS1),		(IEN  | PTD | EN  | M5));	// MM2_TXEN_N

	/*
	 * Control and Debug
	 *
	 */

	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0));	// SYS_32K
	MUX_VAL(CP(SYS_CLKREQ),		(IEN  | PTD | DIS | M0));	// SYS_CLKREQ
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0));	// SYS_nIRQ
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTD | DIS | M0));	// SYS_BOOT0
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | DIS | M0));	// SYS_BOOT1
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTD | DIS | M0));	// SYS_BOOT2
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | DIS | M0));	// SYS_BOOT3
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | DIS | M0));	// SYS_BOOT4
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | DIS | M0));	// SYS_BOOT5
	MUX_VAL(CP(SYS_BOOT6),		(IEN  | PTD | DIS | M0));	// SYS_BOOT6
	MUX_VAL(CP(SYS_OFF_MODE),	(IEN  | PTD | DIS | M0));	// SYS_OFF_MODE
	MUX_VAL(CP(SYS_CLKOUT1),    (IDIS | PTU | EN  | M0));	// SYS_CLKOUT1
	MUX_VAL(CP(SYS_CLKOUT2),    (IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(JTAG_nTRST),		(IEN  | PTD | DIS | M0));	// JTAG_nTRST
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0));	// JTAG_TCK
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0));	// JTAG_TMS
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0));	// JTAG_TDI
	MUX_VAL(CP(JTAG_EMU0),		(IEN  | PTD | DIS | M0));	// JTAG_EMU0
	MUX_VAL(CP(JTAG_EMU1),		(IEN  | PTD | DIS | M0));	// JTAG_EMU1
	MUX_VAL(CP(ETK_CLK_ES2),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_CTL_ES2),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D0_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D1_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D2_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D3_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D4_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D5_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D6_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D7_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D8_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D9_ES2),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D10_ES2),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D11_ES2),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D12_ES2),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D13_ES2),	(IDIS | PTD | DIS | M7));	// Unused

	// Assign ETK_D1[45] as MM2_RXRCV and MM2_TXSE0 USB FS/LS Host
	// (Mode 5)

	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTD | DIS | M5));	// MM2_RXRCV
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTD | DIS | M5));	// MM2_TXSE0

	/*
	 * Die-to-Die
	 *
	 */

	MUX_VAL(CP(D2D_MCAD1),		(IEN  | PTD | EN  | M0));	// D2D_MCAD1
	MUX_VAL(CP(D2D_MCAD2),		(IEN  | PTD | EN  | M0));	// D2D_MCAD2
	MUX_VAL(CP(D2D_MCAD3),		(IEN  | PTD | EN  | M0));	// D2D_MCAD3
	MUX_VAL(CP(D2D_MCAD4),		(IEN  | PTD | EN  | M0));	// D2D_MCAD4
	MUX_VAL(CP(D2D_MCAD5),		(IEN  | PTD | EN  | M0));	// D2D_MCAD5
	MUX_VAL(CP(D2D_MCAD6),		(IEN  | PTD | EN  | M0));	// D2D_MCAD6
	MUX_VAL(CP(D2D_MCAD7),		(IEN  | PTD | EN  | M0));	// D2D_MCAD7
	MUX_VAL(CP(D2D_MCAD8),		(IEN  | PTD | EN  | M0));	// D2D_MCAD8
	MUX_VAL(CP(D2D_MCAD9),		(IEN  | PTD | EN  | M0));	// D2D_MCAD9
	MUX_VAL(CP(D2D_MCAD10),		(IEN  | PTD | EN  | M0));	// D2D_MCAD10
	MUX_VAL(CP(D2D_MCAD11),		(IEN  | PTD | EN  | M0));	// D2D_MCAD11
	MUX_VAL(CP(D2D_MCAD12),		(IEN  | PTD | EN  | M0));	// D2D_MCAD12
	MUX_VAL(CP(D2D_MCAD13),		(IEN  | PTD | EN  | M0));	// D2D_MCAD13
	MUX_VAL(CP(D2D_MCAD14),		(IEN  | PTD | EN  | M0));	// D2D_MCAD14
	MUX_VAL(CP(D2D_MCAD15),		(IEN  | PTD | EN  | M0));	// D2D_MCAD15
	MUX_VAL(CP(D2D_MCAD16),		(IEN  | PTD | EN  | M0));	// D2D_MCAD16
	MUX_VAL(CP(D2D_MCAD17),		(IEN  | PTD | EN  | M0));	// D2D_MCAD17
	MUX_VAL(CP(D2D_MCAD18),		(IEN  | PTD | EN  | M0));	// D2D_MCAD18
	MUX_VAL(CP(D2D_MCAD19),		(IEN  | PTD | EN  | M0));	// D2D_MCAD19
	MUX_VAL(CP(D2D_MCAD20),		(IEN  | PTD | EN  | M0));	// D2D_MCAD20
	MUX_VAL(CP(D2D_MCAD21),		(IEN  | PTD | EN  | M0));	// D2D_MCAD21
	MUX_VAL(CP(D2D_MCAD22),		(IEN  | PTD | EN  | M0));	// D2D_MCAD22
	MUX_VAL(CP(D2D_MCAD23),		(IEN  | PTD | EN  | M0));	// D2D_MCAD23
	MUX_VAL(CP(D2D_MCAD24),		(IEN  | PTD | EN  | M0));	// D2D_MCAD24
	MUX_VAL(CP(D2D_MCAD25),		(IEN  | PTD | EN  | M0));	// D2D_MCAD25
	MUX_VAL(CP(D2D_MCAD26),		(IEN  | PTD | EN  | M0));	// D2D_MCAD26
	MUX_VAL(CP(D2D_MCAD27),		(IEN  | PTD | EN  | M0));	// D2D_MCAD27
	MUX_VAL(CP(D2D_MCAD28),		(IEN  | PTD | EN  | M0));	// D2D_MCAD28
	MUX_VAL(CP(D2D_MCAD29),		(IEN  | PTD | EN  | M0));	// D2D_MCAD29
	MUX_VAL(CP(D2D_MCAD30),		(IEN  | PTD | EN  | M0));	// D2D_MCAD30
	MUX_VAL(CP(D2D_MCAD31),		(IEN  | PTD | EN  | M0));	// D2D_MCAD31
	MUX_VAL(CP(D2D_MCAD32),		(IEN  | PTD | EN  | M0));	// D2D_MCAD32
	MUX_VAL(CP(D2D_MCAD33),		(IEN  | PTD | EN  | M0));	// D2D_MCAD33
	MUX_VAL(CP(D2D_MCAD34),		(IEN  | PTD | EN  | M0));	// D2D_MCAD34
	MUX_VAL(CP(D2D_MCAD35),		(IEN  | PTD | EN  | M0));	// D2D_MCAD35
	MUX_VAL(CP(D2D_MCAD36),		(IEN  | PTD | EN  | M0));	// D2D_MCAD36
	MUX_VAL(CP(D2D_CLK26MI),	(IEN  | PTD | DIS | M0));	// D2D_CLK26MI
	MUX_VAL(CP(D2D_NRESPWRON),	(IEN  | PTD | EN  | M0));	// D2D_NRESPWRON
	MUX_VAL(CP(D2D_NRESWARM),	(IEN  | PTU | EN  | M0));	// D2D_NRESWARM 
	MUX_VAL(CP(D2D_ARM9NIRQ),	(IEN  | PTD | DIS | M0));	// D2D_ARM9NIRQ 
	MUX_VAL(CP(D2D_UMA2P6FIQ),	(IEN  | PTD | DIS | M0));	// D2D_UMA2P6FIQ
	MUX_VAL(CP(D2D_SPINT),		(IEN  | PTD | EN  | M0));	// D2D_SPINT
	MUX_VAL(CP(D2D_FRINT),		(IEN  | PTD | EN  | M0));	// D2D_FRINT
	MUX_VAL(CP(D2D_DMAREQ0),	(IEN  | PTD | DIS | M0));	// D2D_DMAREQ0
	MUX_VAL(CP(D2D_DMAREQ1),	(IEN  | PTD | DIS | M0));	// D2D_DMAREQ1
	MUX_VAL(CP(D2D_DMAREQ2),	(IEN  | PTD | DIS | M0));	// D2D_DMAREQ2
	MUX_VAL(CP(D2D_DMAREQ3),	(IEN  | PTD | DIS | M0));	// D2D_DMAREQ3
	MUX_VAL(CP(D2D_N3GTRST),	(IEN  | PTD | DIS | M0));	// D2D_N3GTRST
	MUX_VAL(CP(D2D_N3GTDI),		(IEN  | PTD | DIS | M0));	// D2D_N3GTDI
	MUX_VAL(CP(D2D_N3GTDO),		(IEN  | PTD | DIS | M0));	// D2D_N3GTDO
	MUX_VAL(CP(D2D_N3GTMS),		(IEN  | PTD | DIS | M0));	// D2D_N3GTMS
	MUX_VAL(CP(D2D_N3GTCK),		(IEN  | PTD | DIS | M0));	// D2D_N3GTCK
	MUX_VAL(CP(D2D_N3GRTCK),	(IEN  | PTD | DIS | M0));	// D2D_N3GRTCK
	MUX_VAL(CP(D2D_MSTDBY),		(IEN  | PTU | EN  | M0));	// D2D_MSTDBY
	MUX_VAL(CP(D2D_SWAKEUP),	(IEN  | PTD | EN  | M0));	// D2D_SWAKEUP
	MUX_VAL(CP(D2D_IDLEREQ),	(IEN  | PTD | DIS | M0));	// D2D_IDLEREQ
	MUX_VAL(CP(D2D_IDLEACK),	(IEN  | PTU | EN  | M0));	// D2D_IDLEACK
	MUX_VAL(CP(D2D_MWRITE),		(IEN  | PTD | DIS | M0));	// D2D_MWRITE
	MUX_VAL(CP(D2D_SWRITE),		(IEN  | PTD | DIS | M0));	// D2D_SWRITE
	MUX_VAL(CP(D2D_MREAD),		(IEN  | PTD | DIS | M0));	// D2D_MREAD
	MUX_VAL(CP(D2D_SREAD),		(IEN  | PTD | DIS | M0));	// D2D_SREAD
	MUX_VAL(CP(D2D_MBUSFLAG),	(IEN  | PTD | DIS | M0));	// D2D_MBUSFLAG
	MUX_VAL(CP(D2D_SBUSFLAG),	(IEN  | PTD | DIS | M0));	// D2D_SBUSFLAG
}

