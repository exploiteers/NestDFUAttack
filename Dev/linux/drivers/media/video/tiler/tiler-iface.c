/*
 * tiler-iface.c
 *
 * TILER driver interace functions for TI TILER hardware block.
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

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/fs.h>		/* fops */
#include <linux/uaccess.h>	/* copy_to_user */
#include <linux/slab.h>		/* kmalloc */
#include <linux/sched.h>	/* current */
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <asm/mach/map.h>	/* for ioremap_page */

#include "_tiler.h"

static bool security = CONFIG_TILER_SECURITY;
static bool ssptr_lookup = true;
static bool offset_lookup = true;

module_param(security, bool, 0644);
MODULE_PARM_DESC(security,
	"Separate allocations by different processes into different pages");
module_param(ssptr_lookup, bool, 0644);
MODULE_PARM_DESC(ssptr_lookup,
	"Allow looking up a block by ssptr - This is a security risk");
module_param(offset_lookup, bool, 0644);
MODULE_PARM_DESC(offset_lookup,
	"Allow looking up a buffer by offset - This is a security risk");

static struct mutex mtx;
static struct list_head procs;	/* list of process info structs */
static struct tiler_ops *ops;	/* shared methods and variables */
static struct blocking_notifier_head notifier;	/* notifier for events */

/*
 *  Event notification methods
 *  ==========================================================================
 */

static s32 tiler_notify_event(int event, void *data)
{
	return blocking_notifier_call_chain(&notifier, event, data);
}

/*
 *  Buffer handling methods
 *  ==========================================================================
 */

struct __buf_info {
	struct list_head by_pid;		/* list of buffers per pid */
	struct tiler_buf_info buf_info;
	struct mem_info *mi[TILER_MAX_NUM_BLOCKS];	/* blocks */
};

/* check if an offset is used */
static bool _m_offs_in_use(u32 offs, u32 length, struct process_info *pi)
{
	struct __buf_info *_b;
	/* have mutex */
	list_for_each_entry(_b, &pi->bufs, by_pid)
		if (_b->buf_info.offset < offs + length &&
		    _b->buf_info.offset + _b->buf_info.length > offs)
			return 1;
	return 0;
}

/* get an offset */
static u32 _m_get_offs(struct process_info *pi, u32 length)
{
	static u32 offs = 0xda7a;

	/* ensure no-one is using this offset */
	while ((offs << PAGE_SHIFT) + length < length ||
	       _m_offs_in_use(offs << PAGE_SHIFT, length, pi)) {
		/* use a pseudo-random generator to get a new offset to try */

		/* Galois LSF: 20, 17 */
		offs = (offs >> 1) ^ (u32)((0 - (offs & 1u)) & 0x90000);
	}

	return offs << PAGE_SHIFT;
}

/* find and lock a block.  process_info is optional */
static struct mem_info *
_m_lock_block(u32 key, u32 id, struct process_info *pi) {
	struct gid_info *gi;
	struct mem_info *mi;

	/* if process_info is given, look there first */
	if (pi) {
		/* have mutex */

		/* find block in process list and free it */
		list_for_each_entry(gi, &pi->groups, by_pid) {
			mi = ops->lock(key, id, gi);
			if (mi)
				return mi;
		}
	}

	/* if not found or no process_info given, find block in global list */
	return ops->lock(key, id, NULL);
}

/* register a buffer */
static s32 _m_register_buf(struct __buf_info *_b, struct process_info *pi)
{
	struct mem_info *mi;
	struct tiler_buf_info *b = &_b->buf_info;
	u32 i, num = b->num_blocks, offs;

	/* check validity */
	if (num > TILER_MAX_NUM_BLOCKS || num == 0)
		return -EINVAL;

	/* find each block */
	b->length = 0;
	for (i = 0; i < num; i++) {
		mi = _m_lock_block(b->blocks[i].key, b->blocks[i].id, pi);
		if (!mi) {
			/* unlock any blocks already found */
			while (i--)
				ops->unlock_free(_b->mi[i], false);
			return -EACCES;
		}
		_b->mi[i] = mi;

		/* we don't keep track of ptr and 1D stride so clear them */
		b->blocks[i].ptr = NULL;
		b->blocks[i].stride = 0;

		ops->describe(mi, b->blocks + i);
		b->length += tiler_size(&mi->blk);
	}

	/* if found all, register buffer */
	offs = _b->mi[0]->blk.phys & ~PAGE_MASK;
	b->offset = _m_get_offs(pi, b->length) + offs;
	b->length -= offs;

	/* have mutex */
	list_add(&_b->by_pid, &pi->bufs);

	return 0;
}

