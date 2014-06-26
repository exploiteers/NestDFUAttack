/*
 * tiler-main.c
 *
 * TILER driver main support functions for TI TILER hardware block.
 *
 * Authors: Lajos Molnar <molnar@ti.com>
 *          David Sin <davidsin@ti.com>
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>			/* struct cdev */
#include <linux/kdev_t.h>		/* MKDEV() */
#include <linux/sched.h>		/* current */
#include <linux/fs.h>			/* register_chrdev_region() */
#include <linux/device.h>		/* struct class */
#include <linux/platform_device.h>	/* platform_device() */
#include <linux/err.h>			/* IS_ERR() */
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/dma-mapping.h>		/* dma_alloc_coherent */
#include <linux/pagemap.h>		/* page_cache_release() */
#include <linux/slab.h>

#include <mach/dmm.h>
#include "tmm.h"
#include "_tiler.h"
#include "tcm/tcm-sita.h"		/* TCM algorithm */

static bool ssptr_id = CONFIG_TILER_SSPTR_ID;
static uint default_align = CONFIG_TILER_ALIGNMENT;
static uint granularity = CONFIG_TILER_GRANULARITY;

/*
 * We can only change ssptr_id if there are no blocks allocated, so that
 * pseudo-random ids and ssptrs do not potentially clash. For now make it
 * read-only.
 */
module_param(ssptr_id, bool, 0444);
MODULE_PARM_DESC(ssptr_id, "Use ssptr as block ID");
module_param_named(align, default_align, uint, 0644);
MODULE_PARM_DESC(align, "Default block ssptr alignment");
module_param_named(grain, granularity, uint, 0644);
MODULE_PARM_DESC(grain, "Granularity (bytes)");

struct tiler_dev {
	struct cdev cdev;
};

struct platform_driver tiler_driver_ldm = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "tiler",
	},
	.probe = NULL,
	.shutdown = NULL,
	.remove = NULL,
};

static struct tiler_ops tiler;		/* shared methods and variables */

static struct list_head blocks;		/* all tiler blocks */
static struct list_head orphan_areas;	/* orphaned 2D areas */
static struct list_head orphan_onedim;	/* orphaned 1D areas */

static s32 tiler_major;
static s32 tiler_minor;
static struct tiler_dev *tiler_device;
static struct class *tilerdev_class;
static struct mutex mtx;
static struct tcm *tcm[TILER_FORMATS];
static struct tmm *tmm[TILER_FORMATS];
static u32 *dmac_va;
static dma_addr_t dmac_pa;

/*
 *  TMM connectors
 *  ==========================================================================
 */
/* wrapper around tmm_map */
static s32 refill_pat(struct tmm *tmm, struct tcm_area *area, u32 *ptr)
{
	s32 res = 0;
	struct pat_area p_area = {0};
	struct tcm_area slice, area_s;

	tcm_for_each_slice(slice, *area, area_s) {
		p_area.x0 = slice.p0.x;
		p_area.y0 = slice.p0.y;
		p_area.x1 = slice.p1.x;
		p_area.y1 = slice.p1.y;

		memcpy(dmac_va, ptr, sizeof(*ptr) * tcm_sizeof(slice));
		ptr += tcm_sizeof(slice);

		if (tmm_map(tmm, p_area, dmac_pa)) {
			res = -EFAULT;
			break;
		}
	}

	return res;
}

/* wrapper around tmm_clear */
static void clear_pat(struct tmm *tmm, struct tcm_area *area)
{
	struct pat_area p_area = {0};
	struct tcm_area slice, area_s;

	tcm_for_each_slice(slice, *area, area_s) {
		p_area.x0 = slice.p0.x;
		p_area.y0 = slice.p0.y;
		p_area.x1 = slice.p1.x;
		p_area.y1 = slice.p1.y;

		tmm_clear(tmm, p_area);
	}
}

/*
 *  ID handling methods
 *  ==========================================================================
 */

/* check if an id is used */
static bool _m_id_in_use(u32 id)
{
	struct mem_info *mi;
	list_for_each_entry(mi, &blocks, global)
		if (mi->blk.id == id)
			return 1;
	return 0;
}

/* get an id */
static u32 _m_get_id(void)
{
	static u32 id = 0x2d7ae;

	/* ensure noone is using this id */
	while (_m_id_in_use(id)) {
		/* generate a new pseudo-random ID */

		/* Galois LSFR: 32, 22, 2, 1 */
		id = (id >> 1) ^ (u32)((0 - (id & 1u)) & 0x80200003u);
	}

	return id;
}

/*
 *  gid_info handling methods
 *  ==========================================================================
 */

/* get or create new gid_info object */
static struct gid_info *_m_get_gi(struct process_info *pi, u32 gid)
{
	struct gid_info *gi;

	/* have mutex */

