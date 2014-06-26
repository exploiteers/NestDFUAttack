/*
 * Driver for MT9V113 CMOS Image Sensor from Micron
 *
 * Based on MT9V032 sensor driver
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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/v4l2-mediabus.h>

#include <media/v4l2-chip-ident.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>
#include <media/mt9v113.h>

#include "mt9v113_regs.h"

/* Macro's */
#define I2C_RETRY_COUNT                 (5)

#define MT9V113_DEF_WIDTH		640
#define MT9V113_DEF_HEIGHT		480

/* Debug functions */
static int debug = 1;
module_param(debug, bool, 0644);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

/*
 * struct mt9v113 - sensor object
 * @subdev: v4l2_subdev associated data
 * @pad: media entity associated data
 * @format: associated media bus format
 * @rect: configured resolution/window
 * @pdata: Board specific
 * @ver: Chip version
 * @power: Current power state (0: off, 1: on)
 */
struct mt9v113 {
	struct v4l2_subdev subdev;
	struct media_pad pad;
	struct v4l2_mbus_framefmt format;
	struct v4l2_rect rect;

	struct v4l2_ctrl_handler ctrls;

	const struct mt9v113_platform_data *pdata;
	unsigned int ver;
	bool power;
};

#define to_mt9v113(sd)	container_of(sd, struct mt9v113, subdev)
/*
 * struct mt9v113_fmt -
 * @mbus_code: associated media bus code
 * @fmt: format descriptor
 */
struct mt9v113_fmt {
	unsigned int mbus_code;
	struct v4l2_fmtdesc fmt;
};
/*
 * List of image formats supported by mt9v113
 * Currently we are using 8 bit and 8x2 bit mode only, but can be
 * extended to 10 bit mode.
 */
