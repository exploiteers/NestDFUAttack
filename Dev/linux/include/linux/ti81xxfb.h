/*
 * File: include/linux/ti81xxfb.h
 * Framebuffer driver header file for TI 81xx
 *
 * Copyright (C) 2009 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * Some code and ideas was from TI OMAP2 Driver by Tomi Valkeinen.
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

#ifndef __LINUX_TI81XXFB_H__
#define __LINUX_TI81XXFB_H__

#include <linux/fb.h>
#include <linux/ioctl.h>
#include <linux/types.h>


#define TI81XXFB_CLUT_ENTRY 256
#define TI81XXFB_CLUT_UNIT 4
#define TI81XXFB_CLUT_SIZE (TI81XXFB_CLUT_ENTRY * TI81XXFB_CLUT_UNIT)
#define TI81XXFB_CLUT_MASK 0xFF

#define TI81XXFB_COEFF_PHASE       0x8
#define TI81XXFB_COEFF_HOR_TAP     0x5
#define TI81XXFB_COEFF_VER_TAP     0x4
#define TI81XXFB_HOR_COEFF_SIZE   (TI81XXFB_COEFF_HOR_TAP * \
					TI81XXFB_COEFF_PHASE)
#define TI81XXFB_VER_COEFF_SIZE   (TI81XXFB_COEFF_VER_TAP * \
					TI81XXFB_COEFF_PHASE)

/*customer ioctl definitions*/
#define TIFB_IOW(num, dtype)  _IOW('N', num, dtype)
#define TIFB_IOR(num, dtype)  _IOR('N', num, dtype)
#define TIFB_IOWR(num, dtype) _IOWR('N', num, dtype)
#define TIFB_IO(num)          _IO('N', num)

#define TIFB_SET_PARAMS   TIFB_IOW(2, struct ti81xxfb_region_params)
#define TIFB_GET_PARAMS   TIFB_IOR(3, struct ti81xxfb_region_params)
#define TIFB_SET_SCINFO   TIFB_IOW(4, struct ti81xxfb_scparams)
#define TIFB_GET_SCINFO   TIFB_IOW(5, struct ti81xxfb_scparams)
#define TIFB_ALLOC        TIFB_IOW(6, int)
#define TIFB_FREE         TIFB_IOR(7, int)
#define TIFB_SETUP_MEM    TIFB_IOW(8, struct ti81xxfb_mem_info)
#define TIFB_QUERY_MEM    TIFB_IOR(9, struct ti81xxfb_mem_info)
#define TIFB_SET_STENC    TIFB_IOW(10, struct ti81xxfb_stenciling_params)


enum ti81xxfb_status {
	TI81XXFB_FEATURE_DISABLE = 0,
	TI81XXFB_FEATURE_ENABLE
};

enum ti81xxfb_rotate_type {
	TI81XXFB_ROTATE_NO = 0,
	TI81XXFB_ROTATE_180_MIRROR,
	TI81XXFB_ROTATE_MIRRORONLY,
	TI81XXFB_ROTATE_180,
	TI81XXFB_ROTATE_270_MIRROR,
	TI81XXFB_ROTATE_270,
	TI81XXFB_ROTATE_90,
	TI81XXFB_ROTATE_90_MIRROR,
	TI81XXFB_ROTATE_MAX
};

enum ti81xxfb_mirroring {
	TI81XXFB_MIRRORING_OFF = 0,
	TI81XXFB_MIRRORING_ON
};

/**
 * ti81xxfb_pix_format
 *	 Define the GRPX data format
 */
enum ti81xxfb_data_format {
	TI81XXFB_RGB565 = 0x1000,
	TI81XXFB_ARGB1555,
	TI81XXFB_RGBA5551,
	TI81XXFB_ARGB4444,
	TI81XXFB_RGBA4444,
	TI81XXFB_ARGB6666,
	TI81XXFB_RGBA6666,
	TI81XXFB_RGB888,
	TI81XXFB_ARGB8888,
	TI81XXFB_RGBA8888,
	TI81XXFB_BMP8 = 0x2000,
	TI81XXFB_BMP4_L,
	TI81XXFB_BMP4_U,
	TI81XXFB_BMP2_OFF0,
	TI81XXFB_BMP2_OFF1,
	TI81XXFB_BMP2_OFF2,
	TI81XXFB_BMP2_OFF3,
	TI81XXFB_BMP1_OFF0,
	TI81XXFB_BMP1_OFF1,
	TI81XXFB_BMP1_OFF2,
	TI81XXFB_BMP1_OFF3,
	TI81XXFB_BMP1_OFF4,
	TI81XXFB_BMP1_OFF5,
	TI81XXFB_BMP1_OFF6,
	TI81XXFB_BMP1_OFF7
};

