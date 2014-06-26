/*
 * linux/arch/arm/mach-omap2/board-am3517evm.c
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 * Author: Ranjith Lohithakshan <ranjithl@ti.com>
 *
 * Based on mach-omap2/board-omap3evm.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as  published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c/pca953x.h>
#include <linux/can/platform/ti_hecc.h>
#include <linux/davinci_emac.h>
#include <linux/irq.h>
#include <linux/i2c/tsc2004.h>
#include <linux/input.h>
#include <linux/tca6416_keypad.h>
#include <linux/mmc/host.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>

#include <mach/hardware.h>
#include <mach/am35xx.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/usb.h>
#include <plat/display.h>
#include <plat/gpmc.h>
#include <plat/nand.h>

#include <media/davinci/vpfe_capture.h>
#include <media/tvp514x.h>

#include "mux.h"
#include "control.h"
#include "hsmmc.h"
#include "board-flash.h"

#define AM35XX_EVM_MDIO_FREQUENCY	(1000000)

#define NAND_BLOCK_SIZE        SZ_128K

static struct mtd_partition am3517evm_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
	{
		.name           = "xloader-nand",
		.offset         = 0,
		.size           = 4*(SZ_128K),
		.mask_flags     = MTD_WRITEABLE
	},
	{
		.name           = "uboot-nand",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 14*(SZ_128K),
		.mask_flags     = MTD_WRITEABLE
	},
	{
		.name           = "params-nand",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 2*(SZ_128K)
	},
	{
		.name           = "linux-nand",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 40*(SZ_128K)
	},
	{
		.name           = "jffs2-nand",
		.size           = MTDPART_SIZ_FULL,
		.offset         = MTDPART_OFS_APPEND,
	},
};

static struct mdio_platform_data am3517_evm_mdio_pdata = {
	.bus_freq	= AM35XX_EVM_MDIO_FREQUENCY,
};

static struct resource am3517_mdio_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE + AM35XX_EMAC_MDIO_OFFSET,
		.end    = AM35XX_IPSS_EMAC_BASE + AM35XX_EMAC_MDIO_OFFSET +
			  SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device am3517_mdio_device = {
	.name		= "davinci_mdio",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(am3517_mdio_resources),
	.resource	= am3517_mdio_resources,
	.dev.platform_data = &am3517_evm_mdio_pdata,
};

static struct emac_platform_data am3517_evm_emac_pdata = {
	.rmii_en	= 1,
};

static struct resource am3517_emac_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE,
		.end    = AM35XX_IPSS_EMAC_BASE + 0x2FFFF,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = INT_35XX_EMAC_C0_RXTHRESH_IRQ,
		.end    = INT_35XX_EMAC_C0_RXTHRESH_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_RX_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_RX_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_TX_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_TX_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_MISC_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_MISC_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device am3517_emac_device = {
	.name		= "davinci_emac",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(am3517_emac_resources),
	.resource	= am3517_emac_resources,
};

static void am3517_enable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR |
		AM35XX_CPGMAC_C0_MISC_PULSE_CLR |
		AM35XX_CPGMAC_C0_RX_THRESH_CLR);
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void am3517_disable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR);
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void am3517_evm_ethernet_init(struct emac_platform_data *pdata)
{
	u32 regval, mac_lo, mac_hi;

	mac_lo = omap_ctrl_readl(AM35XX_CONTROL_FUSE_EMAC_LSB);
	mac_hi = omap_ctrl_readl(AM35XX_CONTROL_FUSE_EMAC_MSB);

	pdata->mac_addr[0] = (u_int8_t)((mac_hi & 0xFF0000) >> 16);
	pdata->mac_addr[1] = (u_int8_t)((mac_hi & 0xFF00) >> 8);
	pdata->mac_addr[2] = (u_int8_t)((mac_hi & 0xFF) >> 0);
	pdata->mac_addr[3] = (u_int8_t)((mac_lo & 0xFF0000) >> 16);
	pdata->mac_addr[4] = (u_int8_t)((mac_lo & 0xFF00) >> 8);
	pdata->mac_addr[5] = (u_int8_t)((mac_lo & 0xFF) >> 0);

	pdata->ctrl_reg_offset		= AM35XX_EMAC_CNTRL_OFFSET;
	pdata->ctrl_mod_reg_offset	= AM35XX_EMAC_CNTRL_MOD_OFFSET;
	pdata->ctrl_ram_offset		= AM35XX_EMAC_CNTRL_RAM_OFFSET;
	pdata->ctrl_ram_size		= AM35XX_EMAC_CNTRL_RAM_SIZE;
	pdata->version			= EMAC_VERSION_2;
	pdata->hw_ram_addr		= AM35XX_EMAC_HW_RAM_ADDR;
	pdata->interrupt_enable		= am3517_enable_ethernet_int;
	pdata->interrupt_disable	= am3517_disable_ethernet_int;
	am3517_emac_device.dev.platform_data	= pdata;
	platform_device_register(&am3517_mdio_device);
	platform_device_register(&am3517_emac_device);
	clk_add_alias(NULL, dev_name(&am3517_mdio_device.dev),
		      NULL, &am3517_emac_device.dev);

	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);
	regval = regval & (~(AM35XX_CPGMACSS_SW_RST));
	omap_ctrl_writel(regval, AM35XX_CONTROL_IP_SW_RESET);
	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);

	return ;
}



/*
 * VPFE - Video Decoder interface
 */