	/* see if group already exist */
	list_for_each_entry(gi, &pi->groups, by_pid) {
		if (gi->gid == gid)
			goto done;
	}

	/* create new group */
	gi = kmalloc(sizeof(*gi), GFP_KERNEL);
	if (!gi)
		return gi;

	memset(gi, 0, sizeof(*gi));
	INIT_LIST_HEAD(&gi->areas);
	INIT_LIST_HEAD(&gi->onedim);
	INIT_LIST_HEAD(&gi->reserved);
	gi->pi = pi;
	gi->gid = gid;
	list_add(&gi->by_pid, &pi->groups);
done:
	/*
	 * Once area is allocated, the group info's ref count will be
	 * decremented as the reference is no longer needed.
	 */
	gi->refs++;
	return gi;
}

/* free gid_info object if empty */
static void _m_try_free_group(struct gid_info *gi)
{
	/* have mutex */
	if (gi && list_empty(&gi->areas) && list_empty(&gi->onedim) &&
	    /* also ensure noone is still using this group */
	    !gi->refs) {
		BUG_ON(!list_empty(&gi->reserved));
		list_del(&gi->by_pid);

		/* if group is tracking kernel objects, we may free even
		   the process info */
		if (gi->pi->kernel && list_empty(&gi->pi->groups)) {
			list_del(&gi->pi->list);
			kfree(gi->pi);
		}

		kfree(gi);
	}
}

/* --- external versions --- */

static struct gid_info *get_gi(struct process_info *pi, u32 gid)
{
	struct gid_info *gi;
	mutex_lock(&mtx);
	gi = _m_get_gi(pi, gid);
	mutex_unlock(&mtx);
	return gi;
}

static void release_gi(struct gid_info *gi)
{
	mutex_lock(&mtx);
	gi->refs--;
	_m_try_free_group(gi);
	mutex_unlock(&mtx);
}

/*
 *  Area handling methods
 *  ==========================================================================
 */

/* allocate an reserved area of size, alignment and link it to gi */
/* leaves mutex locked to be able to add block to area */
static struct area_info *area_new_m(u16 width, u16 height, u16 align,
				  struct tcm *tcm, struct gid_info *gi)
{
	struct area_info *ai = kmalloc(sizeof(*ai), GFP_KERNEL);
	if (!ai)
		return NULL;

	/* set up empty area info */
	memset(ai, 0x0, sizeof(*ai));
	INIT_LIST_HEAD(&ai->blocks);

	/* reserve an allocation area */
	if (tcm_reserve_2d(tcm, width, height, align, &ai->area)) {
		kfree(ai);
		return NULL;
	}

	ai->gi = gi;
	mutex_lock(&mtx);
	list_add_tail(&ai->by_gid, &gi->areas);
	return ai;
}

/* (must have mutex) free an area */
static inline void _m_area_free(struct area_info *ai)
{
	if (ai) {
		list_del(&ai->by_gid);
		kfree(ai);
	}
}

static s32 __analize_area(enum tiler_fmt fmt, u32 width, u32 height,
			  u16 *x_area, u16 *y_area, u16 *band,
			  u16 *align, u16 *offs, u16 *in_offs)
{
	/* input: width, height is in pixels, align, offs in bytes */
	/* output: x_area, y_area, band, align, offs in slots */

	/* slot width, height, and row size */
	u32 slot_row, min_align;
	const struct tiler_geom *g;

	/* width and height must be positive */
	if (!width || !height)
		return -EINVAL;

	/* align must be 2 power */
	if (*align & (*align - 1))
		return -EINVAL;

	if (fmt == TILFMT_PAGE) {
		/* adjust size to accomodate offset, only do page alignment */
		*align = PAGE_SIZE;
		*in_offs = *offs & ~PAGE_MASK;
		width += *in_offs;

		/* for 1D area keep the height (1), width is in tiler slots */
		*x_area = DIV_ROUND_UP(width, tiler.page);
		*y_area = *band = 1;

		if (*x_area * *y_area > tiler.width * tiler.height)
			return -ENOMEM;
		return 0;
	}

	/* format must be valid */
	g = tiler.geom(fmt);
	if (!g)
		return -EINVAL;

	/* get the # of bytes per row in 1 slot */
	slot_row = g->slot_w * g->bpp;

	/* how many slots are can be accessed via one physical page */
	*band = PAGE_SIZE / slot_row;

	/* minimum alignment is at least 1 slot.  Use default if needed */
	min_align = max(slot_row, granularity);
	*align = ALIGN(*align ? : default_align, min_align);

	/* align must still be 2 power (in case default_align is wrong) */
	if (*align & (*align - 1))
		return -EAGAIN;

	/* offset must be multiple of bpp */
	if (*offs & (g->bpp - 1) || *offs >= *align)
		return -EINVAL;

	/* round down the offset to the nearest slot size, and increase width
	   to allow space for having the correct offset */
	width += (*offs & (min_align - 1)) / g->bpp;
	if (in_offs)
		*in_offs = *offs & (min_align - 1);
	*offs &= ~(min_align - 1);

	/* expand width to block size */
	width = ALIGN(width, min_align / g->bpp);

	/* adjust to slots */
	*x_area = DIV_ROUND_UP(width, g->slot_w);
	*y_area = DIV_ROUND_UP(height, g->slot_h);
	*align /= slot_row;
	*offs /= slot_row;

	if (*x_area > tiler.width || *y_area > tiler.height)
		return -ENOMEM;
	return 0;
}

