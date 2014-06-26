/*
 * Code for TI8168 EVM.
 *
 * Copyright (C) 2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c/at24.h>
#include <linux/device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/device.h>
#include <linux/mtd/nand.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/mtd/physmap.h>
#include <linux/phy.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/mcspi.h>
#include <plat/irqs.h>
#include <plat/board.h>
#include <plat/common.h>
#include <plat/asp.h>
#include <plat/usb.h>
#include <plat/mmc.h>
#include <plat/gpio.h>
#include <plat/gpmc.h>
#include <plat/nand.h>

#include "clock.h"
#include "mux.h"
#include "hsmmc.h"
#include "board-flash.h"
#include <mach/board-ti816x.h>

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.caps           = MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,/* Dedicated pins for CD and WP */
		.gpio_wp	= -EINVAL,
		.ocr_mask	= MMC_VDD_33_34,
	},
	{}	/* Terminator */
};

static struct mtd_partition ti816x_evm_norflash_partitions[] = {
	/* bootloader (U-Boot, etc) in first 5 sectors */
	{
		.name		= "bootloader",
		.offset		= 0,
		.size		= 2 * SZ_128K,
		.mask_flags	= MTD_WRITEABLE, /* force read-only */
	},
	/* bootloader params in the next 1 sectors */
	{
		.name		= "env",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_128K,
		.mask_flags	= 0,
	},
	/* kernel */
	{
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 2 * SZ_2M,
		.mask_flags	= 0
	},
	/* file system */
	{
		.name		= "filesystem",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 25 * SZ_2M,
		.mask_flags	= 0
	},
	/* reserved */
	{
		.name		= "reserved",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
		.mask_flags	= 0
	}
};


/* Maximum ports supported by the PCF8575 */
#define VPS_PCF8575_MAX_PORTS           (2u)

/* Macros for accessing for ports */
#define VPS_PCF8575_PORT0               (0u)
#define VPS_PCF8575_PORT1               (1u)

/* Macros for PCF8575 Pins */
#define VPS_PCF8575_PIN0                (0x1)
#define VPS_PCF8575_PIN1                (0x2)
#define VPS_PCF8575_PIN2                (0x4)
#define VPS_PCF8575_PIN3                (0x8)
#define VPS_PCF8575_PIN4                (0x10)
#define VPS_PCF8575_PIN5                (0x20)
#define VPS_PCF8575_PIN6                (0x40)
#define VPS_PCF8575_PIN7                (0x80)

#define VPS_PCF8575_PIN10               (0x1)
#define VPS_PCF8575_PIN11               (0x2)

#define VPS_THS7375_MASK                (VPS_PCF8575_PIN10 | VPS_PCF8575_PIN11)

#define VPS_THS7360_SD_MASK             (VPS_PCF8575_PIN2 | VPS_PCF8575_PIN5)

#define VPS_THS7360_SF_MASK             (VPS_PCF8575_PIN0 |                    \
                                         VPS_PCF8575_PIN1 |                    \
                                         VPS_PCF8575_PIN3 |                    \
                                         VPS_PCF8575_PIN4)

#define NAND_BLOCK_SIZE                SZ_128K

static struct mtd_partition ti816x_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
	{
		.name           = "U-Boot",
		.offset         = 0,    /* Offset = 0x0 */
		.size           = 19 * NAND_BLOCK_SIZE,
		.mask_flags     = MTD_WRITEABLE,        /* force read-only */
	},
	{
		.name           = "U-Boot Env",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x260000 */
		.size           = 1 * NAND_BLOCK_SIZE,
	},
	{
		.name           = "Kernel",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x280000 */
		.size           = 34 * NAND_BLOCK_SIZE,
	},
	{
		.name           = "File System",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x6C0000 */
		.size           = 1601 * NAND_BLOCK_SIZE,
	},
	{
		.name           = "Reserved",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0xCEE0000 */
		.size           = MTDPART_SIZ_FULL,
	},
};

static struct at24_platform_data eeprom_info = {
	.byte_len       = (256*1024) / 8,
	.page_size      = 64,
	.flags          = AT24_FLAG_ADDR16,
};