#define TVP514X_STD_ALL		(V4L2_STD_NTSC | V4L2_STD_PAL)

/* Inputs available at the TVP5146 */
static struct v4l2_input tvp5146_inputs[] = {
	{
		.index	= 0,
		.name	= "Composite",
		.type	= V4L2_INPUT_TYPE_CAMERA,
		.std	= TVP514X_STD_ALL,
	},
	{
		.index	= 1,
		.name	= "S-Video",
		.type	= V4L2_INPUT_TYPE_CAMERA,
		.std	= TVP514X_STD_ALL,
	},
};

static struct tvp514x_platform_data tvp5146_pdata = {
	.clk_polarity	= 0,
	.hs_polarity	= 1,
	.vs_polarity	= 1
};

static struct vpfe_route tvp5146_routes[] = {
	{
		.input	= INPUT_CVBS_VI1A,
		.output	= OUTPUT_10BIT_422_EMBEDDED_SYNC,
	},
	{
		.input	= INPUT_SVIDEO_VI2C_VI1C,
		.output	= OUTPUT_10BIT_422_EMBEDDED_SYNC,
	},
};

static struct vpfe_subdev_info vpfe_sub_devs[] = {
	{
		.name		= "tvp5146",
		.grp_id		= 0,
		.num_inputs	= ARRAY_SIZE(tvp5146_inputs),
		.inputs		= tvp5146_inputs,
		.routes		= tvp5146_routes,
		.can_route	= 1,
		.ccdc_if_params	= {
			.if_type = VPFE_BT656,
			.hdpol	= VPFE_PINPOL_POSITIVE,
			.vdpol	= VPFE_PINPOL_POSITIVE,
		},
		.board_info	= {
			I2C_BOARD_INFO("tvp5146", 0x5C),
			.platform_data = &tvp5146_pdata,
		},
	},
};

static void am3517_evm_clear_vpfe_intr(int vdint)
{
	unsigned int vpfe_int_clr;

	vpfe_int_clr = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);

	switch (vdint) {
	/* VD0 interrrupt */
	case INT_35XX_CCDC_VD0_IRQ:
		vpfe_int_clr &= ~AM35XX_VPFE_CCDC_VD0_INT_CLR;
		vpfe_int_clr |= AM35XX_VPFE_CCDC_VD0_INT_CLR;
		break;
	/* VD1 interrrupt */
	case INT_35XX_CCDC_VD1_IRQ:
		vpfe_int_clr &= ~AM35XX_VPFE_CCDC_VD1_INT_CLR;
		vpfe_int_clr |= AM35XX_VPFE_CCDC_VD1_INT_CLR;
		break;
	/* VD2 interrrupt */
	case INT_35XX_CCDC_VD2_IRQ:
		vpfe_int_clr &= ~AM35XX_VPFE_CCDC_VD2_INT_CLR;
		vpfe_int_clr |= AM35XX_VPFE_CCDC_VD2_INT_CLR;
		break;
	/* Clear all interrrupts */
	default:
		vpfe_int_clr &= ~(AM35XX_VPFE_CCDC_VD0_INT_CLR |
				AM35XX_VPFE_CCDC_VD1_INT_CLR |
				AM35XX_VPFE_CCDC_VD2_INT_CLR);
		vpfe_int_clr |= (AM35XX_VPFE_CCDC_VD0_INT_CLR |
				AM35XX_VPFE_CCDC_VD1_INT_CLR |
				AM35XX_VPFE_CCDC_VD2_INT_CLR);
		break;
	}
	omap_ctrl_writel(vpfe_int_clr, AM35XX_CONTROL_LVL_INTR_CLEAR);
	vpfe_int_clr = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static struct vpfe_config vpfe_cfg = {
	.num_subdevs	= ARRAY_SIZE(vpfe_sub_devs),
	.i2c_adapter_id	= 3,
	.sub_devs	= vpfe_sub_devs,
	.clr_intr	= am3517_evm_clear_vpfe_intr,
	.card_name	= "DM6446 EVM",
	.ccdc		= "DM6446 CCDC",
};

