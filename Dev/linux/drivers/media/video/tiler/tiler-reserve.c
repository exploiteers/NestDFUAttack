/*
 * tiler-reserve.c
 *
 * TILER driver area reservation functions for TI TILER hardware block.
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

#include "_tiler.h"

static struct tiler_ops *ops;	/* shared methods and variables */
static int band_8;		/* size of 8-bit band in slots */
static int band_16;		/* size of 16-bit band in slots */

/**
 * Calculate the maximum number buffers that can be packed next to each other,
 * and the area they occupy. This method is used for both 2D and NV12 packing.
 *
 * @author a0194118 (7/16/2010)
 *
 * @param o	desired offset
 * @param w	width of one block (>0)
 * @param a	desired alignment
 * @param b	band width (each block must occupy the same number of bands)
 * @param n	pointer to the desired number of blocks to pack.  It will be
 *		updated with the maximum number of blocks that can be packed.
 * @param _area	pointer to store total area needed
 *
 * @return packing efficiency (0-1024)
 */
static u32 tiler_best2pack(u16 o, u16 a, u16 b, u16 w, u16 *n, u16 *_area)
{
	u16 m = 0, max_n = *n;		/* m is mostly n - 1 */
	u16 e = ALIGN(w, a);		/* effective width of one block */
	u32 eff, best_eff = 0;		/* best values */
	u16 stride = ALIGN(o + w, b);	/* block stride */
	u16 area = stride;		/* area needed (for m + 1 blocks) */

	/* NOTE: block #m+1 occupies the range (o + m * e, o + m * e + w) */

	/* see how many blocks we can pack */
	while (m < max_n &&
		/* blocks must fit in tiler container */
		o + m * e + w <= ops->width &&
		/* block stride must be correct */
		stride == ALIGN(area - o - m * e, b)) {

		m++;
		eff = m * w * 1024 / area;
		if (eff > best_eff) {
			/* store packing for best efficiency & smallest area */
			best_eff = eff;
			*n = m;
			if (_area)
				*_area = area;
		}
		/* update area */
		area = ALIGN(o + m * e + w, b);
	}

	return best_eff;
}

/*
 * NV12 Reservation Functions
 *
 * TILER is designed so that a (w * h) * 8bit area is twice as wide as a
 * (w/2 * h/2) * 16bit area.  Since having pairs of such 8-bit and 16-bit
 * blocks is a common usecase for TILER, we optimize packing these into a
 * TILER area.
 *
 * During reservation we want to find the most effective packing (most used area
 * in the smallest overall area)
 *
 * We have two algorithms for packing nv12 blocks: either pack 8- and 16-bit
 * blocks into separate container areas, or pack them together into same area.
 */

/**
 * Calculate effectiveness of packing. We weight total area much higher than
 * packing efficiency to get the smallest overall container use.
 *
 * @param w		width of one (8-bit) block
 * @param n		buffers in a packing
 * @param area		width of packing area
 * @param n_total	total number of buffers to be packed
 * @return effectiveness, the higher the better
 */
static inline u32 nv12_eff(u16 w, u16 n, u16 area, u16 n_total)
{
	return 0x10000000 -
		/* weigh against total area needed (for all buffers) */
		/* 64-slots = -2048 */
		DIV_ROUND_UP(n_total, n) * area * 32 +
		/* packing efficiency (0 - 1024) */
		1024 * n * ((w * 3 + 1) >> 1) / area;
}

/**
 * Fallback nv12 packing algorithm: pack 8 and 16 bit block into separate
 * areas.
 *
 * @author a0194118 (7/16/2010)
 *
 * @param o	desired offset (<a)
 * @param a	desired alignment (>=2)
 * @param w	block width (>0)
 * @param n	number of blocks desired
 * @param area	pointer to store total area needed
 *
 * @return number of blocks that can be allocated
 */
static u16 nv12_separate(u16 o, u16 a, u16 w, u16 n, u16 *area)
{
	tiler_best2pack(o, a, band_8, w, &n, area);
	tiler_best2pack(o >> 1, a >> 1, band_16, (w + 1) >> 1, &n, area);
	*area *= 3;
	return n;
}

/*
 * Specialized NV12 Reservation Algorithms
 *
 * We use 4 packing methods that pack nv12 blocks into the same area.  Together
 * these 4 methods give the optimal result for most possible input parameters.
 *
 * For now we pack into a 64-slot area, so that we don't have to worry about
 * stride issues (all blocks get 4K stride). For some of the algorithms this
 * could be true even if the area was 128.
 */