static const struct mt9v113_fmt mt9v113_fmt_list[] = {
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

/* MT9V113 register set for VGA mode */
static struct mt9v113_reg mt9v113_vga_reg[] = {
	{TOK_WRITE, 0x098C, 0x2739}, /* crop_X0_A */
	{TOK_WRITE, 0x0990, 0x0000}, /* val: 0 */
	{TOK_WRITE, 0x098C, 0x273B}, /* crop_X1_A */
	{TOK_WRITE, 0x0990, 0x027F}, /* val: 639 */
	{TOK_WRITE, 0x098C, 0x273D}, /* crop_Y0_A */
	{TOK_WRITE, 0x0990, 0x0000}, /* val: 0 */
	{TOK_WRITE, 0x098C, 0x273F}, /* crop_Y1_A */
	{TOK_WRITE, 0x0990, 0x01DF}, /* val: 479 */
	{TOK_WRITE, 0x098C, 0x2703}, /* output_width_A */
	{TOK_WRITE, 0x0990, 0x0280}, /* val: 640 */
	{TOK_WRITE, 0x098C, 0x2705}, /* out_height_A */
	{TOK_WRITE, 0x0990, 0x01E0}, /* val: 480 */
	{TOK_WRITE, 0x098C, 0xA103}, /* cmd */
	{TOK_WRITE, 0x0990, 0x0005}, /* val: 5 - Refresh */
	{TOK_DELAY, 0, 100},
	{TOK_TERM, 0, 0},
};

static struct mt9v113_reg mt9v113_yuyv_reg[] = {
	{TOK_WRITE, 0x098C, 0x2755}, /* output_format_A */
	{TOK_WRITE, 0x0990, 0x0002}, /* val: 0x2 - YUYV format */
	{TOK_WRITE, 0x098C, 0xA103}, /* cmd */
	{TOK_WRITE, 0x0990, 0x0005}, /* val: 5 - Refresh */
	{TOK_DELAY, 0, 100},
	{TOK_TERM, 0, 0},
};

static struct mt9v113_reg mt9v113_uyvy_reg[] = {
	{TOK_WRITE, 0x098C, 0x2755}, /* output_format_A */
	{TOK_WRITE, 0x0990, 0x0000}, /* val: 0 */
	{TOK_WRITE, 0x098C, 0xA103}, /* cmd */
	{TOK_WRITE, 0x0990, 0x0005}, /* val: 5 - Refresh */
	{TOK_DELAY, 0, 100},
	{TOK_TERM, 0, 0},
};

static struct mt9v113_reg mt9v113_rgb565_reg[] = {
	{TOK_WRITE, 0x098C, 0x2755}, /* output_format_A */
	{TOK_WRITE, 0x0990, 0x0020}, /* val: 0x20 - RGB enable */
	{TOK_WRITE, 0x098C, 0xA103}, /* cmd */
	{TOK_WRITE, 0x0990, 0x0005}, /* val: 5 - Refresh */
	{TOK_DELAY, 0, 100},
	{TOK_TERM, 0, 0},
};

/* MT9V113 default register values */
static struct mt9v113_reg mt9v113_reg_list[] = {
	{TOK_WRITE, 0x0018, 0x4028},
	{TOK_DELAY, 0, 100},
	{TOK_WRITE, 0x001A, 0x0011},
	{TOK_WRITE, 0x001A, 0x0010},
	{TOK_WRITE, 0x0018, 0x4028},
	{TOK_DELAY, 0, 100},
	{TOK_WRITE, 0x098C, 0x02F0},
	{TOK_WRITE, 0x0990, 0x0000},
	{TOK_WRITE, 0x098C, 0x02F2},
	{TOK_WRITE, 0x0990, 0x0210},
	{TOK_WRITE, 0x098C, 0x02F4},
	{TOK_WRITE, 0x0990, 0x001A},
	{TOK_WRITE, 0x098C, 0x2145},
	{TOK_WRITE, 0x0990, 0x02F4},
	{TOK_WRITE, 0x098C, 0xA134},
	{TOK_WRITE, 0x0990, 0x0001},
	{TOK_WRITE, 0x31E0, 0x0001},
	{TOK_WRITE, 0x001A, 0x0210},
	{TOK_WRITE, 0x001E, 0x0777},
	{TOK_WRITE, 0x0016, 0x42DF},
	{TOK_WRITE, 0x0014, 0x2145},
	{TOK_WRITE, 0x0014, 0x2145},
	{TOK_WRITE, 0x0010, 0x0431},
	{TOK_WRITE, 0x0012, 0x0000},
	{TOK_WRITE, 0x0014, 0x244B},
	{TOK_WRITE, 0x0014, 0x304B},
	{TOK_DELAY, 0, 100},
	{TOK_WRITE, 0x0014, 0xB04A},
	{TOK_WRITE, 0x098C, 0xAB1F},
	{TOK_WRITE, 0x0990, 0x00C7},
	{TOK_WRITE, 0x098C, 0xAB31},
	{TOK_WRITE, 0x0990, 0x001E},
	{TOK_WRITE, 0x098C, 0x274F},
	{TOK_WRITE, 0x0990, 0x0004},
	{TOK_WRITE, 0x098C, 0x2741},
	{TOK_WRITE, 0x0990, 0x0004},
	{TOK_WRITE, 0x098C, 0xAB20},
	{TOK_WRITE, 0x0990, 0x0054},
	{TOK_WRITE, 0x098C, 0xAB21},
	{TOK_WRITE, 0x0990, 0x0046},
	{TOK_WRITE, 0x098C, 0xAB22},
	{TOK_WRITE, 0x0990, 0x0002},
	{TOK_WRITE, 0x098C, 0xAB24},
	{TOK_WRITE, 0x0990, 0x0005},
	{TOK_WRITE, 0x098C, 0x2B28},
	{TOK_WRITE, 0x0990, 0x170C},
	{TOK_WRITE, 0x098C, 0x2B2A},
	{TOK_WRITE, 0x0990, 0x3E80},
	{TOK_WRITE, 0x3210, 0x09A8},
	{TOK_WRITE, 0x098C, 0x2306},
	{TOK_WRITE, 0x0990, 0x0315},
	{TOK_WRITE, 0x098C, 0x2308},
	{TOK_WRITE, 0x0990, 0xFDDC},
	{TOK_WRITE, 0x098C, 0x230A},
	{TOK_WRITE, 0x0990, 0x003A},
	{TOK_WRITE, 0x098C, 0x230C},
	{TOK_WRITE, 0x0990, 0xFF58},
	{TOK_WRITE, 0x098C, 0x230E},
	{TOK_WRITE, 0x0990, 0x02B7},
	{TOK_WRITE, 0x098C, 0x2310},
	{TOK_WRITE, 0x0990, 0xFF31},
	{TOK_WRITE, 0x098C, 0x2312},
	{TOK_WRITE, 0x0990, 0xFF4C},
	{TOK_WRITE, 0x098C, 0x2314},
	{TOK_WRITE, 0x0990, 0xFE4C},
	{TOK_WRITE, 0x098C, 0x2316},
	{TOK_WRITE, 0x0990, 0x039E},
	{TOK_WRITE, 0x098C, 0x2318},
	{TOK_WRITE, 0x0990, 0x001C},
	{TOK_WRITE, 0x098C, 0x231A},
	{TOK_WRITE, 0x0990, 0x0039},
	{TOK_WRITE, 0x098C, 0x231C},
	{TOK_WRITE, 0x0990, 0x007F},
	{TOK_WRITE, 0x098C, 0x231E},
	{TOK_WRITE, 0x0990, 0xFF77},
	{TOK_WRITE, 0x098C, 0x2320},
	{TOK_WRITE, 0x0990, 0x000A},
	{TOK_WRITE, 0x098C, 0x2322},
	{TOK_WRITE, 0x0990, 0x0020},
	{TOK_WRITE, 0x098C, 0x2324},
	{TOK_WRITE, 0x0990, 0x001B},
	{TOK_WRITE, 0x098C, 0x2326},
	{TOK_WRITE, 0x0990, 0xFFC6},
	{TOK_WRITE, 0x098C, 0x2328},
	{TOK_WRITE, 0x0990, 0x0086},
	{TOK_WRITE, 0x098C, 0x232A},
	{TOK_WRITE, 0x0990, 0x00B5},
	{TOK_WRITE, 0x098C, 0x232C},
	{TOK_WRITE, 0x0990, 0xFEC3},
	{TOK_WRITE, 0x098C, 0x232E},
	{TOK_WRITE, 0x0990, 0x0001},
	{TOK_WRITE, 0x098C, 0x2330},
	{TOK_WRITE, 0x0990, 0xFFEF},
	{TOK_WRITE, 0x098C, 0xA348},
	{TOK_WRITE, 0x0990, 0x0008},
	{TOK_WRITE, 0x098C, 0xA349},
	{TOK_WRITE, 0x0990, 0x0002},
	{TOK_WRITE, 0x098C, 0xA34A},
	{TOK_WRITE, 0x0990, 0x0090},
	{TOK_WRITE, 0x098C, 0xA34B},
	{TOK_WRITE, 0x0990, 0x00FF},
	{TOK_WRITE, 0x098C, 0xA34C},
	{TOK_WRITE, 0x0990, 0x0075},
	{TOK_WRITE, 0x098C, 0xA34D},
	{TOK_WRITE, 0x0990, 0x00EF},
	{TOK_WRITE, 0x098C, 0xA351},
	{TOK_WRITE, 0x0990, 0x0000},
	{TOK_WRITE, 0x098C, 0xA352},
	{TOK_WRITE, 0x0990, 0x007F},
	{TOK_WRITE, 0x098C, 0xA354},
	{TOK_WRITE, 0x0990, 0x0043},
	{TOK_WRITE, 0x098C, 0xA355},
	{TOK_WRITE, 0x0990, 0x0001},
	{TOK_WRITE, 0x098C, 0xA35D},
	{TOK_WRITE, 0x0990, 0x0078},
	{TOK_WRITE, 0x098C, 0xA35E},
	{TOK_WRITE, 0x0990, 0x0086},
	{TOK_WRITE, 0x098C, 0xA35F},
	{TOK_WRITE, 0x0990, 0x007E},
	{TOK_WRITE, 0x098C, 0xA360},
	{TOK_WRITE, 0x0990, 0x0082},
	{TOK_WRITE, 0x098C, 0x2361},
	{TOK_WRITE, 0x0990, 0x0040},
	{TOK_WRITE, 0x098C, 0xA363},
	{TOK_WRITE, 0x0990, 0x00D2},
	{TOK_WRITE, 0x098C, 0xA364},
	{TOK_WRITE, 0x0990, 0x00F6},
	{TOK_WRITE, 0x098C, 0xA302},
	{TOK_WRITE, 0x0990, 0x0000},
	{TOK_WRITE, 0x098C, 0xA303},
	{TOK_WRITE, 0x0990, 0x00EF},
	{TOK_WRITE, 0x098C, 0xAB20},
	{TOK_WRITE, 0x0990, 0x0024},
	{TOK_WRITE, 0x098C, 0xA103},
	{TOK_WRITE, 0x0990, 0x0006},
	{TOK_DELAY, 0, 100},
	{TOK_WRITE, 0x098C, 0xA103},
	{TOK_WRITE, 0x0990, 0x0005},
	{TOK_DELAY, 0, 100},
	{TOK_WRITE, 0x098C, 0x222D},
	{TOK_WRITE, 0x0990, 0x0088},
	{TOK_WRITE, 0x098C, 0xA408},
	{TOK_WRITE, 0x0990, 0x0020},
	{TOK_WRITE, 0x098C, 0xA409},
	{TOK_WRITE, 0x0990, 0x0023},
	{TOK_WRITE, 0x098C, 0xA40A},
	{TOK_WRITE, 0x0990, 0x0027},
	{TOK_WRITE, 0x098C, 0xA40B},
	{TOK_WRITE, 0x0990, 0x002A},
	{TOK_WRITE, 0x098C, 0x2411},
	{TOK_WRITE, 0x0990, 0x0088},
	{TOK_WRITE, 0x098C, 0x2413},
	{TOK_WRITE, 0x0990, 0x00A4},
	{TOK_WRITE, 0x098C, 0x2415},
	{TOK_WRITE, 0x0990, 0x0088},
	{TOK_WRITE, 0x098C, 0x2417},
	{TOK_WRITE, 0x0990, 0x00A4},
	{TOK_WRITE, 0x098C, 0xA404},
	{TOK_WRITE, 0x0990, 0x0010},
	{TOK_WRITE, 0x098C, 0xA40D},
	{TOK_WRITE, 0x0990, 0x0002},
	{TOK_WRITE, 0x098C, 0xA40E},
	{TOK_WRITE, 0x0990, 0x0003},
	{TOK_WRITE, 0x098C, 0xA103},
	{TOK_WRITE, 0x0990, 0x0006},
	{TOK_DELAY, 0, 100},
	/* test pattern all white*/
	/* {TOK_WRITE, 0x098C, 0xA766},
	{TOK_WRITE, 0x0990, 0x0001},
	*/
	{TOK_WRITE, 0x098C, 0xA103},
	{TOK_WRITE, 0x0990, 0x0005},
	{TOK_DELAY, 0, 100},
	{TOK_TERM, 0, 0},
};

static int mt9v113_read_reg(struct i2c_client *client, unsigned short reg)
{
	int err = 0;
	struct i2c_msg msg[1];
	unsigned char data[2];
	unsigned short val = 0;

	if (!client->adapter)
		return -ENODEV;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = I2C_TWO_BYTE_TRANSFER;
	msg->buf = data;
	data[0] = (reg & I2C_TXRX_DATA_MASK_UPPER) >> I2C_TXRX_DATA_SHIFT;
	data[1] = (reg & I2C_TXRX_DATA_MASK);
	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0) {
		msg->flags = I2C_M_RD;
		msg->len = I2C_TWO_BYTE_TRANSFER;	/* 2 byte read */
		err = i2c_transfer(client->adapter, msg, 1);
		if (err >= 0) {
			val = ((data[0] & I2C_TXRX_DATA_MASK) <<
				I2C_TXRX_DATA_SHIFT) |
				(data[1] & I2C_TXRX_DATA_MASK);
		}
	}

	v4l_dbg(1, debug, client,
		 "mt9v113_read_reg reg=0x%x, val=0x%x\n", reg, val);

	return (int)(0xffff & val);
}

