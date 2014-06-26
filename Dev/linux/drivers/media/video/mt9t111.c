/*
 * drivers/media/video/mt9t111.c
 *
 * mt9t111 sensor driver
 *
 * Copyright (C) 2010 Texas Instruments Inc
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/v4l2-mediabus.h>

#include <media/v4l2-chip-ident.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>
#include <media/mt9t111.h>

#include "mt9t111_reg.h"

/*
 * struct mt9t111 - main structure for storage of sensor information
 * @subdev: V4L2 sub-device structure structure
 * @pad: Media entity pad structure
 * @format: Media-bus format
 * @rect:
 * @pix: V4L2 pixel format information structure
 * @ctrls: v4l2 control handler
 * @frameival: sub-dev pad frame interval
 * @pdata: access functions and data for platform level information
 * @ver: mt9t111 chip version
 * @power: Turn on OR off the interface
 */
struct mt9t111 {
	struct v4l2_subdev		subdev;
	struct media_pad		pad;
	struct v4l2_mbus_framefmt	format;
	struct v4l2_rect		rect;
	struct v4l2_pix_format		pix;

	struct v4l2_ctrl_handler ctrls;

	struct mt9t111_platform_data	*pdata;
	int				ver;
	unsigned int			power;
};

#define to_mt9t111(sd)  container_of(sd, struct mt9t111, subdev)

/*
 * struct mt9t111_fmt -
 * @mbus_code: associated media bus code
 * @fmt: format descriptor
 */
struct mt9t111_fmt {
	unsigned int mbus_code;
	struct v4l2_fmtdesc fmt;
};

/*
 * List of image formats supported by mt9t111
 * Currently we are using 8 bit and 8x2 bit mode only, but can be
 * extended to 10 bit mode.
 */
static const struct mt9t111_fmt mt9t111_fmt_list[] = {
	{
		.mbus_code = V4L2_MBUS_FMT_UYVY8_2X8,
		.fmt = {
			.index = 0,
			.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.flags = 0,
			.description = "8-bit UYVY 4:2:2 Format",
			.pixelformat = V4L2_PIX_FMT_UYVY,
		},
	},
	{
		.mbus_code = V4L2_MBUS_FMT_YUYV8_2X8,
		.fmt = {
			.index = 1,
			.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.flags = 0,
			.description = "8-bit YUYV 4:2:2 Format",
			.pixelformat = V4L2_PIX_FMT_YUYV,
		},
	},
	{
		.mbus_code = V4L2_MBUS_FMT_RGB565_2X8_LE,
		.fmt = {
			.index = 2,
			.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.flags = 0,
			.description = "16-bit RGB565 format",
			.pixelformat = V4L2_PIX_FMT_RGB565,
		},
	},
};

/*
 * mt9t111_read_reg - Read a value from a register in an mt9t111 sensor device
 * @client: i2c driver client structure
 * @data_length: length of data to be read
 * @reg: register address / offset
 * @val: stores the value that gets read
 *
 * Read a value from a register in an mt9t111 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9t111_read_reg(struct i2c_client *client, u16 reg, u16 *val)
{
	struct i2c_msg msg[1];
	u8 data[4];
	int err;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 2;
	msg->buf = data;
	data[0] = (reg & 0xff00) >> 8;
	data[1] = (reg & 0x00ff);
	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0) {
		msg->flags = I2C_M_RD;
		msg->len = 2;	/* 2 byte read */
		err = i2c_transfer(client->adapter, msg, 1);
		if (err >= 0) {
			*val = ((data[0] & 0x00ff) << 8)
				| (data[1] & 0x00ff);
			return 0;
		}
	}
	return err;
}

