/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file is the core OMAP2/3 support for the generic, Linux
 *      PWM driver / controller, using the OMAP's dual-mode timers.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <mach/hardware.h>
#include <plat/dmtimer.h>
#include <plat/pwm.h>

/* Preprocessor Definitions */

#undef OMAP_PWM_DEBUG

#if defined(OMAP_PWM_DEBUG)
#define DBG(args...)						\
	do {									\
		pr_info(args);						\
	} while (0)
#define DEV_DBG(pwm, args...)				\
	do {									\
		dev_info(&pwm->pdev->dev, args);	\
	} while (0)
#else
#define DBG(args...)						\
	do { } while (0)
#define DEV_DBG(pwm, args...)				\
	do { } while (0)
#endif // defined(OMAP_PWM_DEBUG)

#define DM_TIMER_LOAD_PWM_MIN	0xFFFFFFFD

/* Type Definitions */

/* 
 * As far as clients of the PWM driver are concerned, PWM devices are
 * opaque abstract objects.
 */

typedef struct omap2_pwm_platform_config omap2_pwm_platform_config;

struct pwm_device {
	struct list_head			head;		// Head for all PWMs managed.
	struct platform_device *	pdev;		// Corresponding platform device.
	struct omap_dm_timer *		dm_timer;	// Corresponding dual-mode timer.
	omap2_pwm_platform_config	config;		// Platform-specific configuration.

	const char *				label;		// Descriptive label.

	unsigned int				use_count;	// Use count.
	unsigned int				pwm_id;		// Generic PWM ID requested.
};

/* Function Prototypes */

static int __devinit omap_pwm_probe(struct platform_device *pdev);
static int __devexit omap_pwm_remove(struct platform_device *pdev);


/* Global Variables */

static struct platform_driver omap_pwm_driver = {
	.driver		= {
		.name	= "omap-pwm",
		.owner	= THIS_MODULE,
	},
	.probe		= omap_pwm_probe,
	.remove		= __devexit_p(omap_pwm_remove)
};

/* List and associated lock for managing generic PWM devices bound to
 * this driver.
 */

static DEFINE_MUTEX(pwm_lock);
static LIST_HEAD(pwm_list);

/**
 * pwm_request - request and allocate the specified generic PWM device.
 * @pwm_id: The identifier associated with the desired generic PWM device.
 * @label: An optional pointer to a C string describing the usage of the
 *         requested generic PWM device.
 *
 * Returns a pointer to the requested generic PWM device on success;
 * otherwise, NULL on error.
 */
struct pwm_device *pwm_request(int pwm_id, const char *label)
{
	struct pwm_device * pwm = NULL;
	bool found = false;

	mutex_lock(&pwm_lock);

	// Walk the list of available PWMs and attempt to find a matching
	// ID, regardless of whether it is in use or not.

	list_for_each_entry(pwm, &pwm_list, head) {
		if (pwm->pwm_id == pwm_id) {
			found = true;
			break;
		}
	}

	if (found) {
		if (pwm->use_count == 0) {
			pwm->use_count++;
			pwm->label = label;
		} else {
			pwm = ERR_PTR(-EBUSY);
		}
	} else {
		pwm = ERR_PTR(-ENOENT);
	}

	mutex_unlock(&pwm_lock);

	return (pwm);
}

/**
 * pwm_free - deallocate/release a previously-requested generic PWM device.
 * @pwm: A pointer to the generic PWM device to release.
 */
void pwm_free(struct pwm_device *pwm)
{
	mutex_lock(&pwm_lock);

	if (pwm->use_count) {
		pwm->use_count--;
		pwm->label = NULL;
	} else {
		pr_err("PWM%d has already been freed.\n", pwm->pwm_id);
	}

	mutex_unlock(&pwm_lock);
}

/**
 * pwm_calc_value - determines the counter value for a clock rate and period.
 * @clk_rate: The clock rate, in Hz, of the PWM's clock source to compute the
 *            counter value for.
 * @ns: The period, in nanoseconds, to computer the counter value for.
 *
 * Returns the PWM counter value for the specified clock rate and period.
 */
