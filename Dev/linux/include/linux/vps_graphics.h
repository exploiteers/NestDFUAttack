/*
 *
 * Graphics header for TI81XX VPSS
 *
 * Copyright (C) 2009 TI
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

#ifndef __LINUX_VPS_GRAPHICS_H__
#define __LINUX_VPS_GRAPHICS_H__

#ifdef __KERNEL__
#define VPS_DISP_GRPX_MAX_INST	3
/**
 *	\brief Maximum number of driver instance.
 */
/** \brief Grpx0  driver instance number. */
#define VPS_DISP_INST_GRPX0	   (0u)
/** \brief Grpx1  driver instance number. */
#define VPS_DISP_INST_GRPX1	   (1u)
/** \brief Grpx2  driver instance number. */
#define VPS_DISP_INST_GRPX2	   (2u)


/** \brief GRPX coefficient phase number*/
#define VPS_GRPX_COEFF_PHASE	   (0x8u)
/** \brief GRPX horizontal coefficient tap number*/
#define VPS_GRPX_HOR_COEFF_TAP	   (0x5u)
/** \brief GRPX vertical coefficient tap number*/
#define VPS_GRPX_VER_COEFF_TAP	   (0x4u)
/** \brief size of horizontal coeff size(16bit) */
#define VPS_GRPX_HOR_COEFF_SIZE    (VPS_GRPX_COEFF_PHASE \
					* VPS_GRPX_HOR_COEFF_TAP)
/** \brief size of vertical coeff size(16bit) */
#define VPS_GRPX_VER_COEFF_SIZE  (VPS_GRPX_COEFF_PHASE \
					* VPS_GRPX_VER_COEFF_TAP)

/**\brief max regions to support in one frame  */
#define VPS_GRPX_MAX_REGIONS	   (1)

/**
 *  enum vps_grpx_driver_mode
 *  \brief This enum is used to define the mode of the graphics driver.
 *  frame buffer mode likes what FBDEV behaves.
 *  streaming mode likes V4l2 streaming mode.
 */
enum vps_grpxdrivermode {
	VPS_GRPX_STREAMING_MODE = 0,
	/**< Driver operates as streaming mode (streaming V4L2 mode).*/
	VPS_GRPX_FRAME_BUFFER_MODE,
	/**< Driver operates as frame buffer mode (non-streaming FBDEV mode). */
	VPS_GRPX_MODE_MAX
};


/**
 *	enum vps_grpx_blending_type
 *  \brief This enum is used to define various GRPX display blending type for
 *  regions. GRPX support 3 blending types: Global blending, palette blending
 *  and pixel blending.
 */
enum vps_grpxblendtype {
	VPS_GRPX_BLEND_NO = 0,
	/**< no blending, region ALPHA 0xFF was applied to make region opaque*/
	VPS_GRPX_BLEND_REGION_GLOBAL,
	/**< global region blending, user_defined alpha value was applied  */
	VPS_GRPX_BLEND_COLOR,
	/**< color (palette) blending, alpha value in the CLUT is applied*/
	VPS_GRPX_BLEND_PIXEL,
	/**< pixel(embedding alpha)blending, embedded alpha value is applied */
	VPS_GRPX_BLEND_MAX,
	/**< Should be the last value of this enumeration.
	Will be used by driver for validating the input parameters. */
};

/**
 *	enum vps_grpx_transparancy_type
 *  \brief This enum is used to define the GRPX transparency masking type for
 *  regions. number of bits will be masked out from user_defined color key
 *  comparison.
 *  Number bits of RGB component data is compared to the transparency data
 *  which is decided by the configuration data.  For example, if 3BIT_MASK is
 *  select, then MSB 5 bits of RGB data is compared
 *  to the transparency data.
 */