/**
 * Packing types are marked using a letter sequence, capital letters denoting
 * 8-bit blocks, lower case letters denoting corresponding 16-bit blocks.
 *
 * All methods have the following parameters. They also define the maximum
 * number of coordinates that could potentially be packed.
 *
 * @param o, a, w, n offset, alignment, width, # of blocks as usual
 * @param area		pointer to store area needed for packing
 * @param p		pointer to store packing coordinates
 * @return		number of blocks that can be packed
 */

/* Method A: progressive packing: AAAAaaaaBBbbCc into 64-slot area */
#define MAX_A 21
static int nv12_A(u16 o, u16 a, u16 w, u16 n, u16 *area, u8 *p)
{
	u16 x = o, u, l, m = 0;
	*area = band_8;

	while (x + w < *area && m < n) {
		/* current 8bit upper bound (a) is next 8bit lower bound (B) */
		l = u = (*area + x) >> 1;

		/* pack until upper bound */
		while (x + w <= u && m < n) {
			/* save packing */
			BUG_ON(m + 1 >= MAX_A);
			*p++ = x;
			*p++ = l;
			l = (*area + x + w + 1) >> 1;
			x = ALIGN(x + w - o, a) + o;
			m++;
		}
		x = ALIGN(l - o, a) + o;	/* set new lower bound */
	}
	return m;
}

/* Method -A: regressive packing: cCbbBBaaaaAAAA into 64-slot area */
static int nv12_revA(u16 o, u16 a, u16 w, u16 n, u16 *area, u8 *p)
{
	u16 m;

	/* this is a mirrored packing of method A */
	n = nv12_A((a - (o + w) % a) % a, a, w, n, area, p);

	/* reverse packing */
	for (m = 0; m < n; m++) {
		*p = *area - *p - w;
		p++;
		*p = *area - *p - ((w + 1) >> 1);
		p++;
	}
	return n;
}

/* Method B: simple layout: aAbcBdeCfgDhEFGH */
#define MAX_B 8
static int nv12_B(u16 o, u16 a, u16 w, u16 n, u16 *area, u8 *p)
{
	u16 e  = (o + w) % a;	/* end offset */
	u16 o1 = (o >> 1) % a;			/* half offset */
	u16 e1 = ((o + w + 1) >> 1) % a;	/* half end offset */
	u16 o2 = o1 + (a >> 2);			/* 2nd half offset */
	u16 e2 = e1 + (a >> 2);			/* 2nd half end offset */
	u16 m = 0;
	*area = band_8;

	/* ensure 16-bit blocks don't overlap 8-bit blocks */

	/* width cannot wrap around alignment, half block must be before block,
	   2nd half can be before or after */
	if (w < a && o < e && e1 <= o && (e2 <= o || o2 >= e))
		while (o + w <= *area && m < n) {
			BUG_ON(m + 1 >= MAX_B);
			*p++ = o;
			*p++ = o >> 1;
			m++;
			o += a;
		}
	return m;
}

/* Method C: butterfly layout: AAbbaaBB */
#define MAX_C 20
static int nv12_C(u16 o, u16 a, u16 w, u16 n, u16 *area, u8 *p)
{
	int m = 0;
	u16 o2, e = ALIGN(w, a), i = 0, j = 0;
	*area = band_8;
	o2 = *area - (a - (o + w) % a) % a;	/* end of last possible block */

	m = (min(o2 - 2 * o, 2 * o2 - o - *area) / 3 - w) / e + 1;
	for (i = j = 0; i < m && j < n; i++, j++) {
		BUG_ON(j + 1 >= MAX_C);
		*p++ = o + i * e;
		*p++ = (o + i * e + *area) >> 1;
		if (++j < n) {
			*p++ = o2 - i * e - w;
			*p++ = (o2 - i * e - w) >> 1;
		}
	}
	return j;
}

/* Method D: for large allocation: aA or Aa */
#define MAX_D 1
static int nv12_D(u16 o, u16 a, u16 w, u16 n, u16 *area, u8 *p)
{
	u16 o1, w1 = (w + 1) >> 1, d;
	*area = ALIGN(o + w, band_8);

	for (d = 0; n > 0 && d + o + w <= *area; d += a) {
		/* try to fit 16-bit before 8-bit */
		o1 = ((o + d) % band_8) >> 1;
		if (o1 + w1 <= o + d) {
			*p++ = o + d;
			*p++ = o1;
			return 1;
		}

		/* try to fit 16-bit after 8-bit */
		o1 += ALIGN(d + o + w - o1, band_16);
		if (o1 + w1 <= *area) {
			*p++ = o;
			*p++ = o1;
			return 1;
		}
	}
	return 0;
}

