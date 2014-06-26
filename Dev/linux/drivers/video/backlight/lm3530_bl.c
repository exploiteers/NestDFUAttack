/*
 *  lm3530_bl.c - Linux kernel module for National Semiconductor
 *                LM3530 backlight driver
 *
 *  Copyright (C) 2012 Nest Labs, Inc
 *  Copyright (C) 2011 ST-Ericsson SA.
 *  Copyright (C) 2009 Motorola, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Based on leds-lm3530.c by Dan Murphy <D.Murphy@motorola.com> and
 *  Shreshtha Kumar SAHU <shreshthakumar.sahu@stericsson.com>
 */

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/lm3530_bl.h>
#include <linux/gpio.h>
#include <linux/fb.h>

#define LM3530_DEV "lcd-backlight"
#define LM3530_NAME "lm3530-backlight"

#define LM3530_GEN_CONFIG		0x10
#define LM3530_ALS_CONFIG		0x20
#define LM3530_BRT_RAMP_RATE		0x30
#define LM3530_ALS_ZONE_REG		0x40
#define LM3530_ALS_IMP_SELECT		0x41
#define LM3530_BRT_CTRL_REG		0xA0
#define LM3530_ALS_ZB0_REG		0x60
#define LM3530_ALS_ZB1_REG		0x61
#define LM3530_ALS_ZB2_REG		0x62
#define LM3530_ALS_ZB3_REG		0x63
#define LM3530_ALS_Z0T_REG		0x70
#define LM3530_ALS_Z1T_REG		0x71
#define LM3530_ALS_Z2T_REG		0x72
#define LM3530_ALS_Z3T_REG		0x73
#define LM3530_ALS_Z4T_REG		0x74
#define LM3530_REG_MAX			15

/* General Control Register */
#define LM3530_EN_I2C_SHIFT		(0)
#define LM3530_RAMP_LAW_SHIFT		(1)
#define LM3530_MAX_CURR_SHIFT		(2)
#define LM3530_EN_PWM_SHIFT		(5)
#define LM3530_PWM_POL_SHIFT		(6)
#define LM3530_EN_PWM_SIMPLE_SHIFT	(7)

#define LM3530_ENABLE_I2C		(1 << LM3530_EN_I2C_SHIFT)
#define LM3530_ENABLE_PWM		(1 << LM3530_EN_PWM_SHIFT)
#define LM3530_POL_LOW			(1 << LM3530_PWM_POL_SHIFT)
#define LM3530_ENABLE_PWM_SIMPLE	(1 << LM3530_EN_PWM_SIMPLE_SHIFT)

/* ALS Config Register Options */
#define LM3530_ALS_AVG_TIME_SHIFT	(0)
#define LM3530_EN_ALS_SHIFT		(3)
#define LM3530_ALS_SEL_SHIFT		(5)

#define LM3530_ENABLE_ALS		(3 << LM3530_EN_ALS_SHIFT)

/* Brightness Ramp Rate Register */
#define LM3530_BRT_RAMP_FALL_SHIFT	(0)
#define LM3530_BRT_RAMP_RISE_SHIFT	(3)

#define LM3530_BRT_RAMP_FALL            (0x7 << LM3530_BRT_RAMP_FALL_SHIFT)
#define LM3530_BRT_RAMP_RISE            (0x7 << LM3530_BRT_RAMP_RISE_SHIFT)

/* ALS Resistor Select */
#define LM3530_ALS1_IMP_SHIFT		(0)
#define LM3530_ALS2_IMP_SHIFT		(4)

/* Zone Boundary Register defaults */
#define LM3530_ALS_ZB_MAX		(4)
#define LM3530_ALS_WINDOW_mV		(1000)
#define LM3530_ALS_OFFSET_mV		(4)