/**
 * Find a place where a 2D block would fit into a 2D area of the
 * same height.
 *
 * @author a0194118 (3/19/2010)
 *
 * @param w	Width of the block.
 * @param align	Alignment of the block.
 * @param offs	Offset of the block (within alignment)
 * @param ai	Pointer to area info
 * @param next	Pointer to the variable where the next block
 *		will be stored.  The block should be inserted
 *		before this block.
 *
 * @return the end coordinate (x1 + 1) where a block would fit,
 *	   or 0 if it does not fit.
 *
 * (must have mutex)
 */
static u16 _m_blk_find_fit(u16 w, u16 align, u16 offs,
		     struct area_info *ai, struct list_head **before)
{
	int x = ai->area.p0.x + w + offs;
	struct mem_info *mi;

	/* area blocks are sorted by x */
	list_for_each_entry(mi, &ai->blocks, by_area) {
		/* check if buffer would fit before this area */
		if (x <= mi->area.p0.x) {
			*before = &mi->by_area;
			return x;
		}
		x = ALIGN(mi->area.p1.x + 1 - offs, align) + w + offs;
	}
	*before = &ai->blocks;

	/* check if buffer would fit after last area */
	return (x <= ai->area.p1.x + 1) ? x : 0;
}

/* (must have mutex) adds a block to an area with certain x coordinates */
static inline
struct mem_info *_m_add2area(struct mem_info *mi, struct area_info *ai,
				u16 x0, u16 w, struct list_head *before)
{
	mi->parent = ai;
	mi->area = ai->area;
	mi->area.p0.x = x0;
	mi->area.p1.x = x0 + w - 1;
	list_add_tail(&mi->by_area, before);
	ai->nblocks++;
	return mi;
}

static struct mem_info *get_2d_area(u16 w, u16 h, u16 align, u16 offs, u16 band,
					struct gid_info *gi, struct tcm *tcm)
{
	struct area_info *ai = NULL;
	struct mem_info *mi = NULL;
	struct list_head *before = NULL;
	u16 x = 0;   /* this holds the end of a potential area */

	/* allocate map info */

	/* see if there is available prereserved space */
	mutex_lock(&mtx);
	list_for_each_entry(mi, &gi->reserved, global) {
		if (mi->area.tcm == tcm &&
		    tcm_aheight(mi->area) == h &&
		    tcm_awidth(mi->area) == w &&
		    (mi->area.p0.x & (align - 1)) == offs) {
			/* this area is already set up */

			/* remove from reserved list */
			list_del(&mi->global);
			goto done;
		}
	}
	mutex_unlock(&mtx);

	/* if not, reserve a block struct */
	mi = kmalloc(sizeof(*mi), GFP_KERNEL);
	if (!mi)
		return mi;
	memset(mi, 0, sizeof(*mi));

	/* see if allocation fits in one of the existing areas */
	/* this sets x, ai and before */
	mutex_lock(&mtx);
	list_for_each_entry(ai, &gi->areas, by_gid) {
		if (ai->area.tcm == tcm &&
		    tcm_aheight(ai->area) == h) {
			x = _m_blk_find_fit(w, align, offs, ai, &before);
			if (x) {
				_m_add2area(mi, ai, x - w, w, before);
				goto done;
			}
		}
	}
	mutex_unlock(&mtx);

	/* if no area fit, reserve a new one */
	ai = area_new_m(ALIGN(w + offs, max(band, align)), h,
		      max(band, align), tcm, gi);
	if (ai) {
		_m_add2area(mi, ai, ai->area.p0.x + offs, w, &ai->blocks);
	} else {
		/* clean up */
		kfree(mi);
		return NULL;
	}

done:
	mutex_unlock(&mtx);
	return mi;
}

/* layout reserved 2d blocks in a larger area */
/* NOTE: band, w, h, a(lign), o(ffs) is in slots */
static s32 lay_2d(enum tiler_fmt fmt, u16 n, u16 w, u16 h, u16 band,
		      u16 align, u16 offs, struct gid_info *gi,
		      struct list_head *pos)
{
	u16 x, x0, e = ALIGN(w, align), w_res = (n - 1) * e + w;
	struct mem_info *mi = NULL;
	struct area_info *ai = NULL;