static struct resource vpfe_resources[] = {
	{
		.start	= INT_35XX_CCDC_VD0_IRQ,
		.end	= INT_35XX_CCDC_VD0_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= INT_35XX_CCDC_VD1_IRQ,
		.end	= INT_35XX_CCDC_VD1_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 vpfe_capture_dma_mask = DMA_BIT_MASK(32);
static struct platform_device vpfe_capture_dev = {
	.name		= CAPTURE_DRV_NAME,
	.id		= -1,
	.num_resources	= ARRAY_SIZE(vpfe_resources),
	.resource	= vpfe_resources,
	.dev = {
		.dma_mask		= &vpfe_capture_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &vpfe_cfg,
	},
};

static struct resource dm644x_ccdc_resource[] = {
	/* CCDC Base address */
	{
		.start	= AM35XX_IPSS_VPFE_BASE,
		.end	= AM35XX_IPSS_VPFE_BASE + 0xffff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device dm644x_ccdc_dev = {
	.name		= "dm644x_ccdc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(dm644x_ccdc_resource),
	.resource	= dm644x_ccdc_resource,
	.dev = {
		.dma_mask		= &vpfe_capture_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

#define LCD_PANEL_PWR		176
#define LCD_PANEL_BKLIGHT_PWR	182
#define LCD_PANEL_PWM		181

static struct i2c_board_info __initdata am3517evm_i2c1_boardinfo[] = {
	{
		I2C_BOARD_INFO("s35390a", 0x30),
	},
};

/*
 * RTC - S35390A
 */
#define GPIO_RTCS35390A_IRQ	55

static void __init am3517_evm_rtc_init(void)
{
	int r;

	omap_mux_init_gpio(GPIO_RTCS35390A_IRQ, OMAP_PIN_INPUT_PULLUP);
	r = gpio_request(GPIO_RTCS35390A_IRQ, "rtcs35390a-irq");
	if (r < 0) {
		printk(KERN_WARNING "failed to request GPIO#%d\n",
				GPIO_RTCS35390A_IRQ);
		return;
	}
	r = gpio_direction_input(GPIO_RTCS35390A_IRQ);
	if (r < 0) {
		printk(KERN_WARNING "GPIO#%d cannot be configured as input\n",
				GPIO_RTCS35390A_IRQ);
		gpio_free(GPIO_RTCS35390A_IRQ);
		return;
	}
	am3517evm_i2c1_boardinfo[0].irq = gpio_to_irq(GPIO_RTCS35390A_IRQ);
}

/*
 * I2C GPIO Expander - TCA6416
 */

/* Mounted on Base-Board */
static struct pca953x_platform_data am3517evm_gpio_expander_info_0 = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
};
static struct i2c_board_info __initdata am3517evm_i2c2_boardinfo[] = {
	{
		I2C_BOARD_INFO("tlv320aic23", 0x1A),
	},
	{
		I2C_BOARD_INFO("tca6416", 0x21),
		.platform_data = &am3517evm_gpio_expander_info_0,
	},
};

/* Mounted on UI Card */

/*Keypad Initialization */
#define KEYPAD_IRQ		160
#define KEYPAD_PIN_MASK		0xFFC0

#define KEYPAD_BUTTON(ev_type, ev_code, act_low) \
{						\
	.type		= ev_type,		\
	.code		= ev_code,		\
	.active_low	= act_low,		\
}

#define KEYPAD_BUTTON_LOW(event_code)	\
	KEYPAD_BUTTON(EV_KEY, event_code, 1)

static int tca6416_keypad_init_irq(void)
{
	int ret = 0;

	ret = gpio_request(KEYPAD_IRQ, "tca6416-keypad-irq");
	if (ret < 0) {
		printk(KERN_WARNING "failed to request GPIO#%d: %d\n",
				KEYPAD_IRQ, ret);
		return ret;
	}
	if (gpio_direction_input(KEYPAD_IRQ)) {
		printk(KERN_WARNING "GPIO#%d cannot be configured as "
				"input\n", KEYPAD_IRQ);
		return -ENXIO;
	}

	return ret;
}

static struct tca6416_button am3517_gpio_keys[] = {
	KEYPAD_BUTTON_LOW(KEY_DOWN),
	KEYPAD_BUTTON_LOW(KEY_UP),
	KEYPAD_BUTTON_LOW(KEY_MENU),
	KEYPAD_BUTTON_LOW(KEY_MODE),
	KEYPAD_BUTTON_LOW(KEY_LEFTSHIFT),
	KEYPAD_BUTTON_LOW(KEY_REWIND),
	KEYPAD_BUTTON_LOW(KEY_FORWARD),
	KEYPAD_BUTTON_LOW(KEY_STOP),
	KEYPAD_BUTTON_LOW(KEY_PLAY),
	KEYPAD_BUTTON_LOW(KEY_RECORD),
};

static struct tca6416_keys_platform_data am3517evm_tca6416_keys_info = {
	.buttons	= am3517_gpio_keys,
	.nbuttons	= ARRAY_SIZE(am3517_gpio_keys),
	.rep		= 0,
	.use_polling	= 1,
	.pinmask	= KEYPAD_PIN_MASK,
};

static struct pca953x_platform_data am3517evm_ui_gpio_expander_info_2 = {
	.gpio_base	= OMAP_MAX_GPIO_LINES + 16,
};
static struct i2c_board_info __initdata am3517evm_i2c3_boardinfo[] = {
	{
		I2C_BOARD_INFO("tca6416-keys", 0x20),
		.platform_data = &am3517evm_tca6416_keys_info,
	},
	{
		I2C_BOARD_INFO("tca6416", 0x21),
		.platform_data = &am3517evm_ui_gpio_expander_info_2,
	},
};

/*
 * TSC 2004 Support
 */
#define	GPIO_TSC2004_IRQ	65

static int tsc2004_init_irq(void)
{
	int ret = 0;

	ret = gpio_request(GPIO_TSC2004_IRQ, "tsc2004-irq");
	if (ret < 0) {
		printk(KERN_WARNING "failed to request GPIO#%d: %d\n",
				GPIO_TSC2004_IRQ, ret);
		return ret;
	}

	if (gpio_direction_input(GPIO_TSC2004_IRQ)) {
		printk(KERN_WARNING "GPIO#%d cannot be configured as "
				"input\n", GPIO_TSC2004_IRQ);
		return -ENXIO;
	}

	gpio_set_debounce(GPIO_TSC2004_IRQ, 0xa);
	return ret;
}

static void tsc2004_exit_irq(void)
{
	gpio_free(GPIO_TSC2004_IRQ);
}

static int tsc2004_get_irq_level(void)
{
	return gpio_get_value(GPIO_TSC2004_IRQ) ? 0 : 1;
}

struct tsc2004_platform_data am3517evm_tsc2004data = {
	.model = 2004,
	.x_plate_ohms = 180,
	.get_pendown_state = tsc2004_get_irq_level,
	.init_platform_hw = tsc2004_init_irq,
	.exit_platform_hw = tsc2004_exit_irq,
};

static struct i2c_board_info __initdata am3517evm_tsc_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("tsc2004", 0x4B),
		.type		= "tsc2004",
		.platform_data	= &am3517evm_tsc2004data,
	},
};

static int __init am3517_evm_i2c_init(void)
{
	omap_register_i2c_bus(1, 400, NULL, 0);
	omap_register_i2c_bus(2, 400, am3517evm_i2c2_boardinfo,
			ARRAY_SIZE(am3517evm_i2c2_boardinfo));
	omap_register_i2c_bus(3, 400, am3517evm_i2c3_boardinfo,
			ARRAY_SIZE(am3517evm_i2c3_boardinfo));

	return 0;
}

static int lcd_enabled;
static int dvi_enabled;

#if defined(CONFIG_PANEL_SHARP_LQ043T1DG01) || \
		defined(CONFIG_PANEL_SHARP_LQ043T1DG01_MODULE)
static void __init am3517_evm_display_init(void)
{
	int r;

	omap_mux_init_gpio(LCD_PANEL_PWR, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(LCD_PANEL_BKLIGHT_PWR, OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_gpio(LCD_PANEL_PWM, OMAP_PIN_INPUT_PULLDOWN);
	/*
	 * Enable GPIO 182 = LCD Backlight Power
	 */
	r = gpio_request(LCD_PANEL_BKLIGHT_PWR, "lcd_backlight_pwr");
	if (r) {
		printk(KERN_ERR "failed to get lcd_backlight_pwr\n");
		return;
	}
	gpio_direction_output(LCD_PANEL_BKLIGHT_PWR, 1);
	/*
	 * Enable GPIO 181 = LCD Panel PWM
	 */
	r = gpio_request(LCD_PANEL_PWM, "lcd_pwm");
	if (r) {
		printk(KERN_ERR "failed to get lcd_pwm\n");
		goto err_1;
	}
	gpio_direction_output(LCD_PANEL_PWM, 1);
	/*
	 * Enable GPIO 176 = LCD Panel Power enable pin
	 */
	r = gpio_request(LCD_PANEL_PWR, "lcd_panel_pwr");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_pwr\n");
		goto err_2;
	}
	gpio_direction_output(LCD_PANEL_PWR, 0);

	printk(KERN_INFO "Display initialized successfully\n");
	return;

err_2:
	gpio_free(LCD_PANEL_PWM);
err_1:
	gpio_free(LCD_PANEL_BKLIGHT_PWR);
}
#else
static void __init am3517_evm_display_init(void) {}
#endif

static int am3517_evm_panel_enable_lcd(struct omap_dss_device *dssdev)
{
	if (dvi_enabled) {
		printk(KERN_ERR "cannot enable LCD, DVI is enabled\n");
		return -EINVAL;
	}
	gpio_set_value(LCD_PANEL_PWR, 1);
	lcd_enabled = 1;

	return 0;
}

static void am3517_evm_panel_disable_lcd(struct omap_dss_device *dssdev)
{
	gpio_set_value(LCD_PANEL_PWR, 0);
	lcd_enabled = 0;
}

static struct omap_dss_device am3517_evm_lcd_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "lcd",
	.driver_name		= "sharp_lq_panel",
	.phy.dpi.data_lines 	= 16,
	.platform_enable	= am3517_evm_panel_enable_lcd,
	.platform_disable	= am3517_evm_panel_disable_lcd,
};

static int am3517_evm_panel_enable_tv(struct omap_dss_device *dssdev)
{
	return 0;
}

static void am3517_evm_panel_disable_tv(struct omap_dss_device *dssdev)
{
}

static struct omap_dss_device am3517_evm_tv_device = {
	.type 			= OMAP_DISPLAY_TYPE_VENC,
	.name 			= "tv",
	.driver_name		= "venc",
	.phy.venc.type		= OMAP_DSS_VENC_TYPE_SVIDEO,
	.platform_enable	= am3517_evm_panel_enable_tv,
	.platform_disable	= am3517_evm_panel_disable_tv,
};

static int am3517_evm_panel_enable_dvi(struct omap_dss_device *dssdev)
{
	if (lcd_enabled) {
		printk(KERN_ERR "cannot enable DVI, LCD is enabled\n");
		return -EINVAL;
	}
	dvi_enabled = 1;

	return 0;
}

static void am3517_evm_panel_disable_dvi(struct omap_dss_device *dssdev)
{
	dvi_enabled = 0;
}

static struct omap_dss_device am3517_evm_dvi_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "dvi",
	.driver_name		= "generic_panel",
	.phy.dpi.data_lines	= 24,
	.platform_enable	= am3517_evm_panel_enable_dvi,
	.platform_disable	= am3517_evm_panel_disable_dvi,
};

static struct omap_dss_device *am3517_evm_dss_devices[] = {
	&am3517_evm_lcd_device,
	&am3517_evm_tv_device,
	&am3517_evm_dvi_device,
};

static struct omap_dss_board_info am3517_evm_dss_data = {
	.num_devices	= ARRAY_SIZE(am3517_evm_dss_devices),
	.devices	= am3517_evm_dss_devices,
	.default_device	= &am3517_evm_lcd_device,
};

static struct platform_device am3517_evm_dss_device = {
	.name		= "omapdss",
	.id		= -1,
	.dev		= {
		.platform_data	= &am3517_evm_dss_data,
	},
};

/*
 * Board initialization
 */
static struct omap_board_config_kernel am3517_evm_config[] __initdata = {
};

static struct platform_device *am3517_evm_devices[] __initdata = {
	&am3517_evm_dss_device,
	&dm644x_ccdc_dev,
	&vpfe_capture_dev,
};

static void __init am3517_evm_init_irq(void)
{
	omap_board_config = am3517_evm_config;
	omap_board_config_size = ARRAY_SIZE(am3517_evm_config);
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
	omap_init_irq();
	gpmc_init();
}

static struct omap_musb_board_data musb_board_data = {
	.interface_type         = MUSB_INTERFACE_ULPI,
	.mode                   = MUSB_OTG,
	.power                  = 500,
};

static __init void am3517_evm_musb_init(void)
{
	u32 devconf2;

	/*
	 * Set up USB clock/mode in the DEVCONF2 register.
	 */
	devconf2 = omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2);