/* Zone Target Register defaults */
#define LM3530_DEF_ZT_0			(0x7F)
#define LM3530_DEF_ZT_1			(0x66)
#define LM3530_DEF_ZT_2			(0x4C)
#define LM3530_DEF_ZT_3			(0x33)
#define LM3530_DEF_ZT_4			(0x19)

/* Default brightness from kernel boot arguments, if provided */
static int default_brightness = -1;

/**
 * struct lm3530_data
 * @led_dev: led class device
 * @client: i2c client
 * @pdata: LM3530 platform data
 * @mode: mode of operation - manual, ALS, PWM
 * @regulator: regulator
 * @brighness: previous brightness value
 * @enable: regulator is enabled
 */
struct lm3530_data {
	struct i2c_client *client;
	struct lm3530_platform_data *pdata;
	enum lm3530_mode mode;
        int brightness;
	bool enable;
        int enable_gpio;
};

static const u8 lm3530_reg[LM3530_REG_MAX] = {
	LM3530_GEN_CONFIG,
	LM3530_ALS_CONFIG,
	LM3530_BRT_RAMP_RATE,
	LM3530_ALS_ZONE_REG,
	LM3530_ALS_IMP_SELECT,
	LM3530_BRT_CTRL_REG,
	LM3530_ALS_ZB0_REG,
	LM3530_ALS_ZB1_REG,
	LM3530_ALS_ZB2_REG,
	LM3530_ALS_ZB3_REG,
	LM3530_ALS_Z0T_REG,
	LM3530_ALS_Z1T_REG,
	LM3530_ALS_Z2T_REG,
	LM3530_ALS_Z3T_REG,
	LM3530_ALS_Z4T_REG,
};

static ssize_t lm3530_get_ramp_rise_rate(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t lm3530_set_ramp_rise_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t len);

static ssize_t lm3530_get_ramp_fall_rate(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t lm3530_set_ramp_fall_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t len);

static ssize_t lm3530_get_ramp_law(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t lm3530_set_ramp_law(struct device *dev, struct device_attribute *attr, const char *buf, size_t len);

static DEVICE_ATTR(ramp_rise_rate, (S_IRUGO|S_IWUGO), lm3530_get_ramp_rise_rate, lm3530_set_ramp_rise_rate);
static DEVICE_ATTR(ramp_fall_rate, (S_IRUGO|S_IWUGO), lm3530_get_ramp_fall_rate, lm3530_set_ramp_fall_rate);
static DEVICE_ATTR(ramp_law, (S_IRUGO|S_IWUGO), lm3530_get_ramp_law, lm3530_set_ramp_law);
 
static struct attribute *lm3530_attributes[] = {
	&dev_attr_ramp_rise_rate.attr,
	&dev_attr_ramp_fall_rate.attr,
	&dev_attr_ramp_law.attr,
	NULL
};

static const struct attribute_group lm3530_attr_group = {
	.attrs = lm3530_attributes,
};

static int __init lm3530_set_default_brightness(char *brightness)
{ 
    if (brightness != NULL)
    {
        default_brightness = simple_strtol(brightness, NULL, 10);
    }
    
    return 0;
}

static ssize_t lm3530_get_ramp_rise_rate(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct lm3530_platform_data *pdata = client->dev.platform_data;

    return sprintf(buf, "%hhu\n", pdata->brt_ramp_rise);
}

static ssize_t lm3530_set_ramp_rise_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
    u8 ramp_reg_val;
    u8 new_ramp_rate;
    struct i2c_client *client = to_i2c_client(dev);
    struct lm3530_platform_data *pdata = client->dev.platform_data;

    sscanf(buf, "%hhu", &new_ramp_rate);

    if (new_ramp_rate <= LM3530_RAMP_TIME_8s) {
        pdata->brt_ramp_rise = new_ramp_rate;
        ramp_reg_val = i2c_smbus_read_byte_data(client, LM3530_BRT_RAMP_RATE);

        ramp_reg_val = (ramp_reg_val & ~(LM3530_BRT_RAMP_RISE)) | (new_ramp_rate << LM3530_BRT_RAMP_RISE_SHIFT);

		i2c_smbus_write_byte_data(client, LM3530_BRT_RAMP_RATE, ramp_reg_val);
    }

    return strnlen(buf, PAGE_SIZE);
}

