/*
 *    Copyright (c) 2010-2012 Nest Labs, Inc.
 *
 *    (C) Copyright 2004-2008
 *    Texas Instruments, <www.ti.com>
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
 */

#include <common.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/omap_gpmc.h>
#include <asm/arch/sys_proto.h>
#include <linux/string.h>
#include <linux/mtd/nand.h>
#include <i2c.h>
#include <mmc.h>

#include "diamond.h"
#include "diamond-gpio.h"
#include "j49-gpio.h"
#include "nlmodel.h"

/*
 * Preprocessor Definitions
 */

#if defined(CONFIG_DIAMOND_BOARD_MULTI)
#define DIAMOND_BOARD_DEVELOPMENT_MODEL_INDEX	0
#define DIAMOND_BOARD_PROTOTYPE_MODEL_INDEX		1
#define DIAMOND_BOARD_EVT_MODEL_INDEX			2
#define DIAMOND_BOARD_DVT_MODEL_INDEX			3
#define DIAMOND_BOARD_PVT_MODEL_INDEX			4
#define J49_BOARD_PROTOTYPE_MODEL_INDEX			5
#define J49_BOARD_EVT_MODEL_INDEX				6
#define J49_BOARD_DVT_MODEL_INDEX				7
#define J49_BOARD_PRE_PVT_MODEL_INDEX			8
#define J49_BOARD_PVT_MODEL_INDEX				9
#else
#define DIAMOND_BOARD_DEVELOPMENT_MODEL_INDEX	0
#define DIAMOND_BOARD_PROTOTYPE_MODEL_INDEX		0
#define DIAMOND_BOARD_EVT_MODEL_INDEX			0
#define DIAMOND_BOARD_DVT_MODEL_INDEX			0
#define DIAMOND_BOARD_PVT_MODEL_INDEX			0
#define J49_BOARD_PROTOTYPE_MODEL_INDEX			0
#define J49_BOARD_EVT_MODEL_INDEX				0
#define J49_BOARD_DVT_MODEL_INDEX				0
#define J49_BOARD_PRE_PVT_MODEL_INDEX			0
#define J49_BOARD_PVT_MODEL_INDEX				0
#endif /* defined(CONFIG_DIAMOND_BOARD_MULTI) */

#define	DIAMOND_DEFAULT_MODEL_INDEX			J49_BOARD_PROTOTYPE_MODEL_INDEX
#define DIAMOND_DEFAULT_MODEL_IDENTIFIER		"Display-2.0"

/*
 * Type Definitions
 */

struct diamond_model {
	ulong machine;
	const struct nlmodel *model;
	const char *name;
	void (*mux_init)(void);
	void (*gpio_init)(void);
	int (*misc_init)(void);
	int (*eth_init)(bd_t *bis);
};

/*
 * Function Prototypes
 */

#if defined(CONFIG_CMD_NET)
static void setup_net_chip(void);
#endif

#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_development_mux_init(void);
static void diamond_development_gpio_init(void);
static int diamond_development_misc_init(void);
static int diamond_development_eth_init(bd_t *bis);
#endif /* defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_prototype_mux_init(void);
static void diamond_prototype_gpio_init(void);
#endif /* defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_EVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_evt_mux_init(void);
static void diamond_evt_gpio_init(void);
#endif /* defined(CONFIG_DIAMOND_BOARD_EVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_DVT) || defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_dvt_mux_init(void);
static void diamond_dvt_gpio_init(void);
#endif /* defined(CONFIG_DIAMOND_BOARD_DVT) || defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_pvt_mux_init(void);
static void diamond_pvt_gpio_init(void);
#endif /* defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_J49_BOARD_PROTOTYPE) || defined(CONFIG_J49_BOARD_MULTI)
static void j49_prototype_mux_init(void);
static void j49_prototype_gpio_init(void);
#endif /* defined(CONFIG_J49_BOARD_PROTOTYPE) || defined(CONFIG_J49_BOARD_MULTI) */

#if defined (CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI)
static void j49_evt_mux_init(void);
static void j49_evt_gpio_init(void);
#endif /* defined (CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI) */

void add_brightness(void);

/*
 * Global Variables
 */

const char *bootdelay_identifier_key = "bootdelay";

#if defined(CONFIG_DIAMOND_BOARD_MULTI)
static struct nlmodel model;

static const char *diamond_family = "Diamond";
static const char *j49_family = "Display";
static const int diamond_product = 1;
static const int j49_product = 2;
#endif /* defined(CONFIG_DIAMOND_BOARD_MULTI) */

/*
 * Astue readers will ask, "Why is the machine type OMAP3EVM for Diamond boards?". Because that's
 * the machine ID that such boards started life as and if a pairing of U-Boot that advertises DIAMOND
 * with a Linux kernel that only knows OMAP3EVM, the system will not boot.
 */
