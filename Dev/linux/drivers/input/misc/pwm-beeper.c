/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *  PWM beeper driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/input.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/input/pwm-beeper.h>

struct pwm_beeper {
	struct input_dev *input;
	struct pwm_device *pwm;
	unsigned long period;
	struct platform_pwm_beeper_data pdata;
};

#define HZ_TO_NANOSECONDS(x) (1000000000UL/(x))

static int pwm_beeper_event(struct input_dev *input,
			    unsigned int type, unsigned int code, int value)
{
	int ret = 0;
	struct pwm_beeper *beeper = input_get_drvdata(input);
	unsigned long period;

	if (type != EV_SND || value < 0)
		return -EINVAL;

	switch (code) {
	case SND_BELL:
		value = value ? 1000 : 0;
		break;
	case SND_TONE:
		break;
	default:
		return -EINVAL;
	}

	if (beeper->pdata.notify) {
		beeper->pdata.notify(&input->dev, value);
	}

	period = ((value == 0) ? 0 : HZ_TO_NANOSECONDS(value));
	
	ret = pwm_config(beeper->pwm, period / 2, period);
	if (ret)
		goto done;

	if (period == 0) {
		pwm_disable(beeper->pwm);
	} else {
		ret = pwm_enable(beeper->pwm);
	}

	beeper->period = period;

 done:
	return ret;
}

static int __devinit pwm_beeper_probe(struct platform_device *pdev)
{
	struct platform_pwm_beeper_data *pdata = NULL;
	struct pwm_beeper *beeper;
	int error;

	pdata = ((struct platform_pwm_beeper_data *)(pdev->dev.platform_data));

	BUG_ON(pdata == NULL);

	if (pdata == NULL) {
		dev_err(&pdev->dev, "Could not find required platform data.\n");
		return -ENOENT;
	}

	beeper = kzalloc(sizeof(*beeper), GFP_KERNEL);
	if (!beeper)
		return -ENOMEM;

	beeper->pwm = pwm_request(pdata->pwm_id, "pwm beeper");
	beeper->pdata = *pdata;

	if (IS_ERR(beeper->pwm)) {
		error = PTR_ERR(beeper->pwm);
		dev_err(&pdev->dev, "Failed to request pwm device: %d\n", error);
		goto err_free;
	}

	beeper->input = input_allocate_device();
	if (!beeper->input) {
		dev_err(&pdev->dev, "Failed to allocate input device\n");
		error = -ENOMEM;
		goto err_pwm_free;
	}
	beeper->input->dev.parent = &pdev->dev;

	beeper->input->name = "pwm-beeper";
	beeper->input->phys = "pwm/input0";
	beeper->input->id.bustype = BUS_HOST;
	beeper->input->id.vendor = 0x001f;
	beeper->input->id.product = 0x0001;
	beeper->input->id.version = 0x0100;

	beeper->input->evbit[0] = BIT(EV_SND);
	beeper->input->sndbit[0] = BIT(SND_TONE) | BIT(SND_BELL);

	beeper->input->event = pwm_beeper_event;

	input_set_drvdata(beeper->input, beeper);

	error = input_register_device(beeper->input);
	if (error) {
		dev_err(&pdev->dev, "Failed to register input device: %d\n", error);
		goto err_input_free;
	}

	platform_set_drvdata(pdev, beeper);

	if (beeper->pdata.init) {
		error = beeper->pdata.init(&pdev->dev);
		if (error) {
			dev_err(&pdev->dev, "Failed platform-specific initialization: %d\n", error);
			goto err_input_free;
		}
	}

	/* Start with the PWM state disabled to synchronize software and
	 * hardware states. If the beeper is never used, yet the PWM block
	 * clocks are running, the driver may prevent the system from
	 * successfully suspending.
	 */

	pwm_disable(beeper->pwm);

	return 0;

err_input_free:
	input_free_device(beeper->input);
err_pwm_free:
	pwm_free(beeper->pwm);
err_free:
	kfree(beeper);

	return error;
}

static int __devexit pwm_beeper_remove(struct platform_device *pdev)
{
	struct pwm_beeper *beeper = platform_get_drvdata(pdev);

	if (beeper->pdata.exit) {
		beeper->pdata.exit(&pdev->dev);
	}

	platform_set_drvdata(pdev, NULL);
	input_unregister_device(beeper->input);

	pwm_disable(beeper->pwm);
	pwm_free(beeper->pwm);

	kfree(beeper);

	return 0;
}

#ifdef CONFIG_PM
static int pwm_beeper_suspend(struct device *dev)
{
	struct pwm_beeper *beeper = dev_get_drvdata(dev);
	int ret = 0;

	if (beeper->pdata.suspend) {
		ret = beeper->pdata.suspend(dev);
		if (ret)
			goto done;
	}

	if (beeper->period)
		pwm_disable(beeper->pwm);

done:
	return ret;
}

static int pwm_beeper_resume(struct device *dev)
{
	struct pwm_beeper *beeper = dev_get_drvdata(dev);
	int ret = 0;

	if (beeper->pdata.resume) {
		ret = beeper->pdata.resume(dev);
		if (ret)
			goto done;
	}

	if (beeper->period) {
		ret = pwm_config(beeper->pwm, beeper->period / 2, beeper->period);
		if (ret)
			goto done;
		ret = pwm_enable(beeper->pwm);
	}

done:
	return ret;
}

static SIMPLE_DEV_PM_OPS(pwm_beeper_pm_ops,
			 pwm_beeper_suspend, pwm_beeper_resume);

#define PWM_BEEPER_PM_OPS (&pwm_beeper_pm_ops)
#else
#define PWM_BEEPER_PM_OPS NULL
#endif

static struct platform_driver pwm_beeper_driver = {
	.probe	= pwm_beeper_probe,
	.remove = __devexit_p(pwm_beeper_remove),
	.driver = {
		.name	= "pwm-beeper",
		.owner	= THIS_MODULE,
		.pm	= PWM_BEEPER_PM_OPS,
	},
};

static int __init pwm_beeper_init(void)
{
	return platform_driver_register(&pwm_beeper_driver);
}
module_init(pwm_beeper_init);

static void __exit pwm_beeper_exit(void)
{
	platform_driver_unregister(&pwm_beeper_driver);
}
module_exit(pwm_beeper_exit);

MODULE_AUTHOR("Lars-Peter Clausen <lars@metafoo.de>");
MODULE_DESCRIPTION("PWM beeper driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-beeper");
