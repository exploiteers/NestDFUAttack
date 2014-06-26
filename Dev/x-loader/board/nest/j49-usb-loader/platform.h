/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *      This file defines data structures and function prototypes for
 *      accessing Digital Phased Lock Loop (DPLL) parameters and
 *      settings.
 *
 */

#ifndef _NEST_J49_PLATFORM_H_
#define _NEST_J49_PLATFORM_H_

#include <asm/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Type Definitions
 */

/* Used to index into DPLL parameter tables */

struct dpll_param {
	u32 m;
	u32 n;
	u32 fsel;
	u32 m2;
};

struct dpll_per_36x_param {
	u32 sys_clk;
	u32 m;
	u32 n;
	u32 m2;
	u32 m3;
	u32 m4;
	u32 m5;
	u32 m6;
	u32 m2div;
};

typedef struct dpll_param dpll_param;

/*
 * Inline Functions
 */
static inline void
delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}


/*
 * Function Prototypes
 *
 * The following functions are exported from platform.S.
 */

extern dpll_param * get_mpu_dpll_param(void);
extern dpll_param * get_iva_dpll_param(void);
extern dpll_param * get_core_dpll_param(void);
extern dpll_param * get_per_dpll_param(void);

extern dpll_param * get_36x_mpu_dpll_param(void);
extern dpll_param * get_36x_iva_dpll_param(void);
extern dpll_param * get_36x_core_dpll_param(void);
extern dpll_param * get_36x_per_dpll_param(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NEST_J49_PLATFORM_H_ */
