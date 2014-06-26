/*
 * linux/drivers/video/backlight/pwm_bl.c
 *
 * simple PWM based backlight control, board code has to setup
 * 1) pin configuration so PWM waveforms can output
 * 2) platform_data being correctly configured
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/slab.h>

/* Default brightness from kernel boot arguments, if provided */
static int default_brightness = -1;

struct pwm_bl_data {
	struct pwm_device	*pwm;
	struct device		*dev;
	bool			enabled;
	const unsigned long     *ramp;
	unsigned int		period;
	unsigned int		lth_brightness;
	int			(*notify)(struct device *,
					  int brightness);
};

static int __init pwm_backlight_set_default_brightness(char *brightness)
{ 
    if (brightness != NULL)
    {
        default_brightness = simple_strtol(brightness, NULL, 10);
    }
    
    return 0;
}

__setup("brightness=", pwm_backlight_set_default_brightness);

static inline int pwm_backlight_get_duty_ns(const struct pwm_bl_data *pb, int brightness, int max)
{
	if (pb->ramp)
		return pb->ramp[brightness];
	else
		return brightness * pb->period / max;
}


static int pwm_backlight_update_status(struct backlight_device *bl)
{
	struct pwm_bl_data *pb = bl_get_data(bl);
	struct platform_pwm_backlight_data *data = bl->dev.parent->platform_data;
	int brightness = bl->props.brightness;
	int max = bl->props.max_brightness;
	int pulse;

	if (bl->props.power != FB_BLANK_UNBLANK)
		brightness = 0;

	if (bl->props.fb_blank != FB_BLANK_UNBLANK)
		brightness = 0;

	if (pb->notify)
		brightness = pb->notify(pb->dev, brightness);

	if (brightness == 0) {
		pwm_config(pb->pwm, 0, pb->period);
		pwm_disable(pb->pwm);
		pb->enabled = false;
	} else {

		/* Convert the brightness step into a PWM pulse duration */
		pulse = pwm_backlight_get_duty_ns(pb, brightness, max);

		BUG_ON(pulse > pb->period);

		/* If the backlight is not enabled check for and run LED
		 * driver-specific start-up requirements.
		 */
		if (!pb->enabled) {
			/* If there is a steady-state enable period required, run it. */
			if (data->driver.enable_ns) {
				pwm_config(pb->pwm, pb->period, pb->period);
				pwm_enable(pb->pwm);
				ndelay(data->driver.enable_ns);

				/* Disable the PWM to until we configure the actual
				 * steady-state configuration to avoid flashing
				 */
				pwm_config(pb->pwm, 0, pb->period);
				pwm_disable(pb->pwm);
			}

			/* If there is a specified start pulse train and the
			 * specified steady-state pulse is less than the minimum
			 * start train pulse, then run the start pulse train for
			 * the required amount of time.
			 */
			if ( (data->driver.start_ms) &&
            	 (pulse < data->driver.start_pulse_min_ns) )
            {
            	pwm_config(pb->pwm,
						   data->driver.start_pulse_ns,
						   data->driver.start_period_ns);
			    pwm_enable(pb->pwm);
			    mdelay(data->driver.start_ms);
			}
		}
		pwm_config(pb->pwm, pulse, pb->period);

		brightness = pb->lth_brightness +
			(brightness * (pb->period - pb->lth_brightness) / max);
		pwm_config(pb->pwm, brightness, pb->period);
		pwm_enable(pb->pwm);
		pb->enabled = true;
	}
	return 0;
}

static int pwm_backlight_get_brightness(struct backlight_device *bl)
{
	return bl->props.brightness;
}

static const struct backlight_ops pwm_backlight_ops = {
	.update_status	= pwm_backlight_update_status,
	.get_brightness	= pwm_backlight_get_brightness,
};