/*
 * mt9t111_write_reg - Write a value to a register in an mt9t111 sensor device
 * @client: i2c driver client structure
 * @data_length: length of data to be read
 * @reg: register address / offset
 * @val: value to be written to specified register
 *
 * Write a value to a register in an mt9t111 sensor device.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9t111_write_reg(struct i2c_client *client, u16 reg, u16 val)
{
	struct i2c_msg msg[1];
	u8 data[20];
	int err;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 4;
	msg->buf = data;
	data[0] = (u8)((reg & 0xff00) >> 8);
	data[1] = (u8)(reg & 0x00ff);
	data[2] = (u8)((val & 0xff00) >> 8);
	data[3] = (u8)(val & 0x00ff);
	err = i2c_transfer(client->adapter, msg, 1);
	if (err < 0)
		return err;

	return 0;
}

/*
 * mt9t111_write_regs - Write registers to an mt9t111 sensor device
 * @client: i2c driver client structure
 * @reg_in: pointer to registers to write
 * @cnt: the number of registers
 *
 * Write registers .
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9t111_write_regs(struct i2c_client *client,
				mt9t111_regs *reg_in, int cnt)
{
	int err = 0, i;
	mt9t111_regs *reg = reg_in;

	for (i = 0; i < cnt; i++) {
		if (reg->delay_time == 0) {
			err |= mt9t111_write_reg(client, reg->addr, reg->data);
		} else if (reg->addr != 0 || reg->data != 0) {
			err |= mt9t111_write_reg(client, reg->addr, reg->data);
			mdelay(reg->delay_time);
		} else {
			mdelay(reg->delay_time);
		}

		if (err < 0) {
			dev_warn(&client->dev, "write reg error, addr = 0x%x,"
					" data = 0x%x \n",
					reg->addr, reg->data);
			return err;
		}
		reg++;
	}
	return err;
}


static int mt9t111_refresh(struct i2c_client *client)
{
	int i, err = 0;
	unsigned short value;

	/* MCU_ADDRESS [SEQ_CMD] -- refresh mode */
	err = mt9t111_write_reg(client, 0x098E, 0x8400);
	if (err)
		return err;
	err = mt9t111_write_reg(client, 0x0990, 0x0006);
	if (err)
		return err;

	/* refresh command */
	err = mt9t111_write_reg(client, 0x098E, 0x8400);
	if (err)
		return err;
	err = mt9t111_write_reg(client, 0x0990, 0x0005);
	if (err)
		return err;

	for (i = 0; i < 100; i++) {
		err = mt9t111_write_reg(client, 0x098E, 0x8400);
		if (err)
			break;
		err = mt9t111_read_reg(client, 0x0990, &value);
		if ((value == 0) || (err))
			break;
		mdelay(5);
	}

	return err;
}

static int mt9t111_enable_pll(struct i2c_client *client)
{
	int i, err = 0;
	unsigned short value;

	err = mt9t111_write_regs(client, pll_regs1,
			   sizeof(pll_regs1) / sizeof(mt9t111_regs));
	if (err)
		return err;

	for (i = 0; i < 100; i++) {
		err = mt9t111_read_reg(client, 0x0014, &value);
		if (((value & 0x8000) != 0) || (err))
			break;
		mdelay(2);
	}
	err = mt9t111_write_regs(client, pll_regs2,
			   sizeof(pll_regs2) / sizeof(mt9t111_regs));
	return err;
}

/*
 * mt9t111_configure - Configure the mt9t111 for the specified image mode
 * @s: pointer to standard V4L2 device structure
 *
 * Configure the mt9t111 for a specified image size, pixel format, and frame
 * period.  xclk is the frequency (in Hz) of the xclk input to the mt9t111.
 * fper is the frame period (in seconds) expressed as a fraction.
 * Returns zero if successful, or non-zero otherwise.
 * The actual frame period is returned in fper.
 */
