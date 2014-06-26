/*
 * (C) Copyright 2005
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _OMAP243X_CLOCKS_H_
#define _OMAP243X_CLOCKS_H_

/* cm_clksel core fields not ratio governed */
#define RX_CLKSEL_DSS1		(0x10 << 8)
#define RX_CLKSEL_DSS2		(0x0 << 13)
#define RX_CLKSEL_SSI		(0x5 << 20)

/* 2430 Ratio's */
/* 2430-Ratio Config 1 */
#define R1_CLKSEL_L3		(4 << 0)
#define R1_CLKSEL_L4		(2 << 5)
#define R1_CLKSEL_USB		(4 << 25)
#define R1_CM_CLKSEL1_CORE_VAL	R1_CLKSEL_USB | RX_CLKSEL_SSI | RX_CLKSEL_DSS2 \
                                | RX_CLKSEL_DSS1 | R1_CLKSEL_L4 | R1_CLKSEL_L3
#define R1_CLKSEL_MPU		(2 << 0)
#define R1_CM_CLKSEL_MPU_VAL	R1_CLKSEL_MPU
#define R1_CLKSEL_DSP		(2 << 0)
#define R1_CLKSEL_DSP_IF	(2 << 5)
#define R1_CM_CLKSEL_DSP_VAL	R1_CLKSEL_DSP | R1_CLKSEL_DSP_IF
#define R1_CLKSEL_GFX		(2 << 0)
#define R1_CM_CLKSEL_GFX_VAL	R1_CLKSEL_GFX
#define R1_CLKSEL_MDM		(4 << 0)
#define R1_CM_CLKSEL_MDM_VAL	R1_CLKSEL_MDM

/* 2430-Ratio Config 2 */
#define R2_CLKSEL_L3		(6 << 0)
#define R2_CLKSEL_L4		(2 << 5)
#define R2_CLKSEL_USB		(2 << 25)
#define R2_CM_CLKSEL1_CORE_VAL	R2_CLKSEL_USB | RX_CLKSEL_SSI | RX_CLKSEL_DSS2 \
                                | RX_CLKSEL_DSS1 | R2_CLKSEL_L4 | R2_CLKSEL_L3
#define R2_CLKSEL_MPU		(2 << 0)
#define R2_CM_CLKSEL_MPU_VAL	R2_CLKSEL_MPU
#define R2_CLKSEL_DSP		(2 << 0)
#define R2_CLKSEL_DSP_IF	(3 << 5)
#define R2_CM_CLKSEL_DSP_VAL	R2_CLKSEL_DSP | R2_CLKSEL_DSP_IF
#define R2_CLKSEL_GFX		(2 << 0)
#define R2_CM_CLKSEL_GFX_VAL    R2_CLKSEL_GFX
#define R2_CLKSEL_MDM		(6 << 0)
#define R2_CM_CLKSEL_MDM_VAL	R2_CLKSEL_MDM

/* 2430-Ratio Boot */
#define RB_CLKSEL_L3		(1 << 0)
#define RB_CLKSEL_L4		(1 << 5)
#define RB_CLKSEL_USB		(1 << 25)
#define RB_CM_CLKSEL1_CORE_VAL	RB_CLKSEL_USB | RX_CLKSEL_SSI | RX_CLKSEL_DSS2 \
                                | RX_CLKSEL_DSS1 | RB_CLKSEL_L4 | RB_CLKSEL_L3
#define RB_CLKSEL_MPU		(1 << 0)
#define RB_CM_CLKSEL_MPU_VAL	RB_CLKSEL_MPU
#define RB_CLKSEL_DSP		(1 << 0)
#define RB_CLKSEL_DSP_IF	(1 << 5)
#define RB_CM_CLKSEL_DSP_VAL	RB_CLKSEL_DSP | RB_CLKSEL_DSP_IF
#define RB_CLKSEL_GFX		(1 << 0)
#define RB_CM_CLKSEL_GFX_VAL	RB_CLKSEL_GFX
#define RB_CLKSEL_MDM		(1 << 0)
#define RB_CM_CLKSEL_MDM_VAL	RB_CLKSEL_MDM

/* 2430 Target modes: Along with each configuration the CPU has several modes
 * which goes along with them. Modes mainly are the addition of descrite DPLL
 * combinations to go along with a ratio.
 */
/* hardware goverend */
#define MX_48M_SRC		(0 << 3)
#define MX_54M_SRC		(0 << 5)
#define MX_APLLS_CLIKIN_12	(3 << 23)
#define MX_APLLS_CLIKIN_13	(2 << 23)
#define MX_APLLS_CLIKIN_19_2	(0 << 23)