	printk(KERN_INFO "packing %u %u buffers into %u width\n",
	       n, w, w_res);

	/* calculate dimensions, band, offs and alignment in slots */
	/* reserve an area */
	ai = area_new_m(ALIGN(w_res + offs, max(band, align)), h,
			max(band, align), tcm[fmt], gi);
	if (!ai)
		return -ENOMEM;

	/* lay out blocks in the reserved area */
	for (n = 0, x = offs; x < w_res; x += e, n++) {
		/* reserve a block struct */
		mi = kmalloc(sizeof(*mi), GFP_KERNEL);
		if (!mi)
			break;

		memset(mi, 0, sizeof(*mi));
		x0 = ai->area.p0.x + x;
		_m_add2area(mi, ai, x0, w, &ai->blocks);
		list_add(&mi->global, pos);
	}

	mutex_unlock(&mtx);
	return n;
}

/* layout reserved nv12 blocks in a larger area */
/* NOTE: area w(idth), w1 (8-bit block width), h(eight) are in slots */
/* p is a pointer to a packing description, which is a list of offsets in
   the area for consecutive 8-bit and 16-bit blocks */
static s32 lay_nv12(int n, u16 w, u16 w1, u16 h, struct gid_info *gi, u8 *p)
{
	u16 wh = (w1 + 1) >> 1, width, x0;
	int m;
	int a = PAGE_SIZE / tiler.geom(TILFMT_8BIT)->slot_w;

	struct mem_info *mi = NULL;
	struct area_info *ai = NULL;
	struct list_head *pos;

	/* reserve area */
	ai = area_new_m(w, h, a, TILFMT_8BIT, gi);
	if (!ai)
		return -ENOMEM;

	/* lay out blocks in the reserved area */
	for (m = 0; m < 2 * n; m++) {
		width =	(m & 1) ? wh : w1;
		x0 = ai->area.p0.x + *p++;

		/* get insertion head */
		list_for_each(pos, &ai->blocks) {
			mi = list_entry(pos, struct mem_info, by_area);
			if (mi->area.p0.x > x0)
				break;
		}

		/* reserve a block struct */
		mi = kmalloc(sizeof(*mi), GFP_KERNEL);
		if (!mi)
			break;

		memset(mi, 0, sizeof(*mi));

		_m_add2area(mi, ai, x0, width, pos);
		list_add(&mi->global, &gi->reserved);
	}

	mutex_unlock(&mtx);
	return n;
}

/* (must have mutex) free block and any freed areas */
static s32 _m_free(struct mem_info *mi)
{
	struct area_info *ai = NULL;
	struct page *page = NULL;
	s32 res = 0;
	u32 i;

	/* release memory */
	if (mi->pg_ptr) {
		for (i = 0; i < mi->num_pg; i++) {
			page = (struct page *)mi->pg_ptr[i];
			if (page) {
				if (!PageReserved(page))
					SetPageDirty(page);
				page_cache_release(page);
			}
		}
		kfree(mi->pg_ptr);
	} else if (mi->mem) {
		tmm_free(tmm[tiler_fmt(mi->blk.phys)], mi->mem);
	}
	clear_pat(tmm[tiler_fmt(mi->blk.phys)], &mi->area);

	/* safe deletion as list may not have been assigned */
	if (mi->global.next)
		list_del(&mi->global);
	if (mi->by_area.next)
		list_del(&mi->by_area);

	/* remove block from area first if 2D */
	if (mi->area.is2d) {
		ai = mi->parent;

		/* check to see if area needs removing also */
		if (ai && !--ai->nblocks) {
			res = tcm_free(&ai->area);
			list_del(&ai->by_gid);
			/* try to remove parent if it became empty */
			_m_try_free_group(ai->gi);
			kfree(ai);
			ai = NULL;
		}
	} else {
		/* remove 1D area */
		res = tcm_free(&mi->area);
		/* try to remove parent if it became empty */
		_m_try_free_group(mi->parent);
	}

	kfree(mi);
	return res;
}

/* (must have mutex) returns true if block was freed */
static bool _m_chk_ref(struct mem_info *mi)
{
	/* check references */
	if (mi->refs)
		return 0;

	if (_m_free(mi))
		printk(KERN_ERR "error while removing tiler block\n");

	return 1;
}

/* (must have mutex) */
static inline bool _m_dec_ref(struct mem_info *mi)
{
	if (mi->refs-- <= 1)
		return _m_chk_ref(mi);

	return 0;
}

/* (must have mutex) */
static inline void _m_inc_ref(struct mem_info *mi)
{
	mi->refs++;
}

/* (must have mutex) returns true if block was freed */
static inline bool _m_try_free(struct mem_info *mi)
{
	if (mi->alloced) {
		mi->refs--;
		mi->alloced = false;
	}
	return _m_chk_ref(mi);
}

/* --- external methods --- */