	/* USB2.0 PHY reference clock is 13 MHz */
	devconf2 &= ~(CONF2_REFFREQ | CONF2_OTGMODE | CONF2_PHY_GPIOMODE);
	devconf2 |=  CONF2_REFFREQ_13MHZ | CONF2_SESENDEN | CONF2_VBDTCTEN
			| CONF2_DATPOL;

	omap_ctrl_writel(devconf2, AM35XX_CONTROL_DEVCONF2);

	usb_musb_init(&musb_board_data);
}

static const struct ehci_hcd_omap_platform_data ehci_pdata __initconst = {
	.port_mode[0] = EHCI_HCD_OMAP_MODE_PHY,
#if defined(CONFIG_PANEL_SHARP_LQ043T1DG01) || \
		defined(CONFIG_PANEL_SHARP_LQ043T1DG01_MODULE)
	.port_mode[1] = EHCI_HCD_OMAP_MODE_UNKNOWN,
#else
	.port_mode[1] = EHCI_HCD_OMAP_MODE_PHY,
#endif
	.port_mode[2] = EHCI_HCD_OMAP_MODE_UNKNOWN,

	.phy_reset  = true,
	.reset_gpio_port[0]  = 57,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL
};


/* NOR flash information */
static struct mtd_partition am3517_evm_norflash_partitions[] = {
	/* primiary bootloader (X-loader) in first 4 sectors(32K) */
	{
		.name           = "X-Loader-NOR",
		.offset         = 0,
		.size           = 4 * SZ_32K,
		.mask_flags     = MTD_WRITEABLE, /* force read-only */
	},
	/* secondary bootloader (U-Boot, etc) in first 5 sectors(128K) */
	{
		.name           = "U-Boot-NOR",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 5 * SZ_128K,
		.mask_flags     = MTD_WRITEABLE, /* force read-only */
	},
	/* bootloader params in the next 2 sectors */
	{
		.name           = "Boot Env-NOR",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 2 * SZ_128K,
		.mask_flags     = 0,
	},
	/* kernel */
	{
		.name           = "Kernel-NOR",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 32 * SZ_128K,
		.mask_flags     = 0
	},
	/* file system */
	{
		.name           = "File System-NOR",
		.offset         = MTDPART_OFS_APPEND,
		.size           = MTDPART_SIZ_FULL,
		.mask_flags     = 0
	},
};

static void __init am3517_nor_init(void)
{
	int cs = 0;
	int norcs = -1;

	/* find out the chip-select on which NOR exists */
	while (cs < GPMC_CS_NUM) {
		u32 ret = 0;

		ret = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG1);
		if ((ret & 0xC00) == 0x0) {
			printk(KERN_INFO "Found NOR on CS%d\n", cs);
			norcs = cs;
			break;
		}
		cs++;
	}

