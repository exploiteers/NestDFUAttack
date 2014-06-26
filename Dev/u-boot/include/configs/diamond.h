/*
 *    Copyright (c) 2010-2012 Nest Labs, Inc.
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
 *      This file defines the u-boot configuration settings for the
 *      Nest Learning Thermostat board.
 *
 *      The file was originally derived from omap3evm.h.
 *
 *      SDRAM Controller (SDRC):
 *        - Samsung K4X51163PI-FCG6 64 MiB DDR SDRAM
 *        - Nanya NTD6M 512MB LPDDR SDRAM
 *
 *      General Purpose Memory Controller (GPMC):
 *        - Micron MT29F2G16ABDHC-ET 256 MiB SLC NAND Flash (Development)
 *        - Micron MT29F2G16ABBEAH4 256 MiB SLC NAND Flash (Non-development)
 *        - Samsung K9F4G08U0C 512 MiB SLC NAND Flash (Development)
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development)
 *
 *      Multimedia Card (MMC) Interface:
 *        - Molex 500998-0900 Secure Digital (SD) Connector
 *        - TI WL1270 Wireless LAN (WLAN) Module
 *
 *      Display Subsystem (DSS):
 *        - Samsung LMS350DF03 HVGA (320 x 480) LCM (Development and Prototype)
 *        - Tianma TM025ZDZ01 (320 x 320) LCM (Production)
 *
 *      Camera Image Signal Processor:
 *        - Unused
 *
 *      Inter-Integrated Circuit (I2C) Controller:
 *        - Channel 1 (Bus 0):
 *          * Texas Instruments TPS65921 Power Management Unit (PMU)
 *
 *        - Channel 2 (Bus 1):
 *        - Si1143 Proximity / Ambient Light Sensor (Development)
 *
 *        - Channel 3 (Bus 2):
 *          * Unused
 *
 *        - Channel 4 (Bus 3):
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
 *        - Channel 2:
 *        - Channel 3:
 *        - Channel 4:
 *        - Channel 5:
 *
 *      Multichannel Serial Peripheral Interface (McSPI):
 *        - Channel 1:
 *        - Channel 2:
 *        - Channel 3:
 *
 *      Interrupt Controller:
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development)
 *        - Si1143 Proximity / Ambient Light Sensor (Development)
 *
 *      GPIO Controller:
 *        - Backplate Detect
 *        - ZigBee
 */

#ifndef __DIAMOND_CONFIG_H
#define __DIAMOND_CONFIG_H

/*
 * ARM Processor Architecture, Family and Product Configuration
 */

#define CONFIG_ARMV7			1
#define CONFIG_OMAP             1
#define CONFIG_OMAP3			1
#define CONFIG_OMAP34XX         1
#define CONFIG_OMAP3430         1
#define	CONFIG_OMAP37XX			1
#define	CONFIG_OMAP3730			1

#define CONFIG_SDRC		1	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>	/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * Assert the board type(s) we are building for.
 *
 * Multiple board support occurs dynamically by detection and parsing
 * of the 'nlModel' environment key/value pair (see board/nest/common/
 * nlmodel.{h,c}).
 */
#undef CONFIG_DIAMOND_BOARD_DEVELOPMENT
#undef CONFIG_DIAMOND_BOARD_PROTOTYPE
#undef CONFIG_DIAMOND_BOARD_EVT
#undef CONFIG_DIAMOND_BOARD_DVT
#undef CONFIG_DIAMOND_BOARD_PVT
#undef CONFIG_DIAMOND_BOARD_MP
#define CONFIG_DIAMOND_BOARD_MULTI	1

#undef CONFIG_J49_BOARD_PROTOTYPE
#undef CONFIG_J49_BOARD_EVT
#undef CONFIG_J49_BOARD_DVT
#undef CONFIG_J49_BOARD_PRE_PVT
#undef CONFIG_J49_BOARD_PVT
#define CONFIG_J49_BOARD_MULTI		1

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#undef CONFIG_DISPLAY_BOARDINFO

/*
 * Clock Definitions
 */

#define V_OSCK							26000000	/* Clock output from T2 */
#define V_SCLK							(V_OSCK >> 1)

#define	CONFIG_SYS_MPU_DPLL_300MHZ		1
#undef	CONFIG_SYS_MPU_DPLL_600MHZ

#define	CONFIG_SYS_CORE_DPLL_332MHZ		1
#undef	CONFIG_SYS_CORE_DPLL_200MHZ
#undef	CONFIG_SYS_CORE_DPLL_400MHZ

#undef CONFIG_USE_IRQ			/* no support for IRQs */

/*
 * The following set of options controls how information gets passed
 * from U-Boot to Linux as a set of tagged records passed in memory.
 */

/*
 * CONFIG_CMDLINE_TAG - This determines whether the U-Boot environment
 * value associated with the 'bootargs' key is tagged and passed.
 */
#define CONFIG_CMDLINE_TAG				1

/*
 * CONFIG_SETUP_MEMORY_TAGS - This determines whether the DRAM memory
 * extents probed and setup by U-Boot are tagged and passed.
 */
#define CONFIG_SETUP_MEMORY_TAGS		1