/* find a block by key/id and lock it */
static struct mem_info *
find_n_lock(u32 key, u32 id, struct gid_info *gi) {
	struct area_info *ai = NULL;
	struct mem_info *mi = NULL;

	mutex_lock(&mtx);

	/* if group is not given, look globally */
	if (!gi) {
		list_for_each_entry(mi, &blocks, global) {
			if (mi->blk.key == key && mi->blk.id == id)
				goto done;
		}
	} else {
		/* is id is ssptr, we know if block is 1D or 2D by the address,
		   so we optimize lookup */
		if (!ssptr_id ||
		    tiler_fmt(id) == TILFMT_PAGE) {
			list_for_each_entry(mi, &gi->onedim, by_area) {
				if (mi->blk.key == key && mi->blk.id == id)
					goto done;
			}
		}

		if (!ssptr_id ||
		    tiler_fmt(id) != TILFMT_PAGE) {
			list_for_each_entry(ai, &gi->areas, by_gid) {
				list_for_each_entry(mi, &ai->blocks, by_area) {
					if (mi->blk.key == key &&
					    mi->blk.id == id)
						goto done;
				}
			}
		}
	}

	mi = NULL;
done:
	/* lock block by increasing its ref count */
	if (mi)
		mi->refs++;

	mutex_unlock(&mtx);

	return mi;
}

/* unlock a block, and optionally free it */
static void unlock_n_free(struct mem_info *mi, bool free)
{
	mutex_lock(&mtx);

	_m_dec_ref(mi);
	if (free)
		_m_try_free(mi);

	mutex_unlock(&mtx);
}

/**
 * Free all blocks in a group:
 *
 * allocated blocks, and unreferenced blocks.  Any blocks/areas still referenced
 * will move to the orphaned lists to avoid issues if a new process is created
 * with the same pid.
 *
 * (must have mutex)
 */
static void destroy_group(struct gid_info *gi)
{
	struct area_info *ai, *ai_;
	struct mem_info *mi, *mi_;
	bool ai_autofreed, need2free;

	mutex_lock(&mtx);

	/* free all allocated blocks, and remove unreferenced ones */

	/*
	 * Group info structs when they become empty on an _m_try_free.
	 * However, if the group info is already empty, we need to
	 * remove it manually
	 */
	need2free = list_empty(&gi->areas) && list_empty(&gi->onedim);
	list_for_each_entry_safe(ai, ai_, &gi->areas, by_gid) {
		ai_autofreed = true;
		list_for_each_entry_safe(mi, mi_, &ai->blocks, by_area)
			ai_autofreed &= _m_try_free(mi);

		/* save orphaned areas for later removal */
		if (!ai_autofreed) {
			need2free = true;
			ai->gi = NULL;
			list_move(&ai->by_gid, &orphan_areas);
		}
	}

	list_for_each_entry_safe(mi, mi_, &gi->onedim, by_area) {
		if (!_m_try_free(mi)) {
			need2free = true;
			/* save orphaned 1D blocks */
			mi->parent = NULL;
			list_move(&mi->by_area, &orphan_onedim);
		}
	}

	/* if group is still alive reserved list should have been
	   emptied as there should be no reference on those blocks */
	if (need2free) {
		BUG_ON(!list_empty(&gi->onedim));
		BUG_ON(!list_empty(&gi->areas));
		_m_try_free_group(gi);
	}

	mutex_unlock(&mtx);
}

/* release (reserved) blocks */
static void release_blocks(struct list_head *reserved)
{
	struct mem_info *mi, *mi_;

	mutex_lock(&mtx);

	/* find block in global list and free it */
	list_for_each_entry_safe(mi, mi_, reserved, global) {
		BUG_ON(mi->refs || mi->alloced);
		_m_free(mi);
	}
	mutex_unlock(&mtx);
}

/* add reserved blocks to a group */
static void add_reserved_blocks(struct list_head *reserved, struct gid_info *gi)
{
	mutex_lock(&mtx);
	list_splice_init(reserved, &gi->reserved);
	mutex_unlock(&mtx);
}

/* find a block by ssptr */
static struct mem_info *find_block_by_ssptr(u32 sys_addr)
{
	struct mem_info *i;
	struct tcm_pt pt;
	u32 x, y;
	enum tiler_fmt fmt;
	const struct tiler_geom *g;

	fmt = tiler_fmt(sys_addr);
	if (fmt == TILFMT_INVALID)
		return NULL;

	g = tiler.geom(fmt);

	/* convert x & y pixel coordinates to slot coordinates */
	tiler.xy(sys_addr, &x, &y);
	pt.x = x / g->slot_w;
	pt.y = y / g->slot_h;

	mutex_lock(&mtx);
	list_for_each_entry(i, &blocks, global) {
		if (tiler_fmt(i->blk.phys) == tiler_fmt(sys_addr) &&
		    tcm_is_in(pt, i->area)) {
			i->refs++;
			goto found;
		}
	}
	i = NULL;

found:
	mutex_unlock(&mtx);
	return i;
}

