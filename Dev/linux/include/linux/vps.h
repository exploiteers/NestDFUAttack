/*
 *
 *
 *  VPSS inteface header file for TI81XX VPSS
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


#ifndef __LINUX_VPS_H__
#define __LINUX_VPS_H__

#ifdef __KERNEL__
#include "vps_cfgcsc.h"


/** \brief Driver ID base for the display driver class. */
#define VPS_DISP_DRV_BASE               (0x00000000u)
/** \brief Driver ID base for the capture driver class. */
#define VPS_CAPT_DRV_BASE               (0x00000100u)
/** \brief Driver ID base for the M2M driver class. */
#define VPS_M2M_DRV_BASE                (0x00000200u)
/** \brief Driver ID base for the Blender based M2M driver class. */
#define VPS_BLEND_DRV_BASE              (0x00000300u)
/** \brief Driver ID base for video decoder driver class. */
#define VPS_VID_DEC_DRV_BASE            (0x00000400u)

/** \brief Driver ID base for video  encoder driver class. */
#define VPS_VID_ENC_DRV_BASE            (0x00000500u)

/** \brief Driver ID base for platform driver class. */
#define VPS_VID_SYSTEM_DRV_BASE         (0x00000600u)

/** \brief Driver ID base for the custom/user driver class. */
#define VPS_USER_DRV_BASE               (0x00001000u)


/** \brief Display controller driver ID used at the time of FVID2 create. */
#define FVID2_VPS_DCTRL_DRV             (VPS_DISP_DRV_BASE + 0x00000000u)
/** \brief Display driver ID used at the time of FVID2 create. */
#define FVID2_VPS_DISP_DRV              (VPS_DISP_DRV_BASE + 0x00000001u)
/** \brief Graphics display driver ID used at the time of FVID2 create. */
#define FVID2_VPS_DISP_GRPX_DRV         (VPS_DISP_DRV_BASE + 0x00000002u)

/** \brief VIP capture driver ID used at the time of FVID2 create. */
#define FVID2_VPS_CAPT_VIP_DRV          (VPS_CAPT_DRV_BASE + 0x00000000u)

/** \brief Noise Filter M2M driver ID used at the time of FVID2 create. */
#define FVID2_VPS_M2M_NSF_DRV           (VPS_M2M_DRV_BASE + 0x00000000u)
/** \brief Scalar Writeback M2M driver ID used at the time of FVID2 create. */
#define FVID2_VPS_M2M_SC_DRV            (VPS_M2M_DRV_BASE + 0x00000001u)
/** \brief DEI HQ/DEI M2M driver ID used at the time of FVID2 create. */
#define FVID2_VPS_M2M_DEI_DRV           (VPS_M2M_DRV_BASE + 0x00000002u)

/** \brief Blender based M2M driver ID used at the time of FVID2 create. */
#define FVID2_VPS_M2M_COMP_DRV         (VPS_BLEND_DRV_BASE + 0x00000000u)


/*
 *  IOCTLs Base address.
 */
/** \brief IOCTL base address for the IOCTLs common to display, capture and
 *  mem to mem drivers. */
#define VPS_COMMON_IOCTL_BASE            (FVID2_USER_BASE + 0x00000000u)
/** \brief IOCTL base address for the display driver IOCTLs. */
#define VPS_DISP_IOCTL_BASE              (FVID2_USER_BASE + 0x00010000u)
/** \brief IOCTL base address for the capture driver IOCTLs. */
#define VPS_CAPT_IOCTL_BASE              (FVID2_USER_BASE + 0x00020000u)
/** \brief IOCTL base address for the M2M driver IOCTLs. */
#define VPS_M2M_IOCTL_BASE               (FVID2_USER_BASE + 0x00030000u)
/** \brief IOCTL base address for the display controller driver IOCTLs. */
#define VPS_DCTRL_IOCTL_BASE             (FVID2_USER_BASE + 0x00040000u)
/** \brief IOCTL base address for the noise filter M2M driver IOCTLs. */
#define VPS_M2M_NSF_IOCTL_BASE           (FVID2_USER_BASE + 0x00050000u)
/** \brief IOCTL base address for the video decoder driver IOCTLs. */
#define VPS_VID_DEC_IOCTL_BASE           (FVID2_USER_BASE + 0x00060000u)
/** \brief IOCTL base address for the video  encoder driver IOCTLs. */
#define VPS_VID_ENC_IOCTL_BASE          (FVID2_USER_BASE + 0x00070000u)
/** \brief IOCTL base address for the video  platform driver IOCTLs. */
#define VPS_VID_SYSTEM_IOCTL_BASE       (FVID2_USER_BASE + 0x00080000u)