static int mt9v113_write_reg(struct i2c_client *client, unsigned short reg,
			     unsigned short val)
{
	int err = -EAGAIN; /* To enter below loop, err has to be negative */
	int trycnt = 0;
	struct i2c_msg msg[1];
	unsigned char data[4];

	v4l_dbg(1, debug, client,
			"mt9v113_write_reg reg=0x%x, val=0x%x\n", reg, val);

	if (!client->adapter)
		return -ENODEV;

	while ((err < 0) && (trycnt < I2C_RETRY_COUNT)) {
		trycnt++;
		msg->addr = client->addr;
		msg->flags = 0;
		msg->len = I2C_FOUR_BYTE_TRANSFER;
		msg->buf = data;
		data[0] = (reg & I2C_TXRX_DATA_MASK_UPPER) >>
			I2C_TXRX_DATA_SHIFT;
		data[1] = (reg & I2C_TXRX_DATA_MASK);
		data[2] = (val & I2C_TXRX_DATA_MASK_UPPER) >>
			I2C_TXRX_DATA_SHIFT;
		data[3] = (val & I2C_TXRX_DATA_MASK);
		err = i2c_transfer(client->adapter, msg, 1);
	}
	if (err < 0)
		printk(KERN_INFO "\n I2C write failed");

	return err;
}

