/*
 * tcm_utils.h
 *
 * Utility functions for implementing TILER container managers.
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

#ifndef TCM_UTILS_H
#define TCM_UTILS_H

#include "../tcm.h"

/* TCM_ALG_NAME must be defined to use the debug methods */

#ifdef DEBUG
#define IFDEBUG(x) x
#else
/* compile-check debug statements even if not DEBUG */
#define IFDEBUG(x) do { if (0) x; } while (0)
#endif

#define P(level, fmt, ...) \
	IFDEBUG(printk(level TCM_ALG_NAME ":%d:%s()" fmt "\n", \
			__LINE__, __func__, ##__VA_ARGS__))

#define P1(fmt, ...) P(KERN_NOTICE, fmt, ##__VA_ARGS__)
#define P2(fmt, ...) P(KERN_INFO, fmt, ##__VA_ARGS__)
#define P3(fmt, ...) P(KERN_DEBUG, fmt, ##__VA_ARGS__)

#define PA(level, msg, p_area) P##level(msg " (%03d %03d)-(%03d %03d)\n", \
	(p_area)->p0.x, (p_area)->p0.y, (p_area)->p1.x, (p_area)->p1.y)

/* assign coordinates to area */
static inline
void assign(struct tcm_area *a, u16 x0, u16 y0, u16 x1, u16 y1)
{
	a->p0.x = x0;
	a->p0.y = y0;
	a->p1.x = x1;
	a->p1.y = y1;
}

#endif
