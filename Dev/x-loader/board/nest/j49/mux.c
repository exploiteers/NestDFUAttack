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
 *      primarily I/O pad multiplexer configuration.
 *
 *      This is originally inherited and split from the OMAP3 EVM
 *      board file.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>

#include "platform.h"

/*
 *  void set_muxconf_regs()
 *
 *  Description:
 *    This routines sets the I/O pad multiplexers for UART, GPMC, SDRC,
 *    GPIO.
 *
 * The commented string gives the final mux configuration for that pin
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

	// Assign GPMC_A9 to GPIO_42 used as a push-pull, active low input
	// from the USB PHY for USB suspend notification.

	MUX_VAL(CP(GPMC_A9),		(IDIS | PTD | DIS | M7));	// GPIO_42
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

	MUX_VAL(CP(CAM_WEN),		(IDIS | PTD | DIS | M7));	// Unused

	/*
	 * Universal Asynchronous Receiver / Transmitter (UART)
	 *
	 */

	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0));	// UART1_TX
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M7));	// UART1_RTS
	MUX_VAL(CP(UART1_CTS),		(IDIS | PTD | DIS | M7));	// UART1_CTS
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0));	// UART1_RX
	/*
	 * Control and Debug
	 *
	 */

	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0));	// SYS_32K
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTD | DIS | M0));	// SYS_BOOT0
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | DIS | M0));	// SYS_BOOT1
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTD | DIS | M0));	// SYS_BOOT2
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | DIS | M0));	// SYS_BOOT3
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | DIS | M0));	// SYS_BOOT4
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | DIS | M0));	// SYS_BOOT5
	MUX_VAL(CP(SYS_BOOT6),		(IEN  | PTD | DIS | M0));	// SYS_BOOT6
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

	MUX_VAL(CP(ETK_D14_ES2),	(IDIS | PTD | DIS | M7));	// MM2_RXRCV
	MUX_VAL(CP(ETK_D15_ES2),	(IDIS | PTD | DIS | M7));	// MM2_TXSE0
}