/* 2430 - standalone, 2*ref*M/(n+1), M/N is for exactness not relock speed */

/* boot (boot) */
#define MB_DPLL_MULT				(1 << 12)
#define MB_DPLL_DIV					(0 << 8)
#define MB_CM_CLKSEL1_PLL_12_VAL     MX_48M_SRC | MX_54M_SRC | MB_DPLL_DIV \
                                    | MB_DPLL_MULT | MX_APLLS_CLIKIN_12

#define MB_CM_CLKSEL1_PLL_13_VAL    MX_48M_SRC | MX_54M_SRC | MB_DPLL_DIV \
                                    | MB_DPLL_MULT | MX_APLLS_CLIKIN_13

#define MB_CM_CLKSEL1_PLL_19_VAL    MX_48M_SRC | MX_54M_SRC | MB_DPLL_DIV \
                                    | MB_DPLL_MULT | MX_APLLS_CLIKIN_19

/* #2   (ratio1) DPLL = 330*2 = 660MHz, L3=165MHz */

#define M2_DPLL_MULT_12		        (55 << 12)
#define M2_DPLL_DIV_12		        (1 << 8)
#define M2_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | M2_DPLL_DIV_12 \
                                    | M2_DPLL_MULT_12 | MX_APLLS_CLIKIN_12
/* Use 658.7MHz instead of 660MHz for LP-Refresh M=76 N=2, relock time issue */
#define M2_DPLL_MULT_13		        (330 << 12)
#define M2_DPLL_DIV_13		        (12 << 8)
#define M2_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | M2_DPLL_DIV_13 \
                                    | M2_DPLL_MULT_13 | MX_APLLS_CLIKIN_13
#define M2_DPLL_MULT_19		        (275 << 12)
#define M2_DPLL_DIV_19		        (15 << 8)
#define M2_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | M2_DPLL_DIV_19 \
                                    | M2_DPLL_MULT_19 | MX_APLLS_CLIKIN_19_2

/* #3   (ratio2) DPLL = 330*2 = 660MHz, L3=110MHz */
#define M3_DPLL_MULT_12		        (55 << 12)
#define M3_DPLL_DIV_12		        (1 << 8)
#define M3_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | M3_DPLL_DIV_12 \
                                    | M3_DPLL_MULT_12 | MX_APLLS_CLIKIN_12
#define M3_DPLL_MULT_13		        (330 << 12)
#define M3_DPLL_DIV_13		        (12 << 8)
#define M3_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | M3_DPLL_DIV_13 \
                                    | M3_DPLL_MULT_13 | MX_APLLS_CLIKIN_13
#define M3_DPLL_MULT_19		        (275 << 12)
#define M3_DPLL_DIV_19		        (15 << 8)
#define M3_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | M3_DPLL_DIV_19 \
                                    | M3_DPLL_MULT_19 | MX_APLLS_CLIKIN_19_2

/* #4   (ratio2), DPLL = 399*2 = 798MHz, L3=133MHz*/
#define M4_DPLL_MULT_12		        (133 << 12)
#define M4_DPLL_DIV_12		        (3 << 8)
#define M4_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | M4_DPLL_DIV_12 \
                                    | M4_DPLL_MULT_12 | MX_APLLS_CLIKIN_12
#define M4_DPLL_MULT_13		        (399 << 12)
#define M4_DPLL_DIV_13		        (12 << 8)
#define M4_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | M4_DPLL_DIV_13 \
                                    | M4_DPLL_MULT_13 | MX_APLLS_CLIKIN_13
#define M4_DPLL_MULT_19		        (145 << 12)
#define M4_DPLL_DIV_19		        (6 << 8)
#define M4_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | M4_DPLL_DIV_19 \
                                    | M4_DPLL_MULT_19 | MX_APLLS_CLIKIN_19_2

/* #5a  (ratio1) baseport-target, target DPLL = 266*2 = 532MHz, L3=133MHz */
#define M5A_DPLL_MULT_12	        (133 << 12)
#define M5A_DPLL_DIV_12		        (5 << 8)
#define M5A_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | M5A_DPLL_DIV_12 \
                                    | M5A_DPLL_MULT_12 | MX_APLLS_CLIKIN_12
#define M5A_DPLL_MULT_13	        (266 << 12)
#define M5A_DPLL_DIV_13		        (12 << 8)
#define M5A_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | M5A_DPLL_DIV_13 \
                                    | M5A_DPLL_MULT_13 | MX_APLLS_CLIKIN_13