/* find a block by ssptr */
static void fill_block_info(struct mem_info *i, struct tiler_block_info *blk)
{
	blk->fmt = tiler_fmt(i->blk.phys);
#ifdef CONFIG_TILER_EXPOSE_SSPTR
	blk->ssptr = i->blk.phys;
#endif
	if (blk->fmt == TILFMT_PAGE) {
		blk->dim.len = i->blk.width;
		blk->group_id = ((struct gid_info *) i->parent)->gid;
	} else {
		blk->stride = tiler_vstride(&i->blk);
		blk->dim.area.width = i->blk.width;
		blk->dim.area.height = i->blk.height;
		blk->group_id = ((struct area_info *) i->parent)->gi->gid;
	}
	blk->id = i->blk.id;
	blk->key = i->blk.key;
	blk->offs = i->blk.phys & ~PAGE_MASK;
	blk->align = PAGE_SIZE;
}

/*
 *  Block operations
 *  ==========================================================================
 */

static struct mem_info *__get_area(enum tiler_fmt fmt, u32 width, u32 height,
				   u16 align, u16 offs, struct gid_info *gi)
{
	u16 x, y, band, in_offs = 0;
	struct mem_info *mi = NULL;
	const struct tiler_geom *g = tiler.geom(fmt);

	/* calculate dimensions, band, offs and alignment in slots */
	if (__analize_area(fmt, width, height, &x, &y, &band, &align, &offs,
			   &in_offs))
		return NULL;

	if (fmt == TILFMT_PAGE)	{
		/* 1D areas don't pack */
		mi = kmalloc(sizeof(*mi), GFP_KERNEL);
		if (!mi)
			return NULL;
		memset(mi, 0x0, sizeof(*mi));

		if (tcm_reserve_1d(tcm[fmt], x * y, &mi->area)) {
			kfree(mi);
			return NULL;
		}

		mutex_lock(&mtx);
		mi->parent = gi;
		list_add(&mi->by_area, &gi->onedim);
	} else {
		mi = get_2d_area(x, y, align, offs, band, gi, tcm[fmt]);
		if (!mi)
			return NULL;

		mutex_lock(&mtx);
	}

	list_add(&mi->global, &blocks);
	mi->alloced = true;
	mi->refs++;
	gi->refs--;
	mutex_unlock(&mtx);

	mi->blk.phys = tiler.addr(fmt,
		mi->area.p0.x * g->slot_w, mi->area.p0.y * g->slot_h)
		+ in_offs;
	return mi;
}

static s32 alloc_block(enum tiler_fmt fmt, u32 width, u32 height,
		u32 align, u32 offs, u32 key, u32 gid, struct process_info *pi,
		struct mem_info **info)
{
	struct mem_info *mi = NULL;
	struct gid_info *gi = NULL;

	*info = NULL;

	/* only support up to page alignment */
	if (align > PAGE_SIZE || offs >= (align ? : default_align) || !pi)
		return -EINVAL;

	/* get group context */
	mutex_lock(&mtx);
	gi = _m_get_gi(pi, gid);
	mutex_unlock(&mtx);

	if (!gi)
		return -ENOMEM;

	/* reserve area in tiler container */
	mi = __get_area(fmt, width, height, align, offs, gi);
	if (!mi) {
		mutex_lock(&mtx);
		gi->refs--;
		_m_try_free_group(gi);
		mutex_unlock(&mtx);
		return -ENOMEM;
	}

	mi->blk.width = width;
	mi->blk.height = height;
	mi->blk.key = key;
	if (ssptr_id) {
		mi->blk.id = mi->blk.phys;
	} else {
		mutex_lock(&mtx);
		mi->blk.id = _m_get_id();
		mutex_unlock(&mtx);
	}

	/* allocate and map if mapping is supported */
	if (tmm_can_map(tmm[fmt])) {
		mi->num_pg = tcm_sizeof(mi->area);

		mi->mem = tmm_get(tmm[fmt], mi->num_pg);
		if (!mi->mem)
			goto cleanup;

		/* Ensure the data reaches to main memory before PAT refill */
		wmb();

		/* program PAT */
		if (refill_pat(tmm[fmt], &mi->area, mi->mem))
			goto cleanup;
	}
	*info = mi;
	return 0;

cleanup:
	mutex_lock(&mtx);
	_m_free(mi);
	mutex_unlock(&mtx);
	return -ENOMEM;

}

static s32 map_block(enum tiler_fmt fmt, u32 width, u32 height,
		     u32 key, u32 gid, struct process_info *pi,
		     struct mem_info **info, u32 usr_addr)
{
	u32 i = 0, tmp = -1, *mem = NULL;
	u8 write = 0;
	s32 res = -ENOMEM;
	struct mem_info *mi = NULL;
	struct page *page = NULL;
	struct task_struct *curr_task = current;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma = NULL;
	struct gid_info *gi = NULL;