static int mt9t111_configure(struct v4l2_subdev *subdev)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	int err = 0;

	err = mt9t111_write_reg(client, 0x001A, 0x001D);
	if (err < 0)
		return err;

	msleep(1);

	err = mt9t111_write_reg(client, 0x001A, 0x0018);
	if (err < 0)
		goto out;

	err = mt9t111_enable_pll(client);
	if (err)
		goto out;

	// Enable Clock pad
	err = mt9t111_write_reg(client, 0x0016, 0x0400);

	err = mt9t111_write_regs(client, def_regs1,
			   sizeof(def_regs1) / sizeof(mt9t111_regs));
	if (err)
		goto out;

	err = mt9t111_write_regs(client, patch_rev6,
			   sizeof(patch_rev6) / sizeof(mt9t111_regs));
	if (err)
		goto out;

	err = mt9t111_write_regs(client, def_regs2,
			   sizeof(def_regs2) / sizeof(mt9t111_regs));
	if (err)
		goto out;

	err = mt9t111_refresh(client);

out:
	return err;
}

/*
 * mt9t111_detect - Detect if an mt9t111 is present, and if so which revision
 * @subdev: pointer to the V4L2 sub-device driver structure
 *
 * Detect if an mt9t111 is present
 * Returns a negative error number if no device is detected, or the
 * non-negative value of the version ID register if a device is detected.
 */
static int mt9t111_detect(struct v4l2_subdev *subdev)
{
	u16 val;
	struct i2c_client *client = v4l2_get_subdevdata(subdev);

	/* chip ID is at address 0 */
	if (mt9t111_read_reg(client, MT9T111_CHIP_ID, &val) < 0)
		return -ENODEV;

	if (val != MT9T111_CHIP_ID_VALUE) {
		dev_err(&client->dev, "model id mismatch received 0x%x"
				" expecting 0x%x\n",
				val, MT9T111_CHIP_ID_VALUE);
		return -ENODEV;
	}

	return (int)val;

}

/* --------------------------------------------------------------------------
 * V4L2 subdev core operations
 */

static int mt9t111_g_chip_ident(struct v4l2_subdev *subdev,
		struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_MT9T111, 0);
}

/*
 * mt9t111_dev_init - sensor init, tries to detect the sensor
 * @subdev: pointer to standard V4L2 subdev structure
 */

static int mt9t111_dev_init(struct v4l2_subdev *subdev)
{
	struct mt9t111 *mt9t111= to_mt9t111(subdev);
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	int rval;

	rval = mt9t111->pdata->s_power(subdev, 1);
	if (rval)
		return rval;

	rval = mt9t111_detect(subdev);
	if (rval < 0) {
		v4l_err(client, "Unable to detect"
				MT9T111_MODULE_NAME "sensor\n");
		goto out;
	}
	mt9t111->ver = rval;
	v4l_info(client, MT9T111_MODULE_NAME
			" chip version 0x%02x detected\n", mt9t111->ver);
	rval = 0;
out:
	mt9t111->pdata->s_power(subdev, 0);
	return rval;
}

/*
 * mt9t111_s_config - set the platform data for future use
 * @subdev: pointer to standard V4L2 subdev structure
 * @irq:
 * @platform_data: sensor platform_data
 */
static int mt9t111_s_config(struct v4l2_subdev *subdev, int irq,
				void *platform_data)
{
	struct mt9t111 *mt9t111= to_mt9t111(subdev);

	if (platform_data == NULL)
		return -ENODEV;

	mt9t111->pdata = platform_data;

	return mt9t111_dev_init(subdev);
}


/*
 * mt9t111_s_power - V4L2 sensor interface handler for s_power
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int mt9t111_s_power(struct v4l2_subdev *subdev, int on)
{
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	int rval;

	if (on) {
		rval = mt9t111->pdata->s_power(subdev, 1);
		if (rval)
			goto out;

		rval = mt9t111_configure(subdev);
		if (rval) {
			mt9t111->pdata->s_power(subdev, 0);
			goto out;
		}
	} else {
		rval = mt9t111->pdata->s_power(subdev, 0);
		if (rval)
			goto out;
	}

	mt9t111->power = on;

out:
	if (rval)
		v4l_err(client, "Unable to set target power state\n");

	return rval;
}

/* --------------------------------------------------------------------------
 * V4L2 subdev file operations
 */