/**
 *  \brief IOCTL base address for the advanced debug capabilities.
 *  Caution: To be used with care.
 */
#define VPS_ADV_IOCTL_BASE              (FVID2_USER_BASE + 0x00070000u)

/**
 *  \brief VPS video buffer alignment. All application buffer address and
 *  line pitch should be aligned to this byte boundary.
 */
#define VPS_BUFFER_ALIGNMENT            (16u)

/** \brief Multi window layout ID base for the user created layouts. */
#define VPS_USER_LAYOUT_ID_BASE         (100u)


/*
 *  Common IOCTLS for display, M2M and capture driver
 */
/**
 *  \brief set the scalar coefficients.
 *
 *  \param cmdargs       [in]  const vps_sccoeffparams *
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
*/
#define IOCTL_VPS_SET_COEFFS           (VPS_COMMON_IOCTL_BASE + 0x0000u)

/**
 *  \brief Set the GRPX region parameters
 *
 *  This IOCTL just passes the GRPX region parameter into the driver to
 *  prepare the necessary memory for GRPX.
 *  This IOCTL could be called only after creating the driver and before the
 *  display or M2M operation is started.
 *
 *  \param cmdArgs       [IN]     Vps_GrpxParamsList *
 *  \param cmdStatusArgs [OUT]    NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_SET_GRPX_PARAMS       (VPS_COMMON_IOCTL_BASE + 0x0002u)
/**
 *  \brief Get the GRPX region parameters
 *
 *  This IOCTL just Return the region parameters back to caller.
 *  This IOCTL could be called after creating the driver and even if the
 *  display or M2M operation is in progress.
 *
 *  \param cmdArgs       [IN]     Vps_GrpxParamsList *
 *  \param cmdStatusArgs [OUT]    NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_GET_GRPX_PARAMS       (VPS_COMMON_IOCTL_BASE + 0x0003u)

/**
 *  \brief get dei context information.
 *
 *  \param cmdargs       [in]     vps_deictxinfo *
 *  \param cmdstatusargs [out]    null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define IOCTL_VPS_GET_DEI_CTX_INFO      (VPS_COMMON_IOCTL_BASE + 0x0004u)
/**
 *  \brief set dei context buffers to driver.
 *
 *  \param cmdargs       [in]     const vps_deictxbuf *
 *  \param cmdstatusargs [out]    null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define IOCTL_VPS_SET_DEI_CTX_BUF       (VPS_COMMON_IOCTL_BASE + 0x0005u)
/**
 *  \brief get dei context buffers from driver.
 *
 *  \param cmdargs       [in]     vps_deictxbuf *
 *  \param cmdstatusargs [out]    null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define IOCTL_VPS_GET_DEI_CTX_BUF       (VPS_COMMON_IOCTL_BASE + 0x0006u)

/**
 *  \brief Creates a mosaic or region based graphic layout depending on the
 *  multi window parameter.
 *
 *  this ioctl creates the necessary infrastructure for the specified layout.
 *  the user has to call select multiple window layout ioctl to explicitly
 *  select a particular layout before starting the display/m2m operation.
 *  this ioctl could be called after creating the driver and even if the
 *  display or m2m operation is in progress.
 *
 *  note: this also supports pip/overlapping windows, in which case
 *  application has to specify the priority for each of the window.
 *
 *  \param cmdArgs       [IN]  const Vps_MultiWinParams *
 *  \param cmdStatusArgs [OUT] Vps_CreateLayoutStatus *
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_CREATE_LAYOUT         (VPS_COMMON_IOCTL_BASE + 0x0007u)

/**
 *  \brief deletes a mosaic or region based graphic layout created by
 *  ioctl_vps_create_layout ioctl.
 *
 *  this ioctl could be called after creating the driver and even if the
 *  display or m2m operation is in progress. when the layout to delete
 *  is used by the current operation or is not created, this returns error.
 *
 *  \param cmdArgs       [IN]  const Vps_DeleteLayoutParams *
 *  \param cmdStatusArgs [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_DELETE_LAYOUT         (VPS_COMMON_IOCTL_BASE + 0x0008u)

/**
 *  \brief Selects the predefiend or user created layout for display or
 *  M2M operation.
 *
 *  This IOCTL should be called before starting the display or M2M operation
 *  to select the default layout to start with.
 *  This IOCTL could not be called once the display starts or request is
 *  queued with the M2M driver. For changing the layout after operation is
 *  started, application could do so by passing the layout ID as a part of
 *  runtime parameter if the specific driver supports this.
 *
 *  \param cmdArgs       [IN]  const Vps_SelectLayoutParams *
 *  \param cmdStatusArgs [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_SELECT_LAYOUT         (VPS_COMMON_IOCTL_BASE + 0x0009u)

/**
 *  \brief Deletes all the mosaic or region based graphic layouts created by
 *  IOCTL_VPS_CREATE_LAYOUT IOCTL.
 *
 *  This IOCTL could not be called when display is in progress with one of the
 *  created layout or when requests are pending with the M2M driver.
 *
 *  \param cmdArgs       [IN]  NULL
 *  \param cmdStatusArgs [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_DELETE_ALL_LAYOUT     (VPS_COMMON_IOCTL_BASE + 0x000Au)

/**
 *  \brief Read the scalar hardware configuration.
 *
 *  This ioctl can be used to read the actual hardware registers of the
 *  scalar.
 *  This is for the advanced user for the hardware debug capability.
 *  For the multiple channel mode of the driver data is returned from the
 *  overlay memory instead of actual hardware registers.
 *
 *  \param cmdArgs       [OUT] Vps_ScRdWrAdvCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_READ_ADV_SC_CFG       (VPS_ADV_IOCTL_BASE + 0x0000u)

/**
 *  \brief Write the scalar hardware configuration.
 *
 *  This ioctl can be used to write the actual hardware registers of the
 *  scalar.
 *  This is for the advanced user for the hardware debug capability.
 *  For the multiple channel mode of the driver data is written to the
 *  overlay memory instead of actual hardware registers.
 *
 *  \param cmdArgs       [IN]  const Vps_ScRdWrAdvCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_WRITE_ADV_SC_CFG      (VPS_ADV_IOCTL_BASE + 0x0001u)

/**
 *  \brief Read the High Quality De-Interlacer hardware configuration.
 *
 *  This ioctl can be used to read the actual hardware registers of the
 *  High Quality De-interlacer.
 *  This is for the advanced user for the hardware debug capability.
 *
 *  \param cmdArgs       [OUT] Vps_DeiHqRdWrAdvCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_READ_ADV_DEIHQ_CFG       (VPS_ADV_IOCTL_BASE + 0x0010u)

/**
 *  \brief Write the High Quality De-Interlacer hardware configuration.
 *
 *  This ioctl can be used to write the actual hardware registers of the
 *  High Quality De-interlacer.
 *  This is for the advanced user for the hardware debug capability.
 *
 *  \param cmdArgs       [IN]  const Vps_DeiHqRdWrAdvCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_WRITE_ADV_DEIHQ_CFG      (VPS_ADV_IOCTL_BASE + 0x0011u)

/**
 *  \brief Read the Medium Quality De-Interlacer hardware configuration.
 *
 *  This ioctl can be used to read the actual hardware registers of the
 *  Medium Quality De-interlacer.
 *  This is for the advanced user for the hardware debug capability.
 *
 *  \param cmdArgs       [OUT] Vps_DeiMqRdWrAdvCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_READ_ADV_DEI_CFG       (VPS_ADV_IOCTL_BASE + 0x0012u)

/**
 *  \brief Write the Medium Quality De-Interlacer hardware configuration.
 *
 *  This ioctl can be used to write the actual hardware registers of the
 *  Medium Quality De-interlacer.
 *  This is for the advanced user for the hardware debug capability.
 *
 *  \param cmdArgs       [IN]  const Vps_DeiMqRdWrAdvCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_WRITE_ADV_DEI_CFG      (VPS_ADV_IOCTL_BASE + 0x0013u)


/** \brief Floor a integer value. */
#define VpsUtils_floor(val, align)  (((val) / (align)) * (align))