/*
 * CONFIG_INITRD_TAG - This determines whether an initial RAM disk
 * image extents are tagged and passed.
 */
#define CONFIG_INITRD_TAG				1

/*
 * CONFIG_REVISION_TAG - This determines whether the board revision
 * is tagged and passed.
 */
#define CONFIG_REVISION_TAG				1

/*
 * CONFIG_SERIAL_TAG - This determines whether the board serial number
 * is tagged and passed.
 */
#define CONFIG_SERIAL_TAG				1

#define CONFIG_SYS_GBL_DATA_SIZE	128	/* bytes reserved for */
						/* initial data */
/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * Initialization Options (from lib_arm/board.c)
 */
#define CONFIG_MISC_INIT_R          1       // Do call misc_init_r()
#undef	CONFIG_BOARD_LATE_INIT				// Do not call board_late_init()

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX				1
#define CONFIG_SYS_NS16550_COM1			OMAP34XX_UART1
#define CONFIG_SERIAL1					1	/* UART1 on OMAP3 */

#define CONFIG_BAUDRATE					115200
#define CONFIG_SYS_BAUDRATE_TABLE		{ 4800, 9600, 19200, 38400, 57600, \
										115200}
/*
 * SD/MMC Support Configuration
 */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC				1
#define CONFIG_OMAP_HSMMC		1
#define CONFIG_DOS_PARTITION	1

/* USB
 * Enable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_MUSB_UDD for Device functionalities.
 */
#undef  CONFIG_USB_OMAP3
#undef  CONFIG_MUSB_HCD
#undef  CONFIG_MUSB_UDC

#ifdef CONFIG_USB_OMAP3

#ifdef CONFIG_MUSB_HCD

#define CONFIG_USB_STORAGE

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_SYS_USB_EVENT_POLL
#define CONFIG_PREBOOT "usb start"
#endif /* CONFIG_USB_KEYBOARD */

#endif /* CONFIG_MUSB_HCD */

#ifdef CONFIG_MUSB_UDC
/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x2464
#define CONFIG_USBD_PRODUCTID		0x0001
#define CONFIG_USBD_MANUFACTURER	"Nest Labs, Inc."
#define CONFIG_USBD_PRODUCT_NAME	"Nest Learning Thermostat"
#endif /* CONFIG_MUSB_UDC */

#endif /* CONFIG_USB_OMAP3 */

/*
 * NOR Flash Driver Configuration Options
 */

/*
 * The Nest Learning Thermostat board contains no NOR flash devices.
 */
#define CONFIG_SYS_NO_FLASH				1

/*
 * NAND Flash Driver Configuration Options
 */

/*
 * The Nest Diamond development board contains two (2) NAND flash
 * devices. The first, the Micron MT29F2G16ABDHC-ET or MT29F2G16ABBEAH4
 * 256 MiB SLC at GPMC chip select 0, and the second, the Samsung
 * K9F4G08U0C 512 MiB SLC at GPMC chip select 3.
 *
 * XXX - TI's version of U-Boot doesn't seem to have a very flexible GPMC
 * configuration (makes all sorts of assupmtions about what should and
 * should not be at a given GPMC address). This probably needs to be
 * revisited or configured in board-specific way in our board
 * file. The implication is we'll probably only get CS0 configured on
 * our behalf.
 */

#define	CONFIG_SYS_NAND_CS0_BASE		NAND_BASE
#define CONFIG_SYS_NAND_CS3_BASE		ONENAND_MAP

#define	CONFIG_SYS_NAND_MT29F2G16		// Micron MT29F2G16
#undef	CONFIG_SYS_NAND_K9F4G08U0C		// Samsung K9F4G08U0C

#if defined(CONFIG_SYS_NAND_MT29F2G16) && defined(CONFIG_SYS_NAND_K9F4G08U0C)
# define CONFIG_SYS_MAX_NAND_DEVICE		2
# define CONFIG_SYS_NAND_BASE_LIST		{ CONFIG_SYS_NAND_CS0_BASE, \
										  CONFIG_SYS_NAND_CS3_BASE }
# define GPMC_NAND_ECC_LP_x16_LAYOUT	1
#elif defined(CONFIG_SYS_NAND_MT29F2G16)
# define CONFIG_SYS_MAX_NAND_DEVICE 	1
# define CONFIG_SYS_NAND_BASE_LIST		{ CONFIG_SYS_NAND_CS0_BASE }
# define GPMC_NAND_ECC_LP_x16_LAYOUT	1
#elif defined(CONFIG_SYS_NAND_K9F4G08U0C)
# define CONFIG_SYS_MAX_NAND_DEVICE 	1
# define CONFIG_SYS_NAND_BASE_LIST		{ CONFIG_SYS_NAND_CS3_BASE }
# define GPMC_NAND_ECC_LP_x8_LAYOUT		1
#else
# define CONFIG_SYS_MAX_NAND_DEVICE		0
#endif /* defined(CONFIG_SYS_NAND_MT29F2G16) && defined(CONFIG_SYS_NAND_K9F4G08U0C) */