/*
 * mt9v113_write_regs : Initializes a list of registers
 *		if token is TOK_TERM, then entire write operation terminates
 *		if token is TOK_DELAY, then a delay of 'val' msec is introduced
 *		if token is TOK_SKIP, then the register write is skipped
 *		if token is TOK_WRITE, then the register write is performed
 *
 * reglist - list of registers to be written
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9v113_write_regs(struct i2c_client *client,
			      const struct mt9v113_reg reglist[])
{
	int err;
	const struct mt9v113_reg *next = reglist;

	for (; next->token != TOK_TERM; next++) {
		if (next->token == TOK_DELAY) {
			msleep(next->val);
			continue;
		}

		if (next->token == TOK_SKIP)
			continue;

		err = mt9v113_write_reg(client, next->reg, next->val);
		if (err < 0) {
			v4l_err(client, "Write failed. Err[%d]\n", err);
			return err;
		}
	}
	return 0;
}

/*
 * Configure the mt9v113 with the current register settings
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9v113_def_config(struct v4l2_subdev *subdev)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);

	/* common register initialization */
	return mt9v113_write_regs(client, mt9v113_reg_list);
}

/*
 * Configure the MT9V113 to VGA mode
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9v113_vga_mode(struct v4l2_subdev *subdev)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);
	struct mt9v113_reg *fmt_reg;
	int ret = 0;

	/*
	 * VGA resolution
	 */
	ret = mt9v113_write_regs(client, mt9v113_vga_reg);
	if (ret) {
		v4l_err(client, "Failed to configure vga resolution\n");
		goto exit;
	}

	if (mt9v113->format.code == V4L2_MBUS_FMT_YUYV8_2X8)
		fmt_reg = mt9v113_yuyv_reg;
	else if (mt9v113->format.code == V4L2_MBUS_FMT_RGB565_2X8_LE)
		fmt_reg = mt9v113_rgb565_reg;
	else
		/* Falling down to default UYVY format */
		fmt_reg = mt9v113_uyvy_reg;

	ret = mt9v113_write_regs(client, fmt_reg);
	if (ret)
		v4l_err(client, "Failed to configure pixel format\n");

