/*
 * include/media/mt9t111.h
 *
 * mt9t111 sensor driver
 *
 * Copyright (C) 2009 Leopard Imaging
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef	MT9T111_H
#define	MT9T111_H

#include <media/v4l2-subdev.h>
#include <media/media-entity.h>

/*
 * Defines and Macros and globals
 */
#define MT9T111_MODULE_NAME		"mt9t111"

/*i2c adress for MT9T111*/
#define MT9T111_I2C_ADDR  		(0x78 >> 1)

#define MT9T111_CLK_MAX			(96000000) /* 96MHz */
#define MT9T111_CLK_MIN			(6000000)  /* 6Mhz */

#define MT9T111_I2C_CONFIG		(1)
#define I2C_ONE_BYTE_TRANSFER		(1)
#define I2C_TWO_BYTE_TRANSFER		(2)
#define I2C_THREE_BYTE_TRANSFER		(3)
#define I2C_FOUR_BYTE_TRANSFER		(4)
#define I2C_TXRX_DATA_MASK		(0x00FF)
#define I2C_TXRX_DATA_MASK_UPPER	(0xFF00)
#define I2C_TXRX_DATA_SHIFT		(8)

struct mt9t111_platform_data {
	int (*s_power) (struct v4l2_subdev *subdev, u32 on);
	int (*set_xclk) (struct v4l2_subdev *subdev, u32 hz);
	int (*configure_interface) (struct v4l2_subdev *subdev, u32 pixclk);
};

#endif	/* ifndef MT9T111 */