/*
 * The Micron MT29F2G16ABDHC-ET or MT29F2G16ABBEAH4 256 MiB SLC NAND
 * flash are designated as the boot NAND because it has the type of
 * ECC supported by the AM3703 boot ROM. The parameters below are those
 * corresponding to these Micron parts.
 */

#define	CONFIG_SYS_NAND_PAGE_SIZE		(2 << 10)
#define	CONFIG_SYS_NAND_BLOCK_SIZE		(128 << 10)

/*
 * Add support for the command line interpreter command 'nandecc' that
 * allows switching between hardware- and software-based ECC for NAND
 * accesses. Hardware ECC is required when reading and writing the
 * initial program loader (IPL) image read by the ARM's boot ROM.
 */
#define CONFIG_NAND_OMAP_GPMC			1

/*
 * I2C Driver Configuration
 */

#define CONFIG_HARD_I2C					1
#define CONFIG_SYS_I2C_SPEED			100000
#define CONFIG_SYS_I2C_SLAVE			1
#define CONFIG_SYS_I2C_BUS				0
#define CONFIG_SYS_I2C_BUS_SELECT		1
#define CONFIG_DRIVER_OMAP34XX_I2C		1

/*
 * CONFIG_I2C_MULTI_BUS - When this is defined, it indicates that the
 * system supports multiple I2C channels/buses and adds the ability
 * for the I2C interactive commands to support switching from one bus
 * to another.
 */
#define CONFIG_I2C_MULTI_BUS			1

/*
 * Power Management Unit Configuration
 */
#define CONFIG_TWL4030_POWER		1

/*
 * GPIO Driver Configuration
 */
#define CONFIG_OMAP_GPIO			1

/*
 * U-Boot Environment Variable Configuration Options
 */

/*
 * CONFIG_PREBOOT - When this option is defined, the existence of the
 * environment variable "preboot" will be checked and the contents of
 * it executed immediately before starting the CONFIG_BOOTDELAY
 * countdown and/or running the auto-boot command.
 */
#ifdef AUPD_LOAD_ADDRESS
#define CONFIG_PREBOOT
#else
#undef CONFIG_PREBOOT
#endif

/*
 * CONFIG_BOOTARGS - This can be used to pass arguments to the bootm
 * command. The value of CONFIG_BOOTARGS goes into the environment
 * value "bootargs".
 */
#undef	CONFIG_BOOTARGS

/*
 * CONFIG_BOOTFILE - This can be used to set a default value for the
 * "bootfile" environment variable. It should be specified in
 * non-string form as it will be "stringified" by U-Boot's build
 * system, where necessary.
 */
#define CONFIG_BOOTFILE						"uImage"

/*
 * CONFIG_EXTRA_ENV_SETTINGS - Define this to contain any number of
 * NULL-terminated strings (variable = value pairs) that will be part
 * of the default environment compiled into the boot image.
 *
 * For example:
 *
 *      #define CONFIG_EXTRA_ENV_SETTINGS		\
 *          "myvar1=value1\0"					\
 *          "myvar2=value2\0"
 */

/*
 * Set Nest model configuration
 */
#define CONFIG_SYS_MODEL_ENV_SETTINGS									\
	"addmodel=setenv bootargs ${bootargs} nlmodel=${nlmodel}\0"

/*
 * Set Linux serial console device and options
 */
#define	CONFIG_SYS_TTY_ENV_SETTINGS										\
	"addtty=setenv bootargs ${bootargs} console=ttyO0,${baudrate}\0"

/*
 * Set Linux root file system device
 */
#define CONFIG_SYS_ROOT_ENV_SETTINGS(device)							\
	"setenv bootargs root=/dev/" #device

#define CONFIG_SYS_FS_ENV_SETTINGS(fs)									\
	"add" #fs "=setenv bootargs ${bootargs} rootfstype=" #fs "\0"

/*
 * Set Linux Ext3 root file system
 */
#define CONFIG_SYS_EXT3_ENV_SETTINGS		CONFIG_SYS_FS_ENV_SETTINGS(ext3)

/*
 * Set Linux JFFS2 root file system
 */
#define	CONFIG_SYS_JFFS2_ENV_SETTINGS		CONFIG_SYS_FS_ENV_SETTINGS(jffs2)

/*
 * Set Linux MMC/SD media boot / root file system
 */
#define CONFIG_SYS_MMCBOOTCOMMAND										\
		"run addtty addmodel && "						\
		"bootm 0x80a00000"



/*
 * Set Linux NFS boot / root file system
 */
#define CONFIG_NFSBOOTCOMMAND											\
		"run addtty addmodel && "						\
		"bootm 0x80a00000"

#define	CONFIG_SYS_NFS_ENV_SETTINGS										\
	" \0"														\

/*
 * Set Linux to suppress most kernel start-up console messages
 */
#define	CONFIG_SYS_QUIET_ENV_SETTINGS									\
	"addquiet=setenv bootargs ${bootargs} quiet\0"

/*
 * Set U-boot to suppress console messages (combines with
 * CONFIG_SILENT_CONSOLE)
 */
#ifdef NEST_BUILD_CONFIG_RELEASE
    #define CONFIG_SYS_SILENT        "silent=1\0"
#else
    #define CONFIG_SYS_SILENT        ""
#endif