static inline int pwm_calc_value(unsigned long clk_rate, int ns)
{
	const unsigned long nanoseconds_per_second = 1000000000;
	int cycles;
	__u64 c;

	c = (__u64)clk_rate * ns;
	do_div(c, nanoseconds_per_second);
	cycles = c;

	return (DM_TIMER_LOAD_PWM_MIN - cycles);
}

/**
 * pwm_config - configures the generic PWM device to the specified parameters.
 * @pwm: A pointer to the PWM device to configure.
 * @duty_ns: The duty period of the PWM, in nanoseconds.
 * @period_ns: The overall period of the PWM, in nanoseconds.
 *
 * Returns 0 if the generic PWM device was successfully configured;
 * otherwise, < 0 on error.
 */
int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns)
{
	int status = 0;
	const bool autoreload = true;
	const bool toggle = true;
	const int trigger = OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE;
	bool enable;
	int load_value, match_value;
	unsigned long clk_rate;

	DEV_DBG(pwm,
			"%s: duty cycle: %d ns, period %d ns\n",
			pwm->label, duty_ns, period_ns);

	clk_rate = clk_get_rate(omap_dm_timer_get_fclk(pwm->dm_timer));

	/* Calculate the appropriate load and match values based on the
	 * specified period and duty cycle. The load value determines the
	 * cycle time and the match value determines the duty cycle.
	 */

	load_value = pwm_calc_value(clk_rate, period_ns);
	match_value = pwm_calc_value(clk_rate, period_ns - duty_ns);

	/* We MUST enable yet stop the associated dual-mode timer before
	 * attempting to write its registers.
	 */

	omap_dm_timer_enable(pwm->dm_timer);
	omap_dm_timer_stop(pwm->dm_timer);

	/* If the dury and overall periods are not the same, then enable
	 * the match nd load comparison functionality. Otherwise, don't to
	 * ensure a steady-state output at 100% duty cycle.
	 */

	enable = (duty_ns != period_ns);

	omap_dm_timer_set_load(pwm->dm_timer, autoreload, load_value);
	omap_dm_timer_set_match(pwm->dm_timer, enable, match_value);

	DEV_DBG(pwm,
			"%s: enable: %u, "
			"load value: %#08x (%d), "
			"match value: %#08x (%d)\n",
			pwm->label,
			enable,
			load_value, load_value,
			match_value, match_value);

	omap_dm_timer_set_pwm(pwm->dm_timer,
						  !pwm->config.polarity,
						  toggle,
						  trigger);

	/* Now that we're done configuring the dual-mode timer, disable it
	 * again. We'll enable and start it later, when requested.
	 */

	omap_dm_timer_disable(pwm->dm_timer);

	return (status);
}

/**
 * pwm_enable - enable the generic PWM device.
 * @pwm: A pointer to the generic PWM device to enable.
 *
 * Returns 0 if the generic PWM device was successfully enabled;
 * otherwise, < 0 on error.
 */
int pwm_enable(struct pwm_device *pwm)
{
	int status = 0;

	/* Enable the counter--always--before attempting to write its
	 * registers and then set the timer to its minimum load value to
	 * ensure we get an overflow event right away once we start it.
	 */

	omap_dm_timer_enable(pwm->dm_timer);
	omap_dm_timer_write_counter(pwm->dm_timer, DM_TIMER_LOAD_PWM_MIN);
	omap_dm_timer_start(pwm->dm_timer);

	return (status);
}

/**
 * pwm_disable - disable the generic PWM device.
 * @pwm: A pointer to the generic PWM device to disable.
 */
void pwm_disable(struct pwm_device *pwm)
{
	omap_dm_timer_enable(pwm->dm_timer);
	omap_dm_timer_stop(pwm->dm_timer);
	omap_dm_timer_disable(pwm->dm_timer);
}

/**
 * omap_pwm_probe - check for the PWM and bind it to the driver.
 * @pdev: A pointer to the platform device node associated with the
 *        PWM instance to be probed for driver binding.
 *
 * Returns 0 if the PWM instance was successfully bound to the driver;
 * otherwise, < 0 on error.
 */
