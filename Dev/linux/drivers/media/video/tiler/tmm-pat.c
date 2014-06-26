/*
 * tmm-pat.c
 *
 * DMM driver support functions for TI TILER hardware block.
 *
 * Author: Lajos Molnar <molnar@ti.com>, David Sin <dsin@ti.com>
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
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <asm/cacheflush.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "tmm.h"

static int param_set_mem(const char *val, struct kernel_param *kp);

/* Memory limit to cache free pages. TILER will eventually use this much */
static u32 cache_limit = CONFIG_TILER_CACHE_LIMIT << 20;

param_check_uint(cache, &cache_limit);
module_param_call(cache, param_set_mem, param_get_uint, &cache_limit, 0644);
__MODULE_PARM_TYPE(cache, "uint");
MODULE_PARM_DESC(cache, "Cache free pages if total memory is under this limit");

/* global state - statically initialized */
static LIST_HEAD(free_list);	/* page cache: list of free pages */
static u32 total_mem;		/* total memory allocated (free & used) */
static u32 refs;		/* number of tmm_pat instances */
static DEFINE_MUTEX(mtx);	/* global mutex */

/* The page struct pointer and physical address of each page.*/
struct mem {
	struct list_head list;
	struct page *pg;	/* page struct */
	u32 pa;			/* physical address */
};

/* Used to keep track of mem per tmm_pat_get_pages call */
struct fast {
	struct list_head list;
	struct mem **mem;	/* array of page info */
	u32 *pa;		/* array of physical addresses */
	u32 num;		/* number of pages */
};

/* TMM PAT private structure */
struct dmm_mem {
	struct list_head fast_list;
	struct dmm *dmm;
};

/* read mem values for a param */
static int param_set_mem(const char *val, struct kernel_param *kp)
{
	u32 a;
	char *p;

	/* must specify memory */
	if (!val)
		return -EINVAL;

	/* parse value */
	a = memparse(val, &p);
	if (p == val || *p)
		return -EINVAL;

	/* store parsed value */
	*(uint *)kp->arg = a;
	return 0;
}

/**
 *  Frees pages in a fast structure.  Moves pages to the free list if there
 *  are	less pages used	than max_to_keep.  Otherwise, it frees the pages
 */
static void free_fast(struct fast *f)
{
	s32 i = 0;

	/* mutex is locked */
	for (i = 0; i < f->num; i++) {
		if (total_mem < cache_limit) {
			/* cache free page if under the limit */
			list_add(&f->mem[i]->list, &free_list);
		} else {
			/* otherwise, free */
			total_mem -= PAGE_SIZE;
			__free_page(f->mem[i]->pg);
		}
	}
	kfree(f->pa);
	kfree(f->mem);
	/* remove only if element was added */
	if (f->list.next)
		list_del(&f->list);
	kfree(f);
}

/* allocate and flush a page */
static struct mem *alloc_mem(void)
{
	struct mem *m = kmalloc(sizeof(*m), GFP_KERNEL);
	if (!m)
		return NULL;
	memset(m, 0, sizeof(*m));

	m->pg = alloc_page(GFP_KERNEL | GFP_DMA);
	if (!m->pg) {
		kfree(m);
		return NULL;
	}

	m->pa = page_to_phys(m->pg);

	/* flush the cache entry for each page we allocate. */
	dmac_flush_range(page_address(m->pg),
				page_address(m->pg) + PAGE_SIZE);
	outer_flush_range(m->pa, m->pa + PAGE_SIZE);

	return m;
}

static void free_page_cache(void)
{
	struct mem *m, *m_;

	/* mutex is locked */
	list_for_each_entry_safe(m, m_, &free_list, list) {
		__free_page(m->pg);
		total_mem -= PAGE_SIZE;
		list_del(&m->list);
		kfree(m);
	}
}

static void tmm_pat_deinit(struct tmm *tmm)
{
	struct fast *f, *f_;
	struct dmm_mem *pvt = (struct dmm_mem *) tmm->pvt;

	mutex_lock(&mtx);

	/* free all outstanding used memory */
	list_for_each_entry_safe(f, f_, &pvt->fast_list, list)
		free_fast(f);

	/* if this is the last tmm_pat, free all memory */
	if (--refs == 0)
		free_page_cache();

	mutex_unlock(&mtx);
}

