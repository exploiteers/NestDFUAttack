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
 */

#ifndef	__BOOTM_H__
#define	__BOOTM_H__

#include "compiler.h"

#include <image.h>

#define BOOTM_STATUS_FAILURE 0
#define BOOTM_STATUS_SUCCESS 1

#define BOOTM_INDEX_START  0
#define BOOTM_INDEX_STOP  -1

typedef struct boot_extent {
	ulong start;
	ulong size;
} boot_extent_t;

typedef int (*device_load)(uint8_t *buffer, int *index);

static inline int bootm_load_successful(int status)
{
	return (status == BOOTM_STATUS_SUCCESS);
}

static inline int bootm_load_continue(int status, int index)
{
	return ((status != BOOTM_STATUS_SUCCESS) && (index != BOOTM_INDEX_STOP));
}

extern int bootm(const uint8_t *buffer);
extern int bootm_binary(const uint8_t *buffer);
#if defined(CONFIG_BOOTM_IMAGE)
extern int bootm_image(const image_header_t *image);
#endif

#endif	/* __BOOTM_H__ */