/* unregister a buffer */
static void _m_unregister_buf(struct __buf_info *_b)
{
	u32 i;

	/* unregister */
	list_del(&_b->by_pid);

	/* no longer using the blocks */
	for (i = 0; i < _b->buf_info.num_blocks; i++)
		ops->unlock_free(_b->mi[i], false);

	kfree(_b);
}

/*
 *  process_info handling methods
 *  ==========================================================================
 */

/* get process info, and increment refs for device tracking */
static struct process_info *__get_pi(pid_t pid, bool kernel)
{
	struct process_info *pi;

	/*
	 * treat all processes as the same, kernel processes are still treated
	 * differently so not to free kernel allocated areas when a user process
	 * closes the tiler driver
	 */
	if (!security)
		pid = 0;

	/* find process context */
	mutex_lock(&mtx);
	list_for_each_entry(pi, &procs, list) {
		if (pi->pid == pid && pi->kernel == kernel)
			goto done;
	}

	/* create process context */
	pi = kmalloc(sizeof(*pi), GFP_KERNEL);
	if (!pi)
		goto done;
	memset(pi, 0, sizeof(*pi));

	pi->pid = pid;
	pi->kernel = kernel;
	INIT_LIST_HEAD(&pi->groups);
	INIT_LIST_HEAD(&pi->bufs);
	list_add(&pi->list, &procs);
done:
	/* increment reference count */
	if (pi && !kernel)
		pi->refs++;
	mutex_unlock(&mtx);
	return pi;
}

/**
 * Free all info kept by a process: all registered buffers, allocated blocks,
 * and unreferenced blocks.  Any blocks/areas still referenced will move to the
 * orphaned lists to avoid issues if a new process is created with the same pid.
 */
static void _m_free_process_info(struct process_info *pi)
{
	struct gid_info *gi, *gi_;
	struct __buf_info *_b = NULL, *_b_ = NULL;

	/* have mutex */

	/* unregister all buffers */
	list_for_each_entry_safe(_b, _b_, &pi->bufs, by_pid)
		_m_unregister_buf(_b);

	BUG_ON(!list_empty(&pi->bufs));

	/* free all allocated blocks, and remove unreferenced ones */
	list_for_each_entry_safe(gi, gi_, &pi->groups, by_pid)
		ops->destroy_group(gi);

	BUG_ON(!list_empty(&pi->groups));
	list_del(&pi->list);
	kfree(pi);
}

/* Free all info kept by all processes.  Called on cleanup. */
static void destroy_processes(void)
{
	struct process_info *pi, *pi_;

	mutex_lock(&mtx);

	list_for_each_entry_safe(pi, pi_, &procs, list)
		_m_free_process_info(pi);
	BUG_ON(!list_empty(&procs));

	mutex_unlock(&mtx);
}

/*
 *  File operations (mmap, ioctl, open, close)
 *  ==========================================================================
 */

