/*
 * BeagleXM: Driver for Leopard Module Board
 *
 * Copyright (C) 2011 Texas Instruments Inc
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
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
 */

#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include <mach/gpio.h>

#include <media/mt9v113.h>

#include <../drivers/media/video/isp/isp.h>

#include "devices.h"

#define CAM_USE_XCLKA			0
#define LEOPARD_RESET_GPIO		98

static struct regulator *beagle_1v8;
static struct regulator *beagle_2v8;

static int beagle_mt9v113_s_power(struct v4l2_subdev *subdev, int on)
{
	struct isp_device *isp = v4l2_dev_to_isp_device(subdev->v4l2_dev);

	if (!beagle_1v8 || !beagle_2v8) {
		dev_err(isp->dev, "No regulator available\n");
		return -ENODEV;
	}
	if (on) {
		/*
		 * Power Up Sequence
		 */
		/* Set RESET_BAR to 0 */
		gpio_set_value(LEOPARD_RESET_GPIO, 0);
		/* Turn on VDD */
		regulator_enable(beagle_1v8);
		mdelay(1);
		regulator_enable(beagle_2v8);

		mdelay(50);
		/* Enable EXTCLK */
		if (isp->platform_cb.set_xclk)
			isp->platform_cb.set_xclk(isp, 24000000, CAM_USE_XCLKA);
		/*
		 * Wait at least 70 CLK cycles (w/EXTCLK = 24MHz):
		 * ((1000000 * 70) / 24000000) = aprox 3 us.
		 */
		udelay(3);
		/* Set RESET_BAR to 1 */
		gpio_set_value(LEOPARD_RESET_GPIO, 1);
		/*
		 * Wait at least 100 CLK cycles (w/EXTCLK = 24MHz):
		 * ((1000000 * 100) / 24000000) = aprox 5 us.
		 */
		udelay(5);
	} else {
		/*
		 * Power Down Sequence
		 */
		if (regulator_is_enabled(beagle_1v8))
			regulator_disable(beagle_1v8);
		if (regulator_is_enabled(beagle_2v8))
			regulator_disable(beagle_2v8);

		if (isp->platform_cb.set_xclk)
			isp->platform_cb.set_xclk(isp, 0, CAM_USE_XCLKA);
	}

	return 0;
}

static int beagle_mt9v113_configure_interface(struct v4l2_subdev *subdev,
					      u32 pixclk)
{
	struct isp_device *isp = v4l2_dev_to_isp_device(subdev->v4l2_dev);

	if (isp->platform_cb.set_pixel_clock)
		isp->platform_cb.set_pixel_clock(isp, pixclk);

	return 0;
}

static struct mt9v113_platform_data beagle_mt9v113_platform_data = {
	.s_power		= beagle_mt9v113_s_power,
	.configure_interface	= beagle_mt9v113_configure_interface,
};


#define MT9V113_I2C_BUS_NUM		2

static struct i2c_board_info beagle_camera_i2c_devices[] = {
	{
		I2C_BOARD_INFO(MT9V113_MODULE_NAME, MT9V113_I2C_ADDR),
		.platform_data = &beagle_mt9v113_platform_data,
	},
};

static struct isp_subdev_i2c_board_info beagle_camera_primary_subdevs[] = {
	{
		.board_info = &beagle_camera_i2c_devices[0],
		.i2c_adapter_id = MT9V113_I2C_BUS_NUM,
	},
	{ NULL, 0 },
};

static struct isp_v4l2_subdevs_group beagle_camera_subdevs[] = {
	{
		.subdevs = beagle_camera_primary_subdevs,
		.interface = ISP_INTERFACE_PARALLEL,
		.bus = {
			.parallel = {
				.data_lane_shift	= 2,
				.clk_pol		= 0,
				.bridge			= 3,
			},
		},
	},
	{ NULL, 0 },
};

static struct isp_platform_data beagle_isp_platform_data = {
	.subdevs = beagle_camera_subdevs,
};

static int __init beagle_cam_init(void)
{
	/*
	 * Regulator supply required for camera interface
	 */
	beagle_1v8 = regulator_get(NULL, "cam_1v8");
	if (IS_ERR(beagle_1v8)) {
		printk(KERN_ERR "cam_1v8 regulator missing\n");
		return PTR_ERR(beagle_1v8);
	}
	beagle_2v8 = regulator_get(NULL, "cam_2v8");
	if (IS_ERR(beagle_2v8)) {
		printk(KERN_ERR "cam_2v8 regulator missing\n");
		regulator_put(beagle_1v8);
		return PTR_ERR(beagle_2v8);
	}
	/*
	 * Sensor reset GPIO
	 */
	if (gpio_request(LEOPARD_RESET_GPIO, "cam_rst") != 0) {
		printk(KERN_ERR "beagle-cam: Could not request GPIO %d\n",
				LEOPARD_RESET_GPIO);
		regulator_put(beagle_1v8);
		regulator_put(beagle_2v8);
		return -ENODEV;
	}
	/* set to output mode */
	gpio_direction_output(LEOPARD_RESET_GPIO, 0);

	omap3_init_camera(&beagle_isp_platform_data);

	return 0;
}

static void __exit beagle_cam_exit(void)
{
	if (regulator_is_enabled(beagle_1v8))
		regulator_disable(beagle_1v8);
	regulator_put(beagle_1v8);
	if (regulator_is_enabled(beagle_2v8))
		regulator_disable(beagle_2v8);
	regulator_put(beagle_2v8);

	gpio_free(LEOPARD_RESET_GPIO);
}

module_init(beagle_cam_init);
module_exit(beagle_cam_exit);

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("BeagleXM Camera Module");
MODULE_LICENSE("GPL");
