/*
 * OMAP3EVM: Driver for Leopard Module Board
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

#include <media/mt9t111.h>
#include <media/tvp514x.h>

#include <../drivers/media/video/isp/isp.h>

#include "mux.h"
#include "devices.h"

#define CAM_USE_XCLKA			0

#define TVP5146_DEC_RST			98
#define T2_GPIO_2			194
#define nCAM_VD_SEL			157
#define nCAM_VD_EN			200

static struct regulator *omap3evm_1v8;
static struct regulator *omap3evm_2v8;


/* mux id to enable/disable signal routing to different peripherals */
enum omap3evm_cam_mux {
	MUX_EN_TVP5146 = 0,
	MUX_EN_CAMERA_SENSOR,
	MUX_EN_EXP_CAMERA_SENSOR,
	MUX_INVALID,
};

static int omap3evm_regulator_ctrl(u32 on)
{
	if (!omap3evm_1v8 || !omap3evm_2v8) {
		printk(KERN_ERR "No regulator available\n");
		return -ENODEV;
	}

	if (on) {
		/* Turn on VDD */
		regulator_enable(omap3evm_1v8);
		mdelay(1);
		regulator_enable(omap3evm_2v8);
		mdelay(50);
	} else {
		/*
		 * Power Down Sequence
		 */
		if (regulator_is_enabled(omap3evm_1v8))
			regulator_disable(omap3evm_1v8);
		if (regulator_is_enabled(omap3evm_2v8))
			regulator_disable(omap3evm_2v8);
	}

	return 0;
}

/**
 * @brief omap3evm_set_mux - Sets mux to enable/disable signal routing to
 *                             different peripherals present on new EVM board
 *
 * @param mux_id - enum, mux id to enable/disable
 * @param value - enum, ENABLE_MUX for enabling and DISABLE_MUX for disabling
 *
 */
static void omap3evm_set_mux(enum omap3evm_cam_mux mux_id)
{
	switch (mux_id) {
		/*
		 * JP1 jumper need to configure to choose betweek on-board
		 * camera sensor conn and on-board LI-3MC02 camera sensor.
		 */
		case MUX_EN_CAMERA_SENSOR:
			/* Set nCAM_VD_EN (T2_GPIO8) = 0 */
			gpio_set_value_cansleep(nCAM_VD_EN, 0);
			/* Set nCAM_VD_SEL (GPIO157) = 0 */
			gpio_set_value(nCAM_VD_SEL, 0);
			break;

		case MUX_EN_EXP_CAMERA_SENSOR:
			/* Set nCAM_VD_EN (T2_GPIO8) = 1 */
			gpio_set_value_cansleep(nCAM_VD_EN, 1);

			break;

		case MUX_EN_TVP5146:
		default:
			/* Set nCAM_VD_EN (T2_GPIO8) = 0 */
			gpio_set_value_cansleep(nCAM_VD_EN, 0);
			/* Set nCAM_VD_SEL (GPIO157) = 1 */
			gpio_set_value(nCAM_VD_SEL, 1);
			break;
	}
}

/* MT9T111: 3M sensor */

static int omap3evm_mt9t111_s_power(struct v4l2_subdev *subdev, u32 on)
{
	struct isp_device *isp = v4l2_dev_to_isp_device(subdev->v4l2_dev);
	int ret;

	ret = omap3evm_regulator_ctrl(on);
	if (ret)
		return ret;

	omap3evm_set_mux(MUX_EN_CAMERA_SENSOR);

	if (on) {
		/* Enable EXTCLK */
		if (isp->platform_cb.set_xclk)
			isp->platform_cb.set_xclk(isp, 24000000, CAM_USE_XCLKA);
		udelay(5);
	} else {
		if (isp->platform_cb.set_xclk)
			isp->platform_cb.set_xclk(isp, 0, CAM_USE_XCLKA);
	}

	return 0;
}

static struct mt9t111_platform_data omap3evm_mt9t111_platform_data = {
	.s_power		= omap3evm_mt9t111_s_power,
};

/* TVP5146: Video Decoder */

static int omap3evm_tvp514x_s_power(struct v4l2_subdev *subdev, u32 on)
{
	int ret;

	ret = omap3evm_regulator_ctrl(on);
	if (ret)
		return ret;

	omap3evm_set_mux(MUX_EN_TVP5146);

	/* Assert the reset signal */
	gpio_set_value(TVP5146_DEC_RST, 0);
	mdelay(5);
	gpio_set_value(TVP5146_DEC_RST, 1);

	return 0;
}

static struct tvp514x_platform_data omap3evm_tvp514x_platform_data = {
	.s_power		= omap3evm_tvp514x_s_power,
};


#define MT9T111_I2C_BUS_NUM		2
#define TVP514X_I2C_BUS_NUM		3

static struct i2c_board_info omap3evm_camera_i2c_devices[] = {
	{
		I2C_BOARD_INFO(MT9T111_MODULE_NAME, MT9T111_I2C_ADDR),
		.platform_data = &omap3evm_mt9t111_platform_data,
	},
	{
		I2C_BOARD_INFO("tvp5146m2", 0x5C),
		.platform_data	= &omap3evm_tvp514x_platform_data,
	},
};

