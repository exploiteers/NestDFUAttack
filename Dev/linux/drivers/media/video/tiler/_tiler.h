/*
 * _tiler.h
 *
 * TI TILER driver internal shared definitions.
 *
 * Author: Lajos Molnar <molnar@ti.com>
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

#ifndef _TILER_H
#define _TILER_H

#include <linux/kernel.h>
#include <mach/tiler.h>
#include "tcm.h"

#define TILER_FORMATS		(TILFMT_MAX - TILFMT_MIN + 1)

/* per process (thread group) info */
struct process_info {
	struct list_head list;		/* other processes */
	struct list_head groups;	/* my groups */
	struct list_head bufs;		/* my registered buffers */
	pid_t pid;			/* really: thread group ID */
	u32 refs;			/* open tiler devices, 0 for processes
					   tracked via kernel APIs */
	bool kernel;			/* tracking kernel objects */
};

/* per group info (within a process) */
struct gid_info {
	struct list_head by_pid;	/* other groups */
	struct list_head areas;		/* all areas in this pid/gid */
	struct list_head reserved;	/* areas pre-reserved */
	struct list_head onedim;	/* all 1D areas in this pid/gid */
	u32 gid;			/* group ID */
	int refs;			/* instances directly using this ptr */
	struct process_info *pi;	/* parent */
};

/* info for an area reserved from a container */
struct area_info {
	struct list_head by_gid;	/* areas in this pid/gid */
	struct list_head blocks;	/* blocks in this area */
	u32 nblocks;			/* # of blocks in this area */

	struct tcm_area area;		/* area details */
	struct gid_info *gi;		/* link to parent, if still alive */
};

/* info for a block */
struct mem_info {
	struct list_head global;	/* reserved / global blocks */
	struct tiler_block_t blk;	/* block info */
	u32 num_pg;			/* number of pages in page-list */
	u32 usr;			/* user space address */
	u32 *pg_ptr;			/* list of mapped struct page ptrs */
	struct tcm_area area;
	u32 *mem;			/* list of alloced phys addresses */
	int refs;			/* number of times referenced */
	bool alloced;			/* still alloced */

	struct list_head by_area;	/* blocks in the same area / 1D */
	void *parent;			/* area info for 2D, else group info */
};

/* tiler geometry information */
struct tiler_geom {
	u32 x_shft;	/* unused X-bits (as part of bpp) */
	u32 y_shft;	/* unused Y-bits (as part of bpp) */
	u32 bpp;	/* bytes per pixel */
	u32 slot_w;	/* width of each slot (in pixels) */
	u32 slot_h;	/* height of each slot (in pixels) */
	u32 bpp_m;	/* modified bytes per pixel (=1 for page mode) */
};

/* methods and variables shared between source files */
struct tiler_ops {
	/* block operations */
	s32 (*alloc) (enum tiler_fmt fmt, u32 width, u32 height,
			u32 align, u32 offs, u32 key,
			u32 gid, struct process_info *pi,
			struct mem_info **info);
	s32 (*map) (enum tiler_fmt fmt, u32 width, u32 height,
			u32 key, u32 gid, struct process_info *pi,
			struct mem_info **info, u32 usr_addr);
	void (*reserve_nv12) (u32 n, u32 width, u32 height, u32 align, u32 offs,
					u32 gid, struct process_info *pi);
	void (*reserve) (u32 n, enum tiler_fmt fmt, u32 width, u32 height,
			 u32 align, u32 offs, u32 gid, struct process_info *pi);
	void (*unreserve) (u32 gid, struct process_info *pi);

	/* block access operations */
	struct mem_info * (*lock) (u32 key, u32 id, struct gid_info *gi);
	struct mem_info * (*lock_by_ssptr) (u32 sys_addr);
	void (*describe) (struct mem_info *i, struct tiler_block_info *blk);
	void (*unlock_free) (struct mem_info *mi, bool free);

	s32 (*lay_2d) (enum tiler_fmt fmt, u16 n, u16 w, u16 h, u16 band,
			u16 align, u16 offs, struct gid_info *gi,
			struct list_head *pos);
	s32 (*lay_nv12) (int n, u16 w, u16 w1, u16 h, struct gid_info *gi,
									u8 *p);
	/* group operations */
	struct gid_info * (*get_gi) (struct process_info *pi, u32 gid);
	void (*release_gi) (struct gid_info *gi);
	void (*destroy_group) (struct gid_info *pi);

	/* group access operations */
	void (*add_reserved) (struct list_head *reserved, struct gid_info *gi);
	void (*release) (struct list_head *reserved);

	/* area operations */
	s32 (*analize) (enum tiler_fmt fmt, u32 width, u32 height,
			u16 *x_area, u16 *y_area, u16 *band,
			u16 *align, u16 *offs, u16 *in_offs);

	/* process operations */
	void (*cleanup) (void);

	/* geometry operations */
	void (*xy) (u32 ssptr, u32 *x, u32 *y);
	u32 (*addr) (enum tiler_fmt fmt, u32 x, u32 y);
	const struct tiler_geom * (*geom) (enum tiler_fmt fmt);

	/* additional info */
	const struct file_operations *fops;

	bool nv12_packed;	/* whether NV12 is packed into same container */
	u32 page;		/* page size */
	u32 width;		/* container width */
	u32 height;		/* container height */
};

void tiler_iface_init(struct tiler_ops *tiler);
void tiler_geom_init(struct tiler_ops *tiler);
void tiler_reserve_init(struct tiler_ops *tiler);

#endif