enum vps_grpxtransmasktype {
	VPS_GRPX_TRANS_NO_MASK = 0,
	/**< No mask, 8 bits compared */
	VPS_GRPX_TRANS_1BIT_MASK,
	/**< 1bit mask, MSB 7Bits compared */
	VPS_GRPX_TRANS_2BIT_MASK,
	/**< 2bit mask, MSB 6Bits compared */
	VPS_GRPX_TRANS_3BIT_MASK,
	/**< 3bit maks, MSB 5bits compared */
	VPS_GRPX_TRANS_MASK_MAX
	/**< Should be the last value of this enumeration.
	Will be used by driver for validating the input parameters. */
};

/**
 *	struct vps_grpx_scparams
 *	\brief This structure is used to define the configuration for graphic
 *	pholyphase scaler by the application. Those configuration include the
 *	scaler factor(0.25x-4x), phase(0-7) and fineoffset of the scaler
 */
struct vps_grpxscparams {
	u32				inwidth;
	/**< input region width */
	u32				inheight;
	/**< input region height */
	u32				outwidth;
	/**< output region width */
	u32				outheight;
	/**< output region height */
	u32				horfineoffset;
	/**< horizontal fine offset */
	u32				verfineoffset;
	/**< vertical fine offset */
	void				*sccoeff;
	/**< coefficient memory location, if scCoeff is set to NULL,
	     GRPX driver will load the preload scaling coefficients
	     based on the input/output ratio, if scScoeff is not NULL,
	     it must be defined by Vps_GrpxScCoeff to load
	     app defined coefficients.*/
};

/**
 *  struct Vps_GrpxScCoeff
 *  \brief This structure is used to define the coefficient for graphics
 *  pholyphase scaler by the application. The coefficient is in the order
 *  of tap0(phase 0-7), ... tapN(phase 0 - 7)
 */
struct vps_grpxsccoeff {
	u16				hscoeff[VPS_GRPX_HOR_COEFF_SIZE];
	/**< horizontal coefficient */
	u16				vscoeff[VPS_GRPX_VER_COEFF_SIZE];
	/**< vertical coefficients */
};

/**
 *	struct vps_grpx_reg_params
 * \brief This structure is used to define graphics region attributes.
 */
struct vps_grpxregionparams {
	u32				regionwidth;
	/**< width of current region */
	u32				regionheight;
	/**<  height of current region	*/
	u32				regionposx;
	/**< start x position of region in the current frame */
	u32				regionposy;
	/**< start y position of region in the current frame*/
	u32				disppriority;
	/**< display priority of region in the current frame, this priority
	     will be used to switch order between two overlapping regions
	     from two GRPX pipelines. */
	u32				firstregion;
	/**< first region or not, indicate this is the first region in the frame
	   app must set this accordingly*/
	u32				lastregion;
	/**< last region or not, indicate this is the last region in the frame,
	     app must set this accordingly.
	     For single region per frame use case, app must set both
	     firstRegion and lastRegion to TRUE. */
	u32				scenable;
	/**< Enables/Disables the scaling feature for grpahics progressvie
	     input data, scaling is invalid if the input graphics data is
	     interlaced. When both this flag is set to TRUE and  input region
	     size is equal to output region size, scalar performs anti-flicker
	     filtering instead of scaling.
	   */
	u32				stencilingenable;
	/**< Enables/Disables GRPX Stenciling feature for regions.
	     When this feature is enabled, app need provide a region mask
	     table, where each bit in the represent the one pixel in the
	     region, and the GRPX forces the alpha value of the pixel to ZERO
	     in order to "mask"-off the pixel if the bit associated with pixel
	     is one.*/
	u32				bbenable;
	/**< Eanbles/Disables GRPX Boundbox feature for regions.
	     When this feature is enabled, the GRPX supports overwriting alpha
	     values of pixels that make up a 1-pixel wide boundary box of a
	     region with a user defined alpha value so that the flickering
	     around the edges can be minimized. */
	u32				bbalpha;
	/**< stenciling enabled(1) */
	u32				blendalpha;
	/**< blending alpha value only valid if blendType is set to
	     global region*/
	u32				blendtype ;
	/**< blend type */
	u32				transenable;
	/**< transparency enable(1)  */
	u32				transtype;
	/**<  transparency type */
	u32				transcolorrgb24;
	/**< user defined RGB color data to perform transparency masking,
	  it need be in RGB888 format */
};