static struct i2c_board_info __initdata ti816x_i2c_boardinfo0[] = {
	{
		I2C_BOARD_INFO("eeprom", 0x50),
		.platform_data	= &eeprom_info,
	},
	{
		I2C_BOARD_INFO("cpld", 0x23),
	},
	{
		I2C_BOARD_INFO("tlv320aic3x", 0x18),
	},
	{
		I2C_BOARD_INFO("IO Expander", 0x20),
	},

};

static struct i2c_board_info __initdata ti816x_i2c_boardinfo1[] = {
        {
		I2C_BOARD_INFO("pcf8575_1", 0x20),
	}
};


static struct i2c_client *pcf8575_1_client;
static unsigned char pcf8575_port[2] = {0, 0};

int pcf8575_ths7375_enable(enum ti816x_ths_filter_ctrl ctrl)
{
	struct i2c_msg msg = {
			.addr = pcf8575_1_client->addr,
			.flags = 0,
			.len = 2,
		};

	pcf8575_port[1] &= ~VPS_THS7375_MASK;
	pcf8575_port[1] |= (ctrl & VPS_THS7375_MASK);
	msg.buf = pcf8575_port;

	return (i2c_transfer(pcf8575_1_client->adapter, &msg, 1));
}
EXPORT_SYMBOL(pcf8575_ths7375_enable);

int pcf8575_ths7360_sd_enable(enum ti816x_ths_filter_ctrl ctrl)
{
	struct i2c_msg msg = {
		.addr = pcf8575_1_client->addr,
		.flags = 0,
		.len = 2,
	};

	pcf8575_port[0] &= ~VPS_THS7360_SD_MASK;
	switch (ctrl)
	{
		case TI816X_THSFILTER_ENABLE_MODULE:
			pcf8575_port[0] &= ~(VPS_THS7360_SD_MASK);
			break;
		case TI816X_THSFILTER_BYPASS_MODULE:
			pcf8575_port[0] |= VPS_PCF8575_PIN2;
			break;
		case TI816X_THSFILTER_DISABLE_MODULE:
			pcf8575_port[0] |= VPS_THS7360_SD_MASK;
			break;
		default:
			return -EINVAL;
	}

	msg.buf = pcf8575_port;

	return (i2c_transfer(pcf8575_1_client->adapter, &msg, 1));
}
EXPORT_SYMBOL(pcf8575_ths7360_sd_enable);

int pcf8575_ths7360_hd_enable(enum ti816x_ths7360_sf_ctrl ctrl)
{
	struct i2c_msg msg = {
		.addr = pcf8575_1_client->addr,
		.flags = 0,
		.len = 2,
	};

	pcf8575_port[0] &= ~VPS_THS7360_SF_MASK;

	switch(ctrl)
	{
		case TI816X_THS7360_DISABLE_SF:
			pcf8575_port[0] |= VPS_PCF8575_PIN4;
			break;
		case TI816X_THS7360_BYPASS_SF:
			pcf8575_port[0] |= VPS_PCF8575_PIN3;
			break;
		case TI816X_THS7360_SF_SD_MODE:
			pcf8575_port[0] &= ~(VPS_THS7360_SF_MASK);
			break;
		case TI816X_THS7360_SF_ED_MODE:
			pcf8575_port[0] |= VPS_PCF8575_PIN0;
			break;
		case TI816X_THS7360_SF_HD_MODE:
			pcf8575_port[0] |= VPS_PCF8575_PIN1;
			break;
		case TI816X_THS7360_SF_TRUE_HD_MODE:
			pcf8575_port[0] |= VPS_PCF8575_PIN0|VPS_PCF8575_PIN1;
			break;
		default:
			return -EINVAL;
	}

	msg.buf = pcf8575_port;

	return (i2c_transfer(pcf8575_1_client->adapter, &msg, 1));
}
EXPORT_SYMBOL(pcf8575_ths7360_hd_enable);

static int pcf8575_video_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	pcf8575_1_client = client;
	return 0;
}

static int __devexit pcf8575_video_remove(struct i2c_client *client)
{
	pcf8575_1_client = NULL;
	return 0;
}

static const struct i2c_device_id pcf8575_video_id[] = {
        { "pcf8575_1", 0 },
        { }
};