/** \brief Align a integer value. */
#define VpsUtils_align(val, align) VpsUtils_floor(((val) + (align)-1), (align))

/** \brief 8-bit Tiler container pitch in bytes. */
#define VPSUTILS_TILER_CNT_8BIT_PITCH   (16u * 1024u)

/** \brief 16-bit Tiler container pitch in bytes. */
#define VPSUTILS_TILER_CNT_16BIT_PITCH  (32u * 1024u)

/** \brief 32-bit Tiler container pitch in bytes. */
#define VPSUTILS_TILER_CNT_32BIT_PITCH  (32u * 1024u)

/**
 *  enum vpdma_mem_type
 *  \brief Enum for buffer memory type.
 */
enum vps_vpdmamemorytype {
	VPS_VPDMA_MT_NONTILEDMEM = 0,
	/**< 1D non-tiled memory. */
	VPS_VPDMA_MT_TILEDMEM,
	/**< 2D tiled memory. */
	VPS_VPDMA_MT_MAX
	/**< Should be the last value of this enumeration.
	     Will be used by driver for validating the input parameters. */
};
/**
 *  enum vps_memrotatetype
 *  \brief This enum is used to define the memory data rotaion and mirroring
 *  type. The rotation and mirroing is only valid if the memory type is tiler
 *  container mode. For non-tiler and tiler page mode,  rotation and mirroring
 *  is not valid
 */