/*
 * For release builds, panics should reboot after one second
 */
#ifdef NEST_BUILD_CONFIG_RELEASE
	#define CONFIG_PANIC		"1"
#endif

/*
 * Define NAND/JFFS2 boot environment variables for booting from boot
 * partition n.
 */
#define	CONFIG_SYS_NANDBOOTN_ENV_SETTINGS(n, offset, size, device)		\
		"run addtty addmodel && "							\
		"bootm 0x80a00000\0"


/*
 * Offset to and nominal (maybe bigger than actual) size of the
 * primary and secondary Linux kernel images in NAND.
 *
 *  -------------------------------------------------------------------------
 *                                                         Offsets
 *                                               ----------------------------
 *  Alias Description            Size    Blocks  Block     Address     Size
 *  =========================================================================
 *  boot0 Primary Linux Image    8 MiB     64      32    0x00400000     4 MiB
 *  -------------------------------------------------------------------------
 *  boot1 Secondary Linux Image  8 MiB     64     464    0x03A00000    58 MiB
 *  =========================================================================
 *
 *  Although we've reserved 8 MiB of space for kernel images, we only
 *  actually use 4 MiB of it; hence the definition below.
 */
#define	CONFIG_SYS_NANDBOOT0_ENV_OFF	"0x400000"
#define	CONFIG_SYS_NANDBOOT1_ENV_OFF	"0x3A00000"

#define	CONFIG_SYS_NANDBOOT_ENV_SIZE	"0x400000"

#define	CONFIG_SYS_NANDBOOT0_ENV_SIZE	CONFIG_SYS_NANDBOOT_ENV_SIZE
#define	CONFIG_SYS_NANDBOOT1_ENV_SIZE	CONFIG_SYS_NANDBOOT_ENV_SIZE

/*
 * MTD block device files from which the root JFFS2 primary or
 * secondary file systems will be mounted in Linux.
 */
#define	CONFIG_SYS_NANDBOOT0_ENV_DEV	mtdblock7
#define	CONFIG_SYS_NANDBOOT1_ENV_DEV	mtdblock9

/*
 * NAND/JFFS2 boot environment variables for booting from boot
 * partition 0.
 */
#define	CONFIG_SYS_NANDBOOT0_ENV_SETTINGS								\
	CONFIG_SYS_NANDBOOTN_ENV_SETTINGS(0,								\
						   CONFIG_SYS_NANDBOOT0_ENV_OFF,				\
						   CONFIG_SYS_NANDBOOT0_ENV_SIZE,				\
						   CONFIG_SYS_NANDBOOT0_ENV_DEV)

/*
 * NAND/JFFS2 boot environment variables for booting from boot
 * partition 1.
 */
#define	CONFIG_SYS_NANDBOOT1_ENV_SETTINGS								\
	CONFIG_SYS_NANDBOOTN_ENV_SETTINGS(1,								\
						   CONFIG_SYS_NANDBOOT1_ENV_OFF,				\
						   CONFIG_SYS_NANDBOOT1_ENV_SIZE,				\
						   CONFIG_SYS_NANDBOOT1_ENV_DEV)

/*
 * The default boot command
 */
#define CONFIG_BOOTCOMMAND												\
	"run nfsboot || reset"

/*
 * The alternate default boot command (see CONFIG_BOOTCOUNT_LIMIT)
 */
#define CONFIG_SYS_ALTBOOTCOMMAND					   					\
	"run nfsboot || reset"

/*
 * Default address at which to load images when referenced by
 * 'loadaddr' in the environment. This should PRECISELY match
 * CONFIG_SYS_LOAD_ADDR.
 *
 * There might be the temptation to base one on the other; however,
 * the way preprocessor stringification occurs, we end up with a
 * string "SDRAM_BASE + (16 << 20)" or some such if we do that.
 */
#define CONFIG_LOADADDR					0x81000000

/*
 * Default MAC address (ethaddr) and serial# definitions
 *
 * Defining CONFIG_ETHADDR and CONFIG_OVERWRITE_ETHADDR_ONCE along
 * with not defining CONFIG_ENV_OVERWRITE, allows us to overwrite the
 * Ethernet MAC from U-Boot if and only if it is currently the default
 * address.
 *
 * The default address was generated from U-Boots 'gen_eth_addr' found
 * in the 'tools' directory of the U-Boot build.
 */
#define CONFIG_ETHADDR			02:28:3e:3d:43:47

/*
 * Command to "reset" the U-Boot environment.
 *
 * Primary and secondary are contiguous so can both be erased at the same 
 * time.  There are six blocks total, two extra for each region as replacements
 * if blocks go bad.  Therefore to ensure the erase clears the environment data
 * in all cases the erase size is 0xc0000.
 */

#define CONFIG_SYS_PRIMARY_ENV_OFF		"0x340000"
#define CONFIG_SYS_ENV_ERASE_SIZE		"0xc0000"

#define CONFIG_SYS_ERASE_COMMAND(offset, size)						    \
	"nand erase " offset " " size

#define CONFIG_SYS_RESETENV_COMMAND								        \
	CONFIG_SYS_ERASE_COMMAND(CONFIG_SYS_PRIMARY_ENV_OFF,			    \
							 CONFIG_SYS_ENV_ERASE_SIZE)

