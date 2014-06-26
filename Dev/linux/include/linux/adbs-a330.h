/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *      Author: Andrea Mucignat <andrea@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file defines platform-specific data for the Avago
 *      ADBS-A330 optical finger navigation (OFN) sensor.
 */

#ifndef __ADBS_A330_INLUDED_H__
#define __ADBS_A330_INLUDED_H__

enum {
	ADBS_A330_DELTA_X,
	ADBS_A330_DELTA_Y
};

enum {
	ADBS_A330_DIRECTION_NEG = -1,
	ADBS_A330_DIRECTION_POS = 1
};

struct adbs_a330_platform_data {
	const char *vdda_supply;
	int vdda_uv;
	unsigned int motion_gpio;
	unsigned int reset_gpio;
	unsigned int shutdown_gpio;
	int direction;
	int mode;
};

#endif /* __ADBS_A330_INLUDED_H__ */