/**
 *  struct vps_grpxrtparams
 *  \brief This structure is used to define graphics configuration of each
 *  region. This structure is also used to store the runtime region
 *  paramters. App should pass structure pointer with perFrameCfg as
 *  RunTime GRPX parameters in the  FVID2_frame structure.
 */
struct vps_grpxrtparams {
	u32				regid;
	/**< The ID of region in the current frame, this ID must be start
	     from 0. For single region use case, it should be always 0.
	     For the multiple-region use case, it should in the range of
	     0 to (numRegions -1)*/
	u32				format;
	/**< data format */
	u32				pitch[FVID2_MAX_PLANES];
	/**< memory pitch*/
	struct vps_grpxregionparams	 regparams;
	/**< region attributes*/
	u32				rotation;
	/**< rotation type, only valid if GRPX data is stored in the
	     tiler memory*/
	struct vps_grpxscparams		*scparams;
	/**< Parameters for the GRPX Scalar, this field is reserved for
	     future usage, please set it to NULL*/
	void				*stenptr;
	/**< stenciling Ptr */
	u32 stenpitch;
	/**<stenciling pitch(stenciling data buffer) */
};

/**
 *	struct vps_grpxparamlist
 *  \brief This structure is used to define the parameters for all regions,
 *  which will be passed when application call IOCTL_VPS_SET_GRPX_PARAMS
 *  /IOCTL_VPS_GET_GRPX_PARAMS IOCTL to set/get the parameter
 *  for the graphcis driver...
 */
struct vps_grpxparamlist {
	u32				numregions;
	/**< number of regions to display per frame*/
	struct vps_grpxrtparams		*gparams;
	/**< array of the region configurations, size of the array is
       determined by the numRegions, this can not be set to NULL */
	struct vps_grpxscparams		*scparams;
	/**< scaling parameters of the GRPX, this can be NULL only if
	     no region in the frame has scaling feature enable*/
	void				*clutptr;
	/**< pointer to memory containing CLUT data, this is not array,
	    all regions will share the same CLUT.*/
};

/**
 *	struct vps_grpx_rtlist
 *	\brief This structure is used to define the runtime parameter sharing by
 *  shared by all regions in the given frame. This struture should
 *  be passed throught perListCfg in the FVID2_frameList or
 *  FVID2_processList
 */
struct vps_grpxrtlist {
	void				*clutptr;
	/**< pointer to memory containing CLUT data, this is not array,
	    all regions share the same CLUT.*/
	struct vps_grpxscparams		*scparams;
	/**< scalling parameters used by the GRPX, this is not array
	    pointer, this can be NULL if no change on scaling parameters */
};


/**
 *	struct vps_grpx_createparams
 *	\brief Graphics driver create parameter structure to be passed to the
 *	driver at the time of graphics driver create call.
 */
struct vps_grpxcreateparams {
	u32			memtype;
	/**< vpdma memory type. for valid values see #vps_vpdmamemorytype. */
	u32			drvmode;
	/**< graphics driver operation mode.
	for valid values see #vps_grpxdrivermode */
} ;

/**
 *	struct vps_grpx_createstatus
 *	\brief Grpx Driver Status for the display Grpx open call. This status
 *	will return the GRPX driver information back to caller.
 */
struct vps_grpxcreatestatus {
	int			retval;
	/**< Reture value of the create call.*/
	u32                    dispwidth;
	/**< Width of the display at the VENC in pixels to which the grpx driver
	 path is connected. */
	u32                    dispheight;
	/**< Height of the display at the VENC in linesto which the grpx driver
	 path is connected. */

	u32			minbufnum;
	/**< Minimum number of buffers to start GRPX display */
	u32			maxreq;
	/**< Maximum number of request per instance that caller can sumbit
		 without calling dequeue*/
	u32			maxregions;
	/**< Maximum region GRPX driver support for the display */
} ;

#endif
#endif