	if (cs >= GPMC_CS_NUM) {
		printk(KERN_INFO "NOR: Unable to find configuration in GPMC\n");
		return;
	}

	printk(KERN_INFO "Registering NOR on CS%d\n", norcs);
	board_nor_init(am3517_evm_norflash_partitions,
			ARRAY_SIZE(am3517_evm_norflash_partitions), norcs);
}

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	/* USB OTG DRVVBUS offset = 0x212 */
	OMAP3_MUX(SAD2D_MCAD23, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(MCBSP_CLKS, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(GPMC_NCS4, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(SYS_NRESWARM, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

/*
 * HECC information
 */

#define CAN_STB         214
static void hecc_phy_control(int on)
{
        int r;

        r = gpio_request(CAN_STB, "can_stb");
        if (r) {
                printk(KERN_ERR "failed to get can_stb \n");
                return;
        }

        gpio_direction_output(CAN_STB, (on==1)?0:1);
}

static struct resource am3517_hecc_resources[] = {
	{
		.start	= AM35XX_IPSS_HECC_BASE,
		.end	= AM35XX_IPSS_HECC_BASE + 0x3FFF,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_35XX_HECC0_IRQ,
		.end	= INT_35XX_HECC0_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device am3517_hecc_device = {
	.name		= "ti_hecc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(am3517_hecc_resources),
	.resource	= am3517_hecc_resources,
};

static struct ti_hecc_platform_data am3517_evm_hecc_pdata = {
	.scc_hecc_offset	= AM35XX_HECC_SCC_HECC_OFFSET,
	.scc_ram_offset		= AM35XX_HECC_SCC_RAM_OFFSET,
	.hecc_ram_offset	= AM35XX_HECC_RAM_OFFSET,
	.mbx_offset		= AM35XX_HECC_MBOX_OFFSET,
	.int_line		= AM35XX_HECC_INT_LINE,
	.version		= AM35XX_HECC_VERSION,
	.transceiver_switch     = hecc_phy_control,
};

static void am3517_evm_hecc_init(struct ti_hecc_platform_data *pdata)
{
	am3517_hecc_device.dev.platform_data = pdata;
	platform_device_register(&am3517_hecc_device);
}

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= 127,
		.gpio_wp	= 126,
	},
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= 128,
		.gpio_wp	= 129,
	},
	{}      /* Terminator */
};


static void __init am3517_evm_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);

	am3517_evm_i2c_init();
	platform_add_devices(am3517_evm_devices,
				ARRAY_SIZE(am3517_evm_devices));

	omap_serial_init();

	/* Configure GPIO for EHCI port */
	omap_mux_init_gpio(57, OMAP_PIN_OUTPUT);
	usb_ehci_init(&ehci_pdata);
	am3517_evm_hecc_init(&am3517_evm_hecc_pdata);

	clk_add_alias("master", "dm644x_ccdc", "master",
			&vpfe_capture_dev.dev);
	clk_add_alias("slave", "dm644x_ccdc", "slave",
			&vpfe_capture_dev.dev);

	/* DSS */
	am3517_evm_display_init();

	/* NAND */
	board_nand_init(am3517evm_nand_partitions,
					ARRAY_SIZE(am3517evm_nand_partitions), 0, NAND_BUSWIDTH_16);

	/* RTC - S35390A */
	am3517_evm_rtc_init();

	i2c_register_board_info(1, am3517evm_i2c1_boardinfo,
				ARRAY_SIZE(am3517evm_i2c1_boardinfo));
	/*Ethernet*/
	am3517_evm_ethernet_init(&am3517_evm_emac_pdata);

	/* MUSB */
	am3517_evm_musb_init();

	/* TSC 2004 */
	omap_mux_init_gpio(65, OMAP_PIN_INPUT_PULLUP);
	am3517evm_tsc_i2c_boardinfo[0].irq = gpio_to_irq(GPIO_TSC2004_IRQ);
	i2c_register_board_info(1, am3517evm_tsc_i2c_boardinfo,
				ARRAY_SIZE(am3517evm_tsc_i2c_boardinfo));

	/* Init TCA6416 keypad */
	tca6416_keypad_init_irq();

	/* MMC init function */
	omap2_hsmmc_init(mmc);

	/* NOR Flash on Application board */
	am3517_nor_init();
}

MACHINE_START(OMAP3517EVM, "OMAP3517/AM3517 EVM")
	.boot_params	= 0x80000100,
	.map_io		= omap3_map_io,
	.reserve	= omap_reserve,
	.init_irq	= am3517_evm_init_irq,
	.init_machine	= am3517_evm_init,
	.timer		= &omap_timer,
MACHINE_END
