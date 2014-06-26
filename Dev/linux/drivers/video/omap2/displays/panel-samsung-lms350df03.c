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
 *      This file is the LCD panel driver for the Samsung LMS350DF03.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/lms350df03.h>
#include <linux/spi/spi.h>

#include <plat/display.h>

/* Preprocessor Definitions */

/*
 * Driver Strings
 */
#define LMS350DF03_DRIVER_NAME			"Samsung LMS350DF03 LCD Driver"
#define LMS350DF03_DRIVER_VERSION		"2010-10-04"

/*
 * Samsung LMS350DF03 Slave SPI Protocol Definitions
 */

#define LMS350DF03_XSCK_MAX				10000000

#define LMS350DF03_ID					0x74
#define	LMS350DF03_INDEX				((LMS350DF03_ID) | 0x00)
#define LMS350DF03_DATA					((LMS350DF03_ID) | 0x02)
#define LMS350DF03_READ					0x01
#define LMS350DF03_WRITE				0x00

/*
 * Samsung LMS350DF03 Register Indices and Field Definitions
 *
 * In general, all interface driver registers and values are opaque
 * and static; however, Samsung does define a few of them as
 * transparent and caller-mutable.
 */

#define	LMS350DF03_REG_VAL(bit, value)					\
	((value) << (bit))
#define LMS350DF03_REG_VAL_ENCODE(shift, mask, value)	\
	(((value) << (shift)) & (mask))

#define LMS350DF03_INVERSION			0x01
#define LMS350DF03_RGB_INVERSION_ON			LMS350DF03_REG_VAL(11, 0)
#define LMS350DF03_RGB_INVERSION_OFF		LMS350DF03_REG_VAL(11, 1)
#define LMS350DF03_VERT_INVERSION_ON		LMS350DF03_REG_VAL( 9, 0)
#define LMS350DF03_VERT_INVERSION_OFF		LMS350DF03_REG_VAL( 9, 1)
#define LMS350DF03_HORIZ_INVERSION_ON		LMS350DF03_REG_VAL( 8, 0)
#define LMS350DF03_HORIZ_INVERSION_OFF		LMS350DF03_REG_VAL( 8, 1)
#define LMS350DF03_INVERSION_CONSTANT		0x003b

#define LMS350DF03_POLARITY				0x03
#define LMS350DF03_VSYNC_POLARITY_HI		LMS350DF03_REG_VAL(15, 0)
#define LMS350DF03_VSYNC_POLARITY_LO		LMS350DF03_REG_VAL(15, 1)
#define LMS350DF03_HSYNC_POLARITY_HI		LMS350DF03_REG_VAL(14, 0)
#define LMS350DF03_HSYNC_POLARITY_LO		LMS350DF03_REG_VAL(14, 1)
#define LMS350DF03_DOTCLK_POLARITY_HI		LMS350DF03_REG_VAL(13, 0)
#define LMS350DF03_DOTCLK_POLARITY_LO		LMS350DF03_REG_VAL(13, 1)
#define LMS350DF03_ENABLE_POLARITY_LO		LMS350DF03_REG_VAL(12, 0)
#define LMS350DF03_ENABLE_POLARITY_HI		LMS350DF03_REG_VAL(12, 1)
#define LMS350DF03_SYNC_ON_ENABLE_ON		LMS350DF03_REG_VAL( 6, 0)
#define LMS350DF03_SYNC_ON_ENABLE_OFF		LMS350DF03_REG_VAL( 6, 1)
#define LMS350DF03_POLARITY_CONSTANT		0x0000