static struct diamond_model diamond_models[] = {
#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
	[DIAMOND_BOARD_DEVELOPMENT_MODEL_INDEX] = {
		MACH_TYPE_OMAP3EVM,
		NULL,
		"Diamond Development",
		diamond_development_mux_init,
		diamond_development_gpio_init,
		diamond_development_misc_init,
		diamond_development_eth_init
	},
#endif /* defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#if defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) || defined(CONFIG_DIAMOND_BOARD_MULTI)
	[DIAMOND_BOARD_PROTOTYPE_MODEL_INDEX] = {
		MACH_TYPE_OMAP3EVM,
		NULL,
		"Diamond Prototype",
		diamond_prototype_mux_init,
		diamond_prototype_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#if defined(CONFIG_DIAMOND_BOARD_EVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
	[DIAMOND_BOARD_EVT_MODEL_INDEX] = {
		MACH_TYPE_OMAP3EVM,
		NULL,
		"Diamond EVT",
		diamond_evt_mux_init,
		diamond_evt_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_DIAMOND_BOARD_EVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#if defined(CONFIG_DIAMOND_BOARD_DVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
	[DIAMOND_BOARD_DVT_MODEL_INDEX] = {
		MACH_TYPE_OMAP3EVM,
		NULL,
		"Diamond DVT",
		diamond_dvt_mux_init,
		diamond_dvt_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_DIAMOND_BOARD_DVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#if defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
	[DIAMOND_BOARD_PVT_MODEL_INDEX] = {
		MACH_TYPE_OMAP3EVM,
		NULL,
		"Diamond PVT",
		diamond_pvt_mux_init,
		diamond_pvt_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#if defined(CONFIG_J49_BOARD_PROTOTYPE) || defined(CONFIG_J49_BOARD_MULTI)
	[J49_BOARD_PROTOTYPE_MODEL_INDEX] = {
		MACH_TYPE_J49,
		NULL,
		"J49 Prototype",
		j49_prototype_mux_init,
		j49_prototype_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_J49_BOARD_PROTOTYPE) || defined(CONFIG_J49_BOARD_MULTI) */
#if defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_MULTI)
	[J49_BOARD_EVT_MODEL_INDEX] = {
		MACH_TYPE_J49,
		NULL,
		"J49 EVT",
		j49_evt_mux_init,
		j49_evt_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_MULTI) */
#if defined(CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_MULTI)
	[J49_BOARD_DVT_MODEL_INDEX] = {
		MACH_TYPE_J49,
		NULL,
		"J49 DVT",
		j49_evt_mux_init,
		j49_evt_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_MULTI) */
#if defined(CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_MULTI)
	[J49_BOARD_PRE_PVT_MODEL_INDEX] = {
		MACH_TYPE_J49,
		NULL,
		"J49 Pre-PVT",
		j49_evt_mux_init,
		j49_evt_gpio_init,
		NULL,
		NULL
	},
#endif /* defined(CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_MULTI) */
#if defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI)
	[J49_BOARD_PVT_MODEL_INDEX] = {
		MACH_TYPE_J49,
		NULL,
		"J49 PVT",
		j49_evt_mux_init,
		j49_evt_gpio_init,
		NULL,
		NULL
	}
#endif /* defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI) */
};

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"Diamond",
	"NAND",
};

static struct diamond_model *diamond_model = NULL;

static const char *bootargs_key = "bootargs";
static const char *brightness_key = "brightness";
static const char *panic_key = "panic";

DECLARE_GLOBAL_DATA_PTR;

static void inline diamond_gpio_input(unsigned int gpio)
{
    gpio_request(gpio, "");
    gpio_direction_input(gpio);
}

static void inline diamond_gpio_output(unsigned int gpio, int value)
{
    gpio_request(gpio, "");
    gpio_direction_output(gpio, value);
}

static void diamond_generic_gpio_init(void)
{
	/* Backplate GPIOs */

	diamond_gpio_input(DIAMOND_GPIO_BACKPLATE_DETECT);

	/* LCD GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_LCD_RESETB, 0);

	/* WiFi GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_WIFI_ENABLE, 0);
	diamond_gpio_input(DIAMOND_GPIO_WIFI_IRQ);

	/* ZigBee GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_ZIGBEE_RESET_L, 0);
}

#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_development_mux_init(void)
{
	// Assign GPMC_A10 to GPIO_43 used as push-pull, active low output
	// LCD_nENBUFFER for the 1.8V to 3.3V LCD buffer enable.

	MUX_VAL(CP(GPMC_A10),		(IDIS | PTD | DIS | M4));	// GPIO_43

	// Assign to GPMC_NCS3 for Samsung K9F4G08U0C 512 MiB SLC NAND
	// Flash

	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTU | EN  | M0));	// GPMC_nCS3

	// Assign to GPMC_NCS5 for SMSC LAN9220 10/100 Mbit Ethernet MAC
	// and PHY

	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTU | EN  | M0));	// GPMC_nCS5

	// Assign CAM_PCLK to GPIO_97 used as a push-pull, input from the
	// PMU charger suspend signal.

	MUX_VAL(CP(CAM_PCLK),		(IEN  | PTD | DIS | M4));	// GPIO_97

	// Assign CAM_D0 to GPIO_99 used as a push-pull input from the
	// active-low PMU charger charge signal.

	MUX_VAL(CP(CAM_D0),			(IEN  | PTD | DIS | M4));	// GPIO_99

	// Assign CAM_D2 to GPIO_101 used as a push-pull, active high
	// LCD_ENVDD LCD Vdd enable.

	MUX_VAL(CP(CAM_D2),			(IDIS | PTD | DIS | M4));	// GPIO_101

	// Assign CAM_D{6,7,8,9} to GPIO_{105,106,107,108} used as
	// push-pull, active-high inputs ROTARY_{DIR,VALID,LAST_DIR,LAST_VALID}
	// from the rotary input state storage.

	MUX_VAL(CP(CAM_D6),			(IEN  | PTD | DIS | M4));	// GPIO_105
	MUX_VAL(CP(CAM_D7),			(IEN  | PTD | DIS | M4));	// GPIO_106
	MUX_VAL(CP(CAM_D8),			(IEN  | PTD | DIS | M4));	// GPIO_107
	MUX_VAL(CP(CAM_D9),			(IEN  | PTD | DIS | M4));	// GPIO_108

	// Assign CAM_D10 to GPIO_109 used as a push-pull, active low
	// output LCD_nRESET for the Samsung LMS350DF03 LCD reset.

	MUX_VAL(CP(CAM_D10),		(IDIS | PTD | DIS | M4));	// GPIO_109

	// Assign CAM_D11 to GPIO_110 used as a push-pull, active-low
	// output ROTARY_CLEAR to the rotary input state storage.

	MUX_VAL(CP(CAM_D11),		(IDIS | PTD | DIS | M4));	// GPIO_110

	// Assign CAM_STROBE to GPIO used as a push-pull input from the
	// PMU double-current signal.

	MUX_VAL(CP(CAM_STROBE),		(IEN  | PTD | DIS | M4));	// GPIO_126

	// Assign UART3_CTS_RCTX to GPIO_163 used as a push-pull input
	// from the ZigBee CPU.

	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTD | DIS | M4));	// GPIO_163

	// The MCSPI1 interface is unused. Place the associated pins in
	// safe mode (Mode 7).

	MUX_VAL(CP(MCSPI1_CLK),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_SIMO),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_SOMI),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_CS0),		(IDIS | PTD | DIS | M7));	// Unused

	// Surprisingly, per http://e2e.ti.com/support/dsp/
	// omap_applications_processors/f/447/p/54130/193260.aspx#193260,
	// the SPI clock MUST be configured as an input for the SPI
	// interface to work correctly.

	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | DIS | M0));	// MCSPI2_CLK
	MUX_VAL(CP(MCSPI2_SIMO),	(IEN  | PTD | DIS | M0));	// MCSPI2_SIMO
	MUX_VAL(CP(MCSPI2_CS0),		(IEN  | PTD | EN  | M0));	// MCSPI2_CS0

	// Assign MCBSP1_FSR to GPIO_157 used as a push-pull, active-low
	// reset output to the SMSC9220 Ethernet.

	MUX_VAL(CP(MCBSP1_FSR),		(IDIS | PTD | DIS | M4));	// GPIO_157

	// Assign MCBSP1_FSX to GPIO_161 used as a push-pull, active
	// interrupt input from the Silicon Labs 143 Proximity / Ambient
	// Light Sensor

	MUX_VAL(CP(MCBSP1_FSX),		(IEN  | PTD | DIS | M4));	// GPIO_161

	// Assign MCBSP1_CLKX to GPIO_162 used as an externally pulled-up
	// interrupt input from the SMSC9220 Ethernet.

	MUX_VAL(CP(MCBSP1_CLKX),	(IEN  | PTD | DIS | M4));	// GPIO_162

	// Assign MCBSP1_DX to GPIO_158 used as a push-pull, active-high
	// output to the ZigBee power supply.
 	 
 	MUX_VAL(CP(MCBSP1_DX),      (IDIS | PTD | DIS | M4));   // GPIO_158
}

static void diamond_development_gpio_init(void)
{
	diamond_generic_gpio_init();

	/* Backplate GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_BACKPLATE_3V3_ENABLE, 0);

	/* LCD GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_LCD_NENABLE_BUFFER, 1);
	diamond_gpio_output(DIAMOND_GPIO_LCD_ENABLE_VDD, 0);

	/* Rotary Control GPIOs */

	diamond_gpio_input(DIAMOND_GPIO_ROTARY_DIR);
	diamond_gpio_input(DIAMOND_GPIO_ROTARY_VALID);
	diamond_gpio_input(DIAMOND_GPIO_ROTARY_LAST_DIR);
	diamond_gpio_input(DIAMOND_GPIO_ROTARY_LAST_VALID);
	diamond_gpio_output(DIAMOND_GPIO_ROTARY_CLEAR, 1);

	/* Zigbee GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_ZIGBEE_PWR_ENABLE, 0);

	/* Miscellaneous GPIOs */

	diamond_gpio_input(DIAMOND_GPIO_PROX_ALS_IRQ);
}

static int diamond_development_misc_init(void)
{
	int rc = 0;

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif

	return rc;
}

static int diamond_development_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif

	return rc;
}
#endif /* defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_prototype_mux_init(void)
{
	// Assign CAM_D6 to GPIO_105 used as a push-pull, active-low input
	// LCD_ID from the LCD module flexible printed circuit.

	MUX_VAL(CP(CAM_D6),			(IEN  | PTD | DIS | M4));	// GPIO_105

	// Assign I2C3_SCL to GPIO_184 used as a push-pull active-low
	// output OFN_RESET_L to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor NRST input.

	MUX_VAL(CP(I2C3_SCL),		(IDIS | PTD | DIS | M4));	// GPIO_184

	// Assign I2C3_SDA to GPIO_185 used as a push-pull active-high
	// output OFN_SHUTDOWN to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor SHTDWN input.

	MUX_VAL(CP(I2C3_SDA),		(IDIS | PTD | DIS | M4));	// GPIO_185

	// Assign CAM_D9 to GPIO_108 used as a push-pull active-low
	// input OFN_MOTION_L from the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor MOTION output.

	MUX_VAL(CP(CAM_D9),			(IEN  | PTD | DIS | M4));	// GPIO_108

	// Assign CAM_D10 to GPIO_109 used as a push-pull, active low
	// output LCD_nRESET for the Tianma TM025ZDZ01 LCD reset.

	MUX_VAL(CP(CAM_D10),		(IDIS | PTD | DIS | M4));	// GPIO_109

	// The MCSPI1 interface is unused. Place the associated pins in
	// safe mode (Mode 7).

	MUX_VAL(CP(MCSPI1_CLK),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_SIMO),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_SOMI),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_CS0),		(IDIS | PTD | DIS | M7));	// Unused

	// Surprisingly, per http://e2e.ti.com/support/dsp/
	// omap_applications_processors/f/447/p/54130/193260.aspx#193260,
	// the SPI clock MUST be configured as an input for the SPI
	// interface to work correctly.

	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | DIS | M0));	// MCSPI2_CLK
	MUX_VAL(CP(MCSPI2_SIMO),	(IEN  | PTD | DIS | M0));	// MCSPI2_SIMO
	MUX_VAL(CP(MCSPI2_CS0),		(IEN  | PTD | EN  | M0));	// MCSPI2_CS0

	// Assign MCBSP1_DX to GPIO_158 used as a push-pull, active-high
	// output to the ZigBee power supply.
 	 
 	MUX_VAL(CP(MCBSP1_DX),      (IDIS | PTD | DIS | M4));   // GPIO_158
}
	