static int mt9t111_open(struct v4l2_subdev *subdev, struct v4l2_subdev_fh *fh)
{
	struct v4l2_mbus_framefmt *format;
	struct v4l2_rect *crop;

	/*
	 * Default configuration -
	 *	Resolution: VGA
	 *	Format: UYVY
	 *	crop = window
	 */
	crop = v4l2_subdev_get_try_crop(fh, 0);
	crop->left = 0;
	crop->top = 0;
	crop->width = 640;
	crop->height = 480;

	format = v4l2_subdev_get_try_format(fh, 0);
	format->code = V4L2_MBUS_FMT_UYVY8_2X8;
	format->width = 640;
	format->height = 480;
	format->field = V4L2_FIELD_NONE;
	format->colorspace = V4L2_COLORSPACE_JPEG;

	return 0;
}

/* --------------------------------------------------------------------------
 * V4L2 subdev video operations
 */

static int mt9t111_s_stream(struct v4l2_subdev *subdev, int streaming)
{
	/*
         * FIXME: We should put here the specific reg setting to turn on
	 * streaming in sensor.
	 */
	return 0;
}


/* --------------------------------------------------------------------------
 * V4L2 subdev pad operations
 */
/*
 * mt9t111_enum_mbus_code - V4L2 sensor interface handler for pad_ops
 * @subdev: pointer to standard V4L2 sub-device structure
 * @qctrl: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the video_control[] array.  Otherwise, returns -EINVAL if the
 * control is not supported.
 */
static int mt9t111_enum_mbus_code(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_mbus_code_enum *code)
{
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);

	if (code->index >= ARRAY_SIZE(mt9t111_fmt_list))
		return -EINVAL;

	code->code = mt9t111->format.code;

	return 0;
}

static int mt9t111_enum_frame_size(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_frame_size_enum *fse)
{
	int i;

	/* Is requested media-bus format/pixelformat not found on sensor? */
	for (i = 0; i < ARRAY_SIZE(mt9t111_fmt_list); i++) {
		if (fse->code == mt9t111_fmt_list[i].mbus_code )
			goto fmt_found;
	}
	if (i >= ARRAY_SIZE(mt9t111_fmt_list))
		return -EINVAL;

fmt_found:
	/*
	 * Currently only supports VGA resolution
	 */
	fse->min_width = fse->max_width = 640;
	fse->min_height = fse->max_height = 480;

	return 0;
}


static int mt9t111_get_pad_format(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_format *fmt)
{
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);

	fmt->format = mt9t111->format;
	return 0;
}

static int mt9t111_set_pad_format(struct v4l2_subdev *subdev,
				struct v4l2_subdev_fh *fh,
				struct v4l2_subdev_format *fmt)
{
	int i;
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);

	for (i = 0; i < ARRAY_SIZE(mt9t111_fmt_list); i++) {
		if (fmt->format.code == mt9t111_fmt_list[i].mbus_code )
			goto fmt_found;
	}
	if (i >= ARRAY_SIZE(mt9t111_fmt_list))
		return -EINVAL;

fmt_found:
	/*
	 * Only VGA resolution supported
	 */
	fmt->format.width = 640;
	fmt->format.height = 480;
	fmt->format.field = V4L2_FIELD_NONE;
	fmt->format.colorspace = V4L2_COLORSPACE_JPEG;

	mt9t111->format = fmt->format;

	return 0;
}

static int mt9t111_get_crop(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);

	crop->rect = mt9t111->rect;
	return 0;
}

static int mt9t111_set_crop(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);
	struct v4l2_rect rect;

	/*
	 * Only VGA resolution/window is supported
	 */
	rect.left = 0;
	rect.top = 0;
	rect.width = 640;
	rect.height = 480;

	mt9t111->rect = rect;
	crop->rect = rect;

	return 0;
}

static const struct v4l2_subdev_core_ops mt9t111_core_ops = {
	.g_chip_ident = mt9t111_g_chip_ident,
	.s_config = mt9t111_s_config,
	.s_power = mt9t111_s_power,
};

