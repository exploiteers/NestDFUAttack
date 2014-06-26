/*
 * linux/drivers/video/ti81xx/ti81xx_vram.c
 *
 * Copyright (C) 2009 Texas Instruments Inc.
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * Some code and ideas taken from TI OMAP2 Platforms
 * by Tomi Valkeinen.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*#define DEBUG*/
#include <asm/setup.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/seq_file.h>
#include <linux/memblock.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/ti81xxfb.h>
#include <asm/setup.h>

#ifdef DEBUG
#define DBG(format, ...) printk(KERN_DEBUG"VRAM: " format, ## __VA_ARGS__)
#else
#define DBG(format, ...)
#endif

#if (defined(CONFIG_FB_TI81XX) || defined(CONFIG_FB_TI81XX_MODULE) || \
	defined(CONFIG_ARCH_TI81XX))

/* postponed regions are used to temporarily store region information at boot
 * time when we cannot yet allocate the region list */
#define MAX_POSTPONED_REGIONS 10

static int postponed_cnt __initdata;
static struct {
	unsigned long paddr;
	size_t size;
} postponed_regions[MAX_POSTPONED_REGIONS] __initdata;

struct vram_alloc {
	struct list_head list;
	unsigned long paddr;
	unsigned pages;
};

struct vram_region {
	struct list_head list;
	struct list_head alloc_list;
	unsigned long paddr;
	void		*vaddr;
	unsigned pages;
};

static DEFINE_MUTEX(region_mutex);
static LIST_HEAD(region_list);

static inline int region_mem_type(unsigned long paddr)
{
	return TI81XXFB_MEMTYPE_SDRAM;
}

static struct vram_region *ti81xx_vram_create_region(unsigned long paddr,
		void *vaddr, unsigned pages)
{
	struct vram_region *rm;

	rm = kzalloc(sizeof(*rm), GFP_KERNEL);

	if (rm) {
		INIT_LIST_HEAD(&rm->alloc_list);
		rm->paddr = paddr;
		rm->vaddr = vaddr;
		rm->pages = pages;
	}

	return rm;
}

static struct vram_alloc *ti81xx_vram_create_allocation(struct vram_region *vr,
		unsigned long paddr, unsigned pages)
{
	struct vram_alloc *va;
	struct vram_alloc *new;

	new = kzalloc(sizeof(*va), GFP_KERNEL);

	if (!new)
		return NULL;

	new->paddr = paddr;
	new->pages = pages;

	list_for_each_entry(va, &vr->alloc_list, list) {
		if (va->paddr > new->paddr)
			break;
	}

	list_add_tail(&new->list, &va->list);

	return new;
}

static void ti81xx_vram_free_allocation(struct vram_alloc *va)
{
	list_del(&va->list);
	kfree(va);
}

static __init int ti81xx_vram_add_region_postponed(unsigned long paddr,
								size_t size)
{
	if (postponed_cnt == MAX_POSTPONED_REGIONS)
		return -ENOMEM;
	postponed_regions[postponed_cnt].paddr = paddr;
	postponed_regions[postponed_cnt].size = size;
	++postponed_cnt;
	return 0;
}

int ti81xx_vram_add_region(unsigned long paddr, size_t size)
{
	struct vram_region *rm;
	void *vaddr;
	unsigned pages;

	DBG("adding region paddr %08lx size %d\n",
			paddr, size);

	size &= PAGE_MASK;
	pages = size >> PAGE_SHIFT;
	vaddr = ioremap_wc(paddr, size);
	if (NULL == vaddr)
		return -ENOMEM;

	rm = ti81xx_vram_create_region(paddr, vaddr, pages);
	if (rm == NULL) {
		iounmap(vaddr);
		return -ENOMEM;
	}

	list_add(&rm->list, &region_list);
	return 0;
}