exit:
	return ret;
}

/*
 * Detect if an mt9v113 is present, and if so which revision.
 * A device is considered to be detected if the chip ID (LSB and MSB)
 * registers match the expected values.
 * Any value of the rom version register is accepted.
 * Returns ENODEV error number if no device is detected, or zero
 * if a device is detected.
 */
static int mt9v113_detect(struct v4l2_subdev *subdev)
{
	struct mt9v113 *decoder = to_mt9v113(subdev);
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	unsigned short val = 0;

	val = mt9v113_read_reg(client, REG_CHIP_ID);
	v4l_dbg(1, debug, client, "chip id detected 0x%x\n", val);

	if (MT9V113_CHIP_ID != val) {
		/* We didn't read the values we expected, so this must not be
		 * MT9V113.
		 */
		v4l_err(client, "chip id mismatch read 0x%x, expecting 0x%x\n",
				val, MT9V113_CHIP_ID);
		return -ENODEV;
	}

	decoder->ver = val;

	v4l_info(client, "%s found at 0x%x (%s)\n", client->name,
			client->addr << 1, client->adapter->name);
	return 0;
}


/* --------------------------------------------------------------------------
 * V4L2 subdev core operations
 */
/*
 * mt9v113_g_chip_ident - get chip identifier
 */
static int mt9v113_g_chip_ident(struct v4l2_subdev *subdev,
			       struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_MT9V113, 0);
}

/*
 * mt9v113_dev_init - sensor init, tries to detect the sensor
 * @subdev: pointer to standard V4L2 subdev structure
 */
static int mt9v113_dev_init(struct v4l2_subdev *subdev)
{
	struct mt9v113 *decoder = to_mt9v113(subdev);
	int rval;

	rval = decoder->pdata->s_power(subdev, 1);
	if (rval)
		return rval;

	rval = mt9v113_detect(subdev);

	decoder->pdata->s_power(subdev, 0);

	return rval;
}

/*
 * mt9v113_s_config - set the platform data for future use
 * @subdev: pointer to standard V4L2 subdev structure
 * @irq:
 * @platform_data: sensor platform_data
 */
static int mt9v113_s_config(struct v4l2_subdev *subdev, int irq,
			   void *platform_data)
{
	struct mt9v113 *decoder = to_mt9v113(subdev);
	int rval;

	if (platform_data == NULL)
		return -ENODEV;

	decoder->pdata = platform_data;

	rval = mt9v113_dev_init(subdev);
	if (rval)
		return rval;

	return 0;
}

/*
 * mt9v113_s_power - Set power function
 * @subdev: pointer to standard V4L2 subdev structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requested state, if possible.
 */