	*info = NULL;

	/* we only support mapping a user buffer in page mode */
	if (fmt != TILFMT_PAGE)
		return -EPERM;

	/* check if mapping is supported by tmm */
	if (!tmm_can_map(tmm[fmt]))
		return -EPERM;

	/* get group context */
	mutex_lock(&mtx);
	gi = _m_get_gi(pi, gid);
	mutex_unlock(&mtx);

	if (!gi)
		return -ENOMEM;

	/* reserve area in tiler container */
	mi = __get_area(fmt, width, height, 0, 0, gi);
	if (!mi) {
		mutex_lock(&mtx);
		gi->refs--;
		_m_try_free_group(gi);
		mutex_unlock(&mtx);
		return -ENOMEM;
	}

	mi->blk.width = width;
	mi->blk.height = height;
	mi->blk.key = key;
	if (ssptr_id) {
		mi->blk.id = mi->blk.phys;
	} else {
		mutex_lock(&mtx);
		mi->blk.id = _m_get_id();
		mutex_unlock(&mtx);
	}

	mi->usr = usr_addr;

	/* allocate pages */
	mi->num_pg = tcm_sizeof(mi->area);

	mem = kmalloc(mi->num_pg * sizeof(*mem), GFP_KERNEL);
	if (!mem)
		goto done;
	memset(mem, 0x0, sizeof(*mem) * mi->num_pg);

	mi->pg_ptr = kmalloc(mi->num_pg * sizeof(*mi->pg_ptr), GFP_KERNEL);
	if (!mi->pg_ptr)
		goto done;
	memset(mi->pg_ptr, 0x0, sizeof(*mi->pg_ptr) * mi->num_pg);

	/*
	 * Important Note: usr_addr is mapped from user
	 * application process to current process - it must lie
	 * completely within the current virtual memory address
	 * space in order to be of use to us here.
	 */
	down_read(&mm->mmap_sem);
	vma = find_vma(mm, mi->usr);
	res = -EFAULT;

	/*
	 * It is observed that under some circumstances, the user
	 * buffer is spread across several vmas, so loop through
	 * and check if the entire user buffer is covered.
	 */
	while ((vma) && (mi->usr + width > vma->vm_end)) {
		/* jump to the next VMA region */
		vma = find_vma(mm, vma->vm_end + 1);
	}
	if (!vma) {
		printk(KERN_ERR "Failed to get the vma region for "
			"user buffer.\n");
		goto fault;
	}

	if (vma->vm_flags & (VM_WRITE | VM_MAYWRITE))
		write = 1;

	tmp = mi->usr;
	for (i = 0; i < mi->num_pg; i++) {
		if (get_user_pages(curr_task, mm, tmp, 1, write, 1, &page,
									NULL)) {
			if (page_count(page) < 1) {
				printk(KERN_ERR "Bad page count from"
							"get_user_pages()\n");
			}
			mi->pg_ptr[i] = (u32)page;
			mem[i] = page_to_phys(page);
			tmp += PAGE_SIZE;
		} else {
			printk(KERN_ERR "get_user_pages() failed\n");
			goto fault;
		}
	}
	up_read(&mm->mmap_sem);

	/* Ensure the data reaches to main memory before PAT refill */
	wmb();

	if (refill_pat(tmm[fmt], &mi->area, mem))
		goto fault;

	res = 0;
	*info = mi;
	goto done;
fault:
	up_read(&mm->mmap_sem);
done:
	if (res) {
		mutex_lock(&mtx);
		_m_free(mi);
		mutex_unlock(&mtx);
	}
	kfree(mem);
	return res;
}

/*
 *  Driver code
 *  ==========================================================================
 */

