/*
 * Copyright (C) 2005 Texas Instruments.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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

#include <common.h>
#include <bootm.h>
#include <part.h>
#include <fat.h>
#include <image.h>
#include <mmc.h>
#include <timestamp.h>
#include <version.h>
#include <asm/arch/mem.h>

/*
 * Preprocessor Definitions
 */
#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

/* Type Definition */

typedef int (init_fnc_t)(void);

/*
 * Function Prototypes
 */
extern int misc_init_r (void);
extern int mmc_init(int verbose);
extern block_dev_desc_t *mmc_get_dev(int dev);
extern int get_boot_devices_list(const u32** devices_list);

/*
 * Global Variables
 */
const char version_string[] =
	X_LOADER_VERSION" (" X_LOADER_DATE " - " X_LOADER_TIME ")"CONFIG_IDENT_STRING;

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
#ifdef CFG_PRINTF
 	serial_init,		/* serial communications setup */
	display_options,
#endif
  	nand_init,		/* board specific nand init */
  	NULL,
};

static inline int check_load_params(const uint8_t *buffer, const int *index, int count)
{
	if (buffer == NULL || index == NULL || count == 0 || *index < BOOTM_INDEX_START || *index >= count) {
		return (BOOTM_STATUS_FAILURE);
	}

	return (BOOTM_STATUS_SUCCESS);
}

#if (defined(CFG_MMC_ONENAND) || defined (CFG_MMC_NAND)) && defined(CFG_CMD_FAT)
/*
 *  int fat_load()
 *
 *  Description:
 *    This routine attempts to read from the first initialized
 *    Multimedia Card (MMC) device, one of one or more designated
 *    secondary program loaders (SPL) from the a FAT file system on
 *    the first partition of a Master Boot Record volume.
 *
 *  Input(s):
 *    buffer - A pointer to storage for the secondary program loader.
 *    index  - A pointer to the index of boot file to attempt to load.
 *
 *  Output(s):
 *    buffer - A pointer to the secondary program loader.
 *    index  - A pointer to the index of the next boot file to
 *             attempt to load.
 *
 *  Returns:
 *    BOOTM_STATUS_SUCCESS on success; otherwise, BOOTM_STATUS_FAILURE
 *    on error.
 *
 */
static int fat_load(uint8_t *buffer, int *index)
{
#if defined(CONFIG_BOOTFILE)
	const char * const files[] = { CONFIG_BOOTFILE };
#elif defined(CONFIG_BOOTFILE_LIST) && defined(CONFIG_BOOTM_IMAGE)
	const char * const files[] = CONFIG_BOOTFILE_LIST;
#else
	const char * const files[] = { NULL };
#endif /* defined(CONFIG_BOOTFILE) */
	const char * file = NULL;
	const char * failure = NULL;
	const int count = ARRAY_SIZE(files);
	const int device = 0;
	const int partition = 1;
	const int maxsize = 0;
	const int verbose = 1;
	long size = 0;
	block_dev_desc_t *dev_desc = NULL;
	unsigned char status = 0;

	if (!check_load_params(buffer, index, count)) {
		return (BOOTM_STATUS_FAILURE);
	}

	file = files[(*index)++];

	if (*index >= count) {
		*index = BOOTM_INDEX_STOP;
	}

	status = mmc_init(verbose);

	if (status == 0) {
		failure = "MMC initialization failed\n";
		goto fail;
	}

    dev_desc = mmc_get_dev(device);

    if (dev_desc == NULL) {
		failure = "Could not access MMC device\n";
		goto fail;
    }

    fat_register_device(dev_desc, partition);

	size = file_fat_read(file, buffer, maxsize);

	if (size == -1) {
		printf("Could not read file \"%s\"\n", file);
		goto fail;
	}

	printf("\n%ld bytes read\n", size);

	printf("Loading from MMC%d, file \"%s\"\n", device, file);

	return (BOOTM_STATUS_SUCCESS);

 fail:
	if (failure != NULL) {
		puts(failure);
	}

	return (BOOTM_STATUS_FAILURE);
}
#endif /* (defined(CFG_MMC_ONENAND) || defined (CFG_MMC_NAND)) && defined(CFG_CMD_FAT) */