static void diamond_prototype_gpio_init(void)
{
	diamond_generic_gpio_init();

	/* Backplate GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_BACKPLATE_RESET, 0);

	/* LCD GPIOs */

	diamond_gpio_input(DIAMOND_GPIO_LCD_ID);

	/* Optical Finger Navigation (OFN) Sensor GPIOs */

	diamond_gpio_output(DIAMOND_PROTOTYPE_GPIO_OFN_SHUTDOWN, 1);
	diamond_gpio_output(DIAMOND_PROTOTYPE_GPIO_OFN_RESET_L, 0);
	diamond_gpio_input(DIAMOND_GPIO_OFN_MOTION_L);

	/* Zigbee GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_ZIGBEE_PWR_ENABLE, 0);

	/* Miscellaneous GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_PIEZO_NENABLE, 1);
}
#endif /* defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_EVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_evt_mux_init(void)
{
	// Assign CAM_D6 to GPIO_105 used as a push-pull, active-low input
	// LCD_ID from the LCD module flexible printed circuit.

	MUX_VAL(CP(CAM_D6),			(IEN  | PTD | DIS | M4));	// GPIO_105

	// Assign CAM_D11 to GPIO_110 used as a push-pull active-low
	// output OFN_RESET_L to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor NRST input.

	MUX_VAL(CP(CAM_D11),		(IDIS | PTD | DIS | M4));	// GPIO_110

	// Assign CAM_D2 to GPIO_101 used as a push-pull active-high
	// output OFN_SHUTDOWN to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor SHTDWN input.

	MUX_VAL(CP(CAM_D2),			(IDIS | PTD | DIS | M4));	// GPIO_101

	// Assign CAM_D9 to GPIO_108 used as a push-pull active-low
	// input OFN_MOTION_L from the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor MOTION output.

	MUX_VAL(CP(CAM_D9),			(IEN  | PTD | DIS | M4));	// GPIO_108

	// Assign CAM_D10 to GPIO_109 used as a push-pull, active low
	// output LCD_nRESET for the Tianma TM025ZDZ01 LCD reset.

	MUX_VAL(CP(CAM_D10),		(IDIS | PTD | DIS | M4));	// GPIO_109

	// The MCSPI1 interface is unused. Place the associated pins in
	// safe mode (Mode 7).

	MUX_VAL(CP(MCSPI1_CLK),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_SIMO),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_SOMI),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_CS0),		(IDIS | PTD | DIS | M7));	// Unused

	// Surprisingly, per http://e2e.ti.com/support/dsp/
	// omap_applications_processors/f/447/p/54130/193260.aspx#193260,
	// the SPI clock MUST be configured as an input for the SPI
	// interface to work correctly.

	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | DIS | M0));	// MCSPI2_CLK
	MUX_VAL(CP(MCSPI2_SIMO),	(IEN  | PTD | DIS | M0));	// MCSPI2_SIMO
	MUX_VAL(CP(MCSPI2_CS0),		(IEN  | PTD | EN  | M0));	// MCSPI2_CS0

	// Assign MCBSP1_FSX to GPIO_161 used as a push-pull, active high
	// output BATT_DISCONNECT for the battery management circuit.

	MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTD | DIS | M4));	// GPIO_161

	// Assign MCBSP1_DX to GPIO_158 used as a push-pull, active-high
 	// output to the ZigBee power supply.
 	 
 	MUX_VAL(CP(MCBSP1_DX),      (IDIS | PTD | DIS | M4));   // GPIO_158
}
	
static void diamond_evt_gpio_init(void)
{
	diamond_generic_gpio_init();

	/* Backplate GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_BACKPLATE_RESET, 0);

	/* LCD GPIOs */

	diamond_gpio_input(DIAMOND_GPIO_LCD_ID);

	/* Optical Finger Navigation (OFN) Sensor GPIOs */

	diamond_gpio_output(DIAMOND_EVT_GPIO_OFN_SHUTDOWN, 1);
	diamond_gpio_output(DIAMOND_EVT_GPIO_OFN_RESET_L, 0);
	diamond_gpio_input(DIAMOND_GPIO_OFN_MOTION_L);

	/* Zigbee GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_ZIGBEE_PWR_ENABLE, 0);

	/* Miscellaneous GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_PIEZO_NENABLE, 1);
	diamond_gpio_output(DIAMOND_GPIO_BATT_DISCONNECT, 0);
}
#endif /* defined(CONFIG_DIAMOND_BOARD_EVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_DVT) || defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_dvt_mux_init(void)
{
	// Assign CAM_D6 to GPIO_105 used as a push-pull, active-low input
	// LCD_ID from the LCD module flexible printed circuit.

	MUX_VAL(CP(CAM_D6),			(IEN  | PTD | DIS | M4));	// GPIO_105

	// Assign CAM_D11 to GPIO_110 used as a push-pull active-low
	// output OFN_RESET_L to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor NRST input.

	MUX_VAL(CP(CAM_D11),		(IDIS | PTD | DIS | M4));	// GPIO_110

	// Assign CAM_D2 to GPIO_101 used as a push-pull active-high
	// output OFN_SHUTDOWN to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor SHTDWN input.

	MUX_VAL(CP(CAM_D2),			(IDIS | PTD | DIS | M4));	// GPIO_101

	// Assign CAM_D9 to GPIO_108 used as a push-pull active-low
	// input OFN_MOTION_L from the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor MOTION output.

	MUX_VAL(CP(CAM_D9),			(IEN  | PTD | DIS | M4));	// GPIO_108

	// Assign CAM_D10 to GPIO_109 used as a push-pull, active low
	// output LCD_nRESET for the Tianma TM025ZDZ01 LCD reset.

	MUX_VAL(CP(CAM_D10),		(IDIS | PTD | DIS | M4));	// GPIO_109

	// Surprisingly, per http://e2e.ti.com/support/dsp/
	// omap_applications_processors/f/447/p/54130/193260.aspx#193260,
	// the SPI clock MUST be configured as an input for the SPI
	// interface to work correctly.

	MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTD | DIS | M0));	// MCSPI1_CLK
	MUX_VAL(CP(MCSPI1_SIMO),	(IDIS | PTD | DIS | M0));	// MCSPI1_SIMO
	MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTD | DIS | M0));	// MCSPI1_SOMI
	MUX_VAL(CP(MCSPI1_CS0),		(IDIS | PTD | EN  | M0));	// MCSPI1_CS0

	MUX_VAL(CP(MCSPI2_CLK),		(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI2_SIMO),	(IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI2_CS0),		(IDIS | PTD | DIS | M7));	// Unused

	// Assign MCBSP1_FSX to GPIO_161 used as a push-pull, active high
	// output BATT_DISCONNECT for the battery management circuit.

	MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTD | DIS | M4));	// GPIO_161

	// Assign MCBSP1_DX to GPIO_158 used as a push-pull, active-high
	// output to the ZigBee power supply.
 	 
 	MUX_VAL(CP(MCBSP1_DX),      (IDIS | PTD | DIS | M4));   // GPIO_158
}
	
static void diamond_dvt_gpio_init(void)
{
	diamond_generic_gpio_init();

	/* Backplate GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_BACKPLATE_RESET, 0);

	/* LCD GPIOs */

	diamond_gpio_input(DIAMOND_GPIO_LCD_ID);

	/* Optical Finger Navigation (OFN) Sensor GPIOs */

	diamond_gpio_output(DIAMOND_EVT_GPIO_OFN_SHUTDOWN, 1);
	diamond_gpio_output(DIAMOND_EVT_GPIO_OFN_RESET_L, 0);
	diamond_gpio_input(DIAMOND_GPIO_OFN_MOTION_L);

	/* Zigbee GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_ZIGBEE_PWR_ENABLE, 0);

	/* Miscellaneous GPIOs */

	diamond_gpio_output(DIAMOND_GPIO_PIEZO_NENABLE, 1);
	diamond_gpio_output(DIAMOND_GPIO_BATT_DISCONNECT, 0);
}
#endif /* defined(CONFIG_DIAMOND_BOARD_DVT) || defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
static void diamond_pvt_mux_init(void)
{
	diamond_dvt_mux_init();
}
	