#define CONFIG_SYS_RESETENV_ENV_SETTINGS								\
	"resetenv="															\
		CONFIG_SYS_RESETENV_COMMAND "\0"

#define	CONFIG_EXTRA_ENV_SETTINGS										\
	"altbootcmd=" CONFIG_SYS_ALTBOOTCOMMAND "\0"						\
	CONFIG_SYS_EXT3_ENV_SETTINGS										\
	CONFIG_SYS_MODEL_ENV_SETTINGS										\
	CONFIG_SYS_NFS_ENV_SETTINGS		   									\
	CONFIG_SYS_QUIET_ENV_SETTINGS										\
	CONFIG_SYS_TTY_ENV_SETTINGS											\
	CONFIG_SYS_JFFS2_ENV_SETTINGS										\
	CONFIG_SYS_NANDBOOT0_ENV_SETTINGS									\
	CONFIG_SYS_NANDBOOT1_ENV_SETTINGS									\
	CONFIG_SYS_RESETENV_ENV_SETTINGS									\
	CONFIG_SYS_SILENT													\
	""

/*
 * CONFIG_VERSION_VARIABLE - Include a read-only variable 'ver' in the
 * environment containing the same contents reported by the 'version'
 * command.
 */
#undef 	CONFIG_VERSION_VARIABLE

/*
 * Autoboot Configuration Options
 *
 * See README and doc/README.autoboot for more information.
 */

/*
 * CONFIG_BOOTDELAY - Countdown <n> seconds, waiting for console
 * input, until automatically running 'bootcmd'. Modified through the
 * environment variable 'bootdelay'.  String define makes it easy
 * to set the environment variable as needed.
 */
#ifdef NEST_BUILD_CONFIG_RELEASE
    #define CONFIG_BOOTDELAY				5
    #define CONFIG_BOOTDELAY_STR            "5"
#else
    #define CONFIG_BOOTDELAY				1
    #define CONFIG_BOOTDELAY_STR            "1"
#endif



/*
 * CONFIG_ZERO_BOOTDELAY_CHECK - Check for console input to stop the
 * auto boot process even when the environment variable 'bootdelay' is
 * set to 0.
 */
#ifndef NEST_BUILD_CONFIG_RELEASE
    #define CONFIG_ZERO_BOOTDELAY_CHECK
#endif

/*
 * CONFIG_BOOTCOUNT_LIMIT - Use the 'bootcount' and 'bootlimit'
 * environment variables to try 'bootcmd' 'bootlimit' times before
 * "failing over" and resorting to 'altbootcmd'.
 *
 */
#undef  CONFIG_BOOTCOUNT_LIMIT

/*
 * CONFIG_SILENT_CONSOLE - quiet messages on the console.
 * See doc/README.silent for more information.
 */
#ifdef NEST_BUILD_CONFIG_RELEASE
    #define CONFIG_SILENT_CONSOLE
#endif

/*
 * Command line Configuration Options
 */
#include <config_cmd_default.h>	// Enable default commands

/*
 * The default list of commands includes:
 *
 *   CONFIG_CMD_BDI - Provides the 'bdinfo' command which displays some
 *   basic configuration information about the board.
 *
 *   CONFIG_CMD_BOOTD - Provides legacy support for the 'bootd' (boot
 *   default) command.
 *
 *   CONFIG_CMD_CONSOLE - Provides the 'coninfo' command which
 *   displays some basic information about the input/output console.
 *
 *   CONFIG_CMD_ECHO - Provides the 'echo' command.
 *
 *   CONFIG_CMD_EDITENV - Provide the ability to edit an environment
 *   variable.
 *
 *   CONFIG_CMD_FLASH - Provides NOR flash commands (erase, lock, etc.).
 *
 *   CONFIG_CMD_FPGA - Provides FPGA device initialization commands.
 *
 *   CONFIG_CMD_IMI - Provides 'iminfo' image valiation command.
 *
 *   CONFIG_CMD_IMLS - Provides support for probing and listing images.
 *
 *   CONFIG_CMD_ITEST - Provides the 'test' shell command.
 *
 *   CONFIG_CMD_LOADB - Provides the 'loadb' binary booting/loading
 *   over serial command.
 *
 *   CONFIG_CMD_LOADS - Provides the 'loads' S-Record booting/loading
 *   over serial command.
 *
 *   CONFIG_CMD_MEMORY - Provides memory read/modify/write commands.
 *
 *   CONFIG_CMD_MISC - Provides support for commands like sleep.
 *
 *   CONFIG_CMD_NET - Provides networking commands (bootp, tftp, rarp, etc.).
 *
 *   CONFIG_CMD_NFS - Provides network booting/loading over NFS.
 *
 *   CONFIG_CMD_RUN - Provides support for running scripts.
 *
 *   CONFIG_CMD_SAVEENV - Provides the ability to write back
 *   environment changes to non-volatile storage.
 *
 *   CONFIG_CMD_SETGETDCR - Provides PowerPC set/get Device Control
 *   Register (DCR) commands.
 *
 *   CONFIG_CMD_SOURCE - Provides the ability to "source" u-boot
 *   environment scripts in a fashion similar to UNIX shell
 *   interpreters.
 *
 *   CONFIG_CMD_XIMG - Provides support for extracting the specified
 *   part of a multi-image.
 *
 */

