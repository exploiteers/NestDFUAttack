/*
 *    Copyright (c) 2010 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    Copyright (C) 2008 Texas Instruments, Inc.
 *    Copyright (C) 2008-2009 Nokia Corporation
 *
 *      Author: Paul Walmsley
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    Description:
 *      This file defines timing parameters for the Samsung
 *      K4X51163PI-FCG6 MDDR SDRAM at various rates.
 *
 *      The initial values were cloned from the Hynix H8KDS0UN0MER-4EM
 *      used inthe Texas Instruments OMAP3 EVM.
 */

#ifndef ARCH_ARM_MACH_OMAP2_SDRAM_SAMSUNG_K4X51163PI_FCG6
#define ARCH_ARM_MACH_OMAP2_SDRAM_SAMSUNG_K4X51163PI_FCG6

#include <plat/sdrc.h>

/* Micron MT29F2G16ABDHC-ET */

static struct omap_sdrc_params k4x51163pi_sdrc_params[] = {

	// @ 166.000000 MHz

	[0] = {
		.rate			= 166000000,
		.actim_ctrla	= 0x629db485,
		.actim_ctrlb	= 0x00012114,
		.rfr_ctrl		= 0x0004dc01,
		.mr				= 0x00000032,
	},

	// @ 165.941176 MHz

	[1] = {
		.rate			= 165941176,
		.actim_ctrla	= 0x629db485,
		.actim_ctrlb	= 0x00012114,
		.rfr_ctrl		= 0x0004dc01,
		.mr				= 0x00000032,
	},

	// @ 83.000000 MHz

	[2] = {
		.rate			= 83000000,
		.actim_ctrla	= 0x51512283,
		.actim_ctrlb	= 0x0001120c,
		.rfr_ctrl		= 0x00025501,
		.mr				= 0x00000032,
	},

	// @ 82.970588 MHz

	[3] = {
		.rate			= 82970588,
		.actim_ctrla	= 0x51512283,
		.actim_ctrlb	= 0x0001120c,
		.rfr_ctrl		= 0x00025501,
		.mr				= 0x00000032,
	},

	// Terminating entry

	[4] = {
		.rate			= 0
	},
};

#endif /* ARCH_ARM_MACH_OMAP2_SDRAM_SAMSUNG_K4X51163PI_FCG6 */