static ssize_t lm3530_get_ramp_fall_rate(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct lm3530_platform_data *pdata = client->dev.platform_data;

    return sprintf(buf, "%hhu\n", pdata->brt_ramp_fall);
}

static ssize_t lm3530_set_ramp_fall_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
    u8 ramp_reg_val;
    u8 new_ramp_rate;
    struct i2c_client *client = to_i2c_client(dev);
    struct lm3530_platform_data *pdata = client->dev.platform_data;

    sscanf(buf, "%hhu", &new_ramp_rate);

    if (new_ramp_rate <= LM3530_RAMP_TIME_8s) {
        pdata->brt_ramp_fall = new_ramp_rate;
        ramp_reg_val = i2c_smbus_read_byte_data(client, LM3530_BRT_RAMP_RATE);

        ramp_reg_val = (ramp_reg_val & ~(LM3530_BRT_RAMP_FALL)) | (new_ramp_rate << LM3530_BRT_RAMP_FALL_SHIFT);

		i2c_smbus_write_byte_data(client, LM3530_BRT_RAMP_RATE, ramp_reg_val);
    }

    return strnlen(buf, PAGE_SIZE);
}

static ssize_t lm3530_get_ramp_law(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct lm3530_platform_data *pdata = client->dev.platform_data;

    return sprintf(buf, "%hhu\n", pdata->brt_ramp_law);
}

static ssize_t lm3530_set_ramp_law(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
    u8 gen_config_reg;
    u8 new_ramp_law;
    struct i2c_client *client = to_i2c_client(dev);
    struct lm3530_platform_data *pdata = client->dev.platform_data;

    sscanf(buf, "%hhu", &new_ramp_law);

    if ((new_ramp_law == LM3530_RAMP_LAW_EXP) || 
            (new_ramp_law == LM3530_RAMP_LAW_LIN)) {
        pdata->brt_ramp_law = new_ramp_law;

        gen_config_reg = i2c_smbus_read_byte_data(client, LM3530_GEN_CONFIG);
        gen_config_reg &= (~(0x1 << LM3530_RAMP_LAW_SHIFT));
        gen_config_reg |= (new_ramp_law << LM3530_RAMP_LAW_SHIFT);
		i2c_smbus_write_byte_data(client,  LM3530_GEN_CONFIG, gen_config_reg);
    }

    return strnlen(buf, PAGE_SIZE);
}

__setup("brightness=", lm3530_set_default_brightness);