/*
 *  int block_load()
 *
 *  Description:
 *    This routine attempts to read from a block device one of one or
 *    more designated secondary program loaders (SPL) specified by the
 *    array of block extents.
 *
 *  Input(s):
 *    buffer     - A pointer to storage for the secondary program
 *                 loader.
 *    index      - A pointer to the index of boot extent to attempt to
 *                 load.
 *    count      - The total number of extents.
 *    extents    - The array of boot file extents to attempt to load.
 *    device     - A pointer to a C string describing the block device 
 *                 being loaded from.
 *    read       - A pointer to the device-specific function that will
 *                 read the blocks to load.
 *    block_size - The size, in bytes, of the device's fundamental read
 *                 block size.
 *    
 *
 *  Output(s):
 *    buffer     - A pointer to the secondary program loader.
 *    index      - A pointer to the index of the next boot extent to
 *                 attempt to load.
 *
 *  Returns:
 *    BOOTM_STATUS_SUCCESS on success; otherwise, BOOTM_STATUS_FAILURE
 *    on error.
 *
 */
static int block_load(uint8_t *buffer, int *index, int count, const boot_extent_t extents[], const char *device, int (*read)(unsigned char *, ulong), unsigned long block_size)
{
	const boot_extent_t *extent = NULL;
	unsigned long address;
	unsigned long end;
	int read_status;

	if (!check_load_params(buffer, index, count)) {
		return (BOOTM_STATUS_FAILURE);
	}

	extent = &extents[(*index)++];

	if (*index >= count) {
		*index = BOOTM_INDEX_STOP;
	}

	end = extent->start + extent->size + 1;

	for (address = extent->start; address < end; address += block_size) {
		read_status = read(buffer, address);
                
		/* If successful, move the data pointer.  If ECC fails,
		 * report failure to caller.  If skipping bad block,
		 * just continue looping without moving the data pointer.
		 */
		if (read_status == NAND_READ_SUCCESS)
		{
			buffer += block_size;
		}
		else if (read_status == NAND_READ_ECC_FAILURE)
		{
			return (BOOTM_STATUS_FAILURE);
		}
	}

	printf("Loading from %s, offset %#08lx\n", device, extent->start);

	return (BOOTM_STATUS_SUCCESS);
}

#if defined(CFG_GPMC_ONENAND)
/*
 *  int onenand_load()
 *
 *  Description:
 *    This routine attempts to read from a OneNAND device one of one or
 *    more designated secondary program loaders (SPL) specified by the
 *    array of block extents.
 *
 *  Input(s):
 *    buffer - A pointer to storage for the secondary program
 *             loader.
 *    index  - A pointer to the index of boot extent to attempt to
 *             load.
 *
 *  Output(s):
 *    buffer - A pointer to the secondary program loader.
 *    index  - A pointer to the index of the next boot extent to
 *             attempt to load.
 *
 *  Returns:
 *    BOOTM_STATUS_SUCCESS on success; otherwise, BOOTM_STATUS_FAILURE
 *    on error.
 *
 */
static int onenand_load(uint8_t *buffer, int *index)
{
#if defined(CONFIG_ONENAND_BOOTEXTENT)
	const boot_extent_t extents[] = { CONFIG_ONENAND_BOOTEXTENT };
#elif defined(CONFIG_ONENAND_BOOTEXTENT_LIST) && defined(CONFIG_BOOTM_IMAGE)
	const boot_extent_t extents[] = CONFIG_ONENAND_BOOTEXTENT_LIST;
#else
	const boot_extent_t extents[] = { NULL };
#endif /* defined(CONFIG_ONENAND_BOOTEXTENT) */

	return block_load(buffer,
					  index,
					  ARRAY_SIZE(extents),
					  extents,
					  "OneNAND",
					  onenand_read_block,
					  ONENAND_BLOCK_SIZE);
}
#endif /* defined(CFG_GPMC_ONENAND) */

#if defined(CFG_GPMC_NAND)
/*
 *  int nand_load()
 *
 *  Description:
 *    This routine attempts to read from a NAND device one of one or
 *    more designated secondary program loaders (SPL) specified by the
 *    array of block extents.
 *
 *  Input(s):
 *    buffer - A pointer to storage for the secondary program
 *             loader.
 *    index  - A pointer to the index of boot extent to attempt to
 *             load.
 *
 *  Output(s):
 *    buffer - A pointer to the secondary program loader.
 *    index  - A pointer to the index of the next boot extent to
 *             attempt to load.
 *
 *  Returns:
 *    BOOTM_STATUS_SUCCESS on success; otherwise, BOOTM_STATUS_FAILURE
 *    on error.
 *
 */