/**
 * Umbrella nv12 packing method. This selects the best packings from the above
 * methods.  It also contains hardcoded packings for parameter combinations
 * that have more efficient packings. This method provides is guaranteed to
 * provide the optimal packing if 2 <= a <= 64 and w <= 64 and n is large.
 */
#define MAX_ANY 21	/* must be MAX(method-MAX-s, hardcoded n-s) */
static u16 nv12_together(u16 o, u16 a, u16 w, u16 n, u16 *area, u8 *packing)
{
	u16 n_best, a_best, n2, a_, o_, w_;

	/* algo results (packings) */
	u8 pack_A[MAX_A * 2], pack_rA[MAX_A * 2];
	u8 pack_B[MAX_B * 2], pack_C[MAX_C * 2];
	u8 pack_D[MAX_D * 2];

	/*
	 * Hardcoded packings.  They are sorted by increasing area, and then by
	 * decreasing n.  We may not get the best efficiency if less than n
	 * blocks are needed as packings are not necessarily sorted in
	 * increasing order.  However, for those n-s one of the other 4 methods
	 * may return the optimal packing.
	 */
	u8 packings[] = {
		/* n=9, o=2, w=4, a=4, area=64 */
		9, 2, 4, 4, 64,
			/* 8-bit, 16-bit block coordinate pairs */
			2, 33,	6, 35,	10, 37,	14, 39,	18, 41,
			46, 23,	50, 25, 54, 27,	58, 29,
		/* o=0, w=12, a=4, n=3 */
		3, 0, 12, 4, 64,
			0, 32,	12, 38,	48, 24,
		/* end */
		0
	}, *p = packings, *p_best = NULL, *p_end;
	p_end = packings + sizeof(packings) - 1;

	/* see which method gives the best packing */

	/* start with smallest area algorithms A, B & C, stop if we can
	   pack all buffers */
	n_best = nv12_A(o, a, w, n, area, pack_A);
	p_best = pack_A;
	if (n_best < n) {
		n2 = nv12_revA(o, a, w, n, &a_best, pack_rA);
		if (n2 > n_best) {
			n_best = n2;
			p_best = pack_rA;
			*area = a_best;
		}
	}
	if (n_best < n) {
		n2 = nv12_B(o, a, w, n, &a_best, pack_B);
		if (n2 > n_best) {
			n_best = n2;
			p_best = pack_B;
			*area = a_best;
		}
	}
	if (n_best < n) {
		n2 = nv12_C(o, a, w, n, &a_best, pack_C);
		if (n2 > n_best) {
			n_best = n2;
			p_best = pack_C;
			*area = a_best;
		}
	}

	/* traverse any special packings */
	while (*p) {
		n2 = *p++;
		o_ = *p++;
		w_ = *p++;
		a_ = *p++;
		/* stop if we already have a better packing */
		if (n2 < n_best)
			break;

		/* check if this packing is satisfactory */
		if (a_ >= a && o + w + ALIGN(o_ - o, a) <= o_ + w_) {
			*area = *p++;
			n_best = min(n2, n);
			p_best = p;
			break;
		}

		/* skip to next packing */
		p += 1 + n2 * 2;
	}

	/*
	 * If so far unsuccessful, check whether 8 and 16 bit blocks can be
	 * co-packed.  This will actually be done in the end by the normal
	 * allocation, but we need to reserve a big-enough area.
	 */
	if (!n_best) {
		n_best = nv12_D(o, a, w, n, area, pack_D);
		p_best = NULL;
	}

	/* store best packing */
	if (p_best && n_best) {
		BUG_ON(n_best > MAX_ANY);
		memcpy(packing, p_best, n_best * 2 * sizeof(*pack_A));
	}

	return n_best;
}

