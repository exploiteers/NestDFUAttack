/*
 *    Copyright (c) 2010 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program. If not, see
 *    <http://www.gnu.org/licenses/>.
 *
 *    Description:
 *      This file is the LCD panel driver for the Giantplus GPM1145A
 *      320 x 480 TFT LCD display panel using the Ilitek ILI9481
 *      interface driver.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/spi/ili9481.h>
#include <linux/spi/spi.h>

#include <plat/display.h>

/* Preprocessor Definitions */

/*
 * Driver Strings
 */
#define GPM1145A0_DRIVER_NAME			"Giantplus GPM1145A0 LCD Driver"
#define GPM1145A0_DRIVER_VERSION		"2010-10-17"

/*
 * 32- and 8-bit encode and decode macros
 */

#define U32_ENCODE(b3, b2, b1, b0)		((((b3) & 0xff) << 24) |	\
										 (((b2) & 0xff) << 16) |	\
										 (((b1) & 0xff) <<  8) |	\
										 (((b0) & 0xff) <<  0))
#define U32_B3_DECODE(u32)				(((u32) >> 24) & 0xFF)
#define U32_B2_DECODE(u32)				(((u32) >> 16) & 0xFF)
#define U32_B1_DECODE(u32)				(((u32) >>  8) & 0xFF)
#define U32_B0_DECODE(u32)				(((u32) >>  0) & 0xFF)

/*
 * Ilitek ILI9481 Slave SPI Protocol Definitions
 */
#define ILI9481_SPI_XSCK_MAX			8333333
#define ILI9481_SPI_MODE				SPI_MODE_3
#define ILI9481_SPI_BITS_PER_WORD		9
#define ILI9481_SPI_BYTES_PER_WORD		sizeof (ili9481_word)

/*
 * Ilitek ILI9481 Reset Timings
 */
#define ILI9481_TRES_LOW_MS_MIN			10
#define ILI9481_TRES_HIGH_MS_MIN		50

/*
 * Ilitek ILI9481 Commands
 */

#define	ILI9481_REG_VAL(bit, value)					\
	((value) << (bit))
#define ILI9481_REG_VAL_ENCODE(shift, mask, value)	\
	(((value) << (shift)) & (mask))

/* Control Commands */

#define ILI9481_OP_CMD_NOP								0x00
#define ILI9481_OP_CMD_SOFT_RESET						0x01
#define ILI9481_OP_CMD_ENTER_SLEEP_MODE					0x10
#define ILI9481_OP_CMD_EXIT_SLEEP_MODE					0x11
#define ILI9481_OP_CMD_ENTER_PARTIAL_MODE				0x12
#define ILI9481_OP_CMD_ENTER_NORMAL_MODE				0x13
#define ILI9481_OP_CMD_EXIT_INVERT_MODE					0x20
#define ILI9481_OP_CMD_ENTER_INVERT_MODE				0x21
#define ILI9481_OP_CMD_DISPLAY_OFF						0x28
#define ILI9481_OP_CMD_DISPLAY_ON						0x29
#define ILI9481_OP_CMD_TEAR_OFF							0x34
#define ILI9481_OP_CMD_EXIT_IDLE_MODE					0x38
#define ILI9481_OP_CMD_ENTER_IDLE_MODE					0x39

/* Read / Get Commands */

#define ILI9481_OP_GET_RED_CHANNEL						0x06
#define ILI9481_OP_GET_GREEN_CHANNEL					0x07
#define ILI9481_OP_GET_BLUE_CHANNEL						0x08
#define ILI9481_OP_GET_POWER_MODE						0x0A
#define ILI9481_OP_GET_ADDRESS_MODE						0x0B
#define ILI9481_OP_GET_PIXEL_FORMAT						0x0C
#define ILI9481_OP_GET_DISPLAY_MODE						0x0D
#define ILI9481_OP_GET_SIGNAL_MODE						0x0E
#define ILI9481_OP_GET_DIAGNOSTIC_RESULT				0x0F
#define ILI9481_OP_GET_MEMORY_START						0x2E
#define ILI9481_OP_GET_MEMORY_CONTINUE					0x3E
#define ILI9481_OP_GET_SCANLINE							0x45
#define ILI9481_OP_GET_DDB_START						0xA1
#define ILI9481_OP_GET_DEVICE_CODE						0xBF
#define ILI9481_OP_GET_NV_MEMORY_STATUS					0xE2

/* Write / Set Commands */