/*
 * Command line Configuration Overrides
 */
#define	CONFIG_CMD_AUTOSCRIPT	// Software updates performed via other means
#undef	CONFIG_CMD_CRAMFS		// No CRAMFS file system commands
#undef	CONFIG_CMD_DATE			// No date/time commands for the RTC
#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
#define CONFIG_CMD_DHCP			// DHCP command
#elif defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
#undef	CONFIG_CMD_DHCP			// No DHCP command
#endif /*  defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#define	CONFIG_CMD_DIAG			// Diagnostics commands
#undef	CONFIG_CMD_DISPLAY		// Dot matrix display commands
#undef	CONFIG_CMD_EEPROM		// No I2C EEPROM read/write commands
#undef  CONFIG_CMD_ELF			// No support for booting an arbitrary ELF image.
#undef	CONFIG_CMD_EXT2			// No EXT2 file system commands
#define	CONFIG_CMD_FAT			// FAT file system commands
#undef	CONFIG_CMD_FLASH		// No NOR flash commmands (no NOR flash)
#undef	CONFIG_CMD_FPGA			// No FPGA commands
#define	CONFIG_CMD_GPIO			// GPIO get/set commands
#define CONFIG_CMD_I2C			// I2C commands
#define CONFIG_I2C_CMD_TREE		// Extended I2C commands
#undef	CONFIG_CMD_IMLS			// No image listing commmands (no NOR flash)
#undef	CONFIG_CMD_IRQ			// Interrupt status commands
#undef	CONFIG_CMD_JFFS			// No JFFS file system commands (S L O W)
#undef	CONFIG_CMD_JFFS2		// No JFFS2 file system commands (S L O W)
#define CONFIG_CMD_LOG			// Log commands
#define	CONFIG_CMD_LOADB		// No serial booting/loading, network only.
#define	CONFIG_CMD_LOADS		// No serial booting/loading, network only.
#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
#define CONFIG_CMD_MII			// MII PHY SMI commands
#elif defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
#undef	CONFIG_CMD_MII			// No MII PHY SMI commands
#endif /*  defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#define CONFIG_CMD_MMC			// MMC/SD media commands
#undef	CONFIG_CMD_MTDPARTS		// No environment MTD parsing commands
#define CONFIG_CMD_NAND			// NAND flash commands
#if defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
#undef	CONFIG_CMD_NET			// No network commands
#undef	CONFIG_CMD_NFS			// No NFS commands
#endif /* defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) && !defined(CONFIG_DIAMOND_BOARD_MULTI) */
#undef	CONFIG_CMD_PCI			// No PCI commands as PCI is unused
#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
#define CONFIG_CMD_PING			// Ping command
#elif defined(CONFIG_DIAMOND_BOARD_PROTOTYPE) && !defined(CONFIG_DIAMOND_BOARD_MULTI)
#undef	CONFIG_CMD_PING			// No ping command
#endif /*  defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */
#undef  CONFIG_CMD_RARP         // No rarpboot command
#define CONFIG_CMD_REGINFO		// Register information command
#undef	CONFIG_CMD_SETGETDCR	// No PowerPC 4xx set/get DCR commands
#undef	CONFIG_CMD_SNTP			// No SNTP commands
#undef	CONFIG_CMD_SPI			// No SPI commands
#undef	CONFIG_CMD_USB			// No USB commands

/*
 * BOOTP/DHCP Command Configuration Options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define	CONFIG_BOOTP_DNS
#define	CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define	CONFIG_BOOTP_SUBNETMASK

/*
 * MTD Configuration Options
 */
#if defined(CONFIG_CMD_MTDPARTS)
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"

#define DECLARE_MTDPART(name, size, offset) #size "@" #offset "(" #name ")"

#define MTDPARTS_IPL			DECLARE_MTDPART(ipl,            256k,    0k)
#define MTDPARTS_SPL0			DECLARE_MTDPART(spl0,          1536k,  256k)
#define MTDPARTS_SPL1			DECLARE_MTDPART(spl1,          1536k, 1792k)
#define MTDPARTS_ENV0			DECLARE_MTDPART(env0,           384k, 3328k)
#define MTDPARTS_ENV1			DECLARE_MTDPART(env1,           384k, 3712k)
#define MTDPARTS_BOOT0			DECLARE_MTDPART(boot0,            8m,    4m)
#define MTDPARTS_ROOT0			DECLARE_MTDPART(root0,           46m,   12m)
#define MTDPARTS_BOOT1			DECLARE_MTDPART(boot1,            8m,   58m)
#define MTDPARTS_ROOT1			DECLARE_MTDPART(root1,           46m,   66m)
#define MTDPARTS_SYSTEM_CONFIG	DECLARE_MTDPART(system-config,    4m,  112m)
#define MTDPARTS_USER_CONFIG	DECLARE_MTDPART(user-config,      8m,  116m)
#define MTDPARTS_DATA			DECLARE_MTDPART(data,            20m,  124m)
#define MTDPARTS_LOG			DECLARE_MTDPART(log,             20m,  144m)
#define MTDPARTS_SCRATCH		DECLARE_MTDPART(scratch,         92m,  164m)