static struct i2c_driver pcf8575_driver = {
        .driver = {
                .name   = "pcf8575_1",
        },
        .probe          = pcf8575_video_probe,
        .remove         = pcf8575_video_remove,
        .id_table       = pcf8575_video_id,
};


/* FIX ME: Check on the Bit Value */

#define TI816X_EVM_CIR_UART BIT(5)

static struct i2c_client *cpld_reg0_client;

/* CPLD Register 0 Client: used for I/O Control */
static int cpld_reg0_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	u8 data;
	struct i2c_msg msg[2] = {
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = &data,
		},
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &data,
		},
	};

	cpld_reg0_client = client;

	/* Clear UART CIR to enable cir operation. */
		i2c_transfer(client->adapter, msg, 1);
		data &= ~(TI816X_EVM_CIR_UART);
		i2c_transfer(client->adapter, msg + 1, 1);
	return 0;
}

static const struct i2c_device_id cpld_reg_ids[] = {
		{ "cpld_reg0", 0, },
		{ },
};

static struct i2c_driver ti816xevm_cpld_driver = {
	.driver.name    = "cpld_reg0",
	.id_table       = cpld_reg_ids,
	.probe          = cpld_reg0_probe,
};

static int __init ti816x_evm_i2c_init(void)
{
	omap_register_i2c_bus(1, 100, ti816x_i2c_boardinfo0,
		ARRAY_SIZE(ti816x_i2c_boardinfo0));
	omap_register_i2c_bus(2, 100, ti816x_i2c_boardinfo1,
		ARRAY_SIZE(ti816x_i2c_boardinfo1));

	return 0;
}

/* SPI fLash information */
struct mtd_partition ti816x_spi_partitions[] = {
	/* All the partition sizes are listed in terms of NAND block size */
	{
		.name		= "U-Boot",
		.offset		= 0,	/* Offset = 0x0 */
		.size		= 64 * SZ_4K,
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "U-Boot Env",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x40000 */
		.size		= 2 * SZ_4K,
	},
	{
		.name		= "Kernel",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x42000 */
		.size		= 640 * SZ_4K,
	},
	{
		.name		= "File System",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x2C2000 */
		.size		= MTDPART_SIZ_FULL,		/* size = 1.24 MiB */
	}
};

const struct flash_platform_data ti816x_spi_flash = {
	.type		= "w25x32",
	.name		= "spi_flash",
	.parts		= ti816x_spi_partitions,
	.nr_parts	= ARRAY_SIZE(ti816x_spi_partitions),
};

struct spi_board_info __initdata ti816x_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &ti816x_spi_flash,
		.irq		= -1,
		.max_speed_hz	= 75000000,
		.bus_num	= 1,
		.chip_select	= 0,
	},
};

static void __init ti816x_spi_init(void)
{
	spi_register_board_info(ti816x_spi_slave_info,
				ARRAY_SIZE(ti816x_spi_slave_info));
}


static void __init ti8168_evm_init_irq(void)
{
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
	omap_init_irq();
	gpmc_init();
}

static u8 ti8168_iis_serializer_direction[] = {
	TX_MODE,	RX_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
};