static int mt9v113_s_power(struct v4l2_subdev *subdev, int on)
{
	struct mt9v113 *decoder = to_mt9v113(subdev);
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	int rval;

	if (on) {
		rval = decoder->pdata->s_power(subdev, 1);
		if (rval)
			goto out;
		rval = mt9v113_def_config(subdev);
		if (rval) {
			decoder->pdata->s_power(subdev, 0);
			goto out;
		}
		rval = mt9v113_vga_mode(subdev);
		if (rval) {
			decoder->pdata->s_power(subdev, 0);
			goto out;
		}
	} else {
		rval = decoder->pdata->s_power(subdev, 0);
		if (rval)
			goto out;
	}

	decoder->power = on;
out:
	if (rval < 0)
		v4l_err(client, "Unable to set target power state\n");

	return rval;
}

/* --------------------------------------------------------------------------
 * V4L2 subdev file operations
 */
static int mt9v113_open(struct v4l2_subdev *subdev, struct v4l2_subdev_fh *fh)
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
	crop->width = MT9V113_DEF_WIDTH;
	crop->height = MT9V113_DEF_HEIGHT;

	format = v4l2_subdev_get_try_format(fh, 0);
	format->code = V4L2_MBUS_FMT_UYVY8_2X8;
	format->width = MT9V113_DEF_WIDTH;
	format->height = MT9V113_DEF_HEIGHT;
	format->field = V4L2_FIELD_NONE;
	format->colorspace = V4L2_COLORSPACE_JPEG;

	return 0;
}

/* --------------------------------------------------------------------------
 * V4L2 subdev video operations
 */
static int mt9v113_s_stream(struct v4l2_subdev *subdev, int streaming)
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
static int mt9v113_enum_mbus_code(struct v4l2_subdev *subdev,
				  struct v4l2_subdev_fh *fh,
				  struct v4l2_subdev_mbus_code_enum *code)
{
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);

	if (code->index >= ARRAY_SIZE(mt9v113_fmt_list))
		return -EINVAL;

	code->code = mt9v113->format.code;

	return 0;
}

static int mt9v113_enum_frame_size(struct v4l2_subdev *subdev,
				   struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_frame_size_enum *fse)
{
	int i;

	/* Is requested media-bus format/pixelformat not found on sensor? */
	for (i = 0; i < ARRAY_SIZE(mt9v113_fmt_list); i++) {
		if (fse->code == mt9v113_fmt_list[i].mbus_code )
			goto fmt_found;
	}
	if (i >= ARRAY_SIZE(mt9v113_fmt_list))
		return -EINVAL;

fmt_found:
	/*
	 * Currently only supports VGA resolution
	 */
	fse->min_width = fse->max_width = MT9V113_DEF_WIDTH;
	fse->min_height = fse->max_height = MT9V113_DEF_HEIGHT;

	return 0;
}

static int mt9v113_get_pad_format(struct v4l2_subdev *subdev,
				  struct v4l2_subdev_fh *fh,
				  struct v4l2_subdev_format *fmt)
{
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);

	fmt->format = mt9v113->format;
	return 0;
}

static int mt9v113_set_pad_format(struct v4l2_subdev *subdev,
				  struct v4l2_subdev_fh *fh,
				  struct v4l2_subdev_format *fmt)
{
	int i;
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);

	for (i = 0; i < ARRAY_SIZE(mt9v113_fmt_list); i++) {
		if (fmt->format.code == mt9v113_fmt_list[i].mbus_code )
			goto fmt_found;
	}
	if (i >= ARRAY_SIZE(mt9v113_fmt_list))
		return -EINVAL;

fmt_found:
	/*
	 * Only VGA resolution supported
	 */
	fmt->format.width = MT9V113_DEF_WIDTH;
	fmt->format.height = MT9V113_DEF_HEIGHT;
	fmt->format.field = V4L2_FIELD_NONE;
	fmt->format.colorspace = V4L2_COLORSPACE_JPEG;

	mt9v113->format = fmt->format;

	return 0;
}

static int mt9v113_get_crop(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);

	crop->rect = mt9v113->rect;
	return 0;
}

static int mt9v113_set_crop(struct v4l2_subdev *subdev,
		struct v4l2_subdev_fh *fh, struct v4l2_subdev_crop *crop)
{
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);
	struct v4l2_rect rect;

	/*
	 * Only VGA resolution/window is supported
	 */
	rect.left = 0;
	rect.top = 0;
	rect.width = MT9V113_DEF_WIDTH;
	rect.height = MT9V113_DEF_HEIGHT;

	mt9v113->rect = rect;
	crop->rect = rect;

	return 0;
}