static struct v4l2_subdev_file_ops mt9t111_subdev_file_ops = {
	.open	= mt9t111_open,
};

static const struct v4l2_subdev_video_ops mt9t111_video_ops = {
	.s_stream	= mt9t111_s_stream,
};

static const struct v4l2_subdev_pad_ops mt9t111_pad_ops = {
	.enum_mbus_code	= mt9t111_enum_mbus_code,
	.enum_frame_size= mt9t111_enum_frame_size,
	.get_fmt	= mt9t111_get_pad_format,
	.set_fmt	= mt9t111_set_pad_format,
	.get_crop	= mt9t111_get_crop,
	.set_crop	= mt9t111_set_crop,
};

static const struct v4l2_subdev_ops mt9t111_ops = {
	.core	= &mt9t111_core_ops,
	.file	= &mt9t111_subdev_file_ops,
	.video	= &mt9t111_video_ops,
	.pad	= &mt9t111_pad_ops,
};

/*
 * mt9t111_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
static int mt9t111_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct mt9t111 *mt9t111;
	int ret;

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA)) {
		v4l_err(client, "mt9t111: I2C Adapter doesn't support" \
				" I2C_FUNC_SMBUS_WORD\n");
		return -EIO;
	}

	if (!client->dev.platform_data) {
		v4l_err(client, "No platform data!!\n");
		return -ENODEV;
	}

	mt9t111 = kzalloc(sizeof(*mt9t111), GFP_KERNEL);
	if (mt9t111 == NULL) {
		v4l_err(client, "Could not able to alocate memory!!\n");
		return -ENOMEM;
	}

	mt9t111->pdata = client->dev.platform_data;

	/*
	 * Default configuration -
	 *      Resolution: VGA
	 *      Format: UYVY
	 *      crop = window
	 */

	mt9t111->rect.left = 0;
	mt9t111->rect.top = 0;
	mt9t111->rect.width = 640;
	mt9t111->rect.height = 480;

	mt9t111->format.code = V4L2_MBUS_FMT_UYVY8_2X8;
	mt9t111->format.width = 640;
	mt9t111->format.height = 480;
	mt9t111->format.field = V4L2_FIELD_NONE;
	mt9t111->format.colorspace = V4L2_COLORSPACE_JPEG;

	v4l2_i2c_subdev_init(&mt9t111->subdev, client, &mt9t111_ops);
	mt9t111->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	mt9t111->pad.flags = MEDIA_PAD_FLAG_OUTPUT;
	ret = media_entity_init(&mt9t111->subdev.entity, 1, &mt9t111->pad, 0);
	if (ret < 0)
		kfree(mt9t111);

	return ret;
}

/*
 * mt9t111_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device.  Complement of mt9t111_probe().
 */
static int __exit mt9t111_remove(struct i2c_client *client)
{
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	struct mt9t111 *mt9t111 = to_mt9t111(subdev);

	v4l2_device_unregister_subdev(&mt9t111->subdev);
	media_entity_cleanup(&mt9t111->subdev.entity);
	kfree(mt9t111);

	return 0;
}

static const struct i2c_device_id mt9t111_id[] = {
	{ MT9T111_MODULE_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, mt9t111_id);

static struct i2c_driver mt9t111_i2c_driver = {
	.driver = {
		.name = MT9T111_MODULE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = mt9t111_probe,
	.remove = __exit_p(mt9t111_remove),
	.id_table = mt9t111_id,
};

/*
 * mt9t111_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
static int __init mt9t111_init(void)
{
	return i2c_add_driver(&mt9t111_i2c_driver);
}

/*
 * mt9t111_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of mt9t111_init.
 */
static void __exit mt9t111_cleanup(void)
{
	i2c_del_driver(&mt9t111_i2c_driver);
}

module_init(mt9t111_init);
module_exit(mt9t111_cleanup);

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("mt9t111 camera sensor driver");
MODULE_LICENSE("GPL");