static s32 __init tiler_init(void)
{
	dev_t dev  = 0;
	s32 r = -1;
	struct device *device = NULL;
	struct tcm_pt div_pt;
	struct tcm *sita = NULL;
	struct tmm *tmm_pat = NULL;

	tiler.alloc = alloc_block;
	tiler.map = map_block;
	tiler.lock = find_n_lock;
	tiler.unlock_free = unlock_n_free;
	tiler.lay_2d = lay_2d;
	tiler.lay_nv12 = lay_nv12;
	tiler.destroy_group = destroy_group;
	tiler.lock_by_ssptr = find_block_by_ssptr;
	tiler.describe = fill_block_info;
	tiler.get_gi = get_gi;
	tiler.release_gi = release_gi;
	tiler.release = release_blocks;
	tiler.add_reserved = add_reserved_blocks;
	tiler.analize = __analize_area;
	tiler_geom_init(&tiler);
	tiler_reserve_init(&tiler);
	tiler_iface_init(&tiler);

	/* check module parameters for correctness */
	if (default_align > PAGE_SIZE ||
	    default_align & (default_align - 1) ||
	    granularity < 1 || granularity > PAGE_SIZE ||
	    granularity & (granularity - 1))
		return -EINVAL;

	/*
	 * Array of physical pages for PAT programming, which must be a 16-byte
	 * aligned physical address.
	 */
	dmac_va = dma_alloc_coherent(NULL, tiler.width * tiler.height *
					sizeof(*dmac_va), &dmac_pa, GFP_ATOMIC);
	if (!dmac_va)
		return -ENOMEM;

	/* Allocate tiler container manager (we share 1 on OMAP4) */
	div_pt.x = tiler.width;   /* hardcoded default */
	div_pt.y = (3 * tiler.height) / 4;
	sita = sita_init(tiler.width, tiler.height, (void *)&div_pt);

	tcm[TILFMT_8BIT]  = sita;
	tcm[TILFMT_16BIT] = sita;
	tcm[TILFMT_32BIT] = sita;
	tcm[TILFMT_PAGE]  = sita;

	/* Allocate tiler memory manager (must have 1 unique TMM per TCM ) */
	tmm_pat = tmm_pat_init(0);
	tmm[TILFMT_8BIT]  = tmm_pat;
	tmm[TILFMT_16BIT] = tmm_pat;
	tmm[TILFMT_32BIT] = tmm_pat;
	tmm[TILFMT_PAGE]  = tmm_pat;

	tiler.nv12_packed = tcm[TILFMT_8BIT] == tcm[TILFMT_16BIT];

	tiler_device = kmalloc(sizeof(*tiler_device), GFP_KERNEL);
	if (!tiler_device || !sita || !tmm_pat) {
		r = -ENOMEM;
		goto error;
	}

	memset(tiler_device, 0x0, sizeof(*tiler_device));
	if (tiler_major) {
		dev = MKDEV(tiler_major, tiler_minor);
		r = register_chrdev_region(dev, 1, "tiler");
	} else {
		r = alloc_chrdev_region(&dev, tiler_minor, 1, "tiler");
		tiler_major = MAJOR(dev);
	}

	cdev_init(&tiler_device->cdev, tiler.fops);
	tiler_device->cdev.owner = THIS_MODULE;
	tiler_device->cdev.ops   = tiler.fops;

	r = cdev_add(&tiler_device->cdev, dev, 1);
	if (r)
		printk(KERN_ERR "cdev_add():failed\n");

	tilerdev_class = class_create(THIS_MODULE, "tiler");

	if (IS_ERR(tilerdev_class)) {
		printk(KERN_ERR "class_create():failed\n");
		goto error;
	}

	device = device_create(tilerdev_class, NULL, dev, NULL, "tiler");
	if (device == NULL)
		printk(KERN_ERR "device_create() fail\n");

	r = platform_driver_register(&tiler_driver_ldm);

	mutex_init(&mtx);
	INIT_LIST_HEAD(&blocks);
	INIT_LIST_HEAD(&orphan_areas);
	INIT_LIST_HEAD(&orphan_onedim);

error:
	/* TODO: error handling for device registration */
	if (r) {
		kfree(tiler_device);
		tcm_deinit(sita);
		tmm_deinit(tmm_pat);
		dma_free_coherent(NULL, tiler.width * tiler.height *
					sizeof(*dmac_va), dmac_va, dmac_pa);
	}

	return r;
}

static void __exit tiler_exit(void)
{
	int i, j;

	mutex_lock(&mtx);

	/* free all process data */
	tiler.cleanup();

	/* all lists should have cleared */
	BUG_ON(!list_empty(&blocks));
	BUG_ON(!list_empty(&orphan_onedim));
	BUG_ON(!list_empty(&orphan_areas));

	mutex_unlock(&mtx);

	dma_free_coherent(NULL, tiler.width * tiler.height * sizeof(*dmac_va),
							dmac_va, dmac_pa);

	/* close containers only once */
	for (i = TILFMT_MIN; i <= TILFMT_MAX; i++) {
		/* remove identical containers (tmm is unique per tcm) */
		for (j = i + 1; j <= TILFMT_MAX; j++)
			if (tcm[i] == tcm[j]) {
				tcm[j] = NULL;
				tmm[j] = NULL;
			}

		tcm_deinit(tcm[i]);
		tmm_deinit(tmm[i]);
	}

	mutex_destroy(&mtx);
	platform_driver_unregister(&tiler_driver_ldm);
	cdev_del(&tiler_device->cdev);
	kfree(tiler_device);
	device_destroy(tilerdev_class, MKDEV(tiler_major, tiler_minor));
	class_destroy(tilerdev_class);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Lajos Molnar <molnar@ti.com>");
MODULE_AUTHOR("David Sin <davidsin@ti.com>");
module_init(tiler_init);
module_exit(tiler_exit);