#define ILI9481_OP_SET_GAMMA_CURVE						0x26
#define ILI9481_OP_SET_COLUMN_ADDRESS					0x2A
#define ILI9481_OP_SET_PAGE_ADDRESS						0x2B
#define ILI9481_OP_SET_MEMORY_START						0x2C
#define ILI9481_OP_SET_LUT								0x2D
#define ILI9481_OP_SET_PARTIAL_AREA						0x30
#define ILI9481_OP_SET_SCROLL_AREA						0x33
#define ILI9481_OP_SET_TEAR_ON							0x35
#define ILI9481_OP_SET_ADDRESS_MODE						0x36
#define ILI9481_ADDRESS_MODE_PAGE_ADDR_ORDER_TTOB			ILI9481_REG_VAL(7, 0)
#define ILI9481_ADDRESS_MODE_PAGE_ADDR_ORDER_BTOT			ILI9481_REG_VAL(7, 1)
#define ILI9481_ADDRESS_MODE_COL_ADDR_ORDER_TTOB			ILI9481_REG_VAL(6, 0)
#define ILI9481_ADDRESS_MODE_COL_ADDR_ORDER_BTOT			ILI9481_REG_VAL(6, 1)
#define ILI9481_ADDRESS_MODE_PAGE_COL_NORMAL				ILI9481_REG_VAL(5, 0)
#define ILI9481_ADDRESS_MODE_PAGE_COL_REVERSE				ILI9481_REG_VAL(5, 1)
#define ILI9481_ADDRESS_MODE_LINE_ADDR_ORDER_TTOB			ILI9481_REG_VAL(4, 0)
#define ILI9481_ADDRESS_MODE_LINE_ADDR_ORDER_BTOT			ILI9481_REG_VAL(4, 1)
#define ILI9481_ADDRESS_MODE_PIXEL_ORDER_RGB				ILI9481_REG_VAL(3, 0)
#define ILI9481_ADDRESS_MODE_PIXEL_ORDER_BGR				ILI9481_REG_VAL(3, 1)
#define ILI9481_ADDRESS_MODE_HFLIP_OFF						ILI9481_REG_VAL(1, 0)
#define ILI9481_ADDRESS_MODE_HFLIP_ON						ILI9481_REG_VAL(1, 1)
#define ILI9481_ADDRESS_MODE_VFLIP_OFF						ILI9481_REG_VAL(0, 0)
#define ILI9481_ADDRESS_MODE_VFLIP_ON						ILI9481_REG_VAL(0, 1)
#define ILI9481_OP_SET_SCROLL_START						0x37
#define ILI9481_OP_SET_PIXEL_FORMAT						0x3A
#define ILI9481_PIXEL_FORMAT_3BPP							ILI9481_REG_VAL(0, 0x11)
#define ILI9481_PIXEL_FORMAT_16BPP							ILI9481_REG_VAL(0, 0x55)
#define ILI9481_PIXEL_FORMAT_18BPP							ILI9481_REG_VAL(0, 0x66)
#define ILI9481_OP_SET_MEMORY_CONTINUE					0x3C
#define ILI9481_OP_SET_TEAR_SCANLINE					0x44

/* Transfer Commands */