static void diamond_pvt_gpio_init(void)
{
	diamond_dvt_gpio_init();
}
#endif /* defined(CONFIG_DIAMOND_BOARD_PVT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

#if defined(CONFIG_J49_BOARD_PROTOTYPE) || defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI)
static void j49_prototype_mux_init(void)
{

	MUX_VAL(CP(GPMC_A9),	 (IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCSPI1_CS3),	 (IEN  | PTD | EN  | M5));	// LCD_BL_HWEN

  	// Assign CAM_D6 to GPIO_105 used as a push-pull, active-low input
	// LCD_ID from the LCD module flexible printed circuit.
	MUX_VAL(CP(CAM_D6),      (IEN  | PTU | EN | M4));	// GPIO_105

	// Assign CAM_D11 to GPIO_110 used as a push-pull active-low
	// output OFN_RESET_L to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor NRST input.

	MUX_VAL(CP(CAM_D11),     (IDIS | PTD | DIS | M4));	// GPIO_110

	// Assign CAM_D2 to GPIO_101 used as a push-pull active-high
	// output OFN_SHUTDOWN to the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor SHTDWN input.

	MUX_VAL(CP(CAM_D2),      (IDIS | PTD | DIS | M4));	// GPIO_101

	// Assign CAM_D9 to GPIO_108 used as a push-pull active-low
	// input OFN_MOTION_L from the Avago ADBS A320 Optical Finger
	// Navigation (OFN) sensor MOTION output.

	MUX_VAL(CP(CAM_D9),      (IEN  | PTD | DIS | M4));	// GPIO_108

	// Assign CAM_D10 to GPIO_109 used as a push-pull, active low
	// output LCD_nRESET for the Tianma TM025ZDZ01 LCD reset.

	MUX_VAL(CP(CAM_D10),     (IDIS | PTD | DIS | M4));	// GPIO_109

	MUX_VAL(CP(MCSPI1_CLK),  (IDIS | PTD | DIS | M0));	// LCD_SPI_CLK 
	MUX_VAL(CP(MCSPI1_SIMO), (IDIS | PTD | DIS | M0));	// LCD_SPI_MOSI
	MUX_VAL(CP(MCSPI1_SOMI), (IEN  | PTD | DIS | M0));	// LCD_SPI_MISO
	MUX_VAL(CP(MCSPI1_CS0),	 (IDIS | PTD | EN  | M0));	// LCD_SPI_CS_L

	MUX_VAL(CP(MCSPI2_CLK),	 (IEN  | PTD | DIS | M0));	// ZIG_SPI_SCLK
	MUX_VAL(CP(MCSPI2_SIMO), (IDIS | PTD | DIS | M0));	// ZIG_SPI_MOSI
	MUX_VAL(CP(MCSPI2_SOMI), (IEN  | PTD | EN  | M0));	// ZIG_SPI_MISO
	MUX_VAL(CP(MCSPI2_CS0),	 (IDIS | PTD | DIS | M0));	// ZIG_SPI_CS_L
	MUX_VAL(CP(MCSPI2_CS1),  (IEN  | PTU | EN  | M4));	// ZIG_INT_L 
	MUX_VAL(CP(MCBSP3_DX),   (IEN  | PTD | DIS | M4));	// ZIG_PTI_EN
	MUX_VAL(CP(MCBSP3_DR),   (IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCBSP3_CLKX), (IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(MCBSP3_FSX),	 (IEN  | PTD | DIS | M1));	// ZIG_PIT_DATA
	MUX_VAL(CP(MCBSP1_CLKR), (IDIS | PTD | EN  | M7));	// BP_PRIMARY_LDO_DISABLE 

	MUX_VAL(CP(ETK_D14_ES2), (IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(ETK_D15_ES2), (IDIS | PTD | DIS | M7));	// Unused

	MUX_VAL(CP(UART1_RTS),   (IDIS | PTD | DIS | M7));	// Unused
	MUX_VAL(CP(UART1_CTS),	 (IDIS | PTU | DIS | M7));	// Unused

	// Assign MCBSP1_FSX to GPIO_161 used as a push-pull, active high
	// output BATT_DISCONNECT for the battery management circuit.

	MUX_VAL(CP(MCBSP1_FSX),	 (IDIS | PTD | DIS | M4));	// GPIO_161

	// ZIG_PWR_EN set to high Z for now, Andrea to investigate further

	MUX_VAL(CP(MCBSP2_DX),	 (IDIS | PTD | DIS | M7));	
}
	
static void j49_prototype_gpio_init(void)
{
	diamond_generic_gpio_init();

	/* Backplate GPIOs */
	diamond_gpio_output(DIAMOND_GPIO_BACKPLATE_RESET, 0);

	/* LCD GPIOs */
	diamond_gpio_input(DIAMOND_GPIO_LCD_ID);

	/* Optical Finger Navigation (OFN) Sensor GPIOs */
	diamond_gpio_output(DIAMOND_EVT_GPIO_OFN_SHUTDOWN, 1);
	diamond_gpio_output(DIAMOND_EVT_GPIO_OFN_RESET_L, 0);
	diamond_gpio_input(DIAMOND_GPIO_OFN_MOTION_L);

	/* Miscellaneous GPIOs */
	diamond_gpio_output(DIAMOND_GPIO_PIEZO_NENABLE, 1);
	diamond_gpio_output(DIAMOND_GPIO_BATT_DISCONNECT, 0);
}
#endif /* defined(CONFIG_J49_BOARD_PROTOTYPE) || defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_DVT) || defined (CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI) */

#if defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_PRE_PVT)|| defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI)
static void j49_evt_gpio_init(void)
{
    j49_prototype_gpio_init();
}

static void j49_evt_mux_init(void)
{
    j49_prototype_mux_init();
}
#endif /* defined(CONFIG_J49_BOARD_EVT) || defined(CONFIG_J49_BOARD_DVT) || defined(CONFIG_J49_BOARD_PRE_PVT) || defined(CONFIG_J49_BOARD_PVT) || defined(CONFIG_J49_BOARD_MULTI) */

/*
 *  int board_init()
 *
 *  Description:
 *    This routine performs very early hardware initialization.
 *
 *    At the time of invocation, little to nothing has been
 *    initialized. Not interrupts, not the console, not the
 *    environment, not NAND...nothing.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    0 if successful; otherwise, < 0.
 *
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* Default board ID for Linux, we'll fix this up later. */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3EVM;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 *  init misc_init_r()
 *
 *  Description:
 *    This routine performs later miscellaneous platform-dependent
 *    initializations after relocation to RAM has completed.
 *
 *    At the time of invocation, most things, save interrupts and
 *    networking, have been initialized.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    0 if successful; otherwise, < 0.
 *
 */
int misc_init_r(void)
{
	const char *serial;
	const char *bootdelay_str;
	int dirty = 0;
#if defined(CONFIG_PREBOOT)
	char cmd[64] = { '\0' };
#endif

#if defined(CONFIG_DIAMOND_BOARD_MULTI)
	const char *identifier;
	int status;

	status = nlmodel_init(&model);

	if (status != 0)
		goto punt;

	identifier = nlmodel_identifier();

	if (identifier == NULL) {
		setenv(nlmodel_identifier_key, DIAMOND_DEFAULT_MODEL_IDENTIFIER);
		dirty = 1;
		identifier = nlmodel_identifier();
	}

	status = nlmodel_parse(identifier, &model);

	if (status != 0)
		goto punt;

	if ((strcmp(model.family, diamond_family) != 0) && (strcmp(model.family, j49_family) != 0)) {
		printf("Unknown model family '%s'\n", model.family);
		goto punt;
	}

	if (model.product == diamond_product) {
		if (model.revision >= 0 && model.revision <= 3) {
			diamond_model = &diamond_models[DIAMOND_BOARD_DEVELOPMENT_MODEL_INDEX];

		} else if (model.revision == 4 || model.revision == 5) {
			diamond_model = &diamond_models[DIAMOND_BOARD_PROTOTYPE_MODEL_INDEX];

		} else if (model.revision == 6) {
			diamond_model = &diamond_models[DIAMOND_BOARD_EVT_MODEL_INDEX];

		} else if (model.revision == 7 || model.revision == 8) {
			diamond_model = &diamond_models[DIAMOND_BOARD_DVT_MODEL_INDEX];

		} else if (model.revision >= 9) {
			diamond_model = &diamond_models[DIAMOND_BOARD_PVT_MODEL_INDEX];

		}
	} else if (model.product == j49_product) {
		if (model.revision == 0) {
			diamond_model = &diamond_models[J49_BOARD_PROTOTYPE_MODEL_INDEX];
			
		} else if (model.revision == 1) {
			diamond_model = &diamond_models[J49_BOARD_EVT_MODEL_INDEX];

		} else if (model.revision == 2) {
			diamond_model = &diamond_models[J49_BOARD_DVT_MODEL_INDEX];

		} else if (model.revision == 3) {
			diamond_model = &diamond_models[J49_BOARD_PRE_PVT_MODEL_INDEX];

		} else if (model.revision >= 4) {
			diamond_model = &diamond_models[J49_BOARD_PVT_MODEL_INDEX];

		} else {
			printf("Unknown model revision '%d'\n", model.revision);
			goto punt;
		}
	} else {
		printf("Unknown model product '%d'\n", model.product);
		goto punt;
	}

 punt:
	if (diamond_model == NULL)
		diamond_model = &diamond_models[DIAMOND_DEFAULT_MODEL_INDEX];

	diamond_model->model = &model;
#elif defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
	diamond_model = &diamond_models[DIAMOND_BOARD_DEVELOPMENT_MODEL_INDEX];
#elif defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
	diamond_model = &diamond_models[DIAMOND_BOARD_PROTOTYPE_MODEL_INDEX];
#elif defined(CONFIG_DIAMOND_BOARD_EVT) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
	diamond_model = &diamond_models[DIAMOND_BOARD_EVT_MODEL_INDEX];
#elif defined(CONFIG_DIAMOND_BOARD_DVT) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
	diamond_model = &diamond_models[DIAMOND_BOARD_DVT_MODEL_INDEX];
#elif defined(CONFIG_DIAMOND_BOARD_PVT) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
	diamond_model = &diamond_models[DIAMOND_BOARD_PVT_MODEL_INDEX];
#elif defined(CONFIG_J49_BOARD_PROTOTYPE) && !defined(CONFIG_J49_BOARD_MULTI)
	diamond_model = &diamond_models[J49_BOARD_PROTOTYPE_MODEL_INDEX];
#elif defined(CONFIG_J49_BOARD_EVT) && !defined(CONFIG_J49_BOARD_MULTI)
	diamond_model = &diamond_models[J49_BOARD_EVT_MODEL_INDEX];
#elif defined(CONFIG_J49_BOARD_DVT) && !defined(CONFIG_J49_BOARD_MULTI)
	diamond_model = &diamond_models[J49_BOARD_DVT_MODEL_INDEX];
#elif defined(CONFIG_J49_BOARD_PRE_PVT) && !defined(CONFIG_J49_BOARD_MULTI)
	diamond_model = &diamond_models[J49_BOARD_PRE_PVT_MODEL_INDEX];
#elif defined(CONFIG_J49_BOARD_PVT) && !defined(CONFIG_J49_BOARD_MULTI)
	diamond_model = &diamond_models[J49_BOARD_PVT_MODEL_INDEX];
#else
#error "A specific Nest Diamond board type has not been configured!"
#endif /* defined(CONFIG_DIAMOND_BOARD_MULTI) */

	/* Fix bootdelay if necessary */
	bootdelay_str = getenv(bootdelay_identifier_key);
	if ( (bootdelay_str == NULL) || 
		 (strcmp(bootdelay_str, CONFIG_BOOTDELAY_STR) != 0) ) {
		setenv(bootdelay_identifier_key, CONFIG_BOOTDELAY_STR);
		dirty = 1;
	}
    
	if (diamond_model != NULL) {
		if (diamond_model->mux_init) {
			diamond_model->mux_init();
		}

		if (diamond_model->gpio_init) {
			diamond_model->gpio_init();
		}

		if (diamond_model->misc_init) {
			diamond_model->misc_init();
		}

		gd->bd->bi_arch_number = diamond_model->machine;
	}

	printf("Board: Nest %s", diamond_model->name);

#if defined(CONFIG_DIAMOND_BOARD_MULTI)
	printf(", Revision %d", diamond_model->model->revision);
#endif /* defined(CONFIG_DIAMOND_BOARD_MULTI) */

	serial = getenv("serial#");

	if (serial) {
		puts(", Serial ");
		puts(serial);
	}

	putc('\n');

#ifdef NEST_BUILD_CONFIG
	printf("Build: %s\n", NEST_BUILD_CONFIG);
#endif

#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

	dieid_num_r();

	if (dirty) {
		//saveenv();
	}

#if defined(CONFIG_PREBOOT)
#if defined(CONFIG_DFU)
	sprintf( cmd, "source 0x%08x", AUPD_LOAD_ADDRESS );
#endif
	setenv("preboot", cmd);
#endif


	return 0;
}

#if defined(CONFIG_REVISION_TAG)
/*
 *  u32 get_board_rev()
 *
 *  Description:
 *    This routine returns, if available, the BCD-encoded
 *    board/platform/model revision.
 *
 *    While this routine returns a 32-bit value, the U-Boot and Linux
 *    kernel tag/value pair only allow for 16-bits of data. So, we
 *    encode the model product as the upper (or integer) 8-bits and
 *    the model revision as the lower (or fractional) 8-bits.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    The BCD-encoded board/platform revision number.
 *
 */
u32 get_board_rev(void)
{
	u32 rev = 0;
	int product, revision;

	if (diamond_model && diamond_model->model) {
		product = diamond_model->model->product;
		revision = diamond_model->model->revision;

	} else {
		product = NL_MODEL_UNKNOWN;
		revision = NL_MODEL_UNKNOWN;

	}

	rev = ((product & 0xff) << 8 |
		   (revision & 0xff) << 0);

	return rev;
}
#endif

#if defined(CONFIG_SERIAL_TAG)
/*
 *  void get_board_serial()
 *
 *  Description:
 *    This routine attempts to encode the board serial number,
 *    available from the u-boot 'serial#' key/value pair, into the
 *    64-bit structure provided for passing, in memory, serial numbers
 *    from u-boot to the kernel.
 *
 *    Successful implementation of this relies on the fact that
 *    manufacturing is restricting the serial number format to 15
 *    decimal digits which we represent as a zero-padded,
 *    right-aligned hexadecimal number. Were that not the case, we'd
 *    have to pass the serial number through on the kernel command
 *    line or have a u-boot environment driver in the kernel.
 *
 *    NOTE: For diagnostic build configurations, simply set the serial
 *    number passed to the kernel as all 0s such to facilitate USB
 *    device enumeration and driver installation during manufacturing.
 *
 *  Input(s):
 *    serialnr - A pointer to the structure in which to encode the board
 *               serial number.
 *
 *  Output(s):
 *    serialnr - A pointer to the encoded board serial number.
 *
 *  Returns:
 *    N/A
 *
 */
void get_board_serial(struct tag_serialnr *serialnr)
{
	uint32_t high = 0, low = 0;
#if !defined(NEST_BUILD_CONFIG_DIAGNOSTICS)
	const int base = 16;
	uint64_t value;
	const char *serial;
	char *endp;

	serial = getenv("serial#");

	if (serial) {
		value = simple_strtoull(serial, &endp, base);

		if ((endp > (serial + 1)) && (*endp == '\0')) {
			high = (value >> 32) & 0xFFFFFFFF;
			low  = (value >>  0) & 0xFFFFFFFF;
		}
	}
#endif /* !defined(NEST_BUILD_CONFIG_DIAGNOSTICS) */

	serialnr->high = high;
	serialnr->low  = low;
}
#endif /* defined(CONFIG_SERIAL_TAG) */

#if defined(CONFIG_CMD_NET)
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct gpio *gpio1_base = (struct gpio *)OMAP34XX_GPIO1_BASE;
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[5].config1);
	writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[5].config2);
	writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[5].config3);
	writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[5].config4);
	writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[5].config5);
	writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[5].config6);
	writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[5].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	/* Make GPIO 07 as output pin */
	writel(readl(&gpio1_base->oe) & ~(GPIO7), &gpio1_base->oe);

	/* Now send a pulse on the GPIO pin */
	writel(GPIO7, &gpio1_base->setdataout);
	udelay(1);
	writel(GPIO7, &gpio1_base->cleardataout);
	udelay(1);
	writel(GPIO7, &gpio1_base->setdataout);
}
#endif /* defined(CONFIG_CMD_NET) */