static int lm3530_init_registers(struct lm3530_data *drvdata)
{
	int ret = 0;
	int i;
	u8 gen_config;
	u8 als_config = 0;
	u8 brt_ramp;
	u8 als_imp_sel = 0;
	u8 reg_val[LM3530_REG_MAX];
	u8 zones[LM3530_ALS_ZB_MAX];
	u32 als_vmin, als_vmax, als_vstep;
	struct lm3530_platform_data *pltfm = drvdata->pdata;
	struct i2c_client *client = drvdata->client;

	memset(zones, 0, LM3530_ALS_ZB_MAX);

	gen_config = (pltfm->brt_ramp_law << LM3530_RAMP_LAW_SHIFT) |
			((pltfm->max_current & 7) << LM3530_MAX_CURR_SHIFT);

	if (drvdata->mode == LM3530_BL_MODE_ALS) {
		if (pltfm->als_vmax == 0) {
			pltfm->als_vmin = 0;
			pltfm->als_vmax = LM3530_ALS_WINDOW_mV;
		}

		als_vmin = pltfm->als_vmin;
		als_vmax = pltfm->als_vmax;

		if ((als_vmax - als_vmin) > LM3530_ALS_WINDOW_mV)
			pltfm->als_vmax = als_vmax =
				als_vmin + LM3530_ALS_WINDOW_mV;

		/* n zone boundary makes n+1 zones */
		als_vstep = (als_vmax - als_vmin) / (LM3530_ALS_ZB_MAX + 1);

		for (i = 0; i < LM3530_ALS_ZB_MAX; i++)
			zones[i] = (((als_vmin + LM3530_ALS_OFFSET_mV) +
					als_vstep + (i * als_vstep)) * 255)
					/ 1000;

		als_config =
			(pltfm->als_avrg_time << LM3530_ALS_AVG_TIME_SHIFT) |
			(LM3530_ENABLE_ALS) |
			(pltfm->als_input_mode << LM3530_ALS_SEL_SHIFT);

		als_imp_sel =
			(pltfm->als1_resistor_sel << LM3530_ALS1_IMP_SHIFT) |
			(pltfm->als2_resistor_sel << LM3530_ALS2_IMP_SHIFT);

	}

	if (drvdata->mode == LM3530_BL_MODE_PWM)
		gen_config |= (LM3530_ENABLE_PWM) |
				(pltfm->pwm_pol_hi << LM3530_PWM_POL_SHIFT) |
				(LM3530_ENABLE_PWM_SIMPLE);

	brt_ramp = (pltfm->brt_ramp_fall << LM3530_BRT_RAMP_FALL_SHIFT) |
			(pltfm->brt_ramp_rise << LM3530_BRT_RAMP_RISE_SHIFT);

	reg_val[0] = gen_config;	/* LM3530_GEN_CONFIG */
	reg_val[1] = als_config;	/* LM3530_ALS_CONFIG */
	reg_val[2] = brt_ramp;		/* LM3530_BRT_RAMP_RATE */
	reg_val[3] = 0x00;		/* LM3530_ALS_ZONE_REG */
	reg_val[4] = als_imp_sel;	/* LM3530_ALS_IMP_SELECT */
	reg_val[5] = 0;				/* LM3530_BRT_CTRL_REG */
	reg_val[6] = zones[0];		/* LM3530_ALS_ZB0_REG */
	reg_val[7] = zones[1];		/* LM3530_ALS_ZB1_REG */
	reg_val[8] = zones[2];		/* LM3530_ALS_ZB2_REG */
	reg_val[9] = zones[3];		/* LM3530_ALS_ZB3_REG */
	reg_val[10] = LM3530_DEF_ZT_0;	/* LM3530_ALS_Z0T_REG */
	reg_val[11] = LM3530_DEF_ZT_1;	/* LM3530_ALS_Z1T_REG */
	reg_val[12] = LM3530_DEF_ZT_2;	/* LM3530_ALS_Z2T_REG */
	reg_val[13] = LM3530_DEF_ZT_3;	/* LM3530_ALS_Z3T_REG */
	reg_val[14] = LM3530_DEF_ZT_4;	/* LM3530_ALS_Z4T_REG */

	if (!drvdata->enable) {
                gpio_set_value(pltfm->enable_gpio, 1);
                mdelay(3);
		drvdata->enable = true;
	}

	for (i = 0; i < LM3530_REG_MAX; i++) {
		ret = i2c_smbus_write_byte_data(client,
				lm3530_reg[i], reg_val[i]);
		if (ret)
			break;
	}

	/* Set the enable bit to turn on the backlight last */
	if (drvdata->mode == LM3530_BL_MODE_MANUAL ||
		drvdata->mode == LM3530_BL_MODE_ALS) {
		gen_config |= (LM3530_ENABLE_I2C);

		ret = i2c_smbus_write_byte_data(client,
						LM3530_GEN_CONFIG, gen_config);
	}
        
	return ret;
}