int ti81xx_vram_free(unsigned long paddr, void *vaddr, size_t size)
{
	struct vram_region *rm;
	struct vram_alloc *alloc;
	unsigned start, end;

	DBG("free mem paddr %08lx vaddr %p size %d\n",
			paddr, vaddr, size);

	size = PAGE_ALIGN(size);

	mutex_lock(&region_mutex);

	list_for_each_entry(rm, &region_list, list) {
		list_for_each_entry(alloc, &rm->alloc_list, list) {
			start = alloc->paddr;
			end = alloc->paddr + (alloc->pages << PAGE_SHIFT);

			if (start >= paddr && end <= (paddr + size))
				goto found;
		}
	}
	pr_err("FB: can not found the free memory in the list\n");
	mutex_unlock(&region_mutex);
	return -EINVAL;

found:
	ti81xx_vram_free_allocation(alloc);

	mutex_unlock(&region_mutex);
	return 0;
}
EXPORT_SYMBOL(ti81xx_vram_free);

static void *_ti81xx_vram_alloc(int mtype, unsigned pages, unsigned long *paddr)
{
	struct vram_region *rm;
	struct vram_alloc *alloc;
	void  *vaddr;

	list_for_each_entry(rm, &region_list, list) {
		unsigned long start, end;

		DBG("checking region %lx %d\n", rm->paddr, rm->pages);

		if (region_mem_type(rm->paddr) != mtype)
			continue;

		start = rm->paddr;

		list_for_each_entry(alloc, &rm->alloc_list, list) {
			end = alloc->paddr;

			if (end - start >= pages << PAGE_SHIFT)
				goto found;

			start = alloc->paddr + (alloc->pages << PAGE_SHIFT);
		}

		end = rm->paddr + (rm->pages << PAGE_SHIFT);
found:
		if (end - start < (pages << PAGE_SHIFT))
			continue;

		DBG("FOUND %lx, end %lx\n", start, end);

		if (ti81xx_vram_create_allocation(rm, start, pages) == NULL) {
			pr_err("FB: failed to create allocation");
			return NULL;
		}
		*paddr = start;

		vaddr = rm->vaddr + (start - rm->paddr);
		return vaddr;
	}
	pr_err("FB: no memory to allocate\n");
	return NULL;
}

void *ti81xx_vram_alloc(int mtype, size_t size, unsigned long *paddr)
{
	void *vaddr;
	unsigned pages;


	BUG_ON((mtype > TI81XXFB_MEMTYPE_MAX) || (!size));

	DBG("alloc mem type %d size %d\n", mtype, size);

	size = PAGE_ALIGN(size);
	pages = size >> PAGE_SHIFT;

	mutex_lock(&region_mutex);

	vaddr = _ti81xx_vram_alloc(mtype, pages, paddr);

	mutex_unlock(&region_mutex);

	return vaddr;
}
EXPORT_SYMBOL(ti81xx_vram_alloc);

void ti81xx_vram_get_info(unsigned long *vram,
				 unsigned long *free_vram,
				 unsigned long *largest_free_block)
{
	struct vram_region *rm;
	struct vram_alloc *alloc;

	*vram = 0;
	*free_vram = 0;
	*largest_free_block = 0;

	mutex_lock(&region_mutex);

	list_for_each_entry(rm, &region_list, list) {
		unsigned free;
		unsigned long pa;

		pa = rm->paddr;
		*vram += rm->pages << PAGE_SHIFT;

		list_for_each_entry(alloc, &rm->alloc_list, list) {
			free = alloc->paddr - pa;
			*free_vram += free;
			if (free > *largest_free_block)
				*largest_free_block = free;
			pa = alloc->paddr + (alloc->pages << PAGE_SHIFT);
		}

		free = rm->paddr + (rm->pages << PAGE_SHIFT) - pa;
		*free_vram += free;
		if (free > *largest_free_block)
			*largest_free_block = free;
	}

	mutex_unlock(&region_mutex);
}
EXPORT_SYMBOL(ti81xx_vram_get_info);

#if defined(CONFIG_DEBUG_FS)
static int vram_debug_show(struct seq_file *s, void *unused)
{
	struct vram_region *vr;
	struct vram_alloc *va;
	unsigned size;

	mutex_lock(&region_mutex);

	list_for_each_entry(vr, &region_list, list) {
		size = vr->pages << PAGE_SHIFT;
		seq_printf(s, "%08lx-%08lx (%d bytes)\n",
				vr->paddr, vr->paddr + size - 1,
				size);

		list_for_each_entry(va, &vr->alloc_list, list) {
			size = va->pages << PAGE_SHIFT;
			seq_printf(s, "    %08lx-%08lx (%d bytes)\n",
					va->paddr, va->paddr + size - 1,
					size);
		}
	}

	mutex_unlock(&region_mutex);

	return 0;
}