enum vps_memrotationtype {
	VPS_MEM_0_ROTATION = 0,
	/**< no rotation and mirroring. */
	VPS_MEM_180_ROTATION_MIRRORING,
	/**< 180 degree rotation with mirroring. */
	VPS_MEM_0_ROTATION_MIRRORING,
	/**< 0 degree rotation with mirroring. */
	VPS_MEM_180_ROTATION,
	/**< 180 degree rotation. */
	VPS_MEM_270_ROTATION_MIRRORING,
	/**< 270 degree rotation with mirroring. */
	VPS_MEM_270_ROTATION,
	/**< 270 degree rotation. */
	VPS_MEM_90_ROTATION,
	/**< 90 degree rotation. */
	VPS_MEM_90_ROTATION_MIRRORING,
	/**< 90 degree rotation with mirroring. */
	VPS_MEM_ROTATION_MAX
	/**< Should be the last value of this enumeration.
	     Will be used by driver for validating the input parameters. */
};


/**
 *  enum vps_fidpol
 *  \brief Enumerations for Field ID polarity.
 */
enum vps_fidpol {
	VPS_FIDPOL_NORMAL = 0,
	/**< FID = 0, top field */
	VPS_FIDPOL_INVERT
	/**< FID = 0, bottom field */
};

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * struct vps_frameparams
 * \brief Structure for setting the frame parameters like frame height, width,
 * pitch and memory type.
 */
struct vps_frameparams {
	u32                  width;
	/**< Width of frame in pixels. */
	u32                  height;
	/**< Height of frame in lines. */
	u32                  pitch[FVID2_MAX_PLANES];
	/**< Pitch for each planes in bytes. Only required planes needs to be
	     populated. */
	u32                  memtype;
	/**< VPDMA memory type - Tiled buffer or normal non-tiled buffer.
	for valid values see #vps_vpdmamemorytype. */
} ;