#define ILI9481_OP_XFR_COMMAND_ACCESS_PROTECT			0xB0
#define ILI9481_OP_XFR_FRAME_MEM_ACCESS_IFACE_SETTING	0xB3
#define ILI9481_OP_XFR_DISPLAY_MODE_FRAME_MEM_WR_MODE	0xB4
#define ILI9481_DISPLAY_MODE_FRAME_MEM_WR_MODE_DM_SCLK		ILI9481_REG_VAL(0, 0)
#define ILI9481_DISPLAY_MODE_FRAME_MEM_WR_MODE_DM_DPI		ILI9481_REG_VAL(0, 1)
#define ILI9481_DISPLAY_MODE_FRAME_MEM_WR_MODE_RM_DBI		ILI9481_REG_VAL(4, 0)
#define ILI9481_DISPLAY_MODE_FRAME_MEM_WR_MODE_RM_DPI		ILI9481_REG_VAL(4, 1)
#define ILI9481_OP_XFR_PANEL_DRIVING_SETTING			0xC0
#define ILI9481_OP_XFR_NORMAL_MODE_TIMING_SETTING		0xC1
#define ILI9481_OP_XFR_PARTIAL_MODE_TIMING_SETTING		0xC2
#define ILI9481_OP_XFR_IDLE_MODE_TIMING_SETTING			0xC3
#define ILI9481_OP_XFR_FRAME_RATE_AND_INVERSION_CONTROL	0xC5
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_125HZ		ILI9481_REG_VAL(0, 0x0)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_100HZ		ILI9481_REG_VAL(0, 0x1)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_85HZ		ILI9481_REG_VAL(0, 0x2)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_72HZ		ILI9481_REG_VAL(0, 0x3)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_56HZ		ILI9481_REG_VAL(0, 0x4)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_50HZ		ILI9481_REG_VAL(0, 0x5)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_45HZ		ILI9481_REG_VAL(0, 0x6)
#define ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_42HZ		ILI9481_REG_VAL(0, 0x7)
#define ILI9481_OP_XFR_INTERFACE_CONTROL				0xC6
#define ILI9481_INTERFACE_CONTROL_DOUT_ENABLE				ILI9481_REG_VAL(7, 0)
#define ILI9481_INTERFACE_CONTROL_DOUT_DISABLE				ILI9481_REG_VAL(7, 1)
#define ILI9481_INTERFACE_CONTROL_VSPL_LO					ILI9481_REG_VAL(4, 0)
#define ILI9481_INTERFACE_CONTROL_VSPL_HI					ILI9481_REG_VAL(4, 1)
#define ILI9481_INTERFACE_CONTROL_HSPL_LO					ILI9481_REG_VAL(3, 0)
#define ILI9481_INTERFACE_CONTROL_HSPL_HI					ILI9481_REG_VAL(3, 1)
#define ILI9481_INTERFACE_CONTROL_EPL_LO					ILI9481_REG_VAL(1, 0)
#define ILI9481_INTERFACE_CONTROL_EPL_HI					ILI9481_REG_VAL(1, 1)
#define ILI9481_INTERFACE_CONTROL_DPL_RISING				ILI9481_REG_VAL(0, 0)
#define ILI9481_INTERFACE_CONTROL_DPL_FALLING				ILI9481_REG_VAL(0, 1)

#define ILI9481_OP_XFR_GAMMA_SETTING					0xC8
#define ILI9481_OP_XFR_POWER_SETTING					0xD0
#define ILI9481_OP_XFR_VCOM_CONTROL						0xD1
#define ILI9481_OP_XFR_NORMAL_MODE_POWER_SETTING		0xD2
#define ILI9481_OP_XFR_PARTIAL_MODE_POWER_SETTING		0xD3
#define ILI9481_OP_XFR_IDLE_MODE_POWER_SETTING			0xD4
#define ILI9481_OP_XFR_NV_MEMORY_WRITE					0xE0
#define ILI9481_OP_XFR_NV_MEMORY_CONTROL				0xE1
#define ILI9481_OP_XFR_NV_MEMORY_PROTECTION				0xE3

/*
 * All commands and read and write parameters are 9 bits. There are
 * 8-bits of lower-order data and high-order bit, D/CX. For commands,
 * D/CX is set to 0, for parameters, it is set to 1. To accomodate
 * this, we need 16-bits for each parameter.
 */

#define	ILI9481_COMMAND_MASK				0xff
#define ILI9481_PARAM_MASK					0xff

#define ILI9481_COMMAND_DCX					0x00
#define ILI9481_PARAM_DCX					0x01

#define ILI9481_WORD_ENCODE(dcx, mask, x)	(((dcx) << 8) | ((x) & (mask)))
#define ILI9481_WORD_DECODE(x, mask)		((x) & (mask))

#define ILI9481_COMMAND_ENCODE(x)			ILI9481_WORD_ENCODE(ILI9481_COMMAND_DCX, ILI9481_COMMAND_MASK, x)
#define ILI9481_PARAM_ENCODE(x)				ILI9481_WORD_ENCODE(ILI9481_PARAM_DCX, ILI9481_PARAM_MASK, x)
#define ILI9481_PARAM_DECODE(x)				ILI9481_WORD_DECODE(x, ILI9481_PARAM_MASK)

#define ILI9481_PARAMS_TO_BYTES(n)			((n) * ILI9481_SPI_BYTES_PER_WORD)
#define ILI9481_BYTES_TO_PARAMS(b)			((b) / ILI9481_SPI_BYTES_PER_WORD)

#define FFRAMEHZ							60
#define	TFRAMEMS							(1000 / FFRAMEHZ)

#define fdelay(frames)						mdelay(frames * TFRAMEMS)

/* Type Definitions */