static int lm3530_update_status(struct backlight_device *bl)
{
    int brightness = bl->props.brightness;

	int err;

	struct lm3530_data *drvdata = bl_get_data(bl);

	if (bl->props.power != FB_BLANK_UNBLANK)
		brightness = 0;

	if (bl->props.fb_blank != FB_BLANK_UNBLANK)
		brightness = 0;

	switch (drvdata->mode) {
	case LM3530_BL_MODE_MANUAL:

		BUG_ON(brightness > bl->props.max_brightness);

		if (!drvdata->enable) {
			err = lm3530_init_registers(drvdata);
			if (err) {
				dev_err(&drvdata->client->dev,
					"Register Init failed: %d\n", err);
				break;
			}
		}

		/* set the brightness in brightness control register */
		err = i2c_smbus_write_byte_data(drvdata->client,
				LM3530_BRT_CTRL_REG, brightness);
		if (err)
			dev_err(&drvdata->client->dev,
				"Unable to set brightness: %d\n", err);
		else
			drvdata->brightness = brightness;

            // Leaving this out for now.  It will cut off any automatic
            // fade down.  We may want it, though, if we're doing manual
            // fades to save power when the backlight is off. Note that
            // on suspend, it does get disabled.
		/*if (brightness == 0) {
			//err = regulator_disable(drvdata->regulator);
			//if (err)
			//	dev_err(&drvdata->client->dev,
			//		"Disable regulator failed\n");
                        gpio_set_value(drvdata->enable_gpio, 0);
			drvdata->enable = false;
                }*/
		break;
	case LM3530_BL_MODE_ALS:
		break;
	case LM3530_BL_MODE_PWM:
		break;
	default:
		break;
	}

    return 0;
}

static int lm3530_get_brightness(struct backlight_device *bl)
{
    return bl->props.brightness;
}

static struct backlight_ops lm3530_backlight_ops = {
	.update_status	= lm3530_update_status,
	.get_brightness	= lm3530_get_brightness,
};