static int __devinit omap_pwm_probe(struct platform_device *pdev)
{
	struct pwm_device * pwm = NULL;
	struct omap2_pwm_platform_config * pdata = NULL;
	int status = 0;

	pdata = ((struct omap2_pwm_platform_config *)(pdev->dev.platform_data));

	BUG_ON(pdata == NULL);

	if (pdata == NULL) {
		dev_err(&pdev->dev, "Could not find required platform data.\n");
		status = -ENOENT;
		goto done;
	}

	/* Allocate memory for the driver-private PWM data and state */

	pwm = kzalloc(sizeof(struct pwm_device), GFP_KERNEL);

	if (pwm == NULL) {
		dev_err(&pdev->dev, "Could not allocate memory.\n");
		status = -ENOMEM;
		goto done;
	}

	/* Request the OMAP dual-mode timer that will be bound to and
	 * associated with this generic PWM.
	 */

	pwm->dm_timer = omap_dm_timer_request_specific(pdata->timer_id);

	if (pwm->dm_timer == NULL) {
		status = -ENOENT;
		goto err_free;
	}

	/* Configure the source for the dual-mode timer backing this
	 * generic PWM device. The clock source will ultimately determine
	 * how small or large the PWM frequency can be.
	 *
	 * At some point, it's probably worth revisiting moving this to
	 * the configure method and choosing either the slow- or
	 * system-clock source as appropriate for the desired PWM period.
	 */

	omap_dm_timer_set_source(pwm->dm_timer, OMAP_TIMER_SRC_SYS_CLK);

	/* Cache away other miscellaneous driver-private data and state
	 * information and add the driver-private data to the platform
	 * device.
	 */

	pwm->pdev = pdev;
	pwm->pwm_id = pdev->id;
	pwm->config = *pdata;

	platform_set_drvdata(pdev, pwm);

	/* Finally, push the added generic PWM device to the end of the
	 * list of available generic PWM devices.
	 */

	mutex_lock(&pwm_lock);
	list_add_tail(&pwm->head, &pwm_list);
	mutex_unlock(&pwm_lock);

	status = 0;
	goto done;

 err_free:
	kfree(pwm);

 done:
	return (status);
}

/**
 * omap_pwm_remove - unbind the specified PWM platform device from the driver.
 * @pdev: A pointer to the platform device node associated with the
 *        PWM instance to be unbound/removed.
 *
 * Returns 0 if the PWM was successfully removed as a platform device;
 * otherwise, < 0 on error.
 */
static int __devexit omap_pwm_remove(struct platform_device *pdev)
{
	struct pwm_device * pwm = NULL;
	int status = 0;

	/* Attempt to get the driver-private data from the platform device
	 * node.
	 */

	pwm = platform_get_drvdata(pdev);

	if (pwm == NULL) {
		status = -ENODEV;
		goto done;
	}

	/* Remove the generic PWM device from the list of available
	 * generic PWM devices.
	 */

	mutex_lock(&pwm_lock);
	list_del(&pwm->head);
	mutex_unlock(&pwm_lock);

	/* Unbind the OMAP dual-mode timer associated with the generic PWM
	 * device.
	 */

	omap_dm_timer_free(pwm->dm_timer);

	/* Finally, release the memory associated with the driver-private
	 * data and state.
	 */

	kfree(pwm);

 done:
	return (status);
}


/**
 * omap_pwm_init - driver/module insertion entry point
 *
 * This routine is the driver/module insertion entry point. It
 * registers the driver as a platform driver.
 *
 * Returns 0 if the driver/module was successfully registered as a
 * platform driver driver; otherwise, < 0 on error.
 */
static int __init omap_pwm_init(void)
{
	return (platform_driver_register(&omap_pwm_driver));
}

/**
 * omap_pwm_exit - driver/module removal entry point
 *
 * This routine is the driver/module removal entry point. It
 * unregisters the driver as a platform driver.
 */
static void __exit omap_pwm_exit(void)
{
	platform_driver_unregister(&omap_pwm_driver);
}

arch_initcall(omap_pwm_init);
module_exit(omap_pwm_exit);

EXPORT_SYMBOL(pwm_config);
EXPORT_SYMBOL(pwm_disable);
EXPORT_SYMBOL(pwm_enable);
EXPORT_SYMBOL(pwm_free);
EXPORT_SYMBOL(pwm_request);

MODULE_AUTHOR("Grant Erickson <grant@nestlabs.com>");
MODULE_LICENSE("GPLv2");
MODULE_VERSION("2011-02-12");