struct ili9481_device {
	int 						enabled:1,
								suspended:1;
	struct spi_device *			spi;
	struct omap_dss_device *	dssdev;
	struct backlight_device *	backlight;
};

typedef u16 ili9481_word;

/* Function Prototypes */

static int	gpm1145a0_dss_probe(struct omap_dss_device *dssdev);
static void	gpm1145a0_dss_remove(struct omap_dss_device *dssdev);
static int	gpm1145a0_dss_enable(struct omap_dss_device *dssdev);
static void	gpm1145a0_dss_disable(struct omap_dss_device *dssdev);
static int	gpm1145a0_dss_suspend(struct omap_dss_device *dssdev);
static int	gpm1145a0_dss_resume(struct omap_dss_device *dssdev);

static int	ili9481_spi_probe(struct spi_device *spi);
static int	ili9481_spi_remove(struct spi_device *spi);

/* Global Variables */

static struct omap_video_timings giantplus_gpm1145a0_timings = {
	.x_res			= 320,		// Horizontal resolution, pixels
	.y_res			= 480,		// Vertical resolution, pixels

	.pixel_clock	= 8000,		// Pixel clock, kHz

	.hsw			= 2,		// Horizontal synchronization pulse width
	.hfp			= 3,		// Horizontal front porch, pixels clocks
	.hbp			= 3,		// Horizontal back porch, pixel clocks

	.vsw			= 2,		// Vertical synchronization pulse width
	.vfp			= 4,		// Vertical front porch, line clocks
	.vbp			= 2,		// Vertical back porch, line clocks
};

static struct omap_dss_driver giantplus_gpm1145a0_driver = {
	.probe			= gpm1145a0_dss_probe,
	.remove			= gpm1145a0_dss_remove,

	.enable			= gpm1145a0_dss_enable,
	.disable		= gpm1145a0_dss_disable,
	.suspend		= gpm1145a0_dss_suspend,
	.resume			= gpm1145a0_dss_resume,

	.driver         = {
		.name   		= "giantplus_gpm1145a0",
		.owner  		= THIS_MODULE,
	}
};

static struct spi_driver ilitek_ili9481_spi_driver = {
	.driver			= {
		.name			= "ili9481",
		.bus			= &spi_bus_type,
		.owner			= THIS_MODULE,
	},
	.probe			= ili9481_spi_probe,
	.remove			= __devexit_p(ili9481_spi_remove),
};

static struct ili9481_device ili9481_dev;

static void ili9481_transfer(struct ili9481_device * id, u8 operation,
							 const u8 *wbuf, unsigned int wlen,
							 u8 *rbuf, unsigned int rlen)
{
	struct spi_message m;
	struct spi_transfer	*x, xfer[4];
	ili9481_word	command;
	ili9481_word	word;
	int				status;

	BUG_ON(id->spi == NULL);

	dev_dbg(&id->spi->dev, "%s transfer %#02x %u byte%s out @ %p, "
			"%u byte%s in @ %p.\n",
			((wbuf != NULL) ?
			 ((rbuf != NULL) ? "Bi-directional" : "Write") :
			 ((rbuf != NULL) ? "Read" : "Command")),
			operation,
			wlen, ((wlen == 1) ? "" : "s"), wbuf,
			rlen, ((rlen == 1) ? "" : "s"), rbuf);

	spi_message_init(&m);

	memset(xfer, 0, sizeof(xfer));
	x = &xfer[0];

	// At minimum, we have a command, read, write or bi-directional
	// transfer operation to handle that requires a single send
	// phase. Handle that phase.

	command = ILI9481_COMMAND_ENCODE(operation);

	x->tx_buf					= &command;
	x->bits_per_word			= ILI9481_SPI_BITS_PER_WORD;
	x->len						= ILI9481_SPI_BYTES_PER_WORD;

	spi_message_add_tail(x, &m);

	// If we have a write or bi-directional transfer operation, handle
	// the send phase for it.

	if ((wlen > 0) && (wbuf != NULL)) {
		x++;

		x->tx_buf				= wbuf;
		x->bits_per_word		= ILI9481_SPI_BITS_PER_WORD;
		x->len					= wlen;

		spi_message_add_tail(x, &m);
	}

	// If we have a read or bi-directional transfer operation, handle
	// the receive phase for it.

	if ((rlen > 0) && (rbuf != NULL)) {
		x++;
		x->rx_buf				= &word;
		x->len					= 1;

		spi_message_add_tail(x, &m);

		// Arrange for the extra clock before the first
		// data bit.

		if (rlen > 1) {
			x->bits_per_word	= ILI9481_SPI_BITS_PER_WORD;
			x->len				= ILI9481_SPI_BYTES_PER_WORD;

			x++;
			x->rx_buf			= &rbuf[1];
			x->len				= rlen - 1;

			spi_message_add_tail(x, &m);
		}
	}

    status = spi_sync(id->spi, &m);

	if (status < 0) {
		dev_warn(&id->spi->dev, "ILI9481 SPI transfer failed with status %d\n",
				 status);
		goto done;
	}

	if (rlen) {
		rbuf[0] = ILI9481_PARAM_DECODE(word);
	}

 done:
	return;
}