#define	LMS350DF03_BP_ENCODE(x, bp)		LMS350DF03_REG_VAL_ENCODE(0,		\
											LMS350DF03_##bp##_MASK,			\
											(max((u16)(x), \
											  (u16)LMS350DF03_##bp##_MIN) - \
											 LMS350DF03_##bp##_OFFSET))

#define LMS350DF03_VBP					0x08
#define LMS350DF03_VBP_OFFSET				2
#define LMS350DF03_VBP_MIN					3
#define LMS350DF03_VBP_MAX					257
#define LMS350DF03_VBP_MASK					LMS350DF03_REG_VAL(0, 0xff)
#define LMS350DF03_VBP_ENCODE(x)			LMS350DF03_BP_ENCODE(x, VBP)

#define	LMS350DF03_HBP					0x09
#define LMS350DF03_HBP_OFFSET				0
#define LMS350DF03_HBP_MIN					8
#define	LMS350DF03_HBP_MAX					511
#define LMS350DF03_HBP_MASK					LMS350DF03_REG_VAL(0, 0x1ff)
#define LMS350DF03_HBP_ENCODE(x)			LMS350DF03_BP_ENCODE(x, HBP)

#define FFRAMEHZ						60
#define	TFRAMEMS						(1000 / FFRAMEHZ)

#define fdelay(frames)					mdelay(frames * TFRAMEMS)

/* Type Definitions */

struct lms350df03_device {
	int 						enabled:1,
								suspended:1;
	struct spi_device *			spi;
	struct omap_dss_device *	dssdev;
	struct regulator *			vcc_reg;
};

/* Function Prototypes */

static int	samsung_lms350df03_probe(struct omap_dss_device *dssdev);
static void	samsung_lms350df03_remove(struct omap_dss_device *dssdev);
static int	samsung_lms350df03_dss_enable(struct omap_dss_device *dssdev);
static int	samsung_lms350df03_dss_power_on(struct omap_dss_device *dssdev);
static void	samsung_lms350df03_dss_disable(struct omap_dss_device *dssdev);
static void	samsung_lms350df03_dss_power_off(struct omap_dss_device *dssdev);
static int	samsung_lms350df03_suspend(struct omap_dss_device *dssdev);
static int	samsung_lms350df03_resume(struct omap_dss_device *dssdev);

static int	lms350df03_spi_probe(struct spi_device *spi);
static int	lms350df03_spi_remove(struct spi_device *spi);

/* Global Variables */

static struct omap_video_timings samsung_lms350df03_timings = {
	.x_res			= 320,		// Horizontal resolution, pixels
	.y_res			= 480,		// Vertical resolution, pixels

	.pixel_clock	= 9800,		// Pixel clock, kHz

	.hsw			= 4,		// Horizontal synchronization pulse width
	.hfp			= 5,		// Horizontal front porch, pixels clocks
	.hbp			= 11,		// Horizontal back porch, pixel clocks

	.vsw			= 2,		// Vertical synchronization pulse width
	.vfp			= 5,		// Vertical front porch, line clocks
	.vbp			= 5,		// Vertical back porch, line clocks
};

static struct omap_dss_driver samsung_lms350df03_driver = {
	.probe			= samsung_lms350df03_probe,
	.remove			= samsung_lms350df03_remove,

	.enable			= samsung_lms350df03_dss_enable,
	.disable		= samsung_lms350df03_dss_disable,
	.suspend		= samsung_lms350df03_suspend,
	.resume			= samsung_lms350df03_resume,

	.driver         = {
		.name   		= "samsung_lms350df03",
		.owner  		= THIS_MODULE,
	}
};

static struct spi_driver samsung_lms350df03_spi_driver = {
	.driver			= {
		.name			= "lms350df03",
		.bus			= &spi_bus_type,
		.owner			= THIS_MODULE,
	},
	.probe			= lms350df03_spi_probe,
	.remove			= __devexit_p(lms350df03_spi_remove),
};

static struct lms350df03_device lms350df03_dev;

static void lms350df03_control(struct spi_device *spi, u8 reg, u16 value)
{
	u8 indices[3] = { LMS350DF03_INDEX, 0, 0 };
	u8 data[3] = { LMS350DF03_DATA, 0, 0 };

	dev_dbg(&spi->dev, "Writing 0x%04x to 0x%02x\n", value, reg);

	// Send the register (index) to write to.

	indices[2] = reg;

	spi_write(spi, indices, ARRAY_SIZE(indices));

	// Send the data to write at that register (index).

	data[1] = value >> 8;
	data[2] = value & 0xFF;

	spi_write(spi, data, ARRAY_SIZE(data));
}

static void lms350df03_reset(unsigned long gpio, bool inverted)
{
	// The data sheet says that the reset pulse must be asserted for
	// at minimum 30 us. First, ensure the reset line is deasserted
	// for a "sufficiently long" time as we don't know it's initial
	// condition. Then, assert it for at least the required
	// time. Finally, deassert it again.

	gpio_set_value(gpio, !inverted);
	mdelay(4);

	gpio_set_value(gpio, inverted);
	udelay(30 * 2);

	gpio_set_value(gpio, !inverted);
	mdelay(4);	
}

static void samsung_lms350df03_power_on(struct omap_dss_device *dssdev)
{
	struct lms350df03_device *ld = &lms350df03_dev;
	struct spi_device *spi = ld->spi;
	const u16 inversion = (LMS350DF03_RGB_INVERSION_OFF		|
						   LMS350DF03_VERT_INVERSION_OFF	|
						   LMS350DF03_HORIZ_INVERSION_OFF	|
						   LMS350DF03_INVERSION_CONSTANT);
	const u16 polarity = (LMS350DF03_VSYNC_POLARITY_LO		|
						  LMS350DF03_HSYNC_POLARITY_LO		|
						  LMS350DF03_DOTCLK_POLARITY_LO		|
						  LMS350DF03_ENABLE_POLARITY_LO		|
						  LMS350DF03_SYNC_ON_ENABLE_OFF		|
						  LMS350DF03_POLARITY_CONSTANT);

	// Due to differences in how Samsung and Texas Instruments
	// interpret the back porch, we have to account for the sync pulse
	// width in the back porch encoding.

	const u16 vbp = LMS350DF03_VBP_ENCODE(dssdev->panel.timings.vbp +
										  dssdev->panel.timings.vsw - 1);
	const u16 hbp = LMS350DF03_HBP_ENCODE(dssdev->panel.timings.hbp +
										  dssdev->panel.timings.hsw);

	// Wait 1 ms

	mdelay(1);

	// Reset

	lms350df03_control(spi, 0x07, 0x0000);

	// Wait more than 10 ms

	mdelay(10 * 2);

	lms350df03_control(spi, 0x11, 0x222f);
	lms350df03_control(spi, 0x12, 0x0f00);
	lms350df03_control(spi, 0x13, 0x7fe3);
	lms350df03_control(spi, 0x76, 0x2213);
	lms350df03_control(spi, 0x74, 0x0001);
	lms350df03_control(spi, 0x76, 0x0000);
	lms350df03_control(spi, 0x10, 0x560c);

	// Wait more than 6 frames

	fdelay(6 * 2);

	lms350df03_control(spi, 0x12, 0x0c63);

	// Wait more than 5 frames

	fdelay(5 * 2);

	lms350df03_control(spi, LMS350DF03_INVERSION, inversion);
	lms350df03_control(spi, 0x02, 0x0300);
	lms350df03_control(spi, LMS350DF03_POLARITY, polarity);
	lms350df03_control(spi, LMS350DF03_VBP, vbp);
	lms350df03_control(spi, LMS350DF03_HBP, hbp);
	lms350df03_control(spi, 0x76, 0x2213);
	lms350df03_control(spi, 0x0b, 0x3340);
	lms350df03_control(spi, 0x0c, 0x0020);
	lms350df03_control(spi, 0x1c, 0x7770);
	lms350df03_control(spi, 0x76, 0x0000);
	lms350df03_control(spi, 0x0d, 0x0000);
	lms350df03_control(spi, 0x0e, 0x0500);
	lms350df03_control(spi, 0x14, 0x0000);
	lms350df03_control(spi, 0x15, 0x0803);
	lms350df03_control(spi, 0x16, 0x0000);
	lms350df03_control(spi, 0x30, 0x0005);
	lms350df03_control(spi, 0x31, 0x070f);
	lms350df03_control(spi, 0x32, 0x0300);
	lms350df03_control(spi, 0x33, 0x0003);
	lms350df03_control(spi, 0x34, 0x090c);
	lms350df03_control(spi, 0x35, 0x0001);
	lms350df03_control(spi, 0x36, 0x0001);
	lms350df03_control(spi, 0x37, 0x0303);
	lms350df03_control(spi, 0x38, 0x0f09);
	lms350df03_control(spi, 0x39, 0x0105);
}

static void samsung_lms350df03_power_off(struct omap_dss_device *dssdev)
{
	struct lms350df03_device *ld = &lms350df03_dev;
	struct spi_device *spi = ld->spi;

	lms350df03_control(spi, 0x10, 0x0001);
	lms350df03_control(spi, 0x0b, 0x30e1);
	lms350df03_control(spi, 0x07, 0x0102);

	// Wait more than 2 frames

	fdelay(2 * 2);

	lms350df03_control(spi, 0x07, 0x0000);
	lms350df03_control(spi, 0x12, 0x0000);
	lms350df03_control(spi, 0x10, 0x0100);
}

static void samsung_lms350df03_display_on(struct omap_dss_device *dssdev)
{
	struct lms350df03_device *ld = &lms350df03_dev;
	struct spi_device *spi = ld->spi;

	lms350df03_control(spi, 0x07, 0x0001);

	// Wait more than 1 frame

	fdelay(1 * 2);

	lms350df03_control(spi, 0x07, 0x0101);

	// Wait more than 2 frames

	fdelay(2 * 2);

	lms350df03_control(spi, 0x07, 0x0103);
}

static int samsung_lms350df03_probe(struct omap_dss_device *dssdev)
{
	struct spi_device *spi;
	struct lms350df03_platform_data *pdata;
	struct regulator *reg;
	const char *supply;
	int status;

	// Cache a reference to the OMAP DSS device in our device-private
	// data.

	lms350df03_dev.dssdev = dssdev;

	spi = lms350df03_dev.spi;
	pdata = spi->dev.platform_data;

	// If we were supplied platform-specific data, request and assign
	// the reset GPIO.

	if (pdata != NULL) {
		status = gpio_request(pdata->reset.gpio, "lms350df03 reset");

		if (status) {
			dev_err(&dssdev->dev,
					"Couldn't reserve GPIO %ld for lms350df03 reset.\n",
					pdata->reset.gpio);
			goto done;
		}

		status = gpio_direction_output(pdata->reset.gpio,
									   !pdata->reset.inverted);

		if (status) {
			dev_err(&dssdev->dev,
					"Couldn't set GPIO %ld output for lms350df03 reset.\n",
					pdata->reset.gpio);
			goto err;
		}

		supply = pdata->regulator.vcc;

		if (supply != NULL) {
			reg = regulator_get(&dssdev->dev, supply);

			if (IS_ERR(reg)) {
				status = PTR_ERR(reg);
				dev_err(&dssdev->dev, "Could not get requested regulator supply '%s': %d\n", supply, status);
				goto err;

			} else {
				lms350df03_dev.vcc_reg = reg;

			}
		}

	} else {
		dev_warn(&dssdev->dev,
				 "No platform reset data was provided, the display may not "
				 "initialize properly.\n");

	}

	// Configure the panel as a TFT LCD with both horizontal and
	// vertical sync.

	dssdev->panel.config = (OMAP_DSS_LCD_TFT |
							OMAP_DSS_LCD_IVS |
							OMAP_DSS_LCD_IHS);

	// Copy the default timing parameters.

	dssdev->panel.timings = samsung_lms350df03_timings;

 err:
	if (pdata != NULL) {
		gpio_free(pdata->reset.gpio);
	}

 done:
	return (status);
}

static void samsung_lms350df03_remove(struct omap_dss_device *dssdev)
{
	struct spi_device *spi = lms350df03_dev.spi;
	struct lms350df03_platform_data *pdata = spi->dev.platform_data;

	samsung_lms350df03_dss_disable(dssdev);

	if (lms350df03_dev.vcc_reg) {
		regulator_put(lms350df03_dev.vcc_reg);
	}

	if (pdata != NULL) {
		gpio_free(pdata->reset.gpio);
	}

	return;
}

static int samsung_lms350df03_dss_enable(struct omap_dss_device *dssdev)
{
   int r;
   r = samsung_lms350df03_dss_power_on(dssdev);
   dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
   return r;
}

static int samsung_lms350df03_dss_power_on(struct omap_dss_device *dssdev)
{
	struct device * dev = &dssdev->dev;
	struct lms350df03_device *ld = &lms350df03_dev;
	struct spi_device * spi = ld->spi;
	struct lms350df03_platform_data *pdata = spi->dev.platform_data;
	int status = 0;
        int r = 0;
        if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE) return 0;

        r = omapdss_dpi_display_enable(dssdev);
        if (r) return r;

        if (ld->vcc_reg) {
            status = regulator_enable(ld->vcc_reg);
            dev_info(dev, "Enabled regulator with status %d\n", status);
            if (status) {
                goto platform_disable;
            }
        }

	if (dssdev->platform_enable) {
		status = dssdev->platform_enable(dssdev);

		if (status) {
			dev_err(dev, "The platform failed to enable the display.\n");
			goto err1;
		}
	}

	if (pdata) {
		lms350df03_reset(pdata->reset.gpio, pdata->reset.inverted);
	}

	samsung_lms350df03_power_on(dssdev);
	samsung_lms350df03_display_on(dssdev);

	ld->enabled = true;
	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	goto done;

 platform_disable:
    if (dssdev->platform_disable) {
        dssdev->platform_disable(dssdev);
    }

 err1:
        omapdss_dpi_display_disable(dssdev);
 done:
	return (status);
}


static void samsung_lms350df03_dss_disable(struct omap_dss_device *dssdev)
{
       samsung_lms350df03_dss_power_off(dssdev);
       dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
}

static void samsung_lms350df03_dss_power_off(struct omap_dss_device *dssdev)
{
	struct lms350df03_device *ld = NULL;
        if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE) return;

	ld = &lms350df03_dev;
        if (ld->vcc_reg) {
            regulator_disable(ld->vcc_reg);
        }
        omapdss_dpi_display_disable(dssdev);	

	ld->enabled = false;
	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
}

