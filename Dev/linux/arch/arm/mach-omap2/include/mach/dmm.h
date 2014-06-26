/*
 * dmm.h
 *
 * DMM driver support functions for TI DMM-TILER hardware block.
 *
 * Author: David Sin <davidsin@ti.com>
 *
 * Copyright (C) 2009-2010 Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef DMM_H
#define DMM_H

#define DMM_BASE 0x4E000000
#define DMM_SIZE 0x800

#define DMM_REVISION          0x000
#define DMM_HWINFO            0x004
#define DMM_LISA_HWINFO       0x008
#define DMM_DMM_SYSCONFIG     0x010
#define DMM_LISA_LOCK         0x01C
#define DMM_LISA_MAP__0       0x040
#define DMM_LISA_MAP__1       0x044
#define DMM_TILER_HWINFO      0x208
#define DMM_TILER_OR__0       0x220
#define DMM_TILER_OR__1       0x224
#define DMM_PAT_HWINFO        0x408
#define DMM_PAT_GEOMETRY      0x40C
#define DMM_PAT_CONFIG        0x410
#define DMM_PAT_VIEW__0       0x420
#define DMM_PAT_VIEW__1       0x424
#define DMM_PAT_VIEW_MAP__0   0x440
#define DMM_PAT_VIEW_MAP_BASE 0x460
#define DMM_PAT_IRQ_EOI       0x478
#define DMM_PAT_IRQSTATUS_RAW 0x480
#define DMM_PAT_IRQSTATUS     0x490
#define DMM_PAT_IRQENABLE_SET 0x4A0
#define DMM_PAT_IRQENABLE_CLR 0x4B0
#define DMM_PAT_STATUS__0     0x4C0
#define DMM_PAT_STATUS__1     0x4C4
#define DMM_PAT_STATUS__2     0x4C8
#define DMM_PAT_STATUS__3     0x4CC
#define DMM_PAT_DESCR__0      0x500
#define DMM_PAT_AREA__0       0x504
#define DMM_PAT_CTRL__0       0x508
#define DMM_PAT_DATA__0       0x50C
#define DMM_PEG_HWINFO        0x608
#define DMM_PEG_PRIO          0x620
#define DMM_PEG_PRIO_PAT      0x640

/**
 * PAT refill programming mode.
 */
enum pat_mode {
	MANUAL,
	AUTO
};

/**
 * Area definition for DMM physical address translator.
 */
struct pat_area {
	s32 x0:8;
	s32 y0:8;
	s32 x1:8;
	s32 y1:8;
};

/**
 * DMM physical address translator control.
 */
struct pat_ctrl {
	s32 start:4;
	s32 dir:4;
	s32 lut_id:8;
	s32 sync:12;
	s32 ini:4;
};

/**
 * PAT descriptor.
 */
struct pat {
	struct pat *next;
	struct pat_area area;
	struct pat_ctrl ctrl;
	u32 data;
};

/**
 * DMM device data
 */
struct dmm {
	void __iomem *base;
};

/**
 * Create and initialize the physical address translator.
 * @param id    PAT id
 * @return pointer to device data
 */
struct dmm *dmm_pat_init(u32 id);

/**
 * Program the physical address translator.
 * @param dmm   Device data
 * @param desc  PAT descriptor
 * @param mode  programming mode
 * @return an error status.
 */
s32 dmm_pat_refill(struct dmm *dmm, struct pat *desc, enum pat_mode mode);

/**
 * Clean up the physical address translator.
 * @param dmm    Device data
 * @return an error status.
 */
void dmm_pat_release(struct dmm *dmm);

#endif