/**
 *  struct vps_cropconfig
 *  \brief structure containing crop configuration - used in scalar and vcomp.
 */
struct vps_cropconfig {
	u32                  cropstartx;
	/**< Horizontal offset from which picture needs to be cropped. */
	u32                  cropstarty;
	/**< Vertical offset from which picture needs to be cropped. */
	u32                  cropwidth;
	/**< Width of the picture to be cropped. */
	u32                  cropheight;
	/**< Height of the picture to be cropped. */
} ;

/**
 *  struct vps_posconfig
 *  \brief structure containing position configuration - used in vcomp and cig.
 */
struct vps_posconfig {
	u32                  startx;
	/**< horizontal offset from which picture needs to be positioned. */
	u32                  starty;
	/**< vertical offset from which picture needs to be positioned. */
};

/**
 *  struct vps_winformat
 *  \brief structure for setting the mosaic or region based graphic
 *  window for each of the window.
 */
struct vps_winformat {
	u32                  winstartx;
	/**< Horizontal offset in pixel (with respect to frame dimension)
	     from which window starts. */
	u32                  winstarty;
	/**< Vertical offset in line (with respect to frame dimension)
	     from which window starts. */
	u32                  winwidth;
	/**< Width of window in pixels. */
	u32                  winheight;
	/**< Number of lines in a window. For interlaced mode, this should
	     be set to the frame size and not the field size. */
	u32                  pitch[FVID2_MAX_PLANES];
	/**< Pitch in bytes for each of the sub-window buffers.
	     This represents the diifference between two consecutive line
	     address. This is irrespective of whether the video is interlaced
	     or progressive and whether the fields are merged or separated
	     for interlaced video. */
	u32                  dataformat;
	/**< data format for each window.
	     for valid values see #fvid2_dataformat.
	     caution: check the respective driver user/api guide for the
	     data formats supported. some driver may not support
	     separate data formats for each window of the frame. */
	u32                  bpp;
	/**< Bits per pixels for each window.
	     for valid values see #fvid2_bitsperpixel. */
	u32                  priority;
	/**< In case of overlapping windows (as in PIP), priority could be used
	     to choose the window to be displayed in the overlapped region.
	     0 is highest priority, 1 is next and so on...
	     Note that keeping same priority for all windows specifies that
	     there are no overlapping windows. */
};

/**
 *  struct vps_multiwinparams
 *  \brief Structure for setting multiple window (mosaic/region based graphics)
 *  parameters.
 */
struct vps_multiwinparams {
	u32                  channelnum;
	/**< Channel number to which this this config belongs to.
	     This is used in case of multiple buffers queuing/deqeuing using a
	     single call.
	     If only one channel is supported, then this should be set to 0. */
	u32                  numwindows;
	/**< Specifies the number windows that would require to be
	     displayed/processed, winFmt should point to a array that has at least
	     numWindows of entries. */
	const struct vps_winformat    *winfmt;
	/**< Pointer to an array of multiple window format containing the
	     format for each of the mosaic or region based graphics window.
	     The format array should be filled from left to right and from top
	     to bottom for all the windows.
	     The size of the array should be equal to total number of windows
	     and the memory should be allocated by the application. */
} ;

/**
 *  struct vps_createlayoutstatus
 *  \brief structure for getting the status information for create
 *  multiple window (mosaic/region based graphics) layout ioctl.
 */
struct vps_layoutid {
	u32                  layoutid;
	/**< layout id as assigned by driver. this should be used as a reference
	by the application to select the required layout.
	note: that each driver could have different layout id for a particular
	layout. */
};

/**
 *  struct Vps_subframeparams
 *  \brief Configuration for subframe level processing at create time.
 */
struct vps_subframeparams {
	u32              subframemodeenable;
	/**< TRUE : Slice level processing is done for this channel
	     FALSE: Frame level processing is done for this channel. */
	u32            numlinespersubframe;
	/**< Number of lines in a subframe. */
} ;

#endif

#endif
