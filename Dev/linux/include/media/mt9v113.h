/*
 * drivers/media/video/mt9v113.h
 *
 * Copyright (C) 2008 Texas Instruments Inc
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Contributors:
 *     Sivaraj R <sivaraj@ti.com>
 *     Brijesh R Jadav <brijesh.j@ti.com>
 *     Hardik Shah <hardik.shah@ti.com>
 *     Manjunath Hadli <mrh@ti.com>
 *     Karicheri Muralidharan <m-karicheri2@ti.com>
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _MT9V113_H
#define _MT9V113_H

#include <media/v4l2-subdev.h>
#include <media/media-entity.h>

/*
 * Other macros
 */
#define MT9V113_MODULE_NAME		"mt9v113"

/* Number of pixels and number of lines per frame for different standards */
#define VGA_NUM_ACTIVE_PIXELS		640
#define VGA_NUM_ACTIVE_LINES		480
#define QVGA_NUM_ACTIVE_PIXELS		320
#define QVGA_NUM_ACTIVE_LINES		240

struct mt9v113_platform_data {
	int (*s_power)(struct v4l2_subdev *subdev, int on);
	int (*set_xclk)(struct v4l2_subdev *subdev, int hz);
	int (*configure_interface)(struct v4l2_subdev *subdev, u32 pixclk);
};

/*i2c adress for MT9V113*/
#define MT9V113_I2C_ADDR		(0x78 >> 1)

#define I2C_ONE_BYTE_TRANSFER		(1)
#define I2C_TWO_BYTE_TRANSFER		(2)
#define I2C_THREE_BYTE_TRANSFER		(3)
#define I2C_FOUR_BYTE_TRANSFER		(4)
#define I2C_TXRX_DATA_MASK		(0x00FF)
#define I2C_TXRX_DATA_MASK_UPPER	(0xFF00)
#define I2C_TXRX_DATA_SHIFT		(8)

#define MT9V113_VGA_30FPS  (1130)
#define MT9V113_QVGA_30FPS  (1131)

#define MT9V113_CLK_MAX		(54000000) /* 54MHz */
#define MT9V113_CLK_MIN		(6000000)  /* 6Mhz */

#endif				/* ifndef _MT9V113_H */