static const struct v4l2_subdev_core_ops mt9v113_core_ops = {
	.g_chip_ident = mt9v113_g_chip_ident,
	.s_config = mt9v113_s_config,
	.s_power = mt9v113_s_power,
};

static struct v4l2_subdev_file_ops mt9v113_subdev_file_ops = {
	.open		= mt9v113_open,
};

static const struct v4l2_subdev_video_ops mt9v113_video_ops = {
	.s_stream = mt9v113_s_stream,
};

static const struct v4l2_subdev_pad_ops mt9v113_pad_ops = {
	.enum_mbus_code	= mt9v113_enum_mbus_code,
	.enum_frame_size= mt9v113_enum_frame_size,
	.get_fmt	= mt9v113_get_pad_format,
	.set_fmt	= mt9v113_set_pad_format,
	.get_crop	= mt9v113_get_crop,
	.set_crop	= mt9v113_set_crop,
};

static const struct v4l2_subdev_ops mt9v113_ops = {
	.core	= &mt9v113_core_ops,
	.file	= &mt9v113_subdev_file_ops,
	.video	= &mt9v113_video_ops,
	.pad	= &mt9v113_pad_ops,
};


/* -----------------------------------------------------------------------------
 * V4L2 subdev control operations
 */

#define V4L2_CID_TEST_PATTERN		(V4L2_CID_USER_BASE | 0x1001)

/*
 * mt9v113_s_ctrl - V4L2 decoder interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @ctrl: pointer to v4l2_control structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW. Otherwise, returns -EINVAL if the control is not supported.
 */
static int mt9v113_s_ctrl(struct v4l2_ctrl *ctrl)
{
        struct mt9v113 *mt9v113 =
			container_of(ctrl->handler, struct mt9v113, ctrls);

	struct i2c_client *client = v4l2_get_subdevdata(&mt9v113->subdev);
	int err = -EINVAL, value;

	if (ctrl == NULL)
		return err;

	value = (__s32) ctrl->val;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		if (ctrl->val < 0 || ctrl->val > 255) {
			v4l_err(client, "invalid brightness setting %d\n",
					ctrl->val);
			return -ERANGE;
		}
		err = mt9v113_write_reg(client, REG_BRIGHTNESS, value);
		if (err)
			return err;

		mt9v113_reg_list[REG_BRIGHTNESS].val = value;
		break;
	case V4L2_CID_CONTRAST:
		if (ctrl->val < 0 || ctrl->val > 255) {
			v4l_err(client, "invalid contrast setting %d\n",
					ctrl->val);
			return -ERANGE;
		}
		err = mt9v113_write_reg(client, REG_CONTRAST, value);
		if (err)
			return err;

		mt9v113_reg_list[REG_CONTRAST].val = value;
		break;
	case V4L2_CID_SATURATION:
		if (ctrl->val < 0 || ctrl->val > 255) {
			v4l_err(client, "invalid saturation setting %d\n",
					ctrl->val);
			return -ERANGE;
		}
		err = mt9v113_write_reg(client, REG_SATURATION, value);
		if (err)
			return err;

		mt9v113_reg_list[REG_SATURATION].val = value;
		break;
	case V4L2_CID_HUE:
		if (value == 180)
			value = 0x7F;
		else if (value == -180)
			value = 0x80;
		else if (value == 0)
			value = 0;
		else {
			v4l_err(client, "invalid hue setting %d\n",
					ctrl->val);
			return -ERANGE;
		}
		err = mt9v113_write_reg(client, REG_HUE, value);
		if (err)
			return err;

		mt9v113_reg_list[REG_HUE].val = value;
		break;
	case V4L2_CID_AUTOGAIN:
		if (value == 1)
			value = 0x0F;
		else if (value == 0)
			value = 0x0C;
		else {
			v4l_err(client, "invalid auto gain setting %d\n",
					ctrl->val);
			return -ERANGE;
		}
		err = mt9v113_write_reg(client, REG_AFE_GAIN_CTRL, value);
		if (err)
			return err;

		mt9v113_reg_list[REG_AFE_GAIN_CTRL].val = value;
		break;
	case V4L2_CID_TEST_PATTERN:
		switch (ctrl->val) {
		case 0:
			break;
		}
		/* FIXME Add proper Test pattern code here */
		return mt9v113_write_reg(client, 0, 0);

	default:
		v4l_err(client, "invalid control id %d\n", ctrl->id);
		return err;
	}

	v4l_dbg(1, debug, client,
			"Set Control: ID - %d - %d", ctrl->id, ctrl->val);

	return err;
}