static struct isp_subdev_i2c_board_info omap3evm_mt9t111_subdevs[] = {
	{
		.board_info = &omap3evm_camera_i2c_devices[0],
		.i2c_adapter_id = MT9T111_I2C_BUS_NUM,
	},
	{ NULL, 0 },
};

static struct isp_subdev_i2c_board_info omap3evm_tvp514x_subdevs[] = {
	{
		.board_info	= &omap3evm_camera_i2c_devices[1],
		.i2c_adapter_id	= TVP514X_I2C_BUS_NUM,
	},
	{ NULL, 0 },
};

static struct isp_v4l2_subdevs_group omap3evm_camera_subdevs[] = {
	{
		.subdevs = omap3evm_mt9t111_subdevs,
		.interface = ISP_INTERFACE_PARALLEL,
		.bus = {
			.parallel = {
				.data_lane_shift	= 1,
				.clk_pol		= 0,
				.hdpol			= 0,
				.vdpol			= 0,
				.fldmode		= 0,
				.bridge			= 3,
				.is_bt656		= 0,
			},
		},
	},
	{
		.subdevs	= omap3evm_tvp514x_subdevs,
		.interface	= ISP_INTERFACE_PARALLEL,
		.bus		= {
			.parallel	= {
				.width			= 8,
				.data_lane_shift	= 1,
				.clk_pol		= 0,
				.hdpol			= 0,
				.vdpol			= 1,
				.fldmode		= 1,
				.bridge			= 0,
				.is_bt656		= 1,
			},
		},
	},
	{ NULL, 0 },
};

static struct isp_platform_data omap3evm_isp_platform_data = {
	.subdevs = omap3evm_camera_subdevs,
};

static int __init omap3evm_cam_init(void)
{
	int ret = 0;

	/*
	 * Regulator supply required for camera interface
	 */
	omap3evm_1v8 = regulator_get(NULL, "vio_1v8");
	if (IS_ERR(omap3evm_1v8)) {
		printk(KERN_ERR "vio_1v8 regulator missing\n");
		return PTR_ERR(omap3evm_1v8);
	}
	omap3evm_2v8 = regulator_get(NULL, "cam_2v8");
	if (IS_ERR(omap3evm_2v8)) {
		printk(KERN_ERR "cam_2v8 regulator missing\n");
		ret = PTR_ERR(omap3evm_2v8);
		goto err_1;
	}

	/*
	 * First level GPIO enable: T2_GPIO.2
	 */
	ret = gpio_request(T2_GPIO_2, "T2_GPIO.2");
	if (ret) {
		printk(KERN_ERR "failed to get t2_gpio.2\n");
		goto err_2;
	}
	gpio_direction_output(T2_GPIO_2, 0);

	/*
	 * nCAM_VD_SEL (GPIO157)
	 */
	omap_mux_init_gpio(nCAM_VD_SEL, OMAP_PIN_INPUT_PULLUP);
	ret = gpio_request(nCAM_VD_SEL, "cam_vd_sel");
	if (ret) {
		printk(KERN_ERR "failed to get cam_vd_sel\n");
		goto err_3;
	}
	gpio_direction_output(nCAM_VD_SEL, 1);

	/*
	 * EXP_nCAM_VD_EN (T2_GPIO.8)
	 */
	ret = gpio_request(nCAM_VD_EN, "cam_vd_en");
	if (ret) {
		printk(KERN_ERR "failed to get cam_vd_en\n");
		goto err_4;
	}
	gpio_direction_output(nCAM_VD_EN, 0);

	omap_mux_init_gpio(TVP5146_DEC_RST, OMAP_PIN_INPUT_PULLUP);
	if (gpio_request(TVP5146_DEC_RST, "vid-dec reset") < 0) {
		printk(KERN_ERR "failed to get GPIO98_VID_DEC_RES\n");
		goto err_5;
	}
	gpio_direction_output(TVP5146_DEC_RST, 1);

	omap3_init_camera(&omap3evm_isp_platform_data);

	printk(KERN_INFO "omap3evm camera init done successfully...\n");
	return 0;

err_5:
	gpio_free(nCAM_VD_EN);
err_4:
	gpio_free(nCAM_VD_SEL);
err_3:
	gpio_free(T2_GPIO_2);
err_2:
	regulator_put(omap3evm_2v8);
err_1:
	regulator_put(omap3evm_1v8);

	return ret;
}

static void __exit omap3evm_cam_exit(void)
{
	gpio_free(nCAM_VD_EN);
	gpio_free(nCAM_VD_SEL);
	gpio_free(T2_GPIO_2);

	if (regulator_is_enabled(omap3evm_1v8))
		regulator_disable(omap3evm_1v8);
	regulator_put(omap3evm_1v8);
	if (regulator_is_enabled(omap3evm_2v8))
		regulator_disable(omap3evm_2v8);
	regulator_put(omap3evm_2v8);
}

module_init(omap3evm_cam_init);
module_exit(omap3evm_cam_exit);

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("omap3evm Camera Module");
MODULE_LICENSE("GPL");