enum ti81xxfb_mem_mode {
	TI81XXFB_MEM_NONTILER = 0,
	TI81XXFB_MEM_TILER_PAGE ,
	TI81XXFB_MEM_TILER_8,
	TI81XXFB_MEM_TILER_16,
	TI81XXFB_MEM_TILER_32
};

enum ti81xxfb_blending_type {
	TI81XXFB_BLENDING_NO = 0,
	TI81XXFB_BLENDING_GLOBAL,
	TI81XXFB_BLENDING_PALETTE,
	TI81XXFB_BLENDING_PIXEL

};

enum ti81xxfb_transparancy_type {
	TI81XXFB_TRANSP_LSPMASK_NO = 0,
	TI81XXFB_TRANSP_LSPMASK_1,
	TI81XXFB_TRANSP_LSPMASK_2,
	TI81XXFB_TRANSP_LSMMASK_3
};

struct ti81xxfb_mem_info {
	__u32  size;
	/*enum ti81xxfb_mem_mode*/
	__u8  type;
	__u8  reserved[3];
};

struct ti81xxfb_region_params {
	__u16               ridx;
	__u16               pos_x;
	__u16               pos_y;
	__u16               priority;
	/*enum ti81xxfb_status*/
	__u32               firstregion;
	/*enum ti81xxfb_status*/
	__u32               lastregion;
	/*enum ti81xxfb_status*/
	__u32               scalaren;
	/*enum ti81xxfb_status*/
	__u32               stencilingen;
	/*enum ti81xxfb_status*/
	__u32               bben;
	/*enum ti81xxfb_status*/
	__u32               transen;
	/*enum ti81xxfb_blending_type*/
	__u32               blendtype;
	/*enum ti81xxfb_transparancy_type*/
	__u32               transtype;
	__u32               transcolor;
	__u8                bbalpha;
	__u8                blendalpha;
	__u8                reserved[2];
};

struct ti81xxfb_coeff {
	__u16     horcoeff[TI81XXFB_COEFF_HOR_TAP][TI81XXFB_COEFF_PHASE];
	__u16     vercoeff[TI81XXFB_COEFF_VER_TAP][TI81XXFB_COEFF_PHASE];
};

struct ti81xxfb_scparams {
	__u16                   inwidth;
	__u16                   inheight;
	__u16                   outwidth;
	__u16                   outheight;
	struct ti81xxfb_coeff   *coeff;
};

struct ti81xxfb_stenciling_params {
	__u32               pitch;
	__u32               paddr;
};

/**
 * ti81xxfb_regions_list
 *	 Defines the regions list which contain the information
 *	 for each regions in the given frame
 */
struct ti81xxfb_regions_list {
	__u32                           num_reg;
	struct ti81xxfb_region_params   *regions;
};

#ifdef __KERNEL__

#include <linux/fvid2.h>


#define TI81XXFB_MEMTYPE_SDRAM		0u
#define TI81XXFB_MEMTYPE_MAX		0u
#define TI81XX_FB_NUM (3)


/**
 *	 ti81xx_mem_region
 *	  Define the one frame buffer memroy information
 *
 */

struct  ti81xxfb_mem_region {
	u32             paddr;
	void __iomem    *vaddr;
	unsigned long   size;
	bool            alloc; /*allocated by the driver*/
	bool            map; /*kernel mapped by the driver*/
};


/**
 *   ti81xxfb_mem_desc
 *		define the memory regions for all fb
 */
struct ti81xxfb_mem_desc {
	int                         region_cnt;
	struct ti81xxfb_mem_region  mreg[TI81XX_FB_NUM];
};

struct ti81xxfb_platform_data {
	struct ti81xxfb_mem_desc   mem_desc;
	void                       *ctrl_platform_data;
};


extern void ti81xxfb_set_platform_data(struct ti81xxfb_platform_data *data);

#endif

#endif