static int samsung_lms350df03_suspend(struct omap_dss_device *dssdev)
{
	struct lms350df03_device *ld = &lms350df03_dev;
	int status = 0;
        samsung_lms350df03_dss_power_off(dssdev);

        if(!ld->suspended){
          ld->suspended = true;
	  dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;
        }

	return (status);
}

static int samsung_lms350df03_resume(struct omap_dss_device *dssdev)
{
	struct lms350df03_device *ld = NULL;
	int status = 0;
        int r = 0;

	ld = &lms350df03_dev;
        r = samsung_lms350df03_dss_power_on(dssdev);
        if(r) return r;

        if(ld->suspended){
	  ld->suspended = false;
	  dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
        }

	return status;
}

static int lms350df03_spi_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct lms350df03_device *ld = NULL;
	int status = 0;

	// Check to ensure the specified platform SPI clock doesn't exceed
	// the allowed maximum.

	if (spi->max_speed_hz > LMS350DF03_XSCK_MAX) {
		dev_err(dev, "The SPI interface clock must be less than or "
				"equal to %d KHz\n", LMS350DF03_XSCK_MAX / 1000);
        return (-EINVAL);
    }

	// Get the device private data and save a reference to the SPI
	// device pointer.

	ld = &lms350df03_dev;
	ld->spi = spi;

	// The Samsung LMS350DF03 SPI interface requires mode 3 and 8
	// bits-per-word.

	spi->mode = SPI_MODE_3;
	spi->bits_per_word = 8;

	// Set up the SPI controller interface for the chip select channel
	// we'll be using for SPI transactions associated with this
	// device.

	status = spi_setup(spi);

	if (status < 0) {
		dev_err(dev, "Failed to setup SPI controller with error %d\n", status);
		goto done;
	}

	// Register our device private data with the SPI driver.

	spi_set_drvdata(spi, ld);

	// Register our panel-/module-specific methods with the OMAP DSS
	// driver.

	omap_dss_register_driver(&samsung_lms350df03_driver);

 done:
	return (status);
}

static int lms350df03_spi_remove(struct spi_device *spi)
{
	int status = 0;

	omap_dss_unregister_driver(&samsung_lms350df03_driver);

	return (status);
}

static int __init samsung_lms350df03_drv_init(void)
{
	int status = 0;

	pr_info("%s %s\n", LMS350DF03_DRIVER_NAME, LMS350DF03_DRIVER_VERSION);

	status = spi_register_driver(&samsung_lms350df03_spi_driver);

	return (status);
}

static void __exit samsung_lms350df03_drv_exit(void)
{
	spi_unregister_driver(&samsung_lms350df03_spi_driver);
}

module_init(samsung_lms350df03_drv_init);
module_exit(samsung_lms350df03_drv_exit);

MODULE_AUTHOR("Nest Labs, Inc.");
MODULE_DESCRIPTION(LMS350DF03_DRIVER_NAME);
MODULE_LICENSE("GPLv2");