static int __devinit lm3530_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct lm3530_platform_data *pdata = client->dev.platform_data;
	struct lm3530_data *drvdata;
	struct backlight_device *bl;
        struct backlight_properties props;
        int status;
	unsigned gpio;

	if (pdata == NULL) {
		dev_err(&client->dev, "platform data required\n");
		status = -ENODEV;
		goto err_out;
	}

	/* BL mode */
	if (pdata->mode > LM3530_BL_MODE_PWM) {
		dev_err(&client->dev, "Illegal Mode request\n");
		status = -EINVAL;
		goto err_out;
	}

	if (pdata->wait_for_framebuffer) {
		struct device_driver *fb;
		fb = driver_find(pdata->wait_for_framebuffer, &platform_bus_type);
		if (fb == NULL) {
			return -EPROBE_DEFER;
		} else {
			// driver_find increments the retain count
			put_driver(fb);
		}
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "I2C_FUNC_I2C not supported\n");
		status = -EIO;
		goto err_out;
	}

	drvdata = kzalloc(sizeof(struct lm3530_data), GFP_KERNEL);
	if (drvdata == NULL) {
		status = -ENOMEM;
		goto err_out;
	}

	drvdata->mode = pdata->mode;
	drvdata->client = client;
	drvdata->pdata = pdata;
	drvdata->enable = false;

	i2c_set_clientdata(client, drvdata);

	// Request and export the enable GPIO
	gpio = drvdata->pdata->enable_gpio;

	status = gpio_request(gpio, "lm3530 enable");
	if (status) {
		dev_err(&client->dev, "Could not request GPIO %u: %d\n", gpio, status);
		goto dealloc_data;
	}

	status = gpio_direction_output(gpio, 0);
	if (status) {
		dev_err(&client->dev, "Could not set GPIO %u as output to value %d: %d\n", gpio, 1, status);
		goto free_gpio;
	}

	status = gpio_export(gpio, 0);
	if (status) {
		dev_err(&client->dev, "Could not export GPIO %u: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export_link(&client->dev, "enable", gpio);
	if (status) {
		dev_err(&client->dev, "Could not export GPIO %u as link %s: %d\n", gpio, "enable", status);
		goto unexport_gpio;
	}

	drvdata->enable_gpio = gpio;

        memset(&props, 0, sizeof(struct backlight_properties));
        props.max_brightness = pdata->max_brightness;

	bl = backlight_device_register(dev_name(&client->dev), &client->dev,
			drvdata, &lm3530_backlight_ops, &props);

	if (IS_ERR(bl)) {
		dev_err(&client->dev, "failed to register backlight\n");
		status = PTR_ERR(bl);
		goto unexport_gpio;
	}

	bl->props.max_brightness = pdata->max_brightness;

        /* If a default was provided at the kernal command line,
         * use that.  Otherwise, take the platform data default.
         */
        if (default_brightness != -1) {
            bl->props.brightness = default_brightness;
        } else {
            bl->props.brightness = pdata->dft_brightness;
        }

	BUG_ON(pdata->dft_brightness > pdata->max_brightness);

	backlight_update_status(bl);

	platform_set_drvdata(client, bl);

        status = sysfs_create_group(&client->dev.kobj, &lm3530_attr_group);
	if (status) {
		dev_err(&client->dev,
				"Failed to create sysfs group\n");
		goto unexport_gpio;
	}
        
	return 0;

unexport_gpio:
	gpio_unexport(gpio);
free_gpio:
	gpio_free(gpio);
dealloc_data:
	kfree(drvdata);
err_out:

	return status;
}

static int __devexit lm3530_remove(struct i2c_client *client)
{
	struct lm3530_data *drvdata = i2c_get_clientdata(client);
	struct backlight_device *bl = platform_get_drvdata(client);

	gpio_set_value(drvdata->enable_gpio, 0);
	sysfs_remove_group(&client->dev.kobj, &lm3530_attr_group);
	backlight_device_unregister(bl);
	gpio_unexport(drvdata->enable_gpio);
	gpio_free(drvdata->enable_gpio);
	kfree(drvdata);

	return 0;
}

#ifdef CONFIG_PM

static int lm3530_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct lm3530_data *drvdata = bl_get_data(bl);

	gpio_set_value(drvdata->enable_gpio, 0);
    drvdata->enable = false;

    return 0;
}

static int lm3530_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct backlight_device *bl = platform_get_drvdata(pdev);

	lm3530_update_status(bl);

    return 0;
}

#else

#define lm3530_suspend    NULL
#define lm3530_resume     NULL

#endif /* CONFIG_PM */

static void lm3530_shutdown(struct i2c_client *client)
{
	struct lm3530_platform_data *pdata = client->dev.platform_data;

	gpio_set_value(pdata->enable_gpio, 0);
}

static const struct i2c_device_id lm3530_id[] = {
	{LM3530_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, lm3530_id);

static struct dev_pm_ops lm3530_pm_ops = {
	.suspend = lm3530_suspend,
	.resume = lm3530_resume,
};

static struct i2c_driver lm3530_i2c_driver = {
	.probe = lm3530_probe,
	.remove = __devexit_p(lm3530_remove),
	.id_table = lm3530_id,
	.shutdown = lm3530_shutdown,
	.driver = {
		.name = LM3530_NAME,
		.owner = THIS_MODULE,
		.pm = &lm3530_pm_ops,
	},
};

static int __init lm3530_init(void)
{
  return i2c_add_driver(&lm3530_i2c_driver);
}

static void __exit lm3530_exit(void)
{
  i2c_del_driver(&lm3530_i2c_driver);
}

module_init(lm3530_init);
module_exit(lm3530_exit);

MODULE_DESCRIPTION("Back Light driver for LM3530");
MODULE_LICENSE("GPLv2");
MODULE_AUTHOR("Jonathan Solnit <jsolnit@nestlabs.com>");