static struct v4l2_ctrl_ops mt9v113_ctrl_ops = {
	.s_ctrl = mt9v113_s_ctrl,
};

static const struct v4l2_ctrl_config mt9v113_ctrls[] = {
	{
		.ops		= &mt9v113_ctrl_ops,
		.id		= V4L2_CID_TEST_PATTERN,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Test pattern",
		.min		= 0,
		.max		= 1023,
		.step		= 1,
		.def		= 0,
		.flags		= 0,
	}
};


/*
 * mt9v113_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * sub-device.
 */
static int mt9v113_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct mt9v113 *mt9v113;
	int i, ret;

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA)) {
		v4l_err(client, "mt9v113: I2C Adapter doesn't support" \
				" I2C_FUNC_SMBUS_WORD\n");
		return -EIO;
	}
	if (!client->dev.platform_data) {
		v4l_err(client, "No platform data!!\n");
		return -ENODEV;
	}

	mt9v113 = kzalloc(sizeof(*mt9v113), GFP_KERNEL);
	if (mt9v113 == NULL) {
		v4l_err(client, "Could not able to alocate memory!!\n");
		return -ENOMEM;
	}

	mt9v113->pdata = client->dev.platform_data;

	/*
	 * Initialize and register available controls
	 */
	v4l2_ctrl_handler_init(&mt9v113->ctrls, ARRAY_SIZE(mt9v113_ctrls) + 4);
	v4l2_ctrl_new_std(&mt9v113->ctrls, &mt9v113_ctrl_ops,
			V4L2_CID_AUTOGAIN, 0, 1, 1, 1);

	for (i = 0; i < ARRAY_SIZE(mt9v113_ctrls); ++i)
		v4l2_ctrl_new_custom(&mt9v113->ctrls, &mt9v113_ctrls[i], NULL);

	mt9v113->subdev.ctrl_handler = &mt9v113->ctrls;

	if (mt9v113->ctrls.error)
		v4l_info(client, "%s: error while initialization control %d\n",
				__func__, mt9v113->ctrls.error);

	/*
	 * Default configuration -
	 *	Resolution: VGA
	 *	Format: UYVY
	 *	crop = window
	 */
	mt9v113->rect.left = 0;
	mt9v113->rect.top = 0;
	mt9v113->rect.width = MT9V113_DEF_WIDTH;
	mt9v113->rect.height = MT9V113_DEF_HEIGHT;

	mt9v113->format.code = V4L2_MBUS_FMT_UYVY8_2X8;
	mt9v113->format.width = MT9V113_DEF_WIDTH;
	mt9v113->format.height = MT9V113_DEF_HEIGHT;
	mt9v113->format.field = V4L2_FIELD_NONE;
	mt9v113->format.colorspace = V4L2_COLORSPACE_JPEG;

	/*
	 * Register as a subdev
	 */
	v4l2_i2c_subdev_init(&mt9v113->subdev, client, &mt9v113_ops);
	mt9v113->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	/*
	 * Register as media entity
	 */
	mt9v113->pad.flags = MEDIA_PAD_FLAG_OUTPUT;
	ret = media_entity_init(&mt9v113->subdev.entity, 1, &mt9v113->pad, 0);
	if (ret < 0) {
		v4l_err(client, "failed to register as a media entity\n");
		kfree(mt9v113);
	}

	return ret;
}

/*
 * mt9v113_remove - Sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * sub-device.
 */
static int __exit mt9v113_remove(struct i2c_client *client)
{
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	struct mt9v113 *mt9v113 = to_mt9v113(subdev);

	v4l2_device_unregister_subdev(subdev);
	media_entity_cleanup(&subdev->entity);
	kfree(mt9v113);

	return 0;
}

static const struct i2c_device_id mt9v113_id[] = {
	{ MT9V113_MODULE_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, mt9v113_id);

static struct i2c_driver mt9v113_i2c_driver = {
	.driver = {
		.name = MT9V113_MODULE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = mt9v113_probe,
	.remove = __exit_p(mt9v113_remove),
	.id_table = mt9v113_id,
};

static int __init mt9v113_init(void)
{
	return i2c_add_driver(&mt9v113_i2c_driver);
}

static void __exit mt9v113_cleanup(void)
{
	i2c_del_driver(&mt9v113_i2c_driver);
}

module_init(mt9v113_init);
module_exit(mt9v113_cleanup);

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("MT9V113 camera sensor driver");
MODULE_LICENSE("GPL");
