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
 *      This file defines platform data for the PWM beeper driver (see
 *      drivers/input/misc/pwm-beeper.c).
 */

#ifndef __LINUX_INPUT_PWM_BEEPER_H__
#define __LINUX_INPUT_PWM_BEEPER_H__

struct platform_pwm_beeper_data {
	int pwm_id;
	int (*init)(struct device *dev);
	void (*notify)(struct device *dev, int hz);
	int (*suspend)(struct device *dev);
	int (*resume)(struct device *dev);
	void (*exit)(struct device *dev);
};

#endif /* __LINUX_INPUT_PWM_BEEPER_H__ */
