/*
 * tcm_sita.h
 *
 * SImple Tiler Allocator (SiTA) interface.
 *
 * Author: Ravi Ramachandra <r.ramachandra@ti.com>
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

#ifndef TCM_SITA_H
#define TCM_SITA_H

#include "../tcm.h"

/**
 * Create a SiTA tiler container manager.
 *
 * @param width  Container width
 * @param height Container height
 * @param attr   preferred division point between 64-aligned
 *		 allocation (top left), 32-aligned allocations
 *		 (top right), and page mode allocations (bottom)
 *
 * @return TCM instance
 */
struct tcm *sita_init(u16 width, u16 height, struct tcm_pt *attr);

TCM_INIT(sita_init, struct tcm_pt);

#endif /* TCM_SITA_H_ */