static void ili9481_command(struct ili9481_device * id, u8 operation)
{
	ili9481_transfer(id, operation, NULL, 0, NULL, 0);
}

static void ili9481_read(struct ili9481_device * id, u8 operation,
						 u8 *buffer, unsigned int length)
{
	ili9481_transfer(id, operation, NULL, 0, buffer, length);
}

static void ili9481_write(struct ili9481_device * id, u8 operation,
						  const u8 *buffer, unsigned int length)
{
	ili9481_transfer(id, operation, buffer, length, NULL, 0);
}

static void ili9481_reset(unsigned long gpio, bool inverted)
{
	const bool asserted = inverted;
	
	// First, ensure the reset line is deasserted for a "sufficiently
	// long" time as we don't know it's initial condition. Then,
	// assert it for at least the required time. Finally, deassert it
	// again for at least the required time.

	gpio_set_value(gpio, !asserted);
	mdelay(4);

	gpio_set_value(gpio, asserted);
	mdelay(ILI9481_TRES_LOW_MS_MIN * 2);

	gpio_set_value(gpio, !asserted);
	mdelay(ILI9481_TRES_HIGH_MS_MIN * 2);
}

static bool ili9481_detect(struct ili9481_device * id)
{
	const u8 length = 6;
	u8 buffer[length];
	u32 code = 0;
	const char * name = NULL;
	bool detected = false;

	ili9481_read(id, ILI9481_OP_GET_DEVICE_CODE, buffer, length);

	code = U32_ENCODE(buffer[1], buffer[2], buffer[3], buffer[4]);

	switch (code) {

	case 0x02049481:
		name = "Giantplus GPM1145A0";
		detected = true;
		break;

	default:
		dev_err(&id->spi->dev, "Unrecognized display ID: %08x\n", code);
		name = NULL;
		detected = false;
		break;

	}

	if (name != NULL) {
		dev_info(&id->spi->dev, "%s (%08x) LCD detected.\n", name, code);
	}

	return (detected);
}

