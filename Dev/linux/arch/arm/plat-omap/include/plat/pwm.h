/*
 *    Copyright (c) 2010 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file is defines platform-specific configuration data for
 *      the OMAP generic PWM platform driver.
 */

#ifndef _OMAP2_PWM_H
#define _OMAP2_PWM_H

/*
 * This identifies the OMAP dual-mode timer (dmtimer) that will be bound
 * to the PWM.
 */
struct omap2_pwm_platform_config {
	int timer_id;		// The OMAP dual-mode timer ID
	bool polarity;		// The polarity (active-high or -low) of the PWM
};

#endif /* _OMAP2_PWM_H */
