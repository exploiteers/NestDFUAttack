/*
 * Code for TI8148 EVM.
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
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>

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
#include <plat/gpmc.h>
#include <plat/nand.h>

#include "board-flash.h"
#include "clock.h"
#include "hsmmc.h"


static struct omap2_hsmmc_info mmc[] = {
       {
               .mmc            = 1,
               .caps           = MMC_CAP_4_BIT_DATA,
               .gpio_cd        = -EINVAL,/* Dedicated pins for CD and WP */
               .gpio_wp        = -EINVAL,
               .ocr_mask       = MMC_VDD_33_34,
       },
       {}      /* Terminator */
};

static struct at24_platform_data eeprom_info = {
	.byte_len       = (256*1024) / 8,
	.page_size      = 64,
	.flags          = AT24_FLAG_ADDR16,
};

static struct i2c_board_info __initdata ti814x_i2c_boardinfo[] = {
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

static void __init ti814x_evm_i2c_init(void)
{
	/* There are 4 instances of I2C in TI814X but currently only one
	 * instance is being used on the TI8148 EVM
	 */
	omap_register_i2c_bus(1, 100, ti814x_i2c_boardinfo,
				ARRAY_SIZE(ti814x_i2c_boardinfo));
}

static u8 ti8148_iis_serializer_direction[] = {
	TX_MODE,	RX_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
};

static struct snd_platform_data ti8148_evm_snd_data = {
	.tx_dma_offset	= 0x46800000,
	.rx_dma_offset	= 0x46800000,
	.op_mode	= DAVINCI_MCASP_IIS_MODE,
	.num_serializer = ARRAY_SIZE(ti8148_iis_serializer_direction),
	.tdm_slots	= 2,
	.serial_dir	= ti8148_iis_serializer_direction,
	.asp_chan_q	= EVENTQ_2,
	.version	= MCASP_VERSION_2,
	.txnumevt	= 1,
	.rxnumevt	= 1,
};

/* NAND flash information */
static struct mtd_partition ti814x_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
	{
		.name           = "U-Boot-min",
		.offset         = 0,    /* Offset = 0x0 */
		.size           = SZ_128K,
		.mask_flags     = MTD_WRITEABLE,        /* force read-only */
	},
	{
		.name           = "U-Boot",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x0 + 128K */
		.size           = 18 * SZ_128K,
		.mask_flags     = MTD_WRITEABLE,        /* force read-only */
	},
	{
		.name           = "U-Boot Env",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x260000 */
		.size           = 1 * SZ_128K,
	},
	{
		.name           = "Kernel",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x280000 */
		.size           = 34 * SZ_128K,
	},
	{
		.name           = "File System",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x6C0000 */
		.size           = 1601 * SZ_128K,
	},
	{	
		.name           = "Reserved",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0xCEE0000 */
		.size           = MTDPART_SIZ_FULL,
	},
};

/* SPI fLash information */
struct mtd_partition ti8148_spi_partitions[] = {
	/* All the partition sizes are listed in terms of erase size */
	{
		.name		= "U-Boot-min",
		.offset		= 0,	/* Offset = 0x0 */
		.size		= 32 * SZ_4K,
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "U-Boot",
		.offset		= MTDPART_OFS_APPEND, /* Offset = 0x0 + (32*SZ_4K) */
		.size		= 64 * SZ_4K,
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "U-Boot Env",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x40000 + (32*SZ_4K) */
		.size		= 2 * SZ_4K,
	},
	{
		.name		= "Kernel",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x42000 + (32*SZ_4K) */
		.size		= 640 * SZ_4K,
	},
	{
		.name		= "File System",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x2C2000 + (32*SZ_4K) */
		.size		= MTDPART_SIZ_FULL,		/* size ~= 1.1 MiB */
	}
};

const struct flash_platform_data ti8148_spi_flash = {
	.type		= "w25x32",
	.name		= "spi_flash",
	.parts		= ti8148_spi_partitions,
	.nr_parts	= ARRAY_SIZE(ti8148_spi_partitions),
};

struct spi_board_info __initdata ti8148_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &ti8148_spi_flash,
		.irq		= -1,
		.max_speed_hz	= 75000000,
		.bus_num	= 1,
		.chip_select	= 0,
	},
};

void ti8148_spi_init(void)
{
	spi_register_board_info(ti8148_spi_slave_info,
				ARRAY_SIZE(ti8148_spi_slave_info));
}

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

static struct platform_device vpss_device = {
	.name = "vpss",
	.id = -1,
	.dev = {
		.platform_data = NULL,
	},
};
static void __init ti8148_evm_init_irq(void)
{
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
	omap_init_irq();
	gpmc_init();
}

static void __init ti814x_vpss_init(void)
{
	/*FIXME add platform data here*/

	if (platform_device_register(&vpss_device))
		printk(KERN_ERR "failed to register ti814x_vpss device\n");
	else
		printk(KERN_INFO "registered ti814x_vpss device\n");
}

static void __init ti8148_evm_init(void)
{
	omap_serial_init();
	ti814x_evm_i2c_init();
	ti81xx_register_mcasp(0, &ti8148_evm_snd_data);

	omap2_hsmmc_init(mmc);
	board_nand_init(ti814x_nand_partitions,
		ARRAY_SIZE(ti814x_nand_partitions), 0, NAND_BUSWIDTH_16);
	/* initialize usb */
	usb_musb_init(&musb_board_data);

	ti8148_spi_init();
	ti814x_vpss_init();
}

static void __init ti8148_evm_map_io(void)
{
	omap2_set_globals_ti814x();
	ti81xx_map_common_io();
}

MACHINE_START(TI8148EVM, "ti8148evm")
	/* Maintainer: Texas Instruments */
	.boot_params	= 0x80000100,
	.map_io		= ti8148_evm_map_io,
	.reserve         = ti81xx_reserve,
	.init_irq	= ti8148_evm_init_irq,
	.init_machine	= ti8148_evm_init,
	.timer		= &omap_timer,
MACHINE_END