static int pwm_backlight_probe(struct platform_device *pdev)
{
	struct backlight_properties props;
	struct platform_pwm_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl;
	struct pwm_bl_data *pb;
	int ret;

	if (!data) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}

	if (data->init) {
		ret = data->init(&pdev->dev);
		if (ret < 0)
			return ret;
	}

	pb = kzalloc(sizeof(*pb), GFP_KERNEL);
	if (!pb) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	pb->period = data->pwm_period_ns;
	pb->notify = data->notify;
	pb->ramp   = data->ramp;

	/* If specified by platform data, warn about a minimum pulse width
	 * that may be in violation of the LED driver's minimum pulse
	 * width.
	 */

	if (data->driver.pulse_min_ns != 0) {
		const unsigned int pwm_pulse_min_ns =
			((1 * data->pwm_period_ns) / data->max_brightness);

		if (pwm_pulse_min_ns < data->driver.pulse_min_ns) {
			dev_warn(&pdev->dev, "Minimum backlight pulse width (%u ns) is "
					 "less than the driver pulse minimum (%u ns), the "
					 "backlight may turn off unexpectedly.\n",
					 pwm_pulse_min_ns, data->driver.pulse_min_ns);
		}
	}

	pb->lth_brightness = data->lth_brightness *
		(data->pwm_period_ns / data->max_brightness);
	pb->dev = &pdev->dev;

	pb->pwm = pwm_request(data->pwm_id, "backlight");
	if (IS_ERR(pb->pwm)) {
		dev_err(&pdev->dev, "unable to request PWM for backlight\n");
		ret = PTR_ERR(pb->pwm);
		goto err_pwm;
	} else
		dev_dbg(&pdev->dev, "got pwm for backlight\n");

	memset(&props, 0, sizeof(struct backlight_properties));
	props.max_brightness = data->max_brightness;
	bl = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, pb,
				       &pwm_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}


       /* If a default was provided at the kernal command line,
        * use that.  Otherwise, take the platform data default.
        */
        if (default_brightness != -1) {
          bl->props.brightness = default_brightness;
        } else {
          bl->props.brightness = data->dft_brightness;
        }

	BUG_ON(data->dft_brightness > data->max_brightness);

	backlight_update_status(bl);

	platform_set_drvdata(pdev, bl);
	return 0;

err_bl:
	pwm_free(pb->pwm);
err_pwm:
	kfree(pb);
err_alloc:
	if (data->exit)
		data->exit(&pdev->dev);
	return ret;
}

static int pwm_backlight_remove(struct platform_device *pdev)
{
	struct platform_pwm_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = bl_get_data(bl);

	backlight_device_unregister(bl);
	pwm_config(pb->pwm, 0, pb->period);
	pwm_disable(pb->pwm);
	pb->enabled = false;
	pwm_free(pb->pwm);
	kfree(pb);
	if (data->exit)
		data->exit(&pdev->dev);
	return 0;
}

#ifdef CONFIG_PM
static int pwm_backlight_suspend(struct platform_device *pdev,
				 pm_message_t state)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = dev_get_drvdata(&bl->dev);

	if (pb->notify)
		pb->notify(pb->dev, 0);
	pwm_config(pb->pwm, 0, pb->period);
	pwm_disable(pb->pwm);
	return 0;
}

static int pwm_backlight_resume(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);

	backlight_update_status(bl);
	return 0;
}
#else
#define pwm_backlight_suspend	NULL
#define pwm_backlight_resume	NULL
#endif

static struct platform_driver pwm_backlight_driver = {
	.driver		= {
		.name	= "pwm-backlight",
		.owner	= THIS_MODULE,
	},
	.probe		= pwm_backlight_probe,
	.remove		= pwm_backlight_remove,
	.suspend	= pwm_backlight_suspend,
	.resume		= pwm_backlight_resume,
};

static int __init pwm_backlight_init(void)
{
	return platform_driver_register(&pwm_backlight_driver);
}
module_init(pwm_backlight_init);

static void __exit pwm_backlight_exit(void)
{
	platform_driver_unregister(&pwm_backlight_driver);
}
module_exit(pwm_backlight_exit);

MODULE_DESCRIPTION("PWM based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-backlight");