int board_eth_init(bd_t *bis)
{
	int rc = 0;

	if (diamond_model && diamond_model->eth_init) {
		rc = diamond_model->eth_init(bis);
	}

	return rc;
}

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, MMC_MODE_HS_52MHz, 0);
	return 0;
}
#endif

extern int omap_gpmc_nand_init(struct nand_chip *nand);

int board_nand_init(struct nand_chip *nand)
{
	int rc;

	rc = omap_gpmc_nand_init(nand);

	if (rc < 0)
		goto done;

	nand->ecc.mode = NAND_ECC_4BIT_SOFT;

 done:
	return rc;
}

void add_bootarg(const char *name, const char *val)
{
    char buf[256];
    char *cmdline;

    cmdline = getenv (bootargs_key);

    if (cmdline) {
	sprintf(buf, "%s %s=%s",
		cmdline, name, val);
    } else {
	sprintf(buf, "%s=%s",
		name, val);
    }

    setenv (bootargs_key, buf);
}

void add_brightness(void)
{
    const char *brightness_val;

    brightness_val = getenv(brightness_key);
    if (brightness_val != NULL)
    {
	add_bootarg(brightness_key, brightness_val);
    }
}

void add_panic(const char *panic_val)
{
    add_bootarg(panic_key, panic_val);
}

/* 
 * This is where we can do board specific things right before booting
 * the image.
 */
void arch_preboot_os(void)
{
    add_brightness();
#ifdef CONFIG_PANIC
    add_panic(CONFIG_PANIC);
#endif
}
