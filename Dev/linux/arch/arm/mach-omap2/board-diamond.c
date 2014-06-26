/*
 *    Copyright (c) 2010-2012 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file is the core Linux board support file for the Nest
 *      Diamond board. It was originally cloned from the Texas
 *      Instruments OMAP3 EVM board file.
 *
 *      SDRAM Controller (SDRC):
 *        - Samsung K4X51163PI-FCG6 512 Mb x 16 (64 MiB) DDR SDRAM
 *        - Nanya NT6D 512Mb x 16 (64MiB) DDR SDRAM
 *
 *      General Purpose Memory Controller (GPMC):
 *        - Micron MT29F2G16ABDHC-ET 256 MiB SLC NAND Flash (Development)
 *        - Micron MT29F2G16ABBEAH4 256 MiB SLC NAND Flash (Non-development)
 *        - Samsung K9F4G08U0C 512 MiB SLC NAND Flash (Development Only)
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development Only)
 *
 *      Multimedia Card (MMC) Interface:
 *        - Molex 500998-0900 Secure Digital (SD) Connector (Development Only)
 *        - Wireless LAN (WLAN) Module
 *
 *      Display Subsystem (DSS):
 *        - Samsung LMS350DF03 HVGA (320 x 480) LCM (Development and Prototype)
 *        - Tianma TM025ZDZ01 (320 x 320) LCM (Production)
 *
 *      Camera Image Signal Processor:
 *        - Unused
 *
 *      Inter-Integrated Circuit (I2C) Controller:
 *        - Channel 1:
 *          * Texas Instruments TPS65921 Power Management Unit (PMU)
 *
 *        - Channel 2:
 *          * Silicon Labs 1143 Proximity / Ambient Light Sensor (Development Only)
 *          * Avago ADBS A330 Optical Finger Navigation (OFN) Sensor (Non-development)
 *
 *        - Channel 3:
 *          * Unused
 *
 *        - Channel 4:
 *          * Texas Instruments TPS65921 Power Management Unit (PMU)
 *
 *      UART Controller:
 *        - Channel 1:
 *            * Head Unit Serial Console
 *
 *        - Channel 2:
 *            * ZigBee Wireless Module
 *
 *      Multichannel Buffered Serial Port (McBSP):
 *        - Channel 1:
 *            * Unused
 *
 *        - Channel 2:
 *            * Unused
 *
 *        - Channel 3:
 *            * UART2
 *
 *        - Channel 4:
 *            * Unavailable
 *
 *        - Channel 5:
 *            * Unavailable
 *
 *      Multichannel Serial Peripheral Interface (McSPI):
 *        - Channel 1:
 *            * Unused
 *
 *        - Channel 2:
 *            * Samsung LMS350DF03 HVGA (320 x 480) LCM (Development and Prototype)
 *            * Tianma TM025ZDZ01 (320 x 320) LCM (Production)
 *
 *        - Channel 3:
 *            * Unavailable
 *
 *      Interrupt Controller:
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development Only)
 *        - Si1143 Proximity / Ambient Light Sensor (Development Only)
 *
 *      GPIO Controller:
 *        - Bank 1:
 *        - Bank 2:
 *            * 2.11 (43) Samsung LMS350DF03 LCD 1.8V to 3.3V Buffer #Enable (Development Only)
 *
 *        - Bank 3:
 *        - Bank 4:
 *            * 4.5 (101) Samsung LMS350DF03 LCD 3.3V Supply Enable (Development Only)
 *            * 4.13 (109) Samsung  LMS350DF03 LCD #Reset
 *
 *        - Bank 5:
 *        - Bank 6:
 *
 *        - Backplate Detect
 *        - Battery Disconnect
 *        - ZigBee Wireless Module
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/backlight.h>
#include <linux/pwm_backlight.h>
#include <linux/omapfb.h>
#include <linux/si1143.h>
#include <linux/spi/spi.h>
#include <linux/spi/ili9481.h>
#include <linux/spi/lms350df03.h>
#include <linux/spi/s6d05a1.h>
#include <linux/wl12xx.h>
#include <linux/i2c/twl.h>
#include <linux/usb/otg.h>
#include <linux/usb/dynamic.h>
#include <linux/usb/msc.h>
#include <linux/smsc911x.h>
#include <linux/rotary_encoder_lite.h>
#include <linux/rtc.h>
#include <linux/adbs-a330.h>
#include <linux/mmc/host.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/input/pwm-beeper.h>
#include <linux/lm3530_bl.h>

#include <mach/board-diamond-gpio.h>
#include <mach/board-j49-gpio.h>
#include <mach/hardware.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/onenand.h>
#include <plat/gpmc.h>
#include <plat/nand.h>
#include <plat/common.h>
#include <plat/usb.h>
#include <plat/mcspi.h>
#include <plat/clock.h>
#include <plat/omap-pm.h>
#include <plat/display.h>
#include <plat/pwm.h>

#include "mux.h"
#include "hsmmc.h"
#include "sdram-samsung-k4x51163pi-nanya-nt6d.h"
#include "pm.h"
#include "prm-regbits-34xx.h"
#include "board-diamond-battery.h"
#include "board-diamond-backplate.h"
#include "board-diamond-nlmodel.h"
#include "control.h"
#include "board-diamond-zigbee.h"

/* Preprocessor Definitions */

#define diamond_gpio_try_request(gpio, string, action)			\
	if (unlikely(gpio_request(gpio, string))) {					\
		printk(KERN_ERR											\
			   "Failed to request GPIO %u " string "\n", gpio);	\
		action;													\
	}															\

