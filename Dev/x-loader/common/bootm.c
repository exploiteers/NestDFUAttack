/*
 *    Copyright (c) 2010 Grant Erickson <marathon96@gmail.com>
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
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *      This file implements functions that boot the second-stage
 *      boot loader from memory, in either raw, binary format or
 *      in u-boot 'mkimage' format.
 */

#include <common.h>
#include <bootm.h>
#include <image.h>

typedef void (spl)(void);

static void exec(ulong address, const char *what)
{
	printf("\nStarting %s ...", what);

	((spl *)address)();
}

int bootm(const uint8_t *buffer)
{
	int status = BOOTM_STATUS_FAILURE;
#if defined(CONFIG_BOOTM_IMAGE)
	const int format = genimg_get_format((void *)buffer);
	const image_header_t * header = (const image_header_t *)buffer;

	switch (format) {

	case IMAGE_FORMAT_LEGACY:
		status = bootm_image(header);
		break;

	case IMAGE_FORMAT_FIT:
		printf("Unsupported image format FIT (%d)\n", format);
		break;

	case IMAGE_FORMAT_INVALID:
	default:
		printf("Unknown or invalid image format (%d)\n", format);
		break;

	}
#else
	status = bootm_binary(buffer);
#endif /* defined(CONFIG_BOOTM_IMAGE) */

	return (status);
}

int bootm_binary(const uint8_t *buffer)
{
	/* This should never return. */

	exec((ulong)buffer, "second-stage boot loader");

	/* However, if it does, return failed status. */

	return (BOOTM_STATUS_FAILURE);
}

#if defined(CONFIG_BOOTM_IMAGE)
int bootm_image(const image_header_t *header)
{
	const char * failure = NULL;
	const char * type_name = NULL;
	uint32_t load, image_start, image_len;

	/* Display to standard output the image contents. */

	image_print_contents(header);

	/* Validate the image header and image data CRCs */

	puts("   Verifying Checksum ... ");

	{
		if (!image_check_hcrc(header)) {
			failure = "Header Invalid\n";
			goto fail;
		}

		if (!image_check_dcrc(header)) {
			failure = "Data Invalid\n";
			goto fail;
		}
	}

	puts("OK\n");

	/* We ONLY support uncompressed ARM U-Boot firmware images. Check
	 * to make sure that's what we are going to boot.
	 */

	if (!image_check_type(header, IH_TYPE_FIRMWARE)) {
		failure = "Image is not a firmware image\n";
		goto fail;
	}

	if (!image_check_os(header, IH_OS_U_BOOT)) {
		failure = "Image is not u-boot firmware\n";
		goto fail;
	}

	if (image_get_comp(header) != IH_COMP_NONE) {
		failure = "Image is compressed\n";
		goto fail;
	}

	if (!image_check_target_arch(header)) {
		failure = "Image is not built for this processor\n";
		goto fail;
	}

	type_name = genimg_get_type_name(image_get_type(header));

	printf("   Loading %s ... ", type_name);

	{
		load = image_get_load(header);
		image_start = image_get_data(header);
		image_len = image_get_data_size(header);

		memmove_wd((void *)load, (void *)image_start, image_len, CHUNKSZ);
	}

	puts("OK\n");

	/* This should never return. */

	exec(load, type_name);

	/* However, if it does, return failed status. */

 fail:
	puts(failure);

	return (BOOTM_STATUS_FAILURE);
}
#endif /* defined(CONFIG_BOOTM_IMAGE) */