static int vram_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, vram_debug_show, inode->i_private);
}

static const struct file_operations vram_debug_fops = {
	.open           = vram_debug_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int __init ti81xx_vram_create_debugfs(void)
{
	struct dentry *d;

	d = debugfs_create_file("vram", S_IRUGO, NULL,
			NULL, &vram_debug_fops);
	if (IS_ERR(d))
		return PTR_ERR(d);

	return 0;
}
#endif

static __init int ti81xx_vram_init(void)
{
	int i;

	for (i = 0; i < postponed_cnt; i++)
		ti81xx_vram_add_region(postponed_regions[i].paddr,
				postponed_regions[i].size);

#ifdef CONFIG_DEBUG_FS
	if (ti81xx_vram_create_debugfs())
		printk(KERN_ERR "VRAM: Failed to create debugfs file\n");
#endif

	return 0;
}

arch_initcall(ti81xx_vram_init);

/* boottime vram alloc stuff */

/* set from board file */
static u32 ti81xxfb_sdram_vram_start __initdata;
static u32 ti81xxfb_sdram_vram_size __initdata;

/* set from kernel cmdline */
static u32 ti81xxfb_def_sdram_vram_size __initdata;
static u32 ti81xxfb_def_sdram_vram_start __initdata;

static int __init ti81xxfb_early_vram(char *p)
{
	ti81xxfb_def_sdram_vram_size = memparse(p, &p);
	if (*p == ',')
		ti81xxfb_def_sdram_vram_start = simple_strtoul(p + 1, &p, 16);

	printk(KERN_INFO "vram size = %d at %d\n",
		ti81xxfb_def_sdram_vram_size, ti81xxfb_def_sdram_vram_start);
	return 0;
}
early_param("vram", ti81xxfb_early_vram);

/*
 * Called from map_io. We need to call to this early enough so that we
 * can reserve the fixed SDRAM regions before VM could get hold of them.
 */
void __init ti81xxfb_reserve_sdram_memblock(void)
{
	u32 paddr;
	u32 size = 0;

	/* cmdline arg overrides the board file definition */
	if (ti81xxfb_def_sdram_vram_size) {
		size = ti81xxfb_def_sdram_vram_size;
		paddr = ti81xxfb_def_sdram_vram_start;
	}

	if (!size) {
		size = ti81xxfb_sdram_vram_size;
		paddr = ti81xxfb_sdram_vram_start;
	}

#ifdef CONFIG_TI81XX_VPSS_VRAM_SIZE
	if (!size) {
		size = CONFIG_TI81XX_VPSS_VRAM_SIZE * 1024 * 1024;
		paddr = 0;
	}
#endif

	printk(KERN_INFO "reserved size = %d at %d\n",
		size, paddr);


	if (!size)
		return;

	size = PAGE_ALIGN(size);

	if (paddr) {

		if (paddr & ~PAGE_MASK)  {
			pr_err("FB: Illegal SDRAM region for SDRAM\n");
			return;
		}

		if (!memblock_is_region_memory(paddr, size)) {
			pr_err("FB: Illegal SDRAM region 0x%08x..0x%08x for VRAM\n",
					paddr, paddr + size - 1);
			return;
		}

		if (memblock_is_region_reserved(paddr, size)) {
			pr_err("FB: failed to reserve SDRAM\n");
			return;
		}

		if (memblock_reserve(paddr, size) < 0) {
			pr_err("FB: failed o reserve SDRAM - no memory\n");
			return;
		}

	} else
		paddr = memblock_alloc(size, PAGE_SIZE);

	memblock_free(paddr, size);
	memblock_remove(paddr, size);

	ti81xx_vram_add_region_postponed(paddr, size);
	pr_info("FB: Reserving %u bytes SDRAM for VRAM\n", size);
}

void __init ti81xx_set_sdram_vram(u32 size, u32 start)
{
	ti81xxfb_sdram_vram_start = start;
	ti81xxfb_sdram_vram_size = size;

	printk(KERN_INFO "board vram size = %d at %d\n",
		ti81xxfb_sdram_vram_size, ti81xxfb_sdram_vram_start);

}
#endif