static struct snd_platform_data ti8168_evm_snd_data = {
	.tx_dma_offset	= 0x46800000,
	.rx_dma_offset	= 0x46800000,
	.op_mode	= DAVINCI_MCASP_IIS_MODE,
	.num_serializer = ARRAY_SIZE(ti8168_iis_serializer_direction),
	.tdm_slots	= 2,
	.serial_dir	= ti8168_iis_serializer_direction,
	.asp_chan_q	= EVENTQ_2,
	.version	= MCASP_VERSION_2,
	.txnumevt	= 1,
	.rxnumevt	= 1,
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
#ifdef CONFIG_USB_MUSB_OTG
	.mode           = MUSB_OTG,
#elif defined(CONFIG_USB_MUSB_HDRC_HCD)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_USB_GADGET_MUSB_HDRC)
	.mode           = MUSB_PERIPHERAL,
#endif
	.power			= 500,
	.instances              = 1,
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {

	/* PIN mux for non-muxed NOR */
	TI816X_MUX(TIM7_OUT, OMAP_MUX_MODE1),	/* gpmc_a12 */
	TI816X_MUX(UART1_CTSN, OMAP_MUX_MODE1),	/* gpmc_a13 */
	TI816X_MUX(UART1_RTSN, OMAP_MUX_MODE1),	/* gpmc_a14 */
	TI816X_MUX(UART2_RTSN, OMAP_MUX_MODE1),	/* gpmc_a15 */
	TI816X_MUX(SC1_RST, OMAP_MUX_MODE1),	/* gpmc_a15 TODO why 2 gpmc_a15 mux setting? */
	TI816X_MUX(UART2_CTSN, OMAP_MUX_MODE1),	/* gpmc_a16 */
	TI816X_MUX(UART0_RIN, OMAP_MUX_MODE1),	/* gpmc_a17 */
	TI816X_MUX(UART0_DCDN, OMAP_MUX_MODE1),	/* gpmc_a18 */
	TI816X_MUX(UART0_DSRN, OMAP_MUX_MODE1),	/* gpmc_a19 */
	TI816X_MUX(UART0_DTRN, OMAP_MUX_MODE1),	/* gpmc_a20 */
	TI816X_MUX(SPI_SCS3, OMAP_MUX_MODE1),	/* gpmc_a21 */
	TI816X_MUX(SPI_SCS2, OMAP_MUX_MODE1),	/* gpmc_a22 */
	TI816X_MUX(GP0_IO6, OMAP_MUX_MODE2),	/* gpmc_a23 */
	TI816X_MUX(TIM6_OUT, OMAP_MUX_MODE1),	/* gpmc-a24 */
	TI816X_MUX(SC0_DATA, OMAP_MUX_MODE1),	/* gpmc_a25 */
	TI816X_MUX(GPMC_A27, OMAP_MUX_MODE1),	/* gpio-20 for controlling high address */
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#endif

int __init ti_ahci_register(u8 num_inst);
static struct platform_device vpss_device = {
	.name = "vpss",
	.id = -1,
	.dev = {
		.platform_data = NULL,
	},
};

static void __init ti816x_vpss_init(void)
{
	i2c_add_driver(&pcf8575_driver);

	if (platform_device_register(&vpss_device))
		printk(KERN_ERR "failed to register ti816x_vpss device\n");
	else
		printk(KERN_INFO "registered ti816x_vpss device \n");
	/*FIXME add platform data here*/
}



static void __init ti8168_evm_init(void)
{
	ti81xx_mux_init(board_mux);
	omap_serial_init();
	ti816x_evm_i2c_init();
	i2c_add_driver(&ti816xevm_cpld_driver);
	ti81xx_register_mcasp(0, &ti8168_evm_snd_data);
	ti816x_spi_init();
	/* initialize usb */
	usb_musb_init(&musb_board_data);
	board_nand_init(ti816x_nand_partitions,
		ARRAY_SIZE(ti816x_nand_partitions), 0, NAND_BUSWIDTH_16);
	omap2_hsmmc_init(mmc);
	board_nor_init(ti816x_evm_norflash_partitions,
		ARRAY_SIZE(ti816x_evm_norflash_partitions), 0);
	ti816x_vpss_init();
}

static int __init ti8168_evm_gpio_setup(void)
{
	/* GPIO-20 should be low for NOR access beyond 4KiB */
	gpio_request(20, "nor");
	gpio_direction_output(20, 0x0);
	return 0;
}
/* GPIO setup should be as subsys_initcall() as gpio driver
 * is registered in arch_initcall()
 */
subsys_initcall(ti8168_evm_gpio_setup);

static void __init ti8168_evm_map_io(void)
{
	omap2_set_globals_ti816x();
	ti81xx_map_common_io();
}

MACHINE_START(TI8168EVM, "ti8168evm")
	/* Maintainer: Texas Instruments */
	.boot_params	= 0x80000100,
	.map_io		= ti8168_evm_map_io,
	.reserve         = ti81xx_reserve,
	.init_irq	= ti8168_evm_init_irq,
	.init_machine	= ti8168_evm_init,
	.timer		= &omap_timer,
MACHINE_END