/* mmap tiler buffer into user's virtual space */
static s32 tiler_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct __buf_info *_b;
	struct tiler_buf_info *b = NULL;
	u32 i, map_offs, map_size, blk_offs, blk_size, mapped_size;
	struct process_info *pi = filp->private_data;
	u32 offs = vma->vm_pgoff << PAGE_SHIFT;
	u32 size = vma->vm_end - vma->vm_start;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* find tiler buffer to mmap */
	mutex_lock(&mtx);
	list_for_each_entry(_b, &pi->bufs, by_pid) {
		/* we support partial mmaping of a whole tiler buffer */
		if (offs >= (_b->buf_info.offset & PAGE_MASK) &&
		    offs + size <= PAGE_ALIGN(_b->buf_info.offset +
						_b->buf_info.length)) {
			b = &_b->buf_info;
			break;
		}
	}
	mutex_unlock(&mtx);

	/* we use b to detect if we found the bufffer */
	if (!b)
		return -ENXIO;

	/* mmap relevant blocks */
	blk_offs = _b->buf_info.offset;

	/* start at the beginning of the region */
	mapped_size = 0;
	for (i = 0; i < b->num_blocks; i++, blk_offs += blk_size) {
		blk_size = tiler_size(&_b->mi[i]->blk);
		/* see if tiler block is inside the requested region */
		if (offs >= blk_offs + blk_size || offs + size < blk_offs)
			continue;
		/* get the offset and map size for this particular block */
		map_offs = max(offs, blk_offs) - blk_offs;
		map_size = min(size - mapped_size, blk_size);

		/* mmap block */
		if (tiler_mmap_blk(&_b->mi[i]->blk, map_offs, map_size, vma,
				   mapped_size))
			return -EAGAIN;

		/* update mmap region pointer */
		mapped_size += map_size;
	}
	return 0;
}