#define M5A_DPLL_MULT_19	        (180 << 12)
#define M5A_DPLL_DIV_19		        (12 << 8)
#define M5A_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | M5A_DPLL_DIV_19 \
                                    | M5A_DPLL_MULT_19 | MX_APLLS_CLIKIN_19_2

/* #5b  (ratio1) target DPLL = 200*2 = 400MHz, L3=100MHz */
#define M5B_DPLL_MULT_12	        (50 << 12)
#define M5B_DPLL_DIV_12		        (2 << 8)
#define M5B_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | M5B_DPLL_DIV_12 \
                                    | M5B_DPLL_MULT_12 | MX_APLLS_CLIKIN_12
#define M5B_DPLL_MULT_13	        (200 << 12)
#define M5B_DPLL_DIV_13		        (12 << 8)

#define M5B_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | M5B_DPLL_DIV_13 \
                                    | M5B_DPLL_MULT_13 | MX_APLLS_CLIKIN_13
#define M5B_DPLL_MULT_19	        (125 << 12)
#define M5B_DPLL_DIV_19		        (31 << 8)
#define M5B_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | M5B_DPLL_DIV_19 \
                                    | M5B_DPLL_MULT_19 | MX_APLLS_CLIKIN_19_2

/* 2430 - chassis (sedna) */
	/* 165 (ratio1) same as above #2 */
	/* 150 (ratio1)*/
	/* 133 (ratio2) same as above #4 */
	/* 110 (ratio2) same as above #3*/
	/* 104 (ratio2)*/
	/* boot (boot) */

/* high and low operation value */
#define MX_CLKSEL2_PLL_2x_VAL   (2 << 0)
#define MX_CLKSEL2_PLL_1x_VAL   (1 << 0)

/* set defaults for boot up */
#if defined(PRCM_CONFIG_2)     /* ARM-330MHz IVA2-330MHz L3-165MHz */
# define DPLL_OUT	MX_CLKSEL2_PLL_2x_VAL
# define MPU_DIV	R1_CLKSEL_MPU
# define DSP_DIV	R1_CM_CLKSEL_DSP_VAL
# define GFX_DIV	R1_CM_CLKSEL_GFX_VAL
# define BUS_DIV	R1_CM_CLKSEL1_CORE_VAL
# define DPLL_VAL	M2_CM_CLKSEL1_PLL_13_VAL
# define MDM_DIV	R2_CM_CLKSEL_MDM_VAL
#elif defined(PRCM_CONFIG_3)     /* ARM-330MHz IVA2-330MHz L3-110MHz */
# define DPLL_OUT	MX_CLKSEL2_PLL_2x_VAL
# define MPU_DIV	R2_CLKSEL_MPU
# define DSP_DIV	R2_CM_CLKSEL_DSP_VAL
# define GFX_DIV	R2_CM_CLKSEL_GFX_VAL
# define BUS_DIV	R2_CM_CLKSEL1_CORE_VAL
# define DPLL_VAL	M3_CM_CLKSEL1_PLL_13_VAL
# define MDM_DIV	R2_CM_CLKSEL_MDM_VAL
#elif defined(PRCM_CONFIG_5A)  /* ARM-266MHz IVA2-266MHz L3-133MHz */
# define DPLL_OUT	MX_CLKSEL2_PLL_2x_VAL
# define MPU_DIV	R1_CLKSEL_MPU
# define DSP_DIV	R1_CM_CLKSEL_DSP_VAL
# define GFX_DIV	R1_CM_CLKSEL_GFX_VAL
# define BUS_DIV	R1_CM_CLKSEL1_CORE_VAL
# define DPLL_VAL	M5A_CM_CLKSEL1_PLL_13_VAL
# define MDM_DIV	R2_CM_CLKSEL_MDM_VAL
#elif defined(PRCM_CONFIG_5B)  /* ARM-200MHz IVA2-200MHz L3-100MHz */
# define DPLL_OUT	MX_CLKSEL2_PLL_2x_VAL
# define MPU_DIV	R1_CLKSEL_MPU
# define DSP_DIV	R1_CM_CLKSEL_DSP_VAL
# define GFX_DIV	R1_CM_CLKSEL_GFX_VAL
# define BUS_DIV	R1_CM_CLKSEL1_CORE_VAL
# define DPLL_VAL	M5B_CM_CLKSEL1_PLL_13_VAL
# define MDM_DIV	R1_CM_CLKSEL_MDM_VAL
#endif

#endif