#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:"	\
								MTDPARTS_IPL			","	\
								MTDPARTS_SPL0			"," \
								MTDPARTS_SPL1			"," \
								MTDPARTS_ENV0           "," \
								MTDPARTS_ENV1           "," \
								MTDPARTS_BOOT0          "," \
								MTDPARTS_ROOT0          "," \
								MTDPARTS_BOOT1          "," \
								MTDPARTS_ROOT1          "," \
								MTDPARTS_SYSTEM_CONFIG  "," \
								MTDPARTS_USER_CONFIG    "," \
								MTDPARTS_DATA           "," \
								MTDPARTS_LOG            "," \
								MTDPARTS_SCRATCH

#undef CONFIG_MTD_DEBUG
#undef CONFIG_MTD_DEBUG_VERBOSE 1
#endif

/*
 * JFFS2 Configuration Options
 */
#if defined(CONFIG_CMD_JFFS2)
#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_SUMMARY
#define CONFIG_SYS_JFFS2_SORT_FRAGMENTS
#endif

/*
 * Memory Command Configuration Options
 */
/*
 * CONFIG_SYS_MEMTEST_{START,END} - Starting and ending address for a simple,
 * interactive memory test associated with CONFIG_CMD_MEM.
 */
#define CONFIG_SYS_MEMTEST_START		((0 << 20) + OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END			((31 << 20) + OMAP34XX_SDRC_CS0)

/*
 * CONFIG_LOOPW - Enable the 'loopw' sub-command in the memory command suite.
 */
#define CONFIG_LOOPW            		1

/*
 * CONFIG_MX_CYCLIC - Enable the 'mdc' and 'mwc' sub-commands in the
 * memory command suite.
 */
#define CONFIG_MX_CYCLIC        		1

/*
 * POST Configuration Options
 */

/*
 * CONFIG_POST - Power-on Self Test (POST) blocks to run
 */
#undef	CONFIG_POST

/*
 * Miscellaneous Configurable Options
 */

/*
 * Provide verbose command help
 */
#define CONFIG_SYS_LONGHELP				1

/*
 * Interactive command interpreter prompt
 */
#define CONFIG_SYS_PROMPT				"=> "


/*
 * Size of the console I/O and printf buffers
 */
#define CONFIG_SYS_CBSIZE	        	512
#define CONFIG_SYS_PBSIZE          		(CONFIG_SYS_CBSIZE + \
										 sizeof(CONFIG_SYS_PROMPT) + \
										 16)

/*
 * Define 64-bitisms for both strtoul and vsprintf to ensure that we
 * get correct input and output when dealing with large numbers such
 * as offsets in large media devices or large SDRAM capacities.
 */
#define CONFIG_SYS_64BIT_VSPRINTF		1
#define CONFIG_SYS_64BIT_STRTOUL		1

/*
 * Maximum number of interactive command interpreter arguments.
 */
#define CONFIG_SYS_MAXARGS	        	32

/*
 * Maximum boot argument buffer size
 */
#define CONFIG_SYS_BARGSIZE				512

/*
 * CONFIG_SYS_LOAD_ADDDR - Default address to load to / boot from when no
 * explicit address is provided for their interactive commands.
 *
 * Load at 16 MiB from the base of SDRAM.
 */
#define CONFIG_SYS_LOAD_ADDR			(OMAP34XX_SDRC_CS0 + (16 << 20))

/*
 * CONFIG_SYS_HZ - The default decrementer frequency, in 1 ms
 * ticks. This should NEVER be anything but 1000 for any U-Boot
 * configuration.
 */
#define CONFIG_SYS_HZ					1000

/*
 * Add support for interactive command interpreter command line
 * history and editing.
 */
#define CONFIG_CMDLINE_EDITING

/*
 * Add support for autocompletion of interactive command interpreter command line commands and arguments.
 */
#define	CONFIG_AUTO_COMPLETE

/*
 * Use the more advanced hyper utility shell (hush) interactive
 * command interpreter over the simpler and older u-boot native one to
 * gain valuable features just as variable substitution, control
 * statements, etc. vital for supporting a rich boot script grammar.
 *
 * Also, define the command continuation prompt for hush.
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2		"> "

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4 << 10)	/* IRQ stack 4 KiB */
#define CONFIG_STACKSIZE_FIQ	(4 << 10)	/* FIQ stack 4 KiB */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		2
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE		(32 << 20)	/* at least 32 MiB */
#define PHYS_SDRAM_2			OMAP34XX_SDRC_CS1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_TEXT_BASE		0x80100000

/* SDRAM Bank Allocation method */
#define SDRC_R_B_C		1

#define CONFIG_OMAP3_SAMSUNG_DDR	1

/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

#if defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI)
/*
 * SMSC 9115/9118/9220 802.11 10/100 Ethernet
 */
#define CONFIG_NET_MULTI
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE				0x2C000000

/*
 * Enable MII PHY Management
 */