static void ili9481_power_on(struct ili9481_device * id)
{
	int status = 0;
	const unsigned int maxparam = 12;
	ili9481_word params[maxparam];
	unsigned int nparams;
	struct omap_dss_device *dss;
	struct ili9481_platform_data *pdata;
	u32 width, height;

	dss = id->dssdev;
	pdata = id->spi->dev.platform_data;

	// First, allow the platform to do any necessary steps (turn on
	// rails, etc.).

	if (dss->platform_enable) {
		status = dss->platform_enable(dss);

		if (status) {
			dev_err(&dss->dev, "The platform failed to enable the display.\n");
			goto done;
		}
	}

	// Then, wait at least 1 ms.

	mdelay(1 * 2);

	// Next, issue the power-on reset.

	ili9481_reset(pdata->reset.gpio, pdata->reset.inverted);

	// Then, issue a soft reset and wait at least 20 ms.

	ili9481_command(id, ILI9481_OP_CMD_SOFT_RESET);

	mdelay(20 * 2);

	// Then, send the exit sleep mode command and wait at least 80 ms.

	ili9481_command(id, ILI9481_OP_CMD_EXIT_SLEEP_MODE);

	mdelay(80 * 2);

	// Then, program the display settings and power supply operation.

	params[0] = ILI9481_PARAM_ENCODE(ILI9481_ADDRESS_MODE_PAGE_ADDR_ORDER_BTOT |
									 ILI9481_ADDRESS_MODE_COL_ADDR_ORDER_TTOB  |
									 ILI9481_ADDRESS_MODE_PAGE_COL_NORMAL      |
									 ILI9481_ADDRESS_MODE_LINE_ADDR_ORDER_TTOB |
									 ILI9481_ADDRESS_MODE_PIXEL_ORDER_BGR      |
									 ILI9481_ADDRESS_MODE_HFLIP_OFF            |
									 ILI9481_ADDRESS_MODE_VFLIP_OFF);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_SET_ADDRESS_MODE,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(ILI9481_PIXEL_FORMAT_18BPP);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_SET_PIXEL_FORMAT,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x07);
	params[1] = ILI9481_PARAM_ENCODE(0x42);
	params[2] = ILI9481_PARAM_ENCODE(0x1b);
	nparams = 3;
	ili9481_write(id,
				  ILI9481_OP_XFR_POWER_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	params[1] = ILI9481_PARAM_ENCODE(0x24);
	params[2] = ILI9481_PARAM_ENCODE(0x12);
	nparams = 3;
	ili9481_write(id,
				  ILI9481_OP_XFR_VCOM_CONTROL,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x02);
	params[1] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 2;
	ili9481_write(id,
				  ILI9481_OP_XFR_NORMAL_MODE_POWER_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));     

	params[0] = ILI9481_PARAM_ENCODE(0x01);
	params[1] = ILI9481_PARAM_ENCODE(0x22);
	nparams = 2;
	ili9481_write(id,
				  ILI9481_OP_XFR_PARTIAL_MODE_POWER_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));      

	params[0] = ILI9481_PARAM_ENCODE(0x01);
	params[1] = ILI9481_PARAM_ENCODE(0x22);
	nparams = 2;
	ili9481_write(id,
				  ILI9481_OP_XFR_IDLE_MODE_POWER_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x10);
	params[1] = ILI9481_PARAM_ENCODE(0x3b);
	params[2] = ILI9481_PARAM_ENCODE(0x00);
	params[3] = ILI9481_PARAM_ENCODE(0x02);
	params[4] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 5;
	ili9481_write(id,
				  ILI9481_OP_XFR_PANEL_DRIVING_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x10);
	params[1] = ILI9481_PARAM_ENCODE(0x10);
	params[2] = ILI9481_PARAM_ENCODE(0x22);
	nparams = 3;
	ili9481_write(id,
				  ILI9481_OP_XFR_NORMAL_MODE_TIMING_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(ILI9481_FRAME_RATE_AND_INVERSION_CONTROL_56HZ);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_XFR_FRAME_RATE_AND_INVERSION_CONTROL,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(ILI9481_INTERFACE_CONTROL_DOUT_DISABLE |
									 ILI9481_INTERFACE_CONTROL_VSPL_LO      |
									 ILI9481_INTERFACE_CONTROL_HSPL_LO      |
									 ILI9481_INTERFACE_CONTROL_EPL_HI       |
									 ILI9481_INTERFACE_CONTROL_DPL_FALLING);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_XFR_INTERFACE_CONTROL,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	params[1] = ILI9481_PARAM_ENCODE(0x37);
	params[2] = ILI9481_PARAM_ENCODE(0x02);
	params[3] = ILI9481_PARAM_ENCODE(0x26);
	params[4] = ILI9481_PARAM_ENCODE(0x0b);
	params[5] = ILI9481_PARAM_ENCODE(0x08);
	params[6] = ILI9481_PARAM_ENCODE(0x46);
	params[7] = ILI9481_PARAM_ENCODE(0x03);
	params[8] = ILI9481_PARAM_ENCODE(0x77);
	params[9] = ILI9481_PARAM_ENCODE(0x52);
	params[10] = ILI9481_PARAM_ENCODE(0x00);
	params[11] = ILI9481_PARAM_ENCODE(0x1e);
	nparams = 12;
	ili9481_write(id,
				  ILI9481_OP_XFR_GAMMA_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_XFR_NV_MEMORY_WRITE,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_XFR_NV_MEMORY_CONTROL,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	params[1] = ILI9481_PARAM_ENCODE(0x00);
	params[2] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 3;
	ili9481_read(id,
				 ILI9481_OP_GET_NV_MEMORY_STATUS,
				 (u8 *)params,
				 ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	params[1] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 2;
	ili9481_write(id,
				  ILI9481_OP_XFR_NV_MEMORY_PROTECTION,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_XFR_COMMAND_ACCESS_PROTECT,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x02);
	params[1] = ILI9481_PARAM_ENCODE(0x00);
	params[2] = ILI9481_PARAM_ENCODE(0x00);
	params[3] = ILI9481_PARAM_ENCODE(0x00);
	nparams = 4;
	ili9481_write(id,
				  ILI9481_OP_XFR_FRAME_MEM_ACCESS_IFACE_SETTING,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(ILI9481_DISPLAY_MODE_FRAME_MEM_WR_MODE_DM_DPI |
									 ILI9481_DISPLAY_MODE_FRAME_MEM_WR_MODE_RM_DPI);
	nparams = 1;
	ili9481_write(id,
				  ILI9481_OP_XFR_DISPLAY_MODE_FRAME_MEM_WR_MODE,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	// The following three power control commands are undocumented by
	// either Ilitek or Giantplus

	params[0] = ILI9481_PARAM_ENCODE(0x40);
	params[1] = ILI9481_PARAM_ENCODE(0x0f);
	nparams = 2;
	ili9481_write(id,
				  0xf3,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x80);
	nparams = 1;
	ili9481_write(id,
				  0xf6,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	params[0] = ILI9481_PARAM_ENCODE(0x80);
	params[1] = ILI9481_PARAM_ENCODE(0x01);
	nparams = 2;
	ili9481_write(id,
				  0xf7,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	// Next, send the display on command.

	ili9481_command(id, ILI9481_OP_CMD_DISPLAY_ON);

	// Finally, prepare the display to receive write data, setting the
	// column and page addresses to the display extents.

	width = id->dssdev->panel.timings.x_res;

	params[0] = ILI9481_PARAM_ENCODE(U32_B3_DECODE(width - 1));
	params[1] = ILI9481_PARAM_ENCODE(U32_B2_DECODE(width - 1));
	params[2] = ILI9481_PARAM_ENCODE(U32_B1_DECODE(width - 1));
	params[3] = ILI9481_PARAM_ENCODE(U32_B0_DECODE(width - 1));
	nparams = 4;
	ili9481_write(id,
				  ILI9481_OP_SET_COLUMN_ADDRESS,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	height = id->dssdev->panel.timings.y_res;

	params[0] = ILI9481_PARAM_ENCODE(U32_B3_DECODE(height - 1));
	params[1] = ILI9481_PARAM_ENCODE(U32_B2_DECODE(height - 1));
	params[2] = ILI9481_PARAM_ENCODE(U32_B1_DECODE(height - 1));
	params[3] = ILI9481_PARAM_ENCODE(U32_B0_DECODE(height - 1));
	nparams = 4;
	ili9481_write(id,
				  ILI9481_OP_SET_PAGE_ADDRESS,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

	ili9481_write(id,
				  ILI9481_OP_SET_MEMORY_START,
				  (const u8 *)params,
				  ILI9481_PARAMS_TO_BYTES(nparams));

 done:
	return;
}

static void ili9481_power_off(struct ili9481_device * id)
{
	// First, run the display off command

	ili9481_command(id, ILI9481_OP_CMD_DISPLAY_OFF);

	// Next, enter sleep mode

	ili9481_command(id, ILI9481_OP_CMD_ENTER_SLEEP_MODE);

	// Wait at least 2 frames

	fdelay(2 * 2);

	// Finally, allow the platform to do any necessary steps (turn off
	// rails, etc.)

	if (id->dssdev->platform_disable) {
		id->dssdev->platform_disable(id->dssdev);
	}
}

static int	gpm1145a0_dss_probe(struct omap_dss_device *dssdev)
{
	struct ili9481_device *id = NULL;
	struct spi_device *spi = NULL;
	struct ili9481_platform_data *pdata = NULL;
	int status = 0;

	id = &ili9481_dev;

	// Otherwise, the detect succeeded. So, cache a reference to the
	// OMAP DSS device in our device-private data.

	id->dssdev = dssdev;

	spi = id->spi;
	pdata = spi->dev.platform_data;

	// Platform data is required. Without it, we cannot determine the
	// reset GPIO and reset polarity. And without those, we cannot
	// successfully initialize the display.

	if (pdata == NULL) {
		dev_err(&spi->dev, "Could not retrieve platform data for display. "
				"Unable to initialize display.\n");
		status = -EINVAL;
		goto done;
	}

	// If we were supplied platform-specific data, request and assign
	// the reset GPIO.

	status = gpio_request(pdata->reset.gpio, "ili9481 reset");

	if (status) {
		dev_err(&spi->dev,
				"Couldn't reserve GPIO %ld for ili9481 reset.\n",
				pdata->reset.gpio);
		goto done;
	}

	status = gpio_direction_output(pdata->reset.gpio,
								   !pdata->reset.inverted);

	if (status) {
		dev_err(&spi->dev,
				"Couldn't set GPIO %ld output for ili9481 reset.\n",
				pdata->reset.gpio);
		goto err;
	}

	// Configure the panel as a TFT LCD with both horizontal and
	// vertical sync.

	dssdev->panel.config = (OMAP_DSS_LCD_TFT |
							OMAP_DSS_LCD_IVS |
							OMAP_DSS_LCD_IHS);

	// Copy the default timing parameters.

	dssdev->panel.timings = giantplus_gpm1145a0_timings;

 err:
	if (pdata != NULL) {
		gpio_free(pdata->reset.gpio);
	}

 done:
	return (status);
}

static void	gpm1145a0_dss_remove(struct omap_dss_device *dssdev)
{
	struct spi_device *spi;
	struct ili9481_platform_data *pdata;

	spi = ili9481_dev.spi;
	pdata = spi->dev.platform_data;

	if (pdata != NULL) {
		gpio_free(pdata->reset.gpio);
	}

	return;
}

static int gpm1145a0_dss_enable(struct omap_dss_device *dssdev)
{
	struct ili9481_device * id = NULL;
	int status = 0;

	id = &ili9481_dev;

	ili9481_power_on(id);

	id->enabled = true;
	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	return (status);
}

static void	gpm1145a0_dss_disable(struct omap_dss_device *dssdev)
{
	struct ili9481_device * id = NULL;

	id = &ili9481_dev;
	
	ili9481_power_off(id);

	id->enabled = false;
	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
}

static int gpm1145a0_dss_suspend(struct omap_dss_device *dssdev)
{
	struct ili9481_device * id = NULL;
	int status = 0;

	id = &ili9481_dev;
	
	ili9481_power_off(id);

	id->suspended = true;
	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;

	return (status);
}

static int gpm1145a0_dss_resume(struct omap_dss_device *dssdev)
{
	struct ili9481_device * id = NULL;
	int status = 0;

	id = &ili9481_dev;

	ili9481_power_on(id);

	id->suspended = false;
	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	return (status);
}

static int ili9481_spi_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct ili9481_device *id = NULL;
	int status = 0;

	// Check to ensure the specified platform SPI clock doesn't exceed
	// the allowed maximum.

	if (spi->max_speed_hz > ILI9481_SPI_XSCK_MAX) {
		dev_err(dev, "The SPI interface clock must be less than or "
				"equal to %d KHz\n", ILI9481_SPI_XSCK_MAX / 1000);
        return (-EINVAL);
    }

	// Get the device private data and save a reference to the SPI
	// device pointer.

	id = &ili9481_dev;
	id->spi = spi;

  
	// The Ilitek ILI9481 SPI interface requires mode 3. Bits-per-word
	// is variable and is set on a per-transfer basis.

	spi->mode = ILI9481_SPI_MODE;

	// Set up the SPI controller interface for the chip select channel
	// we'll be using for SPI transactions associated with this
	// device.

	status = spi_setup(spi);

	if (status < 0) {
		dev_err(dev, "Failed to setup SPI controller with error %d\n", status);
		goto done;
	}

	// Register our device private data with the SPI driver.

	spi_set_drvdata(spi, id);

#if 0
	// XXX - We cannot currently perform a detect because we have a
	// write-only SPI interface to the panel.
	//
	// First, attempt to probe the panel. If we cannot find a match,
	// there's no sense in going any further.

	if (!ili9481_detect(id)) {
		dev_err(&spi->dev, "Could not detect an ILI9481-compatible panel.\n");
		status = -ENODEV;
		goto done;
	}
#endif

	// Register our panel-/module-specific methods with the OMAP DSS
	// driver.

	omap_dss_register_driver(&giantplus_gpm1145a0_driver);

 done:
	return (status);
}

static int ili9481_spi_remove(struct spi_device *spi)
{
	int status = 0;

	omap_dss_unregister_driver(&giantplus_gpm1145a0_driver);

	return (status);
}

static int __init giantplus_gpm1145a0_init(void)
{
	int status = 0;

	pr_info("%s %s\n", GPM1145A0_DRIVER_NAME, GPM1145A0_DRIVER_VERSION);

	status = spi_register_driver(&ilitek_ili9481_spi_driver);

	return (status);
}

static void __exit giantplus_gpm1145a0_exit(void)
{
	spi_unregister_driver(&ilitek_ili9481_spi_driver);
}

module_init(giantplus_gpm1145a0_init);
module_exit(giantplus_gpm1145a0_exit);

MODULE_AUTHOR("Nest Labs, Inc.");
MODULE_DESCRIPTION(GPM1145A0_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