/* ioctl handler */
static s32 tiler_ioctl(struct file *filp, u32 cmd, unsigned long arg)
{
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *ptep, pte;
	s32 r;
	void __user *data = (void __user *)arg;
	struct process_info *pi = filp->private_data;
	struct __buf_info *_b;
	struct tiler_buf_info buf_info = {0};
	struct tiler_block_info block_info = {0};
	struct mem_info *mi;

	switch (cmd) {
	/* allocate block */
	case TILIOC_GBLK:
		if (copy_from_user(&block_info, data, sizeof(block_info)))
			return -EFAULT;

		switch (block_info.fmt) {
		case TILFMT_PAGE:
			r = ops->alloc(block_info.fmt, block_info.dim.len, 1,
					block_info.align, block_info.offs,
					block_info.key, block_info.group_id,
					pi, &mi);
			break;
		case TILFMT_8BIT:
		case TILFMT_16BIT:
		case TILFMT_32BIT:
			r = ops->alloc(block_info.fmt,
					block_info.dim.area.width,
					block_info.dim.area.height,
					block_info.align, block_info.offs,
					block_info.key, block_info.group_id,
					pi, &mi);
			break;
		default:
			return -EINVAL;
		}
		if (r)
			return r;

		/* fill out block info */
		if (mi) {
			block_info.ptr = NULL;
			ops->describe(mi, &block_info);
		}

		if (copy_to_user(data, &block_info, sizeof(block_info)))
			return -EFAULT;
		break;
	/* free/unmap block */
	case TILIOC_FBLK:
	case TILIOC_UMBLK:
		if (copy_from_user(&block_info, data, sizeof(block_info)))
			return -EFAULT;

		/* search current process first, then all processes */
		mutex_lock(&mtx);
		mi = _m_lock_block(block_info.key, block_info.id, pi);
		mutex_unlock(&mtx);
		if (mi)
			ops->unlock_free(mi, true);

		/* free always succeeds */
		break;
	/* get physical address */
	case TILIOC_GSSP:
		pgd = pgd_offset(current->mm, arg);
		if (!(pgd_none(*pgd) || pgd_bad(*pgd))) {
			pmd = pmd_offset(pgd, arg);
			if (!(pmd_none(*pmd) || pmd_bad(*pmd))) {
				ptep = pte_offset_map(pmd, arg);
				if (ptep) {
					pte = *ptep;
					if (pte_present(pte))
						return (pte & PAGE_MASK) |
							(~PAGE_MASK & arg);
				}
			}
		}
		/* va not in page table, return NULL */
		return (s32) NULL;
		break;
	/* map block */
	case TILIOC_MBLK:
		if (copy_from_user(&block_info, data, sizeof(block_info)))
			return -EFAULT;

		if (!block_info.ptr)
			return -EFAULT;

		r = ops->map(block_info.fmt, block_info.dim.len, 1,
			      block_info.key, block_info.group_id, pi,
			      &mi, (u32)block_info.ptr);
		if (r)
			return r;

		/* fill out block info */
		if (mi)
			ops->describe(mi, &block_info);

		if (copy_to_user(data, &block_info, sizeof(block_info)))
			return -EFAULT;
		break;
#ifndef CONFIG_TILER_SECURE
	/* query buffer information by offset */
	case TILIOC_QBUF:
		if (!offset_lookup)
			return -EPERM;

		if (copy_from_user(&buf_info, data, sizeof(buf_info)))
			return -EFAULT;

		/* find buffer */
		mutex_lock(&mtx);
		r = -ENOENT;
		/* buffer registration is per process */
		list_for_each_entry(_b, &pi->bufs, by_pid) {
			if (buf_info.offset == _b->buf_info.offset) {
				memcpy(&buf_info, &_b->buf_info,
					sizeof(buf_info));
				r = 0;
				break;
			}
		}
		mutex_unlock(&mtx);

		if (r)
			return r;

		if (copy_to_user(data, &_b->buf_info, sizeof(_b->buf_info)))
			return -EFAULT;
		break;
#endif
	/* register buffer */
	case TILIOC_RBUF:
		/* save buffer information */
		_b = kmalloc(sizeof(*_b), GFP_KERNEL);
		if (!_b)
			return -ENOMEM;
		memset(_b, 0, sizeof(*_b));

		if (copy_from_user(&_b->buf_info, data, sizeof(_b->buf_info))) {
			kfree(_b);
			return -EFAULT;
		}

		mutex_lock(&mtx);
		r = _m_register_buf(_b, pi);
		mutex_unlock(&mtx);

		if (r) {
			kfree(_b);
			return -EACCES;
		}

		/* undo registration on failure */
		if (copy_to_user(data, &_b->buf_info, sizeof(_b->buf_info))) {
			mutex_lock(&mtx);
			_m_unregister_buf(_b);
			mutex_unlock(&mtx);
			return -EFAULT;
		}
		break;
	/* unregister a buffer */
	case TILIOC_URBUF:
		if (copy_from_user(&buf_info, data, sizeof(buf_info)))
			return -EFAULT;

		/* find buffer */
		r = -EFAULT;
		mutex_lock(&mtx);
		/* buffer registration is per process */
		list_for_each_entry(_b, &pi->bufs, by_pid) {
			if (buf_info.offset == _b->buf_info.offset) {
				_m_unregister_buf(_b);
				/* only retrieve buffer length */
				buf_info.length = _b->buf_info.length;
				r = 0;
				break;
			}
		}
		mutex_unlock(&mtx);

		if (r)
			return r;

		if (copy_to_user(data, &buf_info, sizeof(buf_info)))
			return -EFAULT;
		break;
	/* prereserv blocks */
	case TILIOC_PRBLK:
		if (copy_from_user(&block_info, data, sizeof(block_info)))
			return -EFAULT;

		if (block_info.fmt == TILFMT_8AND16)
			ops->reserve_nv12(block_info.key,
					  block_info.dim.area.width,
					  block_info.dim.area.height,
					  block_info.align,
					  block_info.offs,
					  block_info.group_id, pi);
		else
			ops->reserve(block_info.key,
				     block_info.fmt,
				     block_info.dim.area.width,
				     block_info.dim.area.height,
				     block_info.align,
				     block_info.offs,
				     block_info.group_id, pi);
		break;
	/* unreserve blocks */
	case TILIOC_URBLK:
		ops->unreserve(arg, pi);
		break;
	/* query a tiler block */
	case TILIOC_QBLK:
		if (copy_from_user(&block_info, data, sizeof(block_info)))
			return -EFAULT;

		if (block_info.id) {
			/* look up by id if specified */
			mutex_lock(&mtx);
			mi = _m_lock_block(block_info.key, block_info.id, pi);
			mutex_unlock(&mtx);
		} else
#ifndef CONFIG_TILER_SECURE
		if (ssptr_lookup) {
			/* otherwise, look up by ssptr if allowed */
			mi = ops->lock_by_ssptr(block_info.ssptr);
		} else
#endif
			return -EPERM;

		if (!mi)
			return -EFAULT;

		/* we don't keep track of ptr and 1D stride so clear them */
		block_info.ptr = NULL;
		block_info.stride = 0;

		ops->describe(mi, &block_info);
		ops->unlock_free(mi, false);

		if (copy_to_user(data, &block_info, sizeof(block_info)))
			return -EFAULT;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/* open tiler driver */
static s32 tiler_open(struct inode *ip, struct file *filp)
{
	struct process_info *pi = __get_pi(current->tgid, false);
	if (!pi)
		return -ENOMEM;

	filp->private_data = pi;
	return 0;
}

/* close tiler driver */
static s32 tiler_release(struct inode *ip, struct file *filp)
{
	struct process_info *pi = filp->private_data;

	mutex_lock(&mtx);
	/* free resources if last device in this process */
	if (0 == --pi->refs) {
		tiler_notify_event(TILER_DEVICE_CLOSE, NULL);
		_m_free_process_info(pi);
	}

	mutex_unlock(&mtx);

	return 0;
}

/* tiler driver file operations */
static const struct file_operations tiler_fops = {
	.open    = tiler_open,
	.unlocked_ioctl   = tiler_ioctl,
	.release = tiler_release,
	.mmap    = tiler_mmap,
};

/* initialize tiler interface */
void tiler_iface_init(struct tiler_ops *tiler)
{
	ops = tiler;
	ops->cleanup = destroy_processes;
	ops->fops = &tiler_fops;

#ifdef CONFIG_TILER_SECURE
	security = true;
	offset_lookup = ssptr_lookup = false;
#endif

	mutex_init(&mtx);
	INIT_LIST_HEAD(&procs);
	BLOCKING_INIT_NOTIFIER_HEAD(&notifier);
}

/*
 *  Kernel APIs
 *  ==========================================================================
 */

s32 tiler_reg_notifier(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;
	return blocking_notifier_chain_register(&notifier, nb);
}
EXPORT_SYMBOL(tiler_reg_notifier);

s32 tiler_unreg_notifier(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;
	return blocking_notifier_chain_unregister(&notifier, nb);
}
EXPORT_SYMBOL(tiler_unreg_notifier);

void tiler_reservex(u32 n, enum tiler_fmt fmt, u32 width, u32 height,
		   u32 align, u32 offs, u32 gid, pid_t pid)
{
	struct process_info *pi = __get_pi(pid, true);

	if (pi)
		ops->reserve(n, fmt, width, height, align, offs, gid, pi);
}
EXPORT_SYMBOL(tiler_reservex);

void tiler_reserve(u32 n, enum tiler_fmt fmt, u32 width, u32 height,
		  u32 align, u32 offs)
{
	tiler_reservex(n, fmt, width, height, align, offs, 0, current->tgid);
}
EXPORT_SYMBOL(tiler_reserve);

void tiler_reservex_nv12(u32 n, u32 width, u32 height, u32 align, u32 offs,
			u32 gid, pid_t pid)
{
	struct process_info *pi = __get_pi(pid, true);

	if (pi)
		ops->reserve_nv12(n, width, height, align, offs, gid, pi);
}
EXPORT_SYMBOL(tiler_reservex_nv12);

void tiler_reserve_nv12(u32 n, u32 width, u32 height, u32 align, u32 offs)
{
	tiler_reservex_nv12(n, width, height, align, offs, 0, current->tgid);
}
EXPORT_SYMBOL(tiler_reserve_nv12);

s32 tiler_allocx(struct tiler_block_t *blk, enum tiler_fmt fmt,
				u32 align, u32 offs, u32 gid, pid_t pid)
{
	struct mem_info *mi;
	struct process_info *pi;
	s32 res;

	BUG_ON(!blk || blk->phys);

	pi = __get_pi(pid, true);
	if (!pi)
		return -ENOMEM;

	res = ops->alloc(fmt, blk->width, blk->height, align, offs, blk->key,
								gid, pi, &mi);
	if (mi) {
		blk->phys = mi->blk.phys;
		blk->id = mi->blk.id;
	}
	return res;
}
EXPORT_SYMBOL(tiler_allocx);

s32 tiler_alloc(struct tiler_block_t *blk, enum tiler_fmt fmt,
		u32 align, u32 offs)
{
	return tiler_allocx(blk, fmt, align, offs, 0, current->tgid);
}
EXPORT_SYMBOL(tiler_alloc);

s32 tiler_mapx(struct tiler_block_t *blk, enum tiler_fmt fmt, u32 gid,
				pid_t pid, u32 usr_addr)
{
	struct mem_info *mi;
	struct process_info *pi;
	s32 res;

	BUG_ON(!blk || blk->phys);

	pi = __get_pi(pid, true);
	if (!pi)
		return -ENOMEM;

	res = ops->map(fmt, blk->width, blk->height, blk->key, gid, pi, &mi,
								usr_addr);
	if (mi) {
		blk->phys = mi->blk.phys;
		blk->id = mi->blk.id;
	}
	return res;

}
EXPORT_SYMBOL(tiler_mapx);

s32 tiler_map(struct tiler_block_t *blk, enum tiler_fmt fmt, u32 usr_addr)
{
	return tiler_mapx(blk, fmt, 0, current->tgid, usr_addr);
}
EXPORT_SYMBOL(tiler_map);

s32 tiler_mmap_blk(struct tiler_block_t *blk, u32 offs, u32 size,
				struct vm_area_struct *vma, u32 voffs)
{
	u32 v, p, len;

	/* mapping must fit into vma */
	BUG_ON(vma->vm_start > vma->vm_start + voffs ||
		vma->vm_start + voffs > vma->vm_start + voffs + size ||
		vma->vm_start + voffs + size > vma->vm_end);

	/* mapping must fit into block */
	BUG_ON(offs > offs + size || offs + size > tiler_size(blk));

	v = tiler_vstride(blk);
	p = tiler_pstride(blk);

	/* remap block portion */
	len = v - (offs % v);	/* initial area to map */
	while (size) {
		/* restrict to size still needs mapping */
		if (len > size)
			len = size;

		vma->vm_pgoff = (blk->phys + offs) >> PAGE_SHIFT;
		if (remap_pfn_range(vma, vma->vm_start + voffs, vma->vm_pgoff,
				    len, vma->vm_page_prot))
			return -EAGAIN;
		voffs += len;
		offs += len + p - v;
		size -= len;
		len = v;	/* subsequent area to map */
	}
	return 0;
}
EXPORT_SYMBOL(tiler_mmap_blk);

s32 tiler_ioremap_blk(struct tiler_block_t *blk, u32 offs, u32 size,
				u32 addr, u32 mtype)
{
	u32 v, p;
	u32 len;		/* area to map */
	const struct mem_type *type = get_mem_type(mtype);

	/* mapping must fit into address space */
	BUG_ON(addr > addr + size);

	/* mapping must fit into block */
	BUG_ON(offs > offs + size || offs + size > tiler_size(blk));

	v = tiler_vstride(blk);
	p = tiler_pstride(blk);

	/* move offset and address to end */
	offs += blk->phys + size;
	addr += size;

	len = v - (offs % v);	/* initial area to map */
	while (size) {
		while (len && size) {
			if (ioremap_page(addr - size, offs - size, type))
				return -EAGAIN;
			len  -= PAGE_SIZE;
			size -= PAGE_SIZE;
		}

		offs += p - v;
		len = v;	/* subsequent area to map */
	}
	return 0;
}
EXPORT_SYMBOL(tiler_ioremap_blk);

void tiler_free(struct tiler_block_t *blk)
{
	/* find block */
	struct mem_info *mi = ops->lock(blk->key, blk->id, NULL);
	if (mi)
		ops->unlock_free(mi, true);
	blk->phys = blk->id = 0;
}
EXPORT_SYMBOL(tiler_free);
