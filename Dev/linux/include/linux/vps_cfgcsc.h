/*
 *
 * CSC header file for TI81XX VPSS
 *
 * Copyright (C) 2010 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.
 */

#ifndef __LINUX_VPS_CFGCSC_H__
#define __LINUX_VPS_CFGCSC_H__

#ifdef __KERNEL__


/** \brief num of coefficients in each set */
#define VPS_CSC_NUM_COEFF           (3u)

/**
 *  \brief enum for specifying per-defined csc co-effs
 */
enum vps_cscmode {
	VPS_CSC_MODE_SDTV_VIDEO_R2Y = 0,
	/**< Select coefficient for SDTV Video */
	VPS_CSC_MODE_SDTV_VIDEO_Y2R,
	/**< Select coefficient for SDTV Video */
	VPS_CSC_MODE_SDTV_GRAPHICS_R2Y,
	/**< Select coefficient for SDTV Graphics */
	VPS_CSC_MODE_SDTV_GRAPHICS_Y2R,
	/**< Select coefficient for SDTV Graphics */
	VPS_CSC_MODE_HDTV_VIDEO_R2Y,
	/**< Select coefficient for HDTV Video */
	VPS_CSC_MODE_HDTV_VIDEO_Y2R,
	/**< Select coefficient for HDTV Video */
	VPS_CSC_MODE_HDTV_GRAPHICS_R2Y,
	/**< Select coefficient for HDTV Graphics */
	VPS_CSC_MODE_HDTV_GRAPHICS_Y2R,
	/**< Select coefficient for HDTV Graphics */
	VPS_CSC_MODE_MAX,
	/**< Should be the last value of this enumeration.
	Will be used by driver for validating the input parameters. */
	VPS_CSC_MODE_NONE = 0xFFFFu
	/**< Used when coefficients are provided */
};


/* ========================================================================== */
/*                         structure declarations                             */
/* ========================================================================== */

/**
 *  struct vps_csccoeff
 * \brief set the coefficients for color space conversion.
 */
struct vps_csccoeff {
	u32              mulcoeff[VPS_CSC_NUM_COEFF][VPS_CSC_NUM_COEFF];
	/**< multiplication coefficients in the format a0, b0, c0
	 *  in the first row, a1, b1, c1 in the second row.
	 *  and a2, b2, c2 in the third row. */
	u32              addcoeff[VPS_CSC_NUM_COEFF];
	/**< addition coefficients. */
};

/**
 *  struct vps_cscconfig
 *  \brief configuration parameters for csc.
 */
struct vps_cscconfig {
	u32                       bypass;
	/**< flag to indicate whether csc to be bypassed or not. */
	u32                       mode;
	/**< used to select one of pre-calculated coefficient sets. used only
	if coeff is null. for valid values see #vps_cscmode. */
	struct vps_csccoeff       *coeff;
	/**< set of user provided coefficients. null if pre-calculated
	coefficients is to be used. */
};


#endif
#endif
