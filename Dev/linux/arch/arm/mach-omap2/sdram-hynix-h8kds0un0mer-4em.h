/*
 * SDRC register values for the Hynix H8KDS0UN0MER-4EM
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ARCH_ARM_MACH_OMAP2_SDRAM_HYNIX_H8KDS0UN0MER4EM
#define __ARCH_ARM_MACH_OMAP2_SDRAM_HYNIX_H8KDS0UN0MER4EM

#include <plat/sdrc.h>

/* Hynix H8KDS0UN0MER-4EM */
static struct omap_sdrc_params h8kds0un0mer4em_sdrc_params[] = {
	[0] = {
		.rate        = 200000000,
		.actim_ctrla = 0x92e1c4c6,
		.actim_ctrlb = 0x0002111c,
		.rfr_ctrl    = 0x0005e601,
		.mr          = 0x00000032,
	},
	[1] = {
		.rate        = 100000000,
		.actim_ctrla = 0x49912283,
		.actim_ctrlb = 0x0002110e,
		.rfr_ctrl    = 0x0002da01,
		.mr          = 0x00000032,
    },
	[2] = {
		.rate        = 0
	},
};

#endif