static int nand_load(uint8_t *buffer, int *index)
{
#if defined(CONFIG_NAND_BOOTEXTENT)
	const boot_extent_t extents[] = { CONFIG_NAND_BOOTEXTENT };
#elif defined(CONFIG_NAND_BOOTEXTENT_LIST) && defined(CONFIG_BOOTM_IMAGE)
	const boot_extent_t extents[] = CONFIG_NAND_BOOTEXTENT_LIST;
#else
	const boot_extent_t extents[] = { NULL };
#endif /* defined(CONFIG_NAND_BOOTEXTENT) */

	return block_load(buffer,
					  index,
					  ARRAY_SIZE(extents),
					  extents,
					  "NAND",
					  nand_read_block,
					  NAND_BLOCK_SIZE);
}
#endif /* defined(CFG_GPMC_NAND) */

/*
 *  void do_arm_boot()
 *
 *  Description:
 *    This routine attempts to boot the system from the specified
 *    boot device after copying the secondary-program loader into
 *    the specified memory buffer.
 *
 *  Input(s):
 *    buffer - A pointer to storage for the secondary program
 *             loader.
 *    type   - The boot device to boot from.
 *
 *  Output(s):
 *    buffer - A pointer to the secondary program loader.
 *
 *  Returns:
 *    N/A
 *
 */
static void do_arm_boot(uint8_t *buffer, u32 type)
{
	int index = BOOTM_INDEX_START;
 	int status = BOOTM_STATUS_FAILURE;
	device_load load_image = NULL;

	/* Determine, based on the boot setting, the source we are going
	 * to attempt to boot from.
	 */

	switch (type) {

#if ((defined(CFG_MMC_ONENAND) || defined (CFG_MMC_NAND)) && defined(CFG_CMD_FAT))
	case MMC_NAND:
	case MMC_ONENAND:
		load_image = fat_load;
		break;
#endif

#if defined(CFG_GPMC_ONENAND)
	case GPMC_ONENAND:
		load_image = onenand_load;
		break;
#endif

#if defined(CFG_GPMC_NAND)
	case GPMC_NAND:
		load_image = nand_load;
		break;
#endif

	default:
		goto fail;

	}

	/* Attempt to load and boot any configured images until we
	 * successfully boot or until we've exhausted all possible images.
	 */

	do {
		status = load_image(buffer, &index);

		if (bootm_load_successful(status)) {
			status = bootm(buffer);
		}

	} while (bootm_load_continue(status, index));
fail:
	return;
}

void start_armboot (void)
{
	const u32* devices_list = NULL;
	int devIdx;
	int boot_devices_count = get_boot_devices_list(&devices_list);
	const char * boot_name = NULL;
	init_fnc_t **init_fnc_ptr;
	uint8_t *buffer = (uint8_t *)CFG_LOADADDR;

   	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			goto fail;
		}
	}

	misc_init_r();

	for( devIdx = 0; devIdx < boot_devices_count; devIdx++ )
	{
		u32 boot_dev = devices_list[devIdx];
		switch (boot_dev) {

		case MMC_NAND:
		case MMC_ONENAND:
			boot_name = "MMC";
			break;

		case GPMC_ONENAND:
			boot_name = "OneNAND";
			break;

		case GPMC_NAND:
			boot_name = "NAND";
			break;
		case USB_PERIPHERAL:
			boot_name = "";
			printf("Trying load from USB\n");
			do_usb();  /* check for usb download */
			break;
		default:
			boot_name = "<unknown>";
			break;

		}

		debug("Trying X-Loader on %s\n", boot_name);
		/* This should never return. */
		do_arm_boot(buffer, boot_dev);
	}

	/* If for some reason it does, we have only one choice, hang. */
	puts("Images exhausted\n");
fail:
	hang();
}

void hang (void)
{
	/* call board specific hang function */
	board_hang();

	/* if board_hang() returns, hange here */
	puts("X-Loader hangs\n");

	for (;;);
}