/* reserve nv12 blocks */
static void reserve_nv12(u32 n, u32 width, u32 height, u32 align, u32 offs,
					u32 gid, struct process_info *pi)
{
	u16 w, h, band, a = align, o = offs;
	struct gid_info *gi;
	int res = 0, res2, i;
	u16 n_t, n_s, area_t, area_s;
	u8 packing[2 * MAX_ANY];
	struct list_head reserved = LIST_HEAD_INIT(reserved);

	/* adjust alignment to the largest slot width (128 bytes) */
	a = max_t(u16, PAGE_SIZE / min(band_8, band_16), a);

	/* Check input parameters for correctness, and support */
	if (!width || !height || !n ||
	    offs >= align || offs & 1 ||
	    align >= PAGE_SIZE ||
	    n > ops->width * ops->height / 2)
		return;

	/* calculate dimensions, band, offs and alignment in slots */
	if (ops->analize(TILFMT_8BIT, width, height, &w, &h, &band, &a, &o,
									NULL))
		return;

	/* get group context */
	gi = ops->get_gi(pi, gid);
	if (!gi)
		return;

	/* reserve in groups until failed or all is reserved */
	for (i = 0; i < n && res >= 0; i += res) {
		/* check packing separately vs together */
		n_s = nv12_separate(o, a, w, n - i, &area_s);
		if (ops->nv12_packed)
			n_t = nv12_together(o, a, w, n - i, &area_t, packing);
		else
			n_t = 0;

		/* pack based on better efficiency */
		res = -1;
		if (!ops->nv12_packed ||
			nv12_eff(w, n_s, area_s, n - i) >
			nv12_eff(w, n_t, area_t, n - i)) {

			/*
			 * Reserve blocks separately into a temporary list, so
			 * that we can free them if unsuccessful. We need to be
			 * able to reserve both 8- and 16-bit blocks as the
			 * offsets of them must match.
			 */
			res = ops->lay_2d(TILFMT_8BIT, n_s, w, h, band_8, a, o,
						gi, &reserved);
			res2 = ops->lay_2d(TILFMT_16BIT, n_s, (w + 1) >> 1, h,
				band_16, a >> 1, o >> 1, gi, &reserved);

			if (res2 < 0 || res < 0 || res != res2) {
				/* clean up */
				ops->release(&reserved);
				res = -1;
			} else {
				/* add list to reserved */
				ops->add_reserved(&reserved, gi);
			}
		}

		/* if separate packing failed, still try to pack together */
		if (res < 0 && ops->nv12_packed && n_t) {
			/* pack together */
			res = ops->lay_nv12(n_t, area_t, w, h, gi, packing);
		}
	}

	ops->release_gi(gi);
}

/**
 * We also optimize packing regular 2D areas as the auto-packing may result in
 * sub-optimal efficiency. This is most pronounced if the area is wider than
 * half a PAGE_SIZE (e.g. 2048 in 8-bit mode, or 1024 in 16-bit mode).
 */

/* reserve 2d blocks */
static void reserve_blocks(u32 n, enum tiler_fmt fmt, u32 width, u32 height,
			   u32 align, u32 offs, u32 gid,
			   struct process_info *pi)
{
	u32 bpt, res = 0, i;
	u16 o = offs, a = align, band, w, h, n_try;
	struct gid_info *gi;
	const struct tiler_geom *g;

	/* Check input parameters for correctness, and support */
	if (!width || !height || !n ||
	    align > PAGE_SIZE || offs >= align ||
	    fmt < TILFMT_8BIT || fmt > TILFMT_32BIT)
		return;

	/* tiler slot in bytes */
	g = ops->geom(fmt);
	bpt = g->slot_w * g->bpp;

	/*
	 *  For blocks narrower than half PAGE_SIZE the default allocation is
	 *  sufficient.  Also check for basic area info.
	 */
	if (width * g->bpp * 2 <= PAGE_SIZE ||
	    ops->analize(fmt, width, height, &w, &h, &band, &a, &o, NULL))
		return;

	/* get group id */
	gi = ops->get_gi(pi, gid);
	if (!gi)
		return;

	/* reserve in groups until failed or all is reserved */
	for (i = 0; i < n && res >= 0; i += res + 1) {
		/* blocks to allocate in one area */
		n_try = min(n - i, ops->width);
		tiler_best2pack(offs, a, band, w, &n_try, NULL);

		res = -1;
		while (n_try > 1) {
			/* adjust res so we fail on 0 return value */
			res = ops->lay_2d(fmt, n_try, w, h, band, a, o,
						gi, &gi->reserved) - 1;
			if (res >= 0)
				break;

			/* reduce n if failed to allocate area */
			n_try--;
		}
	}
	/* keep reserved blocks even if failed to reserve all */

	ops->release_gi(gi);
}

/* unreserve blocks for a group id */
static void unreserve_blocks(u32 gid, struct process_info *pi)
{
	struct gid_info *gi;

	gi = ops->get_gi(pi, gid);
	if (!gi)
		return;

	ops->release(&gi->reserved);

	ops->release_gi(gi);
}

/* initialize shared method pointers and global static variables */
void tiler_reserve_init(struct tiler_ops *tiler)
{
	ops = tiler;

	ops->reserve_nv12 = reserve_nv12;
	ops->reserve = reserve_blocks;
	ops->unreserve = unreserve_blocks;

	band_8 = PAGE_SIZE / ops->geom(TILFMT_8BIT)->slot_w
		/ ops->geom(TILFMT_8BIT)->bpp;
	band_16 = PAGE_SIZE / ops->geom(TILFMT_16BIT)->slot_w
		/ ops->geom(TILFMT_16BIT)->bpp;
}