static u32 *tmm_pat_get_pages(struct tmm *tmm, u32 n)
{
	struct mem *m;
	struct fast *f;
	struct dmm_mem *pvt = (struct dmm_mem *) tmm->pvt;

	f = kmalloc(sizeof(*f), GFP_KERNEL);
	if (!f)
		return NULL;
	memset(f, 0, sizeof(*f));

	/* array of mem struct pointers */
	f->mem = kmalloc(n * sizeof(*f->mem), GFP_KERNEL);

	/* array of physical addresses */
	f->pa = kmalloc(n * sizeof(*f->pa), GFP_KERNEL);

	/* no pages have been allocated yet (needed for cleanup) */
	f->num = 0;

	if (!f->mem || !f->pa)
		goto cleanup;

	memset(f->mem, 0, n * sizeof(*f->mem));
	memset(f->pa, 0, n * sizeof(*f->pa));

	/* fill out fast struct mem array with free pages */
	mutex_lock(&mtx);
	while (f->num < n) {
		/* if there is a free cached page use it */
		if (!list_empty(&free_list)) {
			/* unbind first element from list */
			m = list_first_entry(&free_list, typeof(*m), list);
			list_del(&m->list);
		} else {
			mutex_unlock(&mtx);

			/**
			 * Unlock mutex during allocation and cache flushing.
			 */
			m = alloc_mem();
			if (!m)
				goto cleanup;

			mutex_lock(&mtx);
			total_mem += PAGE_SIZE;
		}

		f->mem[f->num] = m;
		f->pa[f->num++] = m->pa;
	}

	list_add(&f->list, &pvt->fast_list);
	mutex_unlock(&mtx);
	return f->pa;

cleanup:
	free_fast(f);
	return NULL;
}

static void tmm_pat_free_pages(struct tmm *tmm, u32 *page_list)
{
	struct dmm_mem *pvt = (struct dmm_mem *) tmm->pvt;
	struct fast *f, *f_;

	mutex_lock(&mtx);
	/* find fast struct based on 1st page */
	list_for_each_entry_safe(f, f_, &pvt->fast_list, list) {
		if (f->pa[0] == page_list[0]) {
			free_fast(f);
			break;
		}
	}
	mutex_unlock(&mtx);
}

static s32 tmm_pat_map(struct tmm *tmm, struct pat_area area, u32 page_pa)
{
	struct dmm_mem *pvt = (struct dmm_mem *) tmm->pvt;
	struct pat pat_desc = {0};

	/* send pat descriptor to dmm driver */
	pat_desc.ctrl.dir = 0;
	pat_desc.ctrl.ini = 0;
	pat_desc.ctrl.lut_id = 0;
	pat_desc.ctrl.start = 1;
	pat_desc.ctrl.sync = 0;
	pat_desc.area = area;
	pat_desc.next = NULL;

	/* must be a 16-byte aligned physical address */
	pat_desc.data = page_pa;
	return dmm_pat_refill(pvt->dmm, &pat_desc, MANUAL);
}

struct tmm *tmm_pat_init(u32 pat_id)
{
	struct tmm *tmm = NULL;
	struct dmm_mem *pvt = NULL;

	struct dmm *dmm = dmm_pat_init(pat_id);
	if (dmm)
		tmm = kmalloc(sizeof(*tmm), GFP_KERNEL);
	if (tmm)
		pvt = kmalloc(sizeof(*pvt), GFP_KERNEL);
	if (pvt) {
		/* private data */
		pvt->dmm = dmm;
		INIT_LIST_HEAD(&pvt->fast_list);

		/* increate tmm_pat references */
		mutex_lock(&mtx);
		refs++;
		mutex_unlock(&mtx);

		/* public data */
		tmm->pvt = pvt;
		tmm->deinit = tmm_pat_deinit;
		tmm->get = tmm_pat_get_pages;
		tmm->free = tmm_pat_free_pages;
		tmm->map = tmm_pat_map;
		tmm->clear = NULL;   /* not yet supported */

		return tmm;
	}

	kfree(pvt);
	kfree(tmm);
	dmm_pat_release(dmm);
	return NULL;
}
EXPORT_SYMBOL(tmm_pat_init);