#define CONFIG_MII
#endif /* defined(CONFIG_DIAMOND_BOARD_DEVELOPMENT) || defined(CONFIG_DIAMOND_BOARD_MULTI) */

/*
 * U-Boot Environment Variable Non-volatile Storage Configuration Options
 */

/*
 * Do not allow overwriting the 'ethaddr' or 'serial#' Ethernet MAC
 * address or erial number environment variables unless they are set
 * to special values (see CONFIG_ETHADDR).
 */
#undef	CONFIG_ENV_OVERWRITE

/*
 * Defining CONFIG_ETHADDR and CONFIG_OVERWRITE_ETHADDR_ONCE along
 * with not defining CONFIG_ENV_OVERWRITE, allows us to overwrite the
 * Ethernet MAC from U-Boot if and only if it is currently the default
 * address.
 */
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*
 * We only have NAND for non-volatile storage, so we'll store the
 * U-Boot environment there.
 *
 * Location where we expect to find the second-stage boot loader in
 * the boot NAND device.
 *
 * Regardless of the device density used, our primary and secondary
 * environments are always at the same block/sector offsets, so long
 * as the block sizes are the same.
 *
 *  -------------------------------------------------------------------------
 *                                                         Offsets
 *                                               ----------------------------
 *  Alias Description            Size    Blocks  Block     Address     Size
 *  =========================================================================
 *  ipl   X-Loader Image       256 KiB      2       0    0x00000000     0 B
 *  -------------------------------------------------------------------------
 *  spl0  Primary U-Boot Image 1.5 MiB     12       2    0x00040000   256 KiB
 *  -------------------------------------------------------------------------
 *  spl1  Secondary U-Boot     1.5 MiB     12      14    0x001C0000 1.750 MiB
 *        Image
 *  -------------------------------------------------------------------------
 *  env0  Primary U-Boot       384 KiB      3      26    0x00340000 3.250 MiB
 *        Environment [1]     
 *  -------------------------------------------------------------------------
 *  env1  Secondary U-Boot     384 KiB      3      29    0x003A0000 3.625 MiB
 *        Environment [1]
 *  =========================================================================
 *
 *  [1] Even though we reserve 384 KiB and 3 erase blocks of space
 *      for the primary and secondary U-Boot environment, we only
 *      actually use 128 KiB and 1 erase block of it. We need to use
 *      at least one erase block and even though it is excessive, 
 *      anything more is all the more so.
 */

#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_SYS_ENV_SECT_SIZE(n)		((n) * CONFIG_SYS_NAND_BLOCK_SIZE)
#define	CONFIG_SYS_ENV_SECT_OFFSET(n)	((n) * CONFIG_SYS_NAND_BLOCK_SIZE)

/*
 * Size and location of primary environment non-volatile storage
 *
 * While we only use one (1) block for the environment, it effectively
 * uses three (3) with two (2) of those being reserved for future
 * growth, spare blocks, etc.
 */
#define	CONFIG_ENV_OFFSET				CONFIG_SYS_ENV_SECT_OFFSET(26)
#define	CONFIG_ENV_SIZE					CONFIG_SYS_ENV_SECT_SIZE(1)
#define CONFIG_ENV_RANGE                CONFIG_SYS_ENV_SECT_SIZE(3)

/* 
 * Number of times to attempt to write environment before declaring a 
 * block bad.
 */
#define CONFIG_ENV_TRY_COUNT            3

/*
 * Size and location of redundant environment non-volatile storage
 */
# define CONFIG_ENV_OFFSET_REDUND		CONFIG_SYS_ENV_SECT_OFFSET(29)
# define CONFIG_ENV_SIZE_REDUND     	CONFIG_ENV_SIZE

/*
 * XXX - TI's implementation of U-Boot tries to make too many
 * assupmptions about things such as where and what flash devices are
 * (e.g. OneNAND, SMNAND, PISMO, etc.) when we are configured for
 * OMAP3 support as though every OMAP3 board was going to be just like
 * their EVM. So, for now, we have to define this.
 */
#define SMNAND_ENV_OFFSET				CONFIG_ENV_OFFSET

/*
 * U-Boot Image Size and Location Configuration
 */
#define CONFIG_SYS_MONITOR_BASE			(TEXT_BASE)
#define CONFIG_SYS_MONITOR_LEN			CONFIG_SYS_ENV_SECT_SIZE(12)

/*
 * Reserve this much space for the dynamically-allocated memory pool
 * (aka heap). Account for a redundant environment, if so configured.
 */
#if defined(CONFIG_ENV_SIZE_REDUND)
# define CONFIG_SYS_MALLOC_EXTRA_LEN	(CONFIG_ENV_SIZE + \
										 CONFIG_ENV_SIZE_REDUND)
#else
# define CONFIG_SYS_MALLOC_EXTRA_LEN	(CONFIG_ENV_SIZE)
#endif /* defined(CONFIG_ENV_SIZE_REDUND) */ 

# define CONFIG_SYS_MALLOC_LEN			ROUND(CONFIG_SYS_MALLOC_EXTRA_LEN + \
											  (256 << 10),					\
											  (4 << 10))

#endif /* __DIAMOND_CONFIG_H */