#define	diamond_gpio_try_output(gpio, value, action)			\
	do {														\
		diamond_gpio_try_request(gpio, #gpio, action);			\
		if (gpio_direction_output(gpio, value) < 0) {			\
			printk(KERN_ERR										\
				   "Could not set GPIO %u " #gpio				\
				   " as an output with value %u\n",				\
				   gpio, value);								\
			action;												\
		}														\
	} while (0)

#define	diamond_gpio_try_input(gpio, action)					\
	do {														\
		diamond_gpio_try_request(gpio, #gpio, action);			\
		if (gpio_direction_input(gpio) < 0) {					\
			printk(KERN_ERR										\
				   "Could not set GPIO %u " #gpio				\
				   " as an input\n", gpio);						\
			action;												\
		}														\
	} while (0)

#define machine_warn(format, ...)		pr_warning("Machine: " format, ## __VA_ARGS__)
#define	machine_info(format, ...)		pr_info("Machine: " format, ## __VA_ARGS__)

#define GPMC_CS0_BASE					0x60
#define GPMC_CS_SIZE					0x30

#define DIAMOND_ETHR_START				0x2c000000
#define DIAMOND_ETHR_SIZE				1024
#define DIAMOND_ETHR_ID_REV				0x50
#define DIAMOND_SMSC911X_CS				5

#define DIAMOND_BACKLIGHT_PWM_ID		0
#define DIAMOND_PIEZO_PWM_ID			1

#define	DIAMOND_ADBS_A330_VDDA_SUPPLY	"vdda"
#define DIAMOND_ADBS_A330_VDDA_UV		3000000

#define NEST_USB_VENDOR_STRING			"Nest"

#define DIAMOND_USB_VENDOR_STRING		NEST_USB_VENDOR_STRING
#define DIAMOND_USB_PRODUCT_STRING		"Learning Thermostat"
#define DIAMOND_USB_SERIAL_STRING_SIZE	((sizeof(u64) * 2) + 1)

#define J49_USB_VENDOR_STRING			NEST_USB_VENDOR_STRING
#define J49_USB_PRODUCT_STRING			"Learning Thermostat"
#define J49_USB_SERIAL_STRING_SIZE		((sizeof(u64) * 2) + 1)

#define NEST_USB_VENDOR_ID				0x2464

#define DIAMOND_USB_VENDOR_ID			0x2464
#define DIAMOND_USB_PRODUCT_ID			0x0001

#define J49_USB_VENDOR_ID				0x2464
#define J49_USB_PRODUCT_ID				0x0002

/* NOTE: SCSI Inquiry (which is how USB Mass Storage Class (MSC)
 * vendor and product strings are queried), are limited to 8 and 16
 * ASCII characters, for vendor and product, respectively.
 */
#define	DIAMOND_MSC_VENDOR_STRING		"Nest"
#define DIAMOND_MSC_PRODUCT_STRING		"Nest"
#define	DIAMOND_MSC_RELEASE			1

/* Type Definitions */

enum {
	DIAMOND_SPI_LCD,
	J49_SPI_DEV
};

enum {
	DIAMOND_DEVELOPMENT_DATA	= 0,
	DIAMOND_1_4_DATA			= 1,
	DIAMOND_1_5_DATA			= 2,
	DIAMOND_1_6_DATA			= 3,
	DIAMOND_1_7_DATA			= 4,
	DIAMOND_1_8_DATA			= 5,
	DIAMOND_1_9_DATA			= 6,
	DISPLAY_2_0_DATA			= 7,
	DISPLAY_2_1_DATA			= 8,
	DISPLAY_2_2_DATA			= 9,
	DISPLAY_2_3_DATA			= 10,
	DISPLAY_2_4_DATA			= 11
};

struct diamond_init_data {
	const char *name;
	void (*add_devices)(void);
	void (*backplate_init)(const struct nlmodel *model);
	void (*clk_init)(void);
	void (*display_init)(void);
	void (*flash_init)(void);
	void (*i2c_init)(void);
	void (*mmc_init)(void);
	void (*mux_init)(void);
	void (*net_init)(void);
	void (*piezo_init)(void);
	void (*power_init)(void);
	void (*regulator_init)(void);
	void (*serial_init)(void);
	void (*spi_init)(void);
	void (*usb_init)(void);
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_PERIPHERAL,
	.power			= 50,
};

/* Function Prototypes */

extern void twl4030_poweroff(void);

static int	diamond_development_enable_lcd(struct omap_dss_device *dssdev);
static void	diamond_development_disable_lcd(struct omap_dss_device *dssdev);

static int diamond_backlight_notify(struct device *dev, int brightness);

static void __init diamond_backplate_init(const struct nlmodel *model);
static void __init diamond_clk_init(void);
static void __init diamond_flash_init(void);
static void __init diamond_irq_init(void);
static void __init diamond_serial_init(void);
static void __init diamond_usb_init(void);

static void __init diamond_development_add_devices(void);
static void __init diamond_development_display_init(void);
static void __init diamond_development_i2c_init(void);
static void __init diamond_development_mmc_init(void);
static void __init diamond_development_mux_init(void);
static void __init diamond_development_net_init(void);
static void __init diamond_development_regulator_init(void);
static void __init diamond_development_spi_init(void);

static void __init diamond_1_4_add_devices(void);
static void __init diamond_1_4_display_init(void);
static void __init diamond_1_4_i2c_init(void);
static void __init diamond_1_4_mmc_init(void);
static void __init diamond_1_4_mux_init(void);
static void __init diamond_1_4_net_init(void);
static void __init diamond_1_4_piezo_init(void);
static void __init diamond_1_4_regulator_init(void);
static void __init diamond_1_4_spi_init(void);
static void __init diamond_1_4_power_init(void);

static void __init diamond_1_5_add_devices(void);
static void __init diamond_1_5_display_init(void);
static void __init diamond_1_5_i2c_init(void);
static void __init diamond_1_5_mmc_init(void);
static void __init diamond_1_5_mux_init(void);
static void __init diamond_1_5_net_init(void);
static void __init diamond_1_5_piezo_init(void);
static void __init diamond_1_5_regulator_init(void);
static void __init diamond_1_5_spi_init(void);

static void __init diamond_1_6_add_devices(void);
static void __init diamond_1_6_i2c_init(void);
static void __init diamond_1_6_mux_init(void);
static void __init diamond_1_6_power_init(void);

static void __init diamond_1_7_add_devices(void);
static void __init diamond_1_7_mux_init(void);
static void __init diamond_1_7_spi_init(void);

static void __init display_2_0_add_devices(void);
static void __init display_2_0_i2c_init(void);
static void __init display_2_0_mux_init(void);
static void __init display_2_0_spi_init(void);
static void __init display_2_0_net_init(void);

static void __init display_2_1_mux_init(void);

/* Global Variables */

static const char *diamond_family = "Diamond";
static const int diamond_product_default = 1;
static const int diamond_revision_default = 10;

static const char *j49_family = "Display";
static const int j49_product_default = 2;

static struct diamond_init_data diamond_development_data __initdata = {
	.name				= "Development",
	.add_devices		= diamond_development_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_development_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= diamond_development_i2c_init,
	.mmc_init			= diamond_development_mmc_init,
	.mux_init			= diamond_development_mux_init,
	.net_init			= diamond_development_net_init,
	.piezo_init			= NULL,
	.power_init			= NULL,
	.regulator_init		= diamond_development_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= diamond_development_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data diamond_1_4_data __initdata = {
	.name				= "Prototype",
	.add_devices		= diamond_1_4_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_4_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= diamond_1_4_i2c_init,
	.mmc_init			= diamond_1_4_mmc_init,
	.mux_init			= diamond_1_4_mux_init,
	.net_init			= diamond_1_4_net_init,
	.piezo_init			= diamond_1_4_piezo_init,
	.power_init			= diamond_1_4_power_init,
	.regulator_init		= diamond_1_4_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= diamond_1_4_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data diamond_1_5_data __initdata = {
	.name				= "Prototype",
	.add_devices		= diamond_1_5_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= diamond_1_5_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= diamond_1_5_mux_init,
	.net_init			= diamond_1_5_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_4_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= diamond_1_5_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data diamond_1_6_data __initdata = {
	.name				= "EVT",
	.add_devices		= diamond_1_6_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= diamond_1_6_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= diamond_1_6_mux_init,
	.net_init			= diamond_1_5_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= diamond_1_5_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data diamond_1_7_data __initdata = {
	.name				= "DVT",
	.add_devices		= diamond_1_7_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= diamond_1_6_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= diamond_1_7_mux_init,
	.net_init			= diamond_1_5_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= diamond_1_7_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data display_2_0_data __initdata = {
	.name				= "Prototype",
	.add_devices		= display_2_0_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= display_2_0_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= display_2_0_mux_init,
	.net_init			= display_2_0_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= display_2_0_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data display_2_1_data __initdata = {
	.name				= "EVT",
	.add_devices		= display_2_0_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= display_2_0_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= display_2_1_mux_init,
	.net_init			= display_2_0_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= display_2_0_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data display_2_2_data __initdata = {
	.name				= "DVT",
	.add_devices		= display_2_0_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= display_2_0_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= display_2_1_mux_init,
	.net_init			= display_2_0_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= display_2_0_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data display_2_3_data __initdata = {
	.name				= "Pre-PVT",
	.add_devices		= display_2_0_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= display_2_0_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= display_2_1_mux_init,
	.net_init			= display_2_0_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= display_2_0_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data display_2_4_data __initdata = {
	.name				= "PVT",
	.add_devices		= display_2_0_add_devices,
	.backplate_init		= diamond_backplate_init,
	.clk_init			= diamond_clk_init,
	.display_init		= diamond_1_5_display_init,
	.flash_init			= diamond_flash_init,
	.i2c_init			= display_2_0_i2c_init,
	.mmc_init			= diamond_1_5_mmc_init,
	.mux_init			= display_2_1_mux_init,
	.net_init			= display_2_0_net_init,
	.piezo_init			= diamond_1_5_piezo_init,
	.power_init			= diamond_1_6_power_init,
	.regulator_init		= diamond_1_5_regulator_init,
	.serial_init		= diamond_serial_init,
	.spi_init			= display_2_0_spi_init,
	.usb_init			= diamond_usb_init
};

static struct diamond_init_data *diamond_init_data[] __initdata = {
	[DIAMOND_DEVELOPMENT_DATA]	= &diamond_development_data,
	[DIAMOND_1_4_DATA]			= &diamond_1_4_data,
	[DIAMOND_1_5_DATA]			= &diamond_1_5_data,
	[DIAMOND_1_6_DATA]			= &diamond_1_6_data,
	[DIAMOND_1_7_DATA]			= &diamond_1_7_data,
	[DIAMOND_1_8_DATA]			= &diamond_1_7_data,
	[DIAMOND_1_9_DATA]			= &diamond_1_7_data,
	[DISPLAY_2_0_DATA]			= &display_2_0_data,
	[DISPLAY_2_1_DATA]			= &display_2_1_data,
	[DISPLAY_2_2_DATA]			= &display_2_2_data,
	[DISPLAY_2_3_DATA]			= &display_2_3_data,
	[DISPLAY_2_4_DATA]			= &display_2_4_data
};

static bool lcd_enabled = false;

#if defined(CONFIG_HAVE_PWM)
/* OMAP-specific implementations of generic PWM devices required to
 * support the OMAP PWM-driven backlight and piezo devices.
 */
static struct omap2_pwm_platform_config diamond_backlight_pwm_config = {
	.timer_id			= 10,	// GPT10_PWM_EVT
	.polarity			= 1		// Active-high
};

static struct omap2_pwm_platform_config diamond_piezo_pwm_config = {
	.timer_id			= 11,	// GPT11_PWM_EVT
	.polarity			= 1		// Active-high
};

static struct platform_device diamond_backlight_pwm_device = {
	.name				= "omap-pwm",
	.id					= DIAMOND_BACKLIGHT_PWM_ID,
	.dev				= {
		.platform_data	= &diamond_backlight_pwm_config,
	},
};

static struct platform_device diamond_piezo_pwm_device = {
	.name				= "omap-pwm",
	.id					= DIAMOND_PIEZO_PWM_ID,
	.dev				= {
		.platform_data	= &diamond_piezo_pwm_config,
	},
};
#endif /* defined(CONFIG_HAVE_PWM) */

static struct twl4030_ins  __initdata sleep_on_seq[] = {

	/* Turn OFF VAUX2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VAUX2, RES_STATE_OFF), 2},
	/* Turn off HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_OFF), 2},
	/* Turn OFF VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_OFF), 2},
	/* Turn OFF VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_OFF), 2},
	/* Turn OFF VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_OFF), 2},
};

static struct twl4030_script __initdata sleep_on_script = {
	.script	= sleep_on_seq,
	.size	= ARRAY_SIZE(sleep_on_seq),
	.flags	= TWL4030_SLEEP_SCRIPT,
};

/*
 * This script instructs twl4030 to first enable CLKEN, then wakeup the
 * regulators and then all other resources.
 */
static struct twl4030_ins  __initdata wakeup_seq[] = {
	/* Turn on VAUX2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VAUX2, RES_STATE_ACTIVE), 2},
	/* Turn on HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_ACTIVE), 2},
	/* Turn ON VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_ACTIVE), 2},


};

static struct twl4030_script __initdata wakeup_script = {
	.script = wakeup_seq,
	.size   = ARRAY_SIZE(wakeup_seq),
	.flags  = TWL4030_WAKEUP12_SCRIPT | TWL4030_WAKEUP3_SCRIPT,
};

static struct twl4030_ins  __initdata wrst_seq[] = {
	{MSG_SINGULAR(DEV_GRP_ALL, RES_RESET, RES_STATE_OFF), 0x2},
	{MSG_SINGULAR(DEV_GRP_ALL, RES_Main_Ref, RES_STATE_WRST), 0x2},
	{MSG_BROADCAST(DEV_GRP_ALL, RES_GRP_ALL, 0, 0x2, RES_STATE_WRST), 0x2},
	{MSG_SINGULAR(DEV_GRP_ALL, RES_VUSB_3V1, RES_STATE_WRST), 0x2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VAUX2, RES_STATE_WRST), 0x2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_WRST), 0x2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_WRST), 0x7},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_WRST), 0x25},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_RC, 7, 0, RES_STATE_WRST), 0x2},
	{MSG_SINGULAR(DEV_GRP_ALL, RES_RESET, RES_STATE_ACTIVE), 0x2},
};

static struct twl4030_script  __initdata wrst_script = {
	.script = wrst_seq,
	.size   = ARRAY_SIZE(wrst_seq),
	.flags  = TWL4030_WRST_SCRIPT,
};

static struct twl4030_script __initdata *twl4030_scripts[] = {
	&wakeup_script,
	&sleep_on_script,
	&wrst_script
};

/* 
 * Setting the devgroup enables the regulator.  VAUX2 is the only one 
 * that we're not using the default voltage, so we set its inital group 
 * to NULL.  This way it won't turn on until after we set its voltage 
 * at registration.
 */
static struct twl4030_resconfig  __initdata diamond_twl4030_rconfig[] = {
	{ .resource = RES_VPLL1, .devgroup = DEV_GRP_P1, .type = 3,
		.type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_VINTANA1, .devgroup = DEV_GRP_ALL, .type = 1,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VINTANA2, .devgroup = DEV_GRP_ALL, .type = 0,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VINTDIG, .devgroup = DEV_GRP_ALL, .type = 1,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VIO, .devgroup = DEV_GRP_ALL, .type = 2,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VAUX2, .devgroup = DEV_GRP_NULL,
		.type = 3, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_VDD1, .devgroup = DEV_GRP_P1,
		.type = 4, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_VDD2, .devgroup = DEV_GRP_P1,
		.type = 3, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_REGEN, .devgroup = DEV_GRP_ALL, .type = 2,
		.type2 = 1, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_NRES_PWRON, .devgroup = DEV_GRP_ALL, .type = 0,
		.type2 = 1, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_CLKEN, .devgroup = DEV_GRP_ALL, .type = 3,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_SYSEN, .devgroup = DEV_GRP_ALL, .type = 6,
		.type2 = 1, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3,
		.type = 0, .type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ 0, 0},
};

/*
 * This is identical to that of diamond except for turning off VMMC1
 */
static struct twl4030_resconfig  __initdata j49_twl4030_rconfig[] = {
	{ .resource = RES_VPLL1, .devgroup = DEV_GRP_P1, .type = 3,
		.type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_VINTANA1, .devgroup = DEV_GRP_ALL, .type = 1,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VINTANA2, .devgroup = DEV_GRP_ALL, .type = 0,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VINTDIG, .devgroup = DEV_GRP_ALL, .type = 1,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VIO, .devgroup = DEV_GRP_ALL, .type = 2,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VAUX2, .devgroup = DEV_GRP_NULL,
		.type = 3, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_VDD1, .devgroup = DEV_GRP_P1,
		.type = 4, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_VDD2, .devgroup = DEV_GRP_P1,
		.type = 3, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ .resource = RES_REGEN, .devgroup = DEV_GRP_ALL, .type = 2,
		.type2 = 1, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_NRES_PWRON, .devgroup = DEV_GRP_ALL, .type = 0,
		.type2 = 1, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_CLKEN, .devgroup = DEV_GRP_ALL, .type = 3,
		.type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_SYSEN, .devgroup = DEV_GRP_ALL, .type = 6,
		.type2 = 1, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3,
		.type = 0, .type2 = 2, .remap_sleep = RES_STATE_SLEEP },
	{ .resource = RES_VMMC1, .devgroup = DEV_GRP_NULL,
		.type = 2, .type2 = 1, .remap_sleep = RES_STATE_OFF },
	{ 0, 0},
};

static struct twl4030_power_data __initdata diamond_tps65921_scripts_data = {
	.scripts        = twl4030_scripts,
	.num          = ARRAY_SIZE(twl4030_scripts),
	.resource_config = diamond_twl4030_rconfig,
};

static struct twl4030_power_data __initdata j49_tps65921_scripts_data = {
	.scripts        = twl4030_scripts,
	.num          = ARRAY_SIZE(twl4030_scripts),
	.resource_config = j49_twl4030_rconfig,
};

/*
 * XXX - Diamond does not actually have or use any OneNAND. However,
 * the way OMAP board support is written in this kernel, removing any
 * data and code associated with OneNAND from our board results in a
 * kernel panic. So, we leave this as-is for now until we can debug
 * the root cause.
 */
static struct mtd_partition diamond_onenand_partitions[] = {
	{
		.name           = "xloader-onenand",
		.offset         = 0,
		.size           = 4*(64*2048),
		.mask_flags     = MTD_WRITEABLE
	},
	{
		.name           = "uboot-onenand",
		.offset         = MTDPART_OFS_APPEND,
		.size			= 15*(64*2048),
		.mask_flags		= MTD_WRITEABLE
	},
	{
		.name           = "params-onenand",
		.offset         = MTDPART_OFS_APPEND,
		.size			= 1*(64*2048),
	},
	{
		.name           = "linux-onenand",
		.offset         = MTDPART_OFS_APPEND,
		.size			= 40*(64*2048),
	},
	{
		.name           = "jffs2-onenand",
		.offset         = MTDPART_OFS_APPEND,
		.size			= MTDPART_SIZ_FULL,
	},
};

static struct omap_onenand_platform_data diamond_onenand_data = {
	.parts = diamond_onenand_partitions,
	.nr_parts = ARRAY_SIZE(diamond_onenand_partitions),
	.dma_channel	= -1,	/* disable DMA in OMAP OneNAND driver */
};

/*
 * Diamond NAND flash parition sizes.
 *
 * For "raw" partitions, any size evenly divisible by the NAND erase
 * block size (generally 128 KiB) is acceptable.
 *
 * For JFFS2 file system partitions, there's quite a bit of metadata
 * overhead with JFFS2, so any size smaller than 4 MiB is practically
 * useless as overhead at that size is just about less than 50%.
 */

#define DIAMOND_IPL_RAW_NAND_SIZE			( 256 << 10)	// 256 KiB
#define DIAMOND_SPL_RAW_NAND_SIZE			(1536 << 10)	// 1.5 MiB (x2)
#define	DIAMOND_ENV_RAW_NAND_SIZE			( 384 << 10)	// 384 KiB (x2)
#define DIAMOND_BOOT_RAW_NAND_SIZE			(   8 << 20)	//   8 MiB (x2)
#define DIAMOND_ROOT_FS_NAND_SIZE			(  46 << 20)	//  46 MiB (x2)
#define DIAMOND_SYSTEM_CONFIG_FS_NAND_SIZE	(   4 << 20)	//   4 MiB
#define DIAMOND_USER_CONFIG_FS_NAND_SIZE	(   8 << 20)	//   8 MiB
#define DIAMOND_DATA_FS_NAND_SIZE			(  20 << 20)	//  20 MiB
#define DIAMOND_LOG_FS_NAND_SIZE			(  20 << 20)	//  20 MiB
#define DIAMOND_SCRATCH_FS_NAND_SIZE		(  92 << 20)	//  92 MiB

static struct mtd_partition diamond_nand_partitions[] = {
	// First, one partition for the entire device.
	{
		.name			= "nand0",
		.offset			= 0,
		.size			= MTDPART_SIZ_FULL
	},
	// Then, partitions by functional use overlaid on the whole device.
	//
	// Initial- and secondary-program loader-related partitions. These
	// are generally expected to stay constant with increasing NAND
	// density.
	{
		.name           = "ipl",
		.offset         = 0,
		.size           = DIAMOND_IPL_RAW_NAND_SIZE,
		.mask_flags     = MTD_WRITEABLE		// Read-only
	},
	{
		.name           = "spl0",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_SPL_RAW_NAND_SIZE
	},
	{
		.name           = "spl1",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_SPL_RAW_NAND_SIZE
	},
	{
		.name           = "env0",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_ENV_RAW_NAND_SIZE
	},
	{
		.name           = "env1",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_ENV_RAW_NAND_SIZE
	},
	// Kernel- and file system-related partitions. These are generally
	// expected to grow with increasing NAND density.
	{
		.name           = "boot0",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_BOOT_RAW_NAND_SIZE
	},
	{
		.name           = "root0",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_ROOT_FS_NAND_SIZE
	},
	{
		.name           = "boot1",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_BOOT_RAW_NAND_SIZE
	},
	{
		.name           = "root1",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_ROOT_FS_NAND_SIZE
	},
	{
		.name           = "system-config",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_SYSTEM_CONFIG_FS_NAND_SIZE
	},
	{
		.name           = "user-config",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_USER_CONFIG_FS_NAND_SIZE
	},
	{
		.name           = "data",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_DATA_FS_NAND_SIZE
	},
	{
		.name           = "log",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_LOG_FS_NAND_SIZE
	},
	{
		.name           = "scratch",
		.offset         = MTDPART_OFS_APPEND,
		.size           = DIAMOND_SCRATCH_FS_NAND_SIZE
	}
};

static struct omap_nand_platform_data diamond_nand_data = {
	.parts          = diamond_nand_partitions,
	.nr_parts       = ARRAY_SIZE(diamond_nand_partitions),
	.nand_setup     = NULL,
	.dma_channel    = -1,           /* disable DMA in OMAP NAND driver */
	.dev_ready      = NULL,
};

/*
 * The TI OMAP frame buffer (OMAPFB) driver has a number of different
 * ways of allocating frame buffer memory to its various frame buffer
 * devices. Unfortunately, they are not fully unified with the generic
 * Linux frame buffer driver.
 *
 * Regardless, one such method is specifiying the parameters on the
 * kernel command line. For example, the following would instruct the
 * generic linux frame buffer driver to allocate 4 MiB of memory and
 * to then allocate all of it to the OMAP frame buffer zero (0),
 * typically the LCD:
 *
 *   vram=4M omapfb.vram=0:4M
 *
 * Another method is to programmatically use platform data. Because we
 * do not need any dynamism in this area and because we elect to
 * minimize futzing with kernel command line parameters, we take the
 * platform data approach.
 *
 * Because the frame buffer is a virtual entity, all the parameters
 * specified here are in terms of virtual sizes and depths and may not
 * actually reflect the capabilities of the LCD itself.
 *
 * The parameters below basically define a double-buffered 320 x 320
 * (or 480) frame at 4 bytes per pixel.
 */

#define DIAMOND_VFB_BPP		4
#define	DIAMOND_VFB_COUNT	2
#define DIAMOND_VFB_ROUND	(512 << 10)

static struct omapfb_platform_data diamond_320_320_omapfb_config = {
	.mem_desc 				= {
		.region_cnt			= 1,
		.region[0] 			= {
			 .paddr			= 0,
			 .size 			= diamond_roundup(320 * 320 *
									  DIAMOND_VFB_BPP *
									  DIAMOND_VFB_COUNT,
									  DIAMOND_VFB_ROUND)
		 }
	}
};

static struct omapfb_platform_data diamond_320_480_omapfb_config = {
	.mem_desc 				= {
		.region_cnt			= 1,
		.region[0] 			= {
			 .paddr			= 0,
			 .size 			= diamond_roundup(320 * 480 *
									  DIAMOND_VFB_BPP *
									  DIAMOND_VFB_COUNT,
									  DIAMOND_VFB_ROUND)
		 }
	}
};

#if defined(CONFIG_PANEL_SAMSUNG_LMS350DF03)
static struct lms350df03_platform_data lms350df03_development_lcd_config = {
	.reset = {
		.gpio				= DIAMOND_GPIO_LCD_RESETB,
		.inverted			= false
	},
	.regulator = {
		 .vcc				= NULL,
	},
};

static struct lms350df03_platform_data lms350df03_prototype_lcd_config = {
	.reset = {
		.gpio				= DIAMOND_GPIO_LCD_RESETB,
		.inverted			= false
	},
	.regulator = {
		 .vcc				= "lcd2v8",
	},
};

#define LMS350DF03_LCD_MODALIAS			"lms350df03"
#define LMS350DF03_LCD_SPI_MAX_SPEED	10000000
#endif /* defined(CONFIG_PANEL_SAMSUNG_LMS350DF03) */

#if defined(CONFIG_PANEL_GIANTPLUS_GPM1145A0)
static struct ili9481_platform_data ili9481_lcd_config = {
	.reset = {
		.gpio				= DIAMOND_GPIO_LCD_RESETB,
		.inverted			= false
	}
};

#define ILI9481_LCD_MODALIAS			"ili9481"
#define ILI9481_LCD_SPI_MAX_SPEED		8000000
#endif /* defined(CONFIG_PANEL_GIANTPLUS_GPM1145A0) */

#if defined(CONFIG_PANEL_TIANMA_TM025ZDZ01)
static struct s6d05a1_platform_data s6d05a1_lcd_config = {
	.reset = {
		.gpio				= DIAMOND_GPIO_LCD_RESETB,
		.inverted			= false
	},

    .lcd_id = {
    	.gpio               = DIAMOND_GPIO_LCD_ID,
    },

	.regulator = {
		 .vcc				= "lcd2v8",
	},

	// The Tianma display is mounted and oriented such that its
	// natural position is upside down. So, set x- and y-mirroring to
	// effect a 180 degree rotation.

	.orientation = {
		 .x_mirror			= true,
		 .y_mirror			= true,
		 .x_y_exchange		= false
	}
};

#define TM025ZDZ01_LCD_MODALIAS			"s6d05a1"
#define TM025ZDZ01_LCD_SPI_MAX_SPEED	10000000
#endif /* defined(CONFIG_PANEL_TIANMA_TM025ZDZ01) */

#define SPI_DEV_MODALIAS	"spidev"
#define SPI_DEV_MAX_SPEED	1000000

static struct omap2_mcspi_device_config lcd_mcspi_config = {
	.turbo_mode				= false,
	.single_channel			= 0
};

static struct spi_board_info diamond_development_spi_board_info[] = {
	[DIAMOND_SPI_LCD] = {
#if defined(CONFIG_PANEL_SAMSUNG_LMS350DF03)
		.modalias			= LMS350DF03_LCD_MODALIAS,
		.bus_num			= 2,
		.chip_select		= 0,
		.max_speed_hz		= LMS350DF03_LCD_SPI_MAX_SPEED,
		.controller_data	= &lcd_mcspi_config,
		.platform_data		= &lms350df03_development_lcd_config
#endif
	}
};

static struct spi_board_info diamond_1_4_spi_board_info[] = {
	[DIAMOND_SPI_LCD] = {
#if defined(CONFIG_PANEL_SAMSUNG_LMS350DF03)
		.modalias			= LMS350DF03_LCD_MODALIAS,
		.bus_num			= 2,
		.chip_select		= 0,
		.max_speed_hz		= LMS350DF03_LCD_SPI_MAX_SPEED,
		.controller_data	= &lcd_mcspi_config,
		.platform_data		= &lms350df03_prototype_lcd_config
#endif
	}
};

/*
 * Starting with EVT revisions, displays transition for form-factor
 * Tianma TM025ZDZ01 displays.
 */
static struct spi_board_info diamond_1_5_spi_board_info[] = {
	[DIAMOND_SPI_LCD] = {
		.modalias			= TM025ZDZ01_LCD_MODALIAS,
		.bus_num			= 2,
		.chip_select		= 0,
		.max_speed_hz		= TM025ZDZ01_LCD_SPI_MAX_SPEED,
		.controller_data	= &lcd_mcspi_config,
		.platform_data		= &s6d05a1_lcd_config
	}
};

/*
 * Starting with DVT revisions, displays move to McSPI1 from McSPI2
 * to accomodate bi-directional SPI communication.
 */
static struct spi_board_info diamond_1_7_spi_board_info[] = {
	[DIAMOND_SPI_LCD] = {
		.modalias			= TM025ZDZ01_LCD_MODALIAS,
		.bus_num			= 1,
		.chip_select		= 0,
		.max_speed_hz		= TM025ZDZ01_LCD_SPI_MAX_SPEED,
		.controller_data	= &lcd_mcspi_config,
		.platform_data		= &s6d05a1_lcd_config
	}
};

/*
 * Starting with DVT revisions, displays move to McSPI1 from McSPI2
 * to accomodate bi-directional SPI communication.
 */
static struct spi_board_info display_2_0_spi_board_info[] = {
	[DIAMOND_SPI_LCD] = {
		.modalias			= TM025ZDZ01_LCD_MODALIAS,
		.bus_num			= 1,
		.chip_select		= 0,
		.max_speed_hz		= TM025ZDZ01_LCD_SPI_MAX_SPEED,
		.controller_data	= &lcd_mcspi_config,
		.platform_data		= &s6d05a1_lcd_config
	},

	[J49_SPI_DEV] = {
		.modalias			= SPI_DEV_MODALIAS,
		.bus_num			= 2,
		.chip_select		= 0,
		.max_speed_hz		= SPI_DEV_MAX_SPEED,
	}
};

static struct omap_dss_device diamond_development_lcd_device = {
	.name								= "Samsung LMS350DF03",
	.driver_name						= "samsung_lms350df03",
	.phy.dpi.data_lines					= 24,
	.type								= OMAP_DISPLAY_TYPE_DPI,
	.platform_enable					= diamond_development_enable_lcd,
	.platform_disable					= diamond_development_disable_lcd,
};

static struct omap_dss_device diamond_1_4_lcd_device = {
	.name								= "Samsung LMS350DF03",
	.driver_name						= "samsung_lms350df03",
	.phy.dpi.data_lines					= 24,
	.type								= OMAP_DISPLAY_TYPE_DPI,
	.platform_enable					= NULL,
	.platform_disable					= NULL,
};

static struct omap_dss_device diamond_1_5_lcd_device = {
	.name								= "Tianma TM025ZDZ01",
	.driver_name						= "tianma_tm025zdz01",
	.phy.dpi.data_lines					= 24,
	.type								= OMAP_DISPLAY_TYPE_DPI,
	.platform_enable					= NULL,
	.platform_disable					= NULL,
};

/*
 * Both the devices array and the device data will get fixed up in the
 * model-specific display initialization function.
 */
static struct omap_dss_device *diamond_dss_devices[] = {
	NULL,
};

static struct omap_dss_board_info diamond_dss_data = {
	.num_devices						= ARRAY_SIZE(diamond_dss_devices),
	.devices							= diamond_dss_devices,
	.default_device						= NULL,
};

static struct platform_device diamond_dss_device = {
	.name								= "omapdss",
	.id									= -1,
	.dev								= {
		.platform_data = &diamond_dss_data
	},
};

#if defined(CONFIG_WILINK) || defined(CONFIG_WILINK_MODULE)
static struct platform_device diamond_wl1271_device = {
	.name								= "tiwlan_pm_driver",
	.id									= -1,
	.dev								= {
		.platform_data						= NULL,
	},
};

static struct platform_device diamond_wl127x_device = {
	.id									= 1,
	.dev								= {
		.bus							= &platform_bus_type,
	},
};
#endif /* defined(CONFIG_WILINK) || defined(CONFIG_WILINK_MODULE) */

#if defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE)
static struct wl12xx_platform_data diamond_wl12xx_data __initdata = {
	.irq = -1,
	.use_eeprom = false,
	.pwr_in_suspend = true,
};
#endif /* defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE) */

/*
 * Generic PWM backlight platform data and device data
 *
 * The TPS61042 constant current LED drivers which ultimately drives
 * the backlight can handle, per its datasheet "TPS61042: Constant
 * Current LED Driver", a PWM input in the frequency range 100 Hz to
 * 50 KHz (period 10 ms to 20 us).
 *
 * Per "Control (CTRL)", Page 10 of the datasheet, to enable the
 * device, the TPS61042 CTRL signal must be high for 50 us (50000
 * ns). The PWM signal can then be applied with a pulse width (tp)
 * greater or smaller than tON. To force the device into shutdown
 * mode, the CTRL signal must be low for at least 32 ms (32000000 ns).
 *
 * However, also per "Control (CTRL)", Page 10 of the datasheet, to
 * enable the device when the pulse width <= 2.5 us, run with a pulse
 * > 2.5 us for 512 PWM cycles. Experimentally, this method does not
 * work and, in fact, requires a pulse of 8.1 us, which causes a
 * bright flash with white pixels.
 */

#define TPS61042_DISABLE_NS_MIN				32000000
#define TPS61042_ENABLE_NS_MIN				  100000
#define	TPS61042_PULSE_NS_MIN				     400

#define	PWM_BL_PERIOD_NS_100_HZ				10000000
#define PWM_BL_PERIOD_NS_50_KHZ				   20000

#define TPS61042_PWM_PERIOD_NS_MIN			PWM_BL_PERIOD_NS_50_KHZ
#define TPS61042_PWM_PERIOD_NS_MAX			PWM_BL_PERIOD_NS_100_HZ

/* Starting at pulse width smaller than this value requires a soft
 * start.  Datasheet says 2500ns, feedback from TI is to double that
 */
#define TPS61042_START_PULSE_NS_MIN				5000
/* Measured minimum soft start time is 210ms, feedback from TI is 
 * to add 50%.  This value should be independent of soft start
 * frequency, but based on my testing that is false.
 */   
#define TPS61042_START_MS_MIN                    315
/* Soft start algorithm from TI is to use 100Hz, 0.5% duty cycle. If
 * increase from 100Hz, must also increase the duty cycle.
 */
#define TPS61042_START_PWM_PERIOD_NS        PWM_BL_PERIOD_NS_100_HZ
#define TPS61042_START_PULSE_NS             ((TPS61042_START_PWM_PERIOD_NS/1000)*5) 

/* The following backight PWM periods are a sampling of periods that
 * have been experimentally tried. Those in or just in the super-audio
 * frequency range (>= 20 KHz) avoid audible "whine" but also meet the
 * TPS61042 pulse width minimum at the number of brightness steps
 * chosen (100).
 */
#define PWM_BL_PERIOD_NS_1_0_KHZ             1000000
#define PWM_BL_PERIOD_NS_2_0_KHZ              500000
#define PWM_BL_PERIOD_NS_3_2_KHZ              312500
#define PWM_BL_PERIOD_NS_4_0_KHZ			  250000
#define PWM_BL_PERIOD_NS_7_4_KHZ			  135000
#define PWM_BL_PERIOD_NS_7_7_KHZ			  130000
#define PWM_BL_PERIOD_NS_8_3_KHZ			  120000
#define PWM_BL_PERIOD_NS_10_0_KHZ			  100000
#define	PWM_BL_PERIOD_NS_20_0_KHZ			   50000
#define PWM_BL_PERIOD_NS_20_5_KHZ			   48780
#define	PWM_BL_PERIOD_NS_21_0_KHZ			   47619
#define	PWM_BL_PERIOD_NS_22_0_KHZ			   45455
#define PWM_BL_PERIOD_NS_22_2_KHZ			   45000
#define PWM_BL_PERIOD_NS_24_0_KHZ			   41667
#define PWM_BL_PERIOD_NS_25_0_KHZ			   40000


static struct platform_pwm_backlight_data diamond_tps61042_backlight_data = {
	// Generic PWM timer ID to request
	.pwm_id			 		= DIAMOND_BACKLIGHT_PWM_ID,

	// The maximum and default brightness values, without units. These
	// effectively provide the number of possible backlight brightness
	// steps.
	.max_brightness			= 100,
	.dft_brightness			= 50,
	
	// The PWM period, in nanoseconds.
	.pwm_period_ns			= PWM_BL_PERIOD_NS_2_0_KHZ,

	// The TPS61042 LED driver-specific parameters. Based on our
	// frequency and steps, we have a pulse of 450 ns at 1% which
	// means we have to specify not only the enable minimum but also
	// an initial start pulse train.

	.driver					= {
		.enable_ns			= TPS61042_ENABLE_NS_MIN,
		.pulse_min_ns		= TPS61042_PULSE_NS_MIN,
        .start_pulse_min_ns = TPS61042_START_PULSE_NS_MIN,
		.start_pulse_ns		= TPS61042_START_PULSE_NS,
		.start_ms		    = TPS61042_START_MS_MIN,
		.start_period_ns	= TPS61042_START_PWM_PERIOD_NS
	},

	// At this point, we don't have an eye response ramp.
	.ramp					= NULL,

	// Board-specific callback when the backlight changes.
	.notify					= diamond_backlight_notify,
};


static struct platform_device diamond_tps61042_backlight_device = {
	.name				= "pwm-backlight",
	.dev				= {
		.platform_data	= &diamond_tps61042_backlight_data,
	},
};


static struct lm3530_platform_data j49_lm3530_backlight_data = {
	.mode = LM3530_BL_MODE_MANUAL,
	.als_input_mode = LM3530_INPUT_ALS1,
	.max_current = LM3530_FS_CURR_29mA,
	.pwm_pol_hi = true,
	.als_avrg_time = LM3530_ALS_AVRG_TIME_512ms,
	.brt_ramp_law = LM3530_RAMP_LAW_EXP,
	.brt_ramp_fall = LM3530_RAMP_TIME_2s,
	.brt_ramp_rise = LM3530_RAMP_TIME_2s,
	.als1_resistor_sel = LM3530_ALS_IMPD_13_53kOhm,
	.als2_resistor_sel = LM3530_ALS_IMPD_Z,
	.als_vmin = 730, /* mV */
	.als_vmax = 1020, /* mV */
	.max_brightness			= 0x78,
	.dft_brightness			= 0x6c,
	.enable_gpio = J49_GPIO_LCD_BL_HWEN,
	.wait_for_framebuffer = "omapfb"
};

static struct platform_device j49_lm3530_backlight_device = {
	.name				= "lm3530-backlight",
	.dev				= {
	},
};

static struct i2c_board_info __initdata j49_backlight_i2c_info[] = {
    {
        I2C_BOARD_INFO("lm3530-backlight", 0x36),
        .platform_data = &j49_lm3530_backlight_data,
    },
};

#if defined(CONFIG_INPUT_GPIO_ROTARY_ENCODER_LITE)
static struct rotary_encoder_lite_platform_data rotary_enc = {
    .gpio_a             = DIAMOND_GPIO_ROTARY_VALID,
    .gpio_b             = DIAMOND_GPIO_ROTARY_DIR,
    .gpio_clear         = DIAMOND_GPIO_ROTARY_CLEAR,
    .steps              = 360,
    .rollover           = 1
};

static struct platform_device diamond_rotary_device = {
	.name				= "rotary-encoder-lite",
	.dev				= {
		.platform_data	= &rotary_enc,
	},
};
#endif /* defined(CONFIG_INPUT_GPIO_ROTARY_ENCODER_LITE) */

static int diamond_prototype_piezo_init(struct device *dev)
{
	const unsigned int enabled = 0;

	diamond_gpio_try_output(DIAMOND_GPIO_PIEZO_NENABLE,
							!enabled,
							goto fail);

	return 0;

 fail:
	return -EBUSY;
}

static void diamond_prototype_piezo_exit(struct device *dev)
{
	gpio_free(DIAMOND_GPIO_PIEZO_NENABLE);
}

static void diamond_prototype_piezo_notify(struct device *dev, int hz)
{
	const unsigned int enabled = 0;

	gpio_set_value(DIAMOND_GPIO_PIEZO_NENABLE, hz ? enabled : !enabled);
}

static struct platform_pwm_beeper_data diamond_pwm_beeper_data = {
	.pwm_id				= DIAMOND_PIEZO_PWM_ID,
	.init				= NULL,
	.exit				= NULL,
	.notify				= NULL,
};

static struct platform_device diamond_piezo_device = {
	.name				= "pwm-beeper",
	.id					= -1,
	.dev				= {
		.platform_data	= &diamond_pwm_beeper_data,
	},
};

static struct platform_device diamond_backplate_device = {
	.name				= "diamond-backplate",
	.id					= -1,
};

static struct platform_device diamond_battery_device = {
	.name				= "diamond-battery",
	.id					= -1,
};

static struct diamond_zigbee_platform_data diamond_zigbee_data = {
         .reset_gpio                    = DIAMOND_GPIO_ZIGBEE_RESET_L,
         .pwr_enable_gpio               = DIAMOND_GPIO_ZIGBEE_PWR_ENABLE,
         .interrupt_gpio                = 0
};

static struct platform_device diamond_zigbee_device = {
	.name				= "diamond-zigbee",
	.id					= -1,
	.dev				= {
		.platform_data = &diamond_zigbee_data,
	},
};

static struct diamond_zigbee_platform_data j49_zigbee_data = {
	.reset_gpio							= DIAMOND_GPIO_ZIGBEE_RESET_L,
	.pwr_enable_gpio					= 0,
    .interrupt_gpio                     = J49_GPIO_ZIGBEE_INTERRUPT_L,
};

static struct platform_device j49_zigbee_device = {
	.name				= "diamond-zigbee",
	.id					= -1,
	.dev				= {
		.platform_data = &j49_zigbee_data,
	},
};

#if defined(CONFIG_SENSORS_TWL4030_MADC)
static struct platform_device diamond_madc_hwmon_device = {
	.name               = "twl4030_madc_hwmon"
};
#endif

#define ONENAND_MAP	0x20000000 /* OneNand flash */
//Function taken from board_nand_init in board-flash.h
void __init diamond_nand_init(void)
{
	diamond_nand_data.cs		= 0;
	diamond_nand_data.parts		= diamond_nand_partitions;
	diamond_nand_data.nr_parts	= ARRAY_SIZE(diamond_nand_partitions);
	diamond_nand_data.devsize		= NAND_BUSWIDTH_16;
	diamond_nand_data.gpmc_irq        = OMAP_GPMC_IRQ_BASE;
	diamond_nand_data.ecc_opt         = OMAP_ECC_BCH4_CODE_HW;
        
	if (gpmc_nand_init(&diamond_nand_data) < 0)
	  printk(KERN_ERR "Unable to register NAND device\n");
}

static void __init diamond_onenand_init(void)
{
	gpmc_onenand_init(&diamond_onenand_data);
}

static void __init diamond_flash_init(void)
{
	diamond_nand_init();
	diamond_onenand_init();
}

#if (defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE))
static struct resource diamond_smsc911x_resources[] = {
	[0] =	{
		.start	= DIAMOND_ETHR_START,
		.end	= (DIAMOND_ETHR_START + DIAMOND_ETHR_SIZE - 1),
		.flags	= IORESOURCE_MEM,
	},
	[1] =	{
		.start	= OMAP_GPIO_IRQ(DIAMOND_GPIO_ENET_IRQ),
		.end	= OMAP_GPIO_IRQ(DIAMOND_GPIO_ENET_IRQ),
		.flags	= (IORESOURCE_IRQ | IRQF_TRIGGER_LOW),
	},
};

static struct smsc911x_platform_config smsc911x_config = {
	.phy_interface  = PHY_INTERFACE_MODE_MII,
	.irq_polarity   = SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type       = SMSC911X_IRQ_TYPE_OPEN_DRAIN,
	.flags          = (SMSC911X_USE_32BIT | SMSC911X_SAVE_MAC_ADDRESS),
};

static struct platform_device diamond_smsc911x_device = {
	.name		= "smsc911x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(diamond_smsc911x_resources),
	.resource	= &diamond_smsc911x_resources[0],
	.dev		= {
		.platform_data = &smsc911x_config,
	},
};

static inline void __init diamond_smsc911x_init(void)
{
	diamond_gpio_try_input(DIAMOND_GPIO_ENET_IRQ, goto done);

	platform_device_register(&diamond_smsc911x_device);

 done:
	return;
}
#endif /* (defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)) */

static void __init diamond_wlan_init(void)
{
#if defined(CONFIG_WILINK) || defined(CONFIG_WILINK_MODULE)
	/* Create proxy platform device and alias for MMC2 clock,
	 * required for WiLink driver.
	 */

	dev_set_name(&diamond_wl127x_device.dev, "mmci-omap-hs.%d", 1);
	clk_add_alias("ick", "tiwlan_pm_driver", "ick", &diamond_wl127x_device.dev);
	clk_add_alias("fck", "tiwlan_pm_driver", "fck", &diamond_wl127x_device.dev);
#endif
}

static void __init diamond_display_gpio_init(void)
{
	// Attempt to request GPIO outputs for the platform-specific
	// aspects of the LCD control interface, including the 3.3V LCM
	// Vdd enable (active high) and 1.8V to 3.3V buffer enable (active
	// low).

	diamond_gpio_try_output(DIAMOND_GPIO_LCD_NENABLE_BUFFER,
							1,
							goto done);

	diamond_gpio_try_output(DIAMOND_GPIO_LCD_ENABLE_VDD,
							0,
							goto err_1);

	// If we are here, we successfully requested all LCM GPIOs.

	goto done;

	// One of the GPIO requests failed, bail out and release any
	// requested GPIOs.

err_1:
	gpio_free(DIAMOND_GPIO_LCD_NENABLE_BUFFER);

 done:
	return;
}

static int diamond_development_enable_lcd(struct omap_dss_device *dssdev)
{
	int status = 0;

	if (lcd_enabled) {
		return (status);
	}

	// First, enable (active high) the 3.3V supply and wait a few
	// milliseconds.

	gpio_set_value(DIAMOND_GPIO_LCD_ENABLE_VDD, 1);

	mdelay(30);

	// Second, enable (active low) the 1.8V to 3.3V signal buffer and
	// wait a few milliseconds.

	gpio_set_value(DIAMOND_GPIO_LCD_NENABLE_BUFFER, 0);

	mdelay(30);

	lcd_enabled = true;

	return (status);
}

static void diamond_development_disable_lcd(struct omap_dss_device *dssdev)
{
	if (!lcd_enabled) {
		goto done;
	}

	// Second, disable (active low) the 1.8V to 3.3V signal buffer.

	gpio_set_value(DIAMOND_GPIO_LCD_NENABLE_BUFFER, 1);

	// Finally, disable (active high) the 3.3V supply.

	gpio_set_value(DIAMOND_GPIO_LCD_ENABLE_VDD, 0);

	lcd_enabled = false;

 done:
	return;
}

static void diamond_1_6_power_off(void)
{
	diamond_battery_disconnect(true);
}

static void diamond_1_6_halt_system(void)
{
    // reset the backplate
    diamond_backplate_reset();
    // power off
    twl4030_poweroff();
}

/**
 * diamond_backlight_notify - backlight change notification callback
 * @brightness: the brightness value the backlight PWM will be changed to.
 *
 * This routine is called whenever a backlight value change
 * occurs. Nominally, this routine simply returns the changed
 * backlight value, unmodified.
 *
 * However, in the future, this routine could be used to linearize the
 * backlight response in a platform-specific way or to quantize the
 * allowed backlight levels.
 *
 * Returns the backlight PWM value to change to.
 */
static int diamond_backlight_notify(struct device *dev, int brightness)
{
	return brightness;
}

static struct regulator_consumer_supply diamond_development_vaux2_supplies[] = {
	REGULATOR_SUPPLY("hsusb1", NULL),
};

/*
 * This will get fixed up in the model-specific regulator initialization
 * function.
 */
static struct regulator_consumer_supply diamond_prototype_vaux2_supplies[] = {
	REGULATOR_SUPPLY("hsusb1", NULL),
	REGULATOR_SUPPLY("lcd2v8", NULL),
};

static struct regulator_init_data diamond_vaux2 = {
	.constraints = {
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= true,
        .boot_on    = true,
        .initial_mode   = REGULATOR_MODE_NORMAL,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = 0,
	.consumer_supplies = NULL,
};

/*
 * We only define one MMC channel even though two are supported and used.
 * The TI WiLink driver currently in use has it's own SDIO driver which
 * plays outside the normal Linux driver space. Consequently, we cannot
 * normally define the second MMC channel and have the WiLink driver bind
 * to it.
 */
static struct omap2_hsmmc_info diamond_development_tps65921_mmc[] = {
#if defined(CONFIG_MMC_BLOCK)
	{
		.mmc			= 1,
		.caps           = MMC_CAP_4_BIT_DATA,
		.gpio_cd		= -EINVAL,
		.gpio_wp		= 63,
		.cover_only		= true,
		.ocr_mask		= 0,
	},
#endif /* defined(CONFIG_MMC_BLOCK) */
#if !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE)
	{
		.name		= "wl1271",
		.mmc		= 2,
		.caps           = MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD
				| MMC_PM_KEEP_POWER,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.cover_only	= false,
		.ext_clock	= false,
		.nonremovable	= true,
                .power_saving   = false,
		.transceiver	= false,
		.ocr_mask		= 0,
	},
#endif /* !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE) */
	{}	/* Terminator */
};

static struct omap2_hsmmc_info diamond_prototype_tps65921_mmc[] = {
#if !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE)
	{
		.name		= "wl1271",
		.mmc			= 2,
		.caps           = MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD
				| MMC_PM_KEEP_POWER,
		.gpio_cd		= -EINVAL,
		.gpio_wp		= -EINVAL,
		.cover_only		= false,
		.ext_clock		= false,
		.nonremovable	= true,
                .power_saving   = false,
		.transceiver	= false,
		.ocr_mask		= 0,
	},
#endif /* !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE) */
	{}	/* Terminator */
};

static struct twl4030_gpio_platform_data diamond_tps65921_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
};

static struct twl4030_usb_data diamond_tps65921_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
};

#if defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
static struct ohci_hcd_omap_platform_data diamond_ohci_pdata __initdata = {

	.port_mode[0] = OMAP_OHCI_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_OHCI_PORT_MODE_PHY_4PIN_DPDM,
	.port_mode[2] = OMAP_OHCI_PORT_MODE_UNUSED,
};
#endif // defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)

#if defined(CONFIG_USB_DYNAMIC)
static const char *diamond_dynamic_usb_functions[] = {
#if defined(CONFIG_USB_DYNAMIC_MASS_STORAGE)
	"usb_mass_storage",
#endif
#if defined(CONFIG_USB_DYNAMIC_ACM)
	"acm",
#endif
};

static char diamond_serial_number[DIAMOND_USB_SERIAL_STRING_SIZE] = "";

static const struct dynamic_usb_product diamond_dynamic_usb_products[] = {
	{
		.product_id		= DIAMOND_USB_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(diamond_dynamic_usb_functions),
		.functions		= diamond_dynamic_usb_functions,
	},
};

static struct dynamic_usb_platform_data diamond_dynamic_usb_data = {
	.vendor_id			= DIAMOND_USB_VENDOR_ID,
	.product_id			= DIAMOND_USB_PRODUCT_ID,
	.manufacturer_name	= DIAMOND_USB_VENDOR_STRING,
	.product_name		= DIAMOND_USB_PRODUCT_STRING,
	.serial_number		= diamond_serial_number,
	.num_products		= ARRAY_SIZE(diamond_dynamic_usb_products),
	.products			= diamond_dynamic_usb_products,
	.num_functions		= ARRAY_SIZE(diamond_dynamic_usb_functions),
	.functions			= diamond_dynamic_usb_functions,
};

static struct platform_device diamond_dynamic_usb_device = {
	.name				= "dynamic_usb",
	.id					= -1,
	.dev				= {
		.platform_data	= &diamond_dynamic_usb_data,
	},
};

static const struct dynamic_usb_product j49_dynamic_usb_products[] = {
	{
		.product_id		= J49_USB_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(diamond_dynamic_usb_functions),
		.functions		= diamond_dynamic_usb_functions,
	},
};

static struct dynamic_usb_platform_data j49_dynamic_usb_data = {
	.vendor_id			= J49_USB_VENDOR_ID,
	.product_id			= J49_USB_PRODUCT_ID,
	.manufacturer_name	= J49_USB_VENDOR_STRING,
	.product_name		= J49_USB_PRODUCT_STRING,
	.serial_number		= diamond_serial_number,
	.num_products		= ARRAY_SIZE(j49_dynamic_usb_products),
	.products			= j49_dynamic_usb_products,
	.num_functions		= ARRAY_SIZE(diamond_dynamic_usb_functions),
	.functions			= diamond_dynamic_usb_functions,
};

static struct platform_device j49_dynamic_usb_device = {
	.name				= "dynamic_usb",
	.id					= -1,
	.dev				= {
		.platform_data	= &j49_dynamic_usb_data,
	},
};
#endif /* defined(CONFIG_USB_DYNAMIC) */

#if defined(CONFIG_USB_DYNAMIC_MASS_STORAGE)
static struct usb_msc_platform_data diamond_dynamic_usb_msc_data = {
	.vendor				= DIAMOND_MSC_VENDOR_STRING,
	.product			= DIAMOND_MSC_PRODUCT_STRING,
	.release			= DIAMOND_MSC_RELEASE,
	.nluns				= 1,
};

static struct platform_device diamond_dynamic_usb_msc_device = {
	.name				= "usb_mass_storage",
	.id					= -1,
	.dev				= {
		.platform_data	= &diamond_dynamic_usb_msc_data,
	},
};
#endif /* defined(CONFIG_USB_DYNAMIC_MASS_STORAGE) */

#if defined(CONFIG_OMAP2_DSS)
/*
 * Power supplies for digital video interfaces, including the OMAP
 * Display Subsystem (DSS) and its Display Serial Interface (DSI)
 * block.
 *
 * On the prototype instantiation of the Diamond board, the TPS65921
 * VAUX rail at 2.8V is used to power the Tianma TM025ZDZ01 VCC input.
 * This supply is defined elsewhere in this file.
 *
 * Without these, there will be no functioning LCD display. These
 * supplies are largely internal between the TPS65921 and the AM3703;
 * whereas, the LCD panel itself it supplied externally to both.
 */

/* VMMC1 */
struct regulator_consumer_supply twl4030_vmmc1_supply = {
	.supply	= "vmmc",
};

static struct regulator_consumer_supply diamond_vpll2_supplies[] = {
	REGULATOR_SUPPLY("vdvi", NULL),
	{
		.supply	= "vdds_dsi",
		.dev	= &diamond_dss_device.dev,
	},
};

static struct regulator_init_data diamond_tps65921_vpll2 = {
	.constraints = {
		.name			= "VDVI",
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(diamond_vpll2_supplies),
	.consumer_supplies = diamond_vpll2_supplies,
};
#endif /* defined(CONFIG_OMAP2_DSS) */

static struct regulator_consumer_supply diamond_development_vmmc1_supplies[] = {
#if defined(CONFIG_MMC_OMAP_HS)
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.0"),
#endif /* defined(CONFIG_MMC_OMAP_HS) */
};

static struct regulator_consumer_supply diamond_prototype_vmmc1_supplies[] = {
#if defined(CONFIG_MMC_OMAP_HS)
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.0"),
#endif /* defined(CONFIG_MMC_OMAP_HS) */
#if defined(CONFIG_INPUT_ADBS_A330)
	REGULATOR_SUPPLY(DIAMOND_ADBS_A330_VDDA_SUPPLY, "2-0057"),
#endif /* defined(CONFIG_INPUT_ADBS_A330) */
};

#if !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE)
static struct regulator_consumer_supply diamond_vmmc2_supplies[] = {
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.1")
};

static struct regulator_init_data diamond_vmmc2_data = {
	.constraints = {
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
		.always_on			= false,
	},
	.num_consumer_supplies	= ARRAY_SIZE(diamond_vmmc2_supplies),
	.consumer_supplies		= diamond_vmmc2_supplies,
};

static struct fixed_voltage_config diamond_vmmc2_pdata = {
	.supply_name		= "vmmc2",
	.microvolts			= 1800000,
	.gpio				= DIAMOND_GPIO_WIFI_ENABLE,
	.startup_delay		= 70000, /* 70 ms */
	.enable_high		= true,
	.enabled_at_boot	= false,
	.init_data			= &diamond_vmmc2_data,
};

static struct platform_device diamond_vmmc2_device = {
	.name = "reg-fixed-voltage",
	.id = -1,
	.dev = {
		.platform_data = &diamond_vmmc2_pdata,
	},
};
#endif /* !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE) */

#if defined(CONFIG_TWL4030_MADC)
static struct twl4030_madc_platform_data __initdata diamond_tps65921_madc_data = {
	.irq_line = 1
};
#endif

/* VMMC1 for MMC1 pins CMD, CLK, DAT0..DAT3 (20 mA, plus card == max 220 mA) */

struct regulator_init_data vmmc1_data = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &twl4030_vmmc1_supply
};

static struct twl4030_platform_data __initdata diamond_tps65921_data = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	// platform_data for children goes here 
	.power		= &diamond_tps65921_scripts_data,
	.usb		= &diamond_tps65921_usb_data,
	.gpio		= &diamond_tps65921_gpio_data,
	.vmmc1		= &vmmc1_data,
#if defined(CONFIG_TWL4030_MADC)
	.madc		= &diamond_tps65921_madc_data,
#endif

#if defined(CONFIG_OMAP2_DSS)
	.vpll2		= &diamond_tps65921_vpll2,
#endif
	.vaux2		= &diamond_vaux2
};

/*
 * This is identical to diamond except for using j49 scripts data and
 * setting the vmmc1 data to NULL (VMMC1 is off for j49).
 */
static struct twl4030_platform_data __initdata j49_tps65921_data = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.power		= &j49_tps65921_scripts_data,
	.usb		= &diamond_tps65921_usb_data,
	.gpio		= &diamond_tps65921_gpio_data,
#if defined(CONFIG_TWL4030_MADC)
	.madc		= &diamond_tps65921_madc_data,
#endif

#if defined(CONFIG_OMAP2_DSS)
	.vpll2		= &diamond_tps65921_vpll2,
#endif
	.vaux2		= &diamond_vaux2
};

static struct i2c_board_info __initdata diamond_tps65921_i2c_info[] = {
	{
		I2C_BOARD_INFO("twl4030", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.irq = INT_34XX_SYS_NIRQ,
		.platform_data = &diamond_tps65921_data,
	},
};

static struct i2c_board_info __initdata j49_tps65921_i2c_info[] = {
	{
		I2C_BOARD_INFO("twl4030", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.irq = INT_34XX_SYS_NIRQ,
		.platform_data = &j49_tps65921_data,
	},
};

#if defined(CONFIG_SENSORS_SI1143)
static struct si1143_platform_data diamond_si1143_data = {
	.proximity_tx_led1 = 1, 
	.proximity_tx_led2 = 0, 
	.proximity_tx_led3 = 0,
};
#endif /* defined(CONFIG_SENSORS_SI1143) */

#if defined(CONFIG_INPUT_ADBS_A330)
static struct adbs_a330_platform_data diamond_adbs_a330_data = {
	.vdda_supply	= DIAMOND_ADBS_A330_VDDA_SUPPLY,
	.vdda_uv		= DIAMOND_ADBS_A330_VDDA_UV,

	.motion_gpio	= DIAMOND_GPIO_OFN_MOTION_L

	/*
	 * We'll fix up both the shutdown and reset GPIO assignments based
	 * on the model.
	 *
	 * We'll also fix up both direction and mode at runtime based on
	 * the detected model.
	 *
	 * Generally, DIRECTION_NEG for form factor enclosures,
	 * DIRECTION_POS otherwise and DELTA_X for form factor enclosures,
	 * DELTA_Y otherwise.
	 */
};

/*
 * No regulator control necessary for j49, OFN is powered from VIO
 */
static struct adbs_a330_platform_data j49_adbs_a330_data = {
	.motion_gpio	= DIAMOND_GPIO_OFN_MOTION_L

	/*
	 * We'll fix up both the shutdown and reset GPIO assignments based
	 * on the model.
	 *
	 * We'll also fix up both direction and mode at runtime based on
	 * the detected model.
	 *
	 * Generally, DIRECTION_NEG for form factor enclosures,
	 * DIRECTION_POS otherwise and DELTA_X for form factor enclosures,
	 * DELTA_Y otherwise.
	 */
};
#endif /* defined(CONFIG_INPUT_ADBS_A330) */

static struct i2c_board_info __initdata diamond_development_i2c_2_info[] = {
#if defined(CONFIG_SENSORS_SI1143)
	{
		I2C_BOARD_INFO("si1143", 0x5A),
		.flags = I2C_CLIENT_WAKE,
		.irq = DIAMOND_GPIO_PROX_ALS_IRQ,
		.platform_data = &diamond_si1143_data,
	},
#endif /* defined(CONFIG_SENSORS_SI1143) */
};

static struct i2c_board_info __initdata diamond_prototype_i2c_2_info[] = {
#if defined(CONFIG_INPUT_ADBS_A330)
	{
		I2C_BOARD_INFO("avago-adbs-a330", 0x57),
		.platform_data = &diamond_adbs_a330_data,
	}
#endif /* defined(CONFIG_INPUT_ADBS_A330) */
};

static struct i2c_board_info __initdata j49_i2c_2_info[] = {
#if defined(CONFIG_INPUT_ADBS_A330)
	{
		I2C_BOARD_INFO("avago-adbs-a330", 0x57),
		.platform_data = &j49_adbs_a330_data,
	}
#endif /* defined(CONFIG_INPUT_ADBS_A330) */
};

#if defined(CONFIG_GENERIC_CMOS_UPDATE) && defined(CONFIG_RTC_LIB)
/**
 * update_persistent_clock - set the hardware clock time to system time
 * @now: the current system wall clock time.
 *
 * This routine attempts to access the primary hardware real-time
 * clock and, if successful, sets it to the current system wall clock
 * time.
 *
 * This generation (2.6.32) of ARM kernel has an alternative function
 * do_set_rtc in linux/arch/arm/kernel/time.c that does something
 * similar; however, in a more awkward and non-standard way. So much
 * so, that in 2.6.36 kernels, it's gone away entirely.
 *
 * This more or less matches what is in linux/arch/sparc/kernel/time_64.c
 *
 * Returns 0 if the real-time clock was successfully set to the system
 * wall clock time; otherwise, < 0 on error.
 */
int update_persistent_clock(struct timespec now)
{
	const char * name = "rtc0";
	struct rtc_device *rtc;
	int err = -ENODEV;

	rtc = rtc_class_open(name);

	if (rtc) {
		err = rtc_set_mmss(rtc, now.tv_sec);
		rtc_class_close(rtc);
	}

	return err;
}
#endif /* defined(CONFIG_GENERIC_CMOS_UPDATE) && defined(CONFIG_RTC_LIB) */

static struct omap_board_config_kernel diamond_config[] __initdata = {
};

static void __init diamond_irq_init(void)
{
	omap_board_config = diamond_config;
	omap_board_config_size = ARRAY_SIZE(diamond_config);
	omap2_init_common_infrastructure();

	omap2_init_common_devices(k4x51163pi_nt6d_sdrc_params, NULL);
	omap_init_irq();
    gpmc_init();
}

static struct platform_device *diamond_common_devices[] __initdata = {
	&diamond_dss_device,
	&diamond_piezo_pwm_device,
#if defined(CONFIG_WILINK) || defined(CONFIG_WILINK_MODULE)
	&diamond_wl1271_device,
#endif
	&diamond_piezo_device,
	&diamond_backplate_device,
#if defined(CONFIG_USB_DYNAMIC_MASS_STORAGE)
	&diamond_dynamic_usb_msc_device,
#endif
#if !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE)
	&diamond_vmmc2_device,
#endif /* !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE) */
};

static struct platform_device *diamond_development_devices[] __initdata = {
#if defined(CONFIG_USB_DYNAMIC)
	&diamond_dynamic_usb_device,
#endif
#if defined(CONFIG_INPUT_GPIO_ROTARY_ENCODER_LITE)
	&diamond_rotary_device,
#endif
	&diamond_tps61042_backlight_device,
	&diamond_backlight_pwm_device,
	&diamond_zigbee_device,
};

static struct platform_device *diamond_prototype_devices[] __initdata = {
#if defined(CONFIG_USB_DYNAMIC)
	&diamond_dynamic_usb_device,
#endif
#if defined(CONFIG_SENSORS_TWL4030_MADC)
	&diamond_madc_hwmon_device,
#endif
	&diamond_tps61042_backlight_device,
	&diamond_backlight_pwm_device,
	&diamond_zigbee_device,
};

static struct platform_device *diamond_1_6_devices[] __initdata = {
#if defined(CONFIG_USB_DYNAMIC)
	&diamond_dynamic_usb_device,
#endif
#if defined(CONFIG_SENSORS_TWL4030_MADC)
	&diamond_madc_hwmon_device,
#endif
	&diamond_battery_device,
	&diamond_tps61042_backlight_device,
	&diamond_backlight_pwm_device,
	&diamond_zigbee_device,
};

static struct platform_device *display_2_0_devices[] __initdata = {
#if defined(CONFIG_USB_DYNAMIC)
	&j49_dynamic_usb_device,
#endif
#if defined(CONFIG_SENSORS_TWL4030_MADC)
	&diamond_madc_hwmon_device,
#endif
	&diamond_battery_device,
	&j49_lm3530_backlight_device,
	&j49_zigbee_device,
};

#ifdef CONFIG_OMAP_MUX
/*
 * These are listed in schematic order, starting from the first CPU
 * block symbol to the second, upper left corner to lower right
 * corner.
 *
 * Do NOT be tempted to refactor these into a set of common mux
 * settings and board-specific mux settings as omap3_mux_init does not
 * tolerate being called twice and will overwrite previous settings
 * with package defaults before applying the second set of
 * board-specific settings.
 */

#include "board-diamond-development-mux.c"
#include "board-diamond-prototype-mux.c"
#include "board-diamond-evt-mux.c"
#include "board-diamond-dvt-mux.c"
#include "board-j49-prototype-mux.c"
#include "board-j49-evt-mux.c"
#else
#define diamond_development_mux	NULL
#define diamond_prototype_mux	NULL
#define diamond_evt_mux			NULL
#define diamond_dvt_mux			NULL
#define j49_prototype_mux		NULL
#define j49_evt_mux             NULL
#endif

#if defined(CONFIG_USB_DYNAMIC)
/**
 * diamond_serial_number_init - initialize the board serial number used for USB
 *
 * This routine attempts to decode the board serial number, passed
 * from the boot loader via ATAGs, and format it into a
 * NULL-terminated C string used by the USB dynamic multi-function
 * composite gadget interface.
 *
 * Successful implementation of this relies on the fact that
 * manufacturing is restricting the serial number format to 15 decimal
 * digits which we represent as a zero-padded, right-aligned
 * hexadecimal number. Were that not the case, the serial number would
 * have to be passed as a string on the kernel command line or we'd
 * have to have a u-boot environment driver in the kernel.
 */
static void __init diamond_serial_number_init(void)
{
	snprintf(diamond_serial_number,
			 DIAMOND_USB_SERIAL_STRING_SIZE,
			 "%.07X%.08X",
			 system_serial_high,
			 system_serial_low);
}
#else
static inline void diamond_serial_number_init(void) { return; }
#endif /* defined(CONFIG_USB_DYNAMIC) */

static void __init diamond_usb_init(void)
{
	diamond_serial_number_init();

	// USB 2.0 High-speed device support initialization
	usb_musb_init(&musb_board_data);

#if defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
	// USB 2.0 Full-speed host support initialization
	diamond_gpio_try_output(DIAMOND_GPIO_INTUSB_SUSPEND, 0, return);

	usb_ohci_init(&diamond_ohci_pdata);
#else
	diamond_gpio_try_output(DIAMOND_GPIO_INTUSB_SUSPEND, 1, return);
#endif
}

static void __init diamond_backplate_init(const struct nlmodel *model)
{
	diamond_backplate_device.dev.platform_data = (void *)((!strcmp(model->family, diamond_family) && \
													(model->revision <= 3)) ? 0 : 1);
}

static void __init diamond_clk_init(void)
{
	struct clk *sys_clkout1 = NULL;

	/* In this architecture, we use the AM3703 as a 19.2 MHz clock
	 * master and the TPS65921 as a clock slave, fed via the AM3703's
	 * sys_clkout1 pin.
	 *
	 * We need to request the 'sys_clkout1' clock source to at least
	 * give it a reference count of one (1); otherwise, if the power
	 * management code has been configured, it'll get unceremoniously
	 * turned off with a message:
	 *
	 *   Disabling unused clock "sys_clkout1"
	 *
	 * and general system functionality fails thereafter.
	 */

	sys_clkout1 = clk_get(NULL, "sys_clkout1");

	BUG_ON(IS_ERR(sys_clkout1));

	clk_enable(sys_clkout1);
}

static void __init diamond_development_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(diamond_development_mux, flags);
}

static void __init diamond_1_4_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(diamond_prototype_mux, flags);
}

static void __init diamond_1_5_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(diamond_prototype_mux, flags);
}

static void __init diamond_1_6_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(diamond_evt_mux, flags);
}

static void __init diamond_1_7_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(diamond_dvt_mux, flags);
}

static void __init display_2_0_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(j49_prototype_mux, flags);
}

static void __init display_2_1_mux_init(void)
{
	const int flags = OMAP_PACKAGE_CUS;

	omap3_mux_init(j49_evt_mux, flags);
}

static void __init diamond_development_i2c_init(void)
{
	omap_register_i2c_bus(1, 2600, diamond_tps65921_i2c_info,
						  ARRAY_SIZE(diamond_tps65921_i2c_info));
	omap_register_i2c_bus(2, 400, diamond_development_i2c_2_info,
						  ARRAY_SIZE(diamond_development_i2c_2_info));
}

static void __init diamond_1_4_i2c_init(void)
{
#if defined(CONFIG_INPUT_ADBS_A330)
	/* Fix-up the shutdown and reset GPIOs as appropriate. */
	diamond_adbs_a330_data.shutdown_gpio = DIAMOND_PROTOTYPE_GPIO_OFN_SHUTDOWN;
	diamond_adbs_a330_data.reset_gpio	 = DIAMOND_PROTOTYPE_GPIO_OFN_RESET_L;

	/* Fix-up the direction and mode as apppropriate. */

	diamond_adbs_a330_data.direction = ADBS_A330_DIRECTION_POS;
	diamond_adbs_a330_data.mode      = ADBS_A330_DELTA_Y;
#endif /* defined(CONFIG_INPUT_ADBS_A330) */
	omap_register_i2c_bus(1, 2600, diamond_tps65921_i2c_info,
						  ARRAY_SIZE(diamond_tps65921_i2c_info));
	omap_register_i2c_bus(2, 400, diamond_prototype_i2c_2_info,
						  ARRAY_SIZE(diamond_prototype_i2c_2_info));
}

static void __init diamond_1_5_i2c_init(void)
{
	/* Fix-up the shutdown and reset GPIOs as appropriate. */

#if defined(CONFIG_INPUT_ADBS_A330)
	diamond_adbs_a330_data.shutdown_gpio = DIAMOND_PROTOTYPE_GPIO_OFN_SHUTDOWN;
	diamond_adbs_a330_data.reset_gpio	 = DIAMOND_PROTOTYPE_GPIO_OFN_RESET_L;

	/* Fix-up the direction and mode as apppropriate.*/

	diamond_adbs_a330_data.direction = ADBS_A330_DIRECTION_NEG;
	diamond_adbs_a330_data.mode      = ADBS_A330_DELTA_X;
#endif /* defined(CONFIG_INPUT_ADBS_A330) */
	omap_register_i2c_bus(1, 2600, diamond_tps65921_i2c_info,
						  ARRAY_SIZE(diamond_tps65921_i2c_info));
	omap_register_i2c_bus(2, 400, diamond_prototype_i2c_2_info,
						  ARRAY_SIZE(diamond_prototype_i2c_2_info));
}

static void __init diamond_1_6_i2c_init(void)
{
	/* Fix-up the shutdown and reset GPIOs as appropriate.*/

#if defined(CONFIG_INPUT_ADBS_A330)
	diamond_adbs_a330_data.shutdown_gpio = DIAMOND_EVT_GPIO_OFN_SHUTDOWN;
	diamond_adbs_a330_data.reset_gpio	 = DIAMOND_EVT_GPIO_OFN_RESET_L;

	/* Fix-up the direction and mode as apppropriate.*/

	diamond_adbs_a330_data.direction = ADBS_A330_DIRECTION_NEG;
	diamond_adbs_a330_data.mode      = ADBS_A330_DELTA_X;
#endif /* defined(CONFIG_INPUT_ADBS_A330) */
	omap_register_i2c_bus(1, 400, diamond_tps65921_i2c_info,
						  ARRAY_SIZE(diamond_tps65921_i2c_info));
	omap_register_i2c_bus(2, 400, diamond_prototype_i2c_2_info,
						  ARRAY_SIZE(diamond_prototype_i2c_2_info));
}

static void __init display_2_0_i2c_init(void)
{
#if defined(CONFIG_INPUT_ADBS_A330)
    /* Fix-up the shutdown and reset GPIOs as appropriate. */
    j49_adbs_a330_data.shutdown_gpio = DIAMOND_EVT_GPIO_OFN_SHUTDOWN;
    j49_adbs_a330_data.reset_gpio	 = DIAMOND_EVT_GPIO_OFN_RESET_L;

    /* Fix-up the direction and mode as apppropriate. */
    
    j49_adbs_a330_data.direction = ADBS_A330_DIRECTION_POS;
    j49_adbs_a330_data.mode      = ADBS_A330_DELTA_Y;
#endif
    omap_register_i2c_bus(1, 400, j49_tps65921_i2c_info,
                          ARRAY_SIZE(j49_tps65921_i2c_info));
    omap_register_i2c_bus(2, 400, j49_i2c_2_info,
                          ARRAY_SIZE(j49_i2c_2_info));
    
    omap_register_i2c_bus(3, 400, j49_backlight_i2c_info,
                          ARRAY_SIZE(j49_backlight_i2c_info));
}

static void __init diamond_1_4_piezo_init(void)
{
	diamond_pwm_beeper_data.init	= diamond_prototype_piezo_init;
	diamond_pwm_beeper_data.exit	= diamond_prototype_piezo_exit;
	diamond_pwm_beeper_data.notify	= diamond_prototype_piezo_notify;
}

static void __init diamond_1_5_piezo_init(void)
{
	diamond_1_4_piezo_init();
}

static void __init diamond_development_add_devices(void)
{
	platform_add_devices(diamond_common_devices,
						 ARRAY_SIZE(diamond_common_devices));
	platform_add_devices(diamond_development_devices,
						 ARRAY_SIZE(diamond_development_devices));
}

static void __init diamond_1_4_add_devices(void)
{
	platform_add_devices(diamond_common_devices,
						 ARRAY_SIZE(diamond_common_devices));
	platform_add_devices(diamond_prototype_devices,
						 ARRAY_SIZE(diamond_prototype_devices));
}

static void __init diamond_1_5_add_devices(void)
{
	diamond_1_4_add_devices();
}

static void __init diamond_1_6_add_devices(void)
{
	platform_add_devices(diamond_common_devices,
						 ARRAY_SIZE(diamond_common_devices));
	platform_add_devices(diamond_1_6_devices,
						 ARRAY_SIZE(diamond_1_6_devices));
}

/**
 * diamond_1_7_add_devices - Add platform devices for model identifier "Diamond-1.7"
 *
 * Diamond-1.7 has an identical set of devices to Diamond-1.6, so
 * simply add those.
 */
static void __init diamond_1_7_add_devices(void)
{
	platform_add_devices(diamond_common_devices,
						 ARRAY_SIZE(diamond_common_devices));
	platform_add_devices(diamond_1_6_devices,
						 ARRAY_SIZE(diamond_1_6_devices));
}

static void __init display_2_0_add_devices(void)
{
	platform_add_devices(diamond_common_devices,
						 ARRAY_SIZE(diamond_common_devices));
	platform_add_devices(display_2_0_devices,
						 ARRAY_SIZE(display_2_0_devices));
}

static void __init diamond_development_spi_init(void)
{
	spi_register_board_info(diamond_development_spi_board_info,
							ARRAY_SIZE(diamond_development_spi_board_info));
}

static void __init diamond_1_4_spi_init(void)
{
	spi_register_board_info(diamond_1_4_spi_board_info,
							ARRAY_SIZE(diamond_1_4_spi_board_info));
}

static void __init diamond_1_5_spi_init(void)
{
	spi_register_board_info(diamond_1_5_spi_board_info,
							ARRAY_SIZE(diamond_1_5_spi_board_info));
}

static void __init diamond_1_7_spi_init(void)
{
	spi_register_board_info(diamond_1_7_spi_board_info,
							ARRAY_SIZE(diamond_1_7_spi_board_info));
}

static void __init display_2_0_spi_init(void)
{
	spi_register_board_info(display_2_0_spi_board_info,
							ARRAY_SIZE(display_2_0_spi_board_info));
}

static void __init diamond_serial_init(void)
{
	omap_serial_init();
}

static void __init diamond_development_mmc_init(void)
{
	omap2_hsmmc_init(diamond_development_tps65921_mmc);
}

static void __init diamond_1_4_mmc_init(void)
{
	omap2_hsmmc_init(diamond_prototype_tps65921_mmc);
}

static void __init diamond_1_5_mmc_init(void)
{
	diamond_1_4_mmc_init();
}

static void __init diamond_development_display_init(void)
{
	omapfb_set_platform_data(&diamond_320_480_omapfb_config);

	diamond_display_gpio_init();

	diamond_dss_devices[0] = &diamond_development_lcd_device;
	diamond_dss_data.default_device = &diamond_development_lcd_device;
}

static void __init diamond_1_4_display_init(void)
{
	omapfb_set_platform_data(&diamond_320_480_omapfb_config);

	diamond_dss_devices[0] = &diamond_1_4_lcd_device;
	diamond_dss_data.default_device = &diamond_1_4_lcd_device;
}

static void __init diamond_1_5_display_init(void)
{
	omapfb_set_platform_data(&diamond_320_320_omapfb_config);

	diamond_dss_devices[0] = &diamond_1_5_lcd_device;
	diamond_dss_data.default_device = &diamond_1_5_lcd_device;
}

static void __init diamond_development_regulator_init(void)
{
	// First, fix-up the number of and pointer to the consumer
	// supplies for the VMMC1 PMU regulator rail; the static
	// configuration in twl4030-pmic.c is incorrect for this board.

	vmmc1_data.num_consumer_supplies = ARRAY_SIZE(diamond_development_vmmc1_supplies);
	vmmc1_data.consumer_supplies = diamond_development_vmmc1_supplies;

	// Fix up the LCD VPLL2 supply device.

	diamond_vpll2_supplies[0].dev = &diamond_development_lcd_device.dev;

	// Fix up the VAUX2 supplies as appropriate for this model

	diamond_vaux2.num_consumer_supplies = ARRAY_SIZE(diamond_development_vaux2_supplies);
	diamond_vaux2.consumer_supplies = diamond_development_vaux2_supplies;

#if !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE)
	// Fix up the VMMC2 supply device.

	diamond_vmmc2_supplies[0].dev = diamond_development_tps65921_mmc[1].dev;
#endif /* !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE) */
}

static void __init diamond_prototype_regulator_init(struct device *lcd_dev)
{
	// First, fix-up the number of and pointer to the consumer
	// supplies for the VMMC1 PMU regulator rail; the static
	// configuration in twl4030-pmic.c is incorrect for this board.

	vmmc1_data.num_consumer_supplies = ARRAY_SIZE(diamond_prototype_vmmc1_supplies);
	vmmc1_data.consumer_supplies = diamond_prototype_vmmc1_supplies;

	// Fix up the LCD VPLL2 supply device.

	diamond_vpll2_supplies[0].dev = lcd_dev;

	// Fix up the LCD VAUX2 supply device.

	diamond_prototype_vaux2_supplies[1].dev = lcd_dev;

	// Fix up the VAUX2 supplies as appropriate for this model

	diamond_vaux2.num_consumer_supplies = ARRAY_SIZE(diamond_prototype_vaux2_supplies);
	diamond_vaux2.consumer_supplies = diamond_prototype_vaux2_supplies;

#if !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE)
	// Fix up the VMMC2 supply device.

	diamond_vmmc2_supplies[0].dev = diamond_prototype_tps65921_mmc[0].dev;
#endif /* !defined(CONFIG_WILINK) && !defined(CONFIG_WILINK_MODULE) */
}

static void __init diamond_1_4_regulator_init(void)
{
	diamond_prototype_regulator_init(&diamond_1_4_lcd_device.dev);
}

static void __init diamond_1_5_regulator_init(void)
{
	diamond_prototype_regulator_init(&diamond_1_5_lcd_device.dev);
}

static void __init diamond_1_4_power_init(void)
{
	pm_halt_system = twl4030_poweroff;
}

static void __init diamond_1_6_power_init(void)
{
	pm_power_off = diamond_1_6_power_off;
	pm_halt_system = diamond_1_6_halt_system;
}

#if defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE)
static void __init diamond_wl12xx_init(struct wl12xx_platform_data *data, unsigned int gpio, int clock)
{
	int err, irq;
	const char *name = "WL127x IRQ";

	err = gpio_request(gpio, name);
	if (err < 0) {
		pr_err("Could not request GPIO %u for %s: %d\n", gpio, name, err);
		return;
	}

	irq = OMAP_GPIO_IRQ(gpio);
	if (irq < 0) {
		pr_err("Could not assign GPIO %u for %s: %d\n", gpio, name, irq);
		return;
	}

	data->irq = irq;
	data->board_ref_clock = clock;

	wl12xx_set_platform_data(data);
}
#endif /* defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE) */

static void __init diamond_development_net_init(void)
{
	diamond_smsc911x_init();
	diamond_wlan_init();

#if defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE)
	diamond_wl12xx_init(&diamond_wl12xx_data, DIAMOND_GPIO_WIFI_IRQ, WL12XX_REFCLOCK_26);
#endif /* defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE) */
}


static void __init diamond_1_4_net_init(void)
{
	diamond_wlan_init();

#if defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE)
	diamond_wl12xx_init(&diamond_wl12xx_data, DIAMOND_GPIO_WIFI_IRQ, WL12XX_REFCLOCK_26);
#endif /* defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE) */
}

static void __init diamond_1_5_net_init(void)
{
	diamond_1_4_net_init();
}

static void __init display_2_0_net_init(void)
{
#if defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE)
	diamond_wl12xx_init(&diamond_wl12xx_data, DIAMOND_GPIO_WIFI_IRQ, WL12XX_REFCLOCK_26_XTAL);
#endif /* defined(CONFIG_WL12XX) || defined(CONFIG_WL12XX_MODULE) */
}

static void __init diamond_model_init(struct nlmodel *model,
									  const struct diamond_init_data *didp)
{
	if (didp == NULL)
		return;

	if (didp->regulator_init) {
		didp->regulator_init();
	}

	if (didp->power_init) {
		didp->power_init();
	}

	if (didp->backplate_init) {
		didp->backplate_init(model);
	}

	if (didp->display_init) {
		didp->display_init();
	}

	if (didp->mux_init) {
		didp->mux_init();
	}

	if (didp->i2c_init) {
		didp->i2c_init();
	}

	if (didp->piezo_init) {
		didp->piezo_init();
	}

	if (didp->add_devices) {
		didp->add_devices();
	}

	if (didp->spi_init) {
		didp->spi_init();
	}

	if (didp->serial_init) {
		didp->serial_init();
	}

	if (didp->mmc_init) {
		didp->mmc_init();
	}

	if (didp->usb_init) {
		didp->usb_init();
	}

	if (didp->flash_init) {
		didp->flash_init();
	}

	if (didp->net_init) {
		didp->net_init();
	}

	if (didp->clk_init) {
		didp->clk_init();
	}
}

const struct diamond_init_data * __init __diamond_init_data(const char *family, int product, int revision)
{
	const struct diamond_init_data *data = NULL;

	if (family != NULL) {
		if ((strcmp(family, diamond_family) != 0) && strcmp(family, j49_family) != 0) {
			machine_warn("Unsupported model family '%s'\n", family);

		} else {
			if (product == diamond_product_default) {
				if (revision >= 0 && revision <= 3) {
					data = diamond_init_data[DIAMOND_DEVELOPMENT_DATA];

				} else if (revision == 4) {
					data = diamond_init_data[DIAMOND_1_4_DATA];

				} else if (revision == 5) {
					data = diamond_init_data[DIAMOND_1_5_DATA];

				} else if (revision == 6) {
					data = diamond_init_data[DIAMOND_1_6_DATA];

				} else if (revision == 7) {
					data = diamond_init_data[DIAMOND_1_7_DATA];

				} else if (revision == 8) {
					data = diamond_init_data[DIAMOND_1_8_DATA];

				} else if (revision >= 9) {
					data = diamond_init_data[DIAMOND_1_9_DATA];

				} else if (revision != NL_MODEL_UNKNOWN) {
					machine_warn("Unsupported model revision '%d'\n", revision);

				} else {
					machine_warn("Unknown model revision.\n");

				}
				
			} else if (product == j49_product_default) {
				if (revision == 0) {
					data = diamond_init_data[DISPLAY_2_0_DATA];

				} else if (revision == 1) {
					data = diamond_init_data[DISPLAY_2_1_DATA];

				} else if (revision == 2) {
					data = diamond_init_data[DISPLAY_2_2_DATA];

				} else if (revision == 3) {
					data = diamond_init_data[DISPLAY_2_3_DATA];

				} else if (revision >= 4) {
					data = diamond_init_data[DISPLAY_2_4_DATA];

				} else if (revision != NL_MODEL_UNKNOWN) {
					machine_warn("Unsupported model revision '%d'\n", revision);

				} else {
					machine_warn("Unknown model revision.\n");
					
				}
				
			} else if (product != NL_MODEL_UNKNOWN) {
				machine_warn("Unsupported model product '%d'\n", product);

			} else {
				machine_warn("Unknown model product.\n");

			}
		}
	}

	return data;
}

/**
 * diamond_init - machine-specific initialization
 *
 * This is the machine-specific initialization function called from
 * the .init_machine method of the machine defintion structure.
 *
 * This attempts to determine the machine model we are running on,
 * first from the 'nlmodel=<family>-<product>.<revision>' kernel
 * command line argument passed in by the boot loader, second the
 * BCD-encoded <product:8><revision:8> also passed in from the boot
 * loader and encoded in system_rev and, finally, by simply guessing.
 *
 */
static void __init diamond_init(void)
{
	int status;
	struct nlmodel model;
	const char *identifier = NULL;
	const struct diamond_init_data *didp = NULL;
	const char *family = NULL;
	int product = NL_MODEL_UNKNOWN, revision = NL_MODEL_UNKNOWN;
	bool initialized;

	status = nlmodel_init(&model);
	
	initialized = (status == 0);

	if (status != 0) {
		machine_warn("Could not initialize model.\n");

	} else {
		identifier = nlmodel_identifier();

		if (identifier == NULL) {
			machine_warn("Could not determine the model identifier.\n");

		} else {
			status = nlmodel_parse(identifier, &model);

			if (status != 0) {
				machine_warn("Unrecognized model identifer '%s'\n", identifier);

			} else {
				family   = model.family;
				product  = model.product;
				revision = model.revision;

				didp = __diamond_init_data(family, product, revision);
			}
		}
	}

	/* Assume a Diamond family and fall back and try to BCD-decode
	 * (8-bits each, integer and fractional components) the product
	 * and revision from the system revision global as it's possible
	 * that the boot loader can still pass that via tags but the
	 * 'nlmodel=...' was missed.
	 */

	if (didp == NULL) {
		machine_warn("Trying fallback model information.\n");

		if (family == NULL) {
			family = diamond_family;
		}

		product	 = (system_rev >> 8) & 0xFF;
		revision = (system_rev >> 0) & 0xFF;

		/* Treat a BCD-encoded value of 0x0000 the same as unknown (0xFFFF). */

		if (!product && !revision) {
			product = NL_MODEL_UNKNOWN;
			revision = NL_MODEL_UNKNOWN;
		}

		if (model.product != NL_MODEL_UNKNOWN) {
			product	 = model.product;
		}

		if (model.revision != NL_MODEL_UNKNOWN) {
			revision = model.revision;
		}

		didp = __diamond_init_data(family, product, revision);
	}

	/* The fallback did not work, guess. */

	if (didp == NULL) {
		machine_warn("Guessing model information.\n");

		family   = diamond_family;
		product  = diamond_product_default;
		revision = diamond_revision_default;

		didp = __diamond_init_data(family, product, revision);

		BUG_ON(didp == NULL);
	}

	/* Log what model we decoded, fell back to or guessed. */

	if ((family != NULL) && (revision != NL_MODEL_UNKNOWN)) {
		if ((didp != NULL) && (didp->name != NULL)) {
			machine_info("Nest %s %s, Revision %d\n",
						 family,
						 didp->name,
						 revision);

		} else {
			machine_info("Nest %s, Revision %d\n",
						 family,
						 revision);

		}
	}

	/* Perform the model-specific initialization. */

	diamond_model_init(&model, didp);

	if (initialized) {
		nlmodel_destroy(&model);
	}
}

/* This board file will support the following board machine identifiers. */

MACHINE_START(OMAP3EVM, "Nest Diamond")
	.boot_params	= 0x80000100,
	.map_io			= omap3_map_io,
    .reserve	    = omap_reserve,
	.init_irq		= diamond_irq_init,
	.init_machine	= diamond_init,
	.timer			= &omap_timer,
MACHINE_END

/* MACH_DIAMOND - http://www.arm.linux.org.uk/developer/machines/list.php?id=3745 */
MACHINE_START(DIAMOND, "Nest Diamond")
	.boot_params	= 0x80000100,
	.map_io			= omap3_map_io,
    .reserve        = omap_reserve,
	.init_irq		= diamond_irq_init,
	.init_machine	= diamond_init,
	.timer			= &omap_timer,
MACHINE_END

/* MACH_J49 - http://www.arm.linux.org.uk/developer/machines/list.php?id=4082 */
MACHINE_START(J49, "Nest J49")
	.boot_params	= 0x80000100,
	.map_io			= omap3_map_io,
    .reserve        = omap_reserve,
	.init_irq		= diamond_irq_init,
	.init_machine	= diamond_init,
	.timer			= &omap_timer,
MACHINE_END
