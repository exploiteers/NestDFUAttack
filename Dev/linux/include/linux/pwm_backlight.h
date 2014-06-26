/*
 * Generic PWM backlight driver data - see drivers/video/backlight/pwm_bl.c
 */
#ifndef __LINUX_PWM_BACKLIGHT_H
#define __LINUX_PWM_BACKLIGHT_H

struct platform_pwm_backlight_data {
	int pwm_id;
      	/* The platform can either specify simply the maximum and default
	 * brightness steps or those plus a set max_brightness PWM pulse
	 * widths, in nanoseconds. In the former case, the PWM pulses are
	 * linear from 1 to max_brightness. In the latter case, the
	 * platform specifies whatever ramp (e.g. eye response) makes
	 * sense.
	 */
	unsigned int max_brightness;
	unsigned int dft_brightness;
	const unsigned long *ramp;
	unsigned int lth_brightness;
	unsigned int pwm_period_ns;
      	/* The platform can optionally specify LED driver-specific
	 * parameters such as a constant enable duration, minimum pulse
	 * width and an initial start pulse train.
	 */
	struct {
		int enable_ns;
		int pulse_min_ns;
        int start_pulse_min_ns;
		int start_pulse_ns;
		int start_ms;
		int start_period_ns;
	} driver;
	int (*init)(struct device *dev);
	int (*notify)(struct device *dev, int brightness);
	void (*exit)(struct device *dev);
};

#endif
