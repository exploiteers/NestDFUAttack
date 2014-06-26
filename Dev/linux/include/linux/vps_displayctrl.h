/*
 *
 * Display Controller Header file for TI81XX VPSS
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

#ifndef __LINUX_VPS_DISPLAYCTRL_H__
#define __LINUX_VPS_DISPLAYCTRL_H__

#ifdef __KERNEL__
/*
 *  Display controller driver IOCTLs.
 */

/*
 *  Macros for display controller instance numbers to be passed as instance ID
 *  at the time of driver create.
 *  Note: These are read only macros. Don't modify the value of these macros.
 */
/** \brief Display controller instance 0. */
#define VPS_DCTRL_INST_0                    (0u)

/** \brief Command to set the entire VPS display path configuration in
 *  one shot.
 *
 *  All the VPS modules handled by the Display Controller can be
 *  represented by a graph, where node represents a module like blender,
 *  mux etc. and edge is present between two nodes if they are connected. All
 *  VPS paths can be configured by this IOCTL in one shot. Use macro
 *  defined in this file for input path, muxes, vcomp, cig input, cig
 *  output and blender as the node numbers.
 *
 *  This IOCTL takes either name of the pre-defined configuration or
 *  list of edges
 *  connecting nodes and configures display paths.
 *  It first validates these paths and then configures VPS for the display
 *  paths. It configures all the center modules, except blender. Blender will
 *  only be enabled when venc is configured with the given mode.
 *
 * \par this ioctl cannot be used for clearing configuration on a path
 *      streaming path.
 *
 * \param   cmdargs [in] pointer of type vps_dcconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_CONFIG              (VPS_DCTRL_IOCTL_BASE + 0x1u)

/** \brief Command to clear the VPS display path configuration in
 *  one shot.
 *
 *  this ioctl takes either name of the pre-defined configuration or
 *  list of edges
 *  connecting nodes and clears the configuration. it also stops vencs.
 *
 *  It does not validates the edge list. It simply disables the edge
 *  connecting nodes. For the vencs, it checks for the validity and then
 *  disables the venc if there are not errors.
 *  Use macro defined in this file for input path, muxes, vcomp, cig input, cig
 *  output and blender as the node numbers.
 *
 * \par this ioctl cannot be used for clearing configuration on a path
 *      streaming path.
 *
 * \param   cmdargs [in] pointer of type vps_dcconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_CLEAR_CONFIG            (VPS_DCTRL_IOCTL_BASE + 0x2u)

/** \brief Command to set output in the given Venc.
 *
 *  This IOCTL is used to set the output i.e. composite, s-video etc. on
 *  the given venc if venc is not on. For the tied vencs, venc has to
 *  be stopped first, then output can can be changed.
 *
 * \param   cmdargs [in] pointer of type vps_dcoutputinfo
 *
 * \return  vps_sok if successful, else suitable error code
 *
 */
#define IOCTL_VPS_DCTRL_SET_VENC_OUTPUT         (VPS_DCTRL_IOCTL_BASE + 0x3u)

/** \brief Command to get output in the given Venc.
 *
 * \param   cmdargs [in] pointer of type vps_dcoutputinfo
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VENC_OUTPUT         (VPS_DCTRL_IOCTL_BASE + 0x4u)

/** \brief maximum number of basic ioctl commands
 *
 *  marker used to denote the maximum number of basic ioctls supported
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_BASIC_MAX               (VPS_DCTRL_IOCTL_BASE + 0x9u)

/** \brief Maximum number of characters in the string for specifying
 * node name */
#define VPS_DC_MAX_OUTPUT_NAME            (20u)

/** \brief Maximum number of edges in the list of edges in Dc_config
 * structure */
#define VPS_DC_MAX_EDGES                (46u)

/* Following macros define bitmasks for the Vencs. Here, bitmasks are
   used for identifying Vencs so that tied vencs can be easily specified
   and configured. Two vencs, which uses same pixel clock and whose
   vsync are synchronized, can be tied together. SD Venc cannot
   be tied with any other venc since it supports only SD modes and HD
   vencs supports only HD Modes. Even if it cannot be tied, same
   mechanism of configuring VENC is used for the SD VENC. */

/** \brief Bitmask for HDMI VENC */
#define VPS_DC_VENC_HDMI                (0x1u)

/** \brief Bitmask for HDCOMP VENC */
#define VPS_DC_VENC_HDCOMP              (0x2u)

/** \brief Bitmask for DVO2 VENC */
#define VPS_DC_VENC_DVO2                (0x4u)
/** \brief Bitmask for SD VENC */
#define VPS_DC_VENC_SD                  (0x8u)

/** \brief Defines maximum number of venc info structure, which can be
  * passed in setconfig API
  **/
#define VPS_DC_MAX_VENC                 (4u)
/** \brief on-chip encoder identifier - rf */
#define VPS_DC_ENCODER_RF                       (0x1u)

/** \brief on-chip encoder identifier - max guard */
#define VPS_DC_MAX_ENCODER                      (0x2u)

/**
 *  enum vps_dcusecase
 *  \brief Enum for selecting VPS configuration for the specific use
 *  case or user defined use case.
 *  Display Controller supports few pre-defined configurations. Pre-defined
 *  configurations configures all vps modules, including vencs, handled by
 *  display controller. Once a
 *  pre-defined configuration is used, all other parameters will be ignored
 *  in the Vps_DcConfig structure and display controller will be
 *  configured as per the pre-defined configuration.
 */
enum vps_dcusecase {
	VPS_DC_TRIDISPLAY = 0,
	/**< TRIDISPLAY configuration: Pre-defined configuration in which
	     HDMI, HDVENCA and SDVENC are used to provide three outputs. Both
	     the HDVENCs are running 1080p mode at 60 fps and SDVENC is
	     running NTSC mode. Private
	     path0 is connected to HDMI output through VCOMP, Private path1
	     is connected to HDVENCA throug CIG and transcode path is connected
	     to SDVENC. */
	VPS_DC_DUALHDDISPLAY,
	/**< DUALDISPLAY configuration: Pre-defined configuration in which
	     HDMI and DVO2 are used to provide two HD outputs. Both
	     the HDVENCs are running 1080p mode at 60 fps. Private
	     path0 is connected to HDMI output through VCOMP and Private path1
	     is connected to HDVENCA throug CIG. */
	VPS_DC_DUALHDSDDISPLAY,
	/**< DUALDISPLAY configuration: Pre-defined configuration in which
	     HDMI and SDVENC are used to provide one HD and one SD output.
	     HDVENC is running 1080p mode at 60 fps and SD VENC is running
	     PAL mode. Private path0 is connected to HDMI output through VCOMP
	     and Private path1 is connected to SDVENC. */
	VPS_DC_USERSETTINGS,
	/**< User Defined paths for VPS */
	VPS_DC_NUM_USECASE
	/**< This must be the last Enum */
};

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief dvo format
 */
enum vps_dcdigitalfmt {
	VPS_DC_DVOFMT_SINGLECHAN = 0,
	/**< Ouput data format is single channel with embedded sync */
	VPS_DC_DVOFMT_DOUBLECHAN,
	/**< Output data format is dual channel with embedded sync */
	VPS_DC_DVOFMT_TRIPLECHAN_EMBSYNC,
	/**< Output data format is tripple channel with embedded sync */
	VPS_DC_DVOFMT_TRIPLECHAN_DISCSYNC,
	/**< Ouptut data format is triple channel with discrete sync */
	VPS_DC_DVOFMT_DOUBLECHAN_DISCSYNC,
	/**< Output data format is dual channel with discrete sync */
	VPS_DC_DVOFMT_MAX
	/**< This should be the last Enum */
};

/**
 * \brief analog format
 */
enum vps_dcanalogfmt {
	VPS_DC_A_OUTPUT_COMPOSITE = 0,
	/**< Analog output format composite */
	VPS_DC_A_OUTPUT_SVIDEO,
	/**< Analog output format svideo */
	VPS_DC_A_OUTPUT_COMPONENT,
	/**< Analog output format component */
	VPS_DC_A_OUTPUT_MAX
};

/**
 * \brief Signal polarity
 */
enum vps_dcsignalpolarity {
	VPS_DC_POLARITY_ACT_HIGH = 0,
	/**< Signal polarity Active high */
	VPS_DC_POLARITY_ACT_LOW = 1,
	/**< Signal polarity Active low */
	VPS_DC_POLARITY_MAX = 2,
	/**< Signal polarity Active low */
};

/**
 * \brief structure containing output information. this structure is used
 *  to set output in the output node. outputs are like composite,
 *  component etc.. this structure is used as an argument to
 *  ioctl_vps_dctrl_set_venc_output ioctl.
 */
struct vps_dcoutputinfo {
	u32			vencnodenum;
	/**< node number of the venc */
	u32			dvofmt;
	/**< digital output. See #vps_dcgigitalfmt for the possible Values */
	u32			afmt;
	/**< Analog output. See #Vps_dcanalogfmt for the possible Values */
	enum fvid2_dataformat  dataformat;
	/**< Output Data format from Venc. Currently, valid values are
	 FVID2_DF_RGB24_888, FVID2_DF_YUV444P, FVID2_DF_YUV422SP_UV */
	u32 dvofidpolarity;
	/**< Polarity for the field id signal for the digital output only
	 valid values see #vps_dcsignalpolarity */
	u32 dvovspolarity;
	/**< Polarity for the vertical sync signal for the digital output only
	 valid values see #vps_dcsignalpolarity */
	u32 dvohspolarity;
	/**< Polarity for the horizontal sync signal for the digital output only
	 valid values see #vps_dcsignalpolarity */
	u32 dvoactvidpolarity;
	/**< Polarity for the active video signal for the digital output only
	 valid values see #vps_dcsignalpolarity */

};

/**
 * \brief Structure containing edge information. Edge is a connection
 *	between two nodes i.e. two modules (like CIG and Blend) in VPS.
 *	VPS can be represented by a Directed Acyclic graph, where each
 *	module is node and edge is present between two nodes if they
 *	are connected.
 *	All VPS paths can be configured in one shot by API vps_dcsetConfig().
 *	This api takes the array of edges connected between nodes.
 *	This structure is used to specify individual edge information.
 */
struct vps_dcedgeinfo {
	u32			startnode;
	/**< starting node (vps module) of the edge */
	u32			endnode;
	/**< End node (VPS Module) of the edge */
} ;
/**
 *  struct Vps_DcTimingInfo
 *  \brief Structure for configuring the HDVenc timing parameters.
 */
struct vps_dctiminginfo {
	u32			hfrontporch;
	/**< Horizontal front porch. Same for both fields in case of interlaced
	display */
	u32			hbackporch;
	/**< Horizontal back porch */
	u32			hsynclen;
	/**< Horizontal sync length. Same for both fields in case of interlaced
	display */
	u32			vfrontporch[2];
	/**< Vertical front porch for each field or frame */
	u32			vbackporch[2];
	/**< Vertical back porch for each field or frame */
	u32			vsynclen[2];
	/**< Vertical sync length for each field */
	u32			width;
	/**< Active width for each frame. Same for both fields in case
	     of interlaced display */
	u32			height;
	/**< Active height of each field or frame */
	u32			scanformat;
	/**< Scan format */
	u32			mode;
	/**< mode */

};

/**
 * \brief Structure containing mode information.
 */
struct vps_dcmodeinfo {
	u32                            vencid;
	/**< Identifies the VENC on which mode parameters is to be applied.
	     Use one of VPS_DC_VENC_HDMI, VPS_DC_VENC_HDCOMP,
	     VPS_DC_VENC_DVO2, VPS_DC_VENC_SD macro for this variable. */

	struct	fvid2_modeinfo         minfo;
	/**< timing information.  if this is null this will be used to configure
	     the timings instead of the standard modeid. currently this is not
	     supported */
	u32                             mode;
	/**< VENC Mode */
	u32                             isvencrunning;
	/**< Flag to indicate whether VENC is running or not. This is
	     read only parameter returned from the display controller to
	     indicated whether given venc is running or not. */
	u32                             numinpath;
	/**< This is read only parameter returned from the display
	     controller to inform number of input paths connected to
	     the this vencs. */

};

/**
 * struct vps_dcvencinfo
 * \brief Structure containing venc information. This structure is used is
 *	vps_dcsetConfig API to configure mode in the Vencs. It also is used to
 *	inform which vencs are tied. Two vencs are tied when both are running
 *	on the same pixel clock speed and vsync signal for both the vencs are
 *	synchronized. If two vencs are tied, same mode is set in both the vencs
 *	and	they will be enabled at the same time in order to have both
 *	synchronized.
 */
struct vps_dcvencinfo {
	struct        vps_dcmodeinfo modeinfo[VPS_DC_MAX_VENC];
	/**< Mode Information to be set the Venc. */
	u32           tiedvencs;
	/**< bitmask of tied vencs. two vencs, which uses same pixel clock
	     and whose vsync are synchronized, can be tied together. */
	u32          numvencs;
	/**< Name of the mode to be set the Vencs */
};

/**
 * struct vps_dcconfig
 * \brief Structure contaning set of edges for creating complete VPS
 *	connection mesh and VENC information. This structure is used in
 *	vps_dcsetConfig API to configure Complete VPS mesh statically in
 *	one shot.
 *	It has set of edges, which describes how individual modules are
 *	connected to each other and finally to the VENC. It also configures
 *	the mode in the	VENCs and tells which vencs are tied.
 */
struct vps_dcconfig {
	u32                          usecase;
	/**< Indicates which use case is to be configured for. Media
	     Controller provides set of standard configuration
	     for some standard use cases. Application can
	     directly If it is standard use case, there is no need
	     to specify next arguments. Application can also specify
	     user defined path configuration by specifying
	     VPS_DC_USERSETTINGS in this argument and providing list of
	     edges */
	struct vps_dcedgeinfo        edgeinfo[VPS_DC_MAX_EDGES];
	/**< List of edges connecting vps modules. Display controller parse
	     these edges and enables/disables input/output path in the
	     appropriate VPS module. This edge tells which module is
	     connected to which module enabling output in edge start module
	     and input in edge end module. */
	u32                         numedges;
	/**< Number edge in the edgeInfo array */
	struct vps_dcvencinfo       vencinfo;
	/**< Structure containing Venc Information like mode to be configured
	     and which are tied. */
};

/**
 *	struct vps_dcyuvcolor
 * \brief Structure containing color information in YCrCb could be used for
 *	background and alternate color.
 */
struct vps_dcyuvcolor {
	u16 yluma;
	/**< Luma/Y Value */
	u16 crchroma;
	/**< Chroma/Cr Value */
	u16 cbchroma;
	/**< Chroma/Cb Value */
};

/**
 *	struct vps_dcrgbcolor
 * \brief Structure containing color information in YCrCb could be used for
 *	background and alternate color.
 */
struct vps_dcrgbcolor {
	u16 r;
	/**< Red Value */
	u16 g;
	/**< Green Value */
	u16 b;
	/**< Blue Value */
};

/* ======================================================================== */
/* ======================================================================== */
/*                             RunTime Configuration                          */
/* ======================================================================== */
/* ======================================================================== */

/* ======================================================================== */
/*                           Macros & Typedefs                                */
/* ======================================================================== */


/** \brief start of runtime control
 *
 *  marker used to denote the begining of runtime control ioctls
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_RT_BASE             (VPS_DCTRL_IOCTL_BASIC_MAX + 0x1u)

/** \brief Command for setting Vcomp runtime configuration.
 *
 *  This IOCTL is used to set the runtime configuration in VCOMP. This
 *  configuration includes setting the priority of the input
 *  video window. This can be even changed when streaming is on on
 *  both the video windows.
 *
 *  This IOCTL takes pointer to the structure Vps_DcVcompRtConfig as
 *  an argument and return 0 on success or negative error code on failure. */
#define IOCTL_VPS_DCTRL_SET_VCOMP_RTCONFIG  (VPS_DCTRL_IOCTL_RT_BASE + 0x1u)

/** \brief command for getting vcomp runtime configuration.
 *
 *  this ioctl is used to get the runtime configuration from vcomp. this
 *  configuration includes the priority of the input
 *  video window.
 *
 * \param   cmdargs [in] pointer of type vps_dcvcomprtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VCOMP_RTCONFIG  (VPS_DCTRL_IOCTL_RT_BASE + 0x2u)

/** \brief command for setting comp runtime configuration.
 *
 *  this ioctl is used to set the runtime configuration in comp. this
 *  configuration includes setting the priority of the input video
 *  window and graphics windows. it also specifies whether to use
 *  global or pixel base alpha blending. this can be even changed
 *  when streaming is on. specify nodeid of the comp in the
 *  vps_dccomprtconfig structure to set configuration in specific comp.
 *
 * \param   cmdargs [in] pointer of type vps_dccomprtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_COMP_RTCONFIG   (VPS_DCTRL_IOCTL_RT_BASE + 0x3u)

/** \brief command for getting comp runtime configuration.
 *
 *  this ioctl is used to get the runtime configuration from comp.
 *  specify nodeid of the comp in the vps_dccomprtconfig structure to
 *  get configuration in specific comp.
 *
 * \param   cmdargs [in] pointer of type vps_dccomprtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_COMP_RTCONFIG   (VPS_DCTRL_IOCTL_RT_BASE + 0x4u)

/** \brief command for setting cig runtime configuration.
 *
 *  this ioctl is used to set the runtime configuration in cig. this
 *  configuration includes alpha value
 *  for the blending and transparency color value for all three output video
 *  windwos. specify nodeid of the cig in the
 *  vps_dccigrtconfig structure to set configuration in specific cig output.
 *
 * \param   cmdargs [in] pointer of type vps_dccigrtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_CIG_RTCONFIG    (VPS_DCTRL_IOCTL_RT_BASE + 0x5u)

/** \brief command for getting cig runtime configuration.
 *
 *  this ioctl is used to get the runtime configuration in cig. specify
 *  nodeid of the cig in the vps_dccigrtconfig structure to set
 *  configuration in specific cig output.
 *
 * \param   cmdargs [in] pointer of type vps_dccigrtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_CIG_RTCONFIG    (VPS_DCTRL_IOCTL_RT_BASE + 0x6u)

/** \brief maximum number of runtime ioctl commands
 *
 *  marker used to denote the maximum number of runtime ioctls supported
 *
 *  \par caution ensure that runtime ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_RT_MAX              (VPS_DCTRL_IOCTL_RT_BASE + 0x9u)


/**
 *  enum vps_dccigtransmask
 *	\brief Enumerations for transparency color mask bit (Number of LSB
 *       bits to mask when checking for pixel transparency).
 */
enum vps_dccigtransmask {
	VPS_DC_CIG_TM_NO_MASK = 0,
	/**< Disable Masking */
	VPS_DC_CIG_TM_MASK_1_LSB,
	/**< Mask 1 LSB before checking */
	VPS_DC_CIG_TM_MASK_2_LSB,
	/**< Mask 2 LSB before checking */
	VPS_DC_CIG_TM_MASK_3_LSB
	/**< Mask 3 LSB before checking */
};

/**
 * \brief enum vps_dccompdisplayorder_t
 * Enum for display order selection. PLease note alpha blending is done from
 * bottom to top layer
 */
enum vps_dccompdisplayorder {
	VPS_DC_COMP_DISPLAY_VID_ORDER = 0,
	/* Video layer display order.
	From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_G0_ORDER,
	/* Graphic0 layer display order when g_reorder = 1.
	 From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_G1_ORDER,
	/* Graphic1 layer display order when g_reorder = 1.
	From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_G2_ORDER,
	/* Graphic2 layer display order when g_reorder = 1.
	From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_ORDER_MAX
	/* Defined to find out maximum*/
};

/**
 * \brief enum vps_dccompfeedbkpathselect_t
 *  Enum for selection of Feedback data. The feedback video data can
 *  be selected from video alpha blending or final alpha blending. This
 *  enum is used for selecting source of the feedback path.
 */
enum vps_dccompfeedbkpathselect {
	VPS_DC_COMP_OUT_VIDEO_ALPHA_BLENDING = 0,
	/* select data from video alpha blending */
	VPS_DC_COMP_OUT_FINAL_ALPHA_BLENDING
	/* select data from final alpha blending */
};

/* ======================================================================== */
/*                         Structure Declarations                             */
/* ======================================================================== */

/**
 * struct vps_dcvcomprtconfig
 * \brief Structure containing runtime configurable parameters for VCOMP
 *	from the display controller. This just includes which of the input video
 *  window is on the top when compositing in the VCOMP. This structure is
 *  passed as an argument to the IOCTL IOCTL_VPS_DCTRL_SET_VCOMP_RTCONFIG.
 */
struct vps_dcvcomprtconfig {
	u32		isprimaryvideoontop;
	/**< DEI_HQ input video path is considered as the privide video path.
	     This flag indicates whether primary video window is on top or not.
	     If it is false, Aux video window will be on the top. */
};

/**
 * struct vps_dccigrtconfig
 * \brief Structure containing runtime configurable parameters for CIG
 *	from the display controller. This includes Blending and transparency
 *  for the output video windows. This structure is passed as an
 *  argument to IOCTL IOCTL_VPS_DCTRL_SET_CIG_RTCONFIG.
 */
struct vps_dccigrtconfig {
	u32				nodeid;
	/**< Id of the node. Use one of VPS_DC_CIG_CONSTRAINED_OUTPUT,
	     VPS_DC_CIG_NON_CONSTRAINED_OUTPUT, VPS_DC_CIG_PIP_OUTPUT as
	     node id for configuring CIG runtime configuration. */
	u32				transparency;
	/**< Enable Transparency */
	u32				mask;
	/**< Transparency color mask bit enum type*/
	u32				alphablending;
	/**< Enable alpha blending */
	u8				alphavalue;
	/**< Alpha blending value */
	struct vps_dcrgbcolor		transcolor;
	/**< Transparency color in RGB */
};

/**
 * struct vps_dccomprtconfig
 * \brief Structure containing runtime configurable parameters for COMP
 *	from the display controller. These parameters includes priority of the
 *	input layers and whether to use global reordering or not. For each of
 *	the comp this runtime configurable parameters can be set separately. For
 *	setting runtime parameters for all blenders, this ioctl should be called
 *      multiple times. This structure is passed as an
 *      argument to IOCTL IOCTL_VPS_DCTRL_SET_COMP_RTCONFIG
 */
struct vps_dccomprtconfig {
	u32                       nodeid;
	/**< id of the node. use one of vps_dc_hdmi_blend,
	vps_dc_hdcomp_blend, vps_dc_dvo2_blend,
	vps_dc_sdvenc_blend as node id for runtime configuration of comp */
	u32                       ispipbasevideo;
	/**< CIG PIP output goes to all threee HD Blenders. This flag indicates
	     whether this pip video is base video or not. */
	u32                       fbpath;
	/**< Selects Feedback data path. These selects the source of the
	     feedback path. Feedback path from the blender is
	     supported only for the DVO1 and DVO2 output in the hardware. There
	     is not feedback path from the other blenders, so these parameter
	     will not be used for these blender */
	u32                      isglobalreorderenable;
	/**< Enables/Disables global reordering. If global reordering is
	     enabled, Display order/priority for the input paths are specified
	     in the displayOrder member of this structure. Otherwise, only
	     display order of the video window is used from the displayOrder
	     array. Graphics will bring priority per pixel. */
	u32                     displayorder[VPS_DC_COMP_DISPLAY_ORDER_MAX];
	/**< decides order of priority between three graphic and video
	     if isGlobalReorderEnable is set TRUE. It is array of size 4 and
	     user has to configure priority.  In the case,if gReorderMode is
	     set 0, user has to provide priority for vidoe layer only and for
	     graphic layer, it will be taken from settings or bits [35:32]
	     in the data bus.*/
};
/* ========================================================================== */
/* ========================================================================== */
/*                             Advanced Configuration                         */
/* ========================================================================== */
/* ========================================================================== */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief advanced configurations
 *
 *  marker used to denote the begining of ioctls that would be required for
 *  for advanced control
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_ADV_BASE    (VPS_DCTRL_IOCTL_RT_MAX + 0x1u)

/** \brief Command to enumerate VPS nodes and its information for the
 *  node available at the given index.
 *
 *  This IOCTL is used for enumerating and then dynamically connecting different
 *  VPS modules. Enumeration starts from the node number 0 and
 *  continues untill it returns negative error code. On each index, this IOCTL
 *  returns node and its information like type of node number, name, number of
 *  inputs and number output available at given index. Only nodes, in
 *  which inputs can be enabled/disabled, can be enumerated with this
 *  ioctl. It returns node number from macros defined for input path,
 *  muxes, vcomp, cig input, cig output and blender.
 *
 * \param   cmdargs [in/out] pointer of type vps_dcenumnode, which would be
 *                           updated with input node information. provided that
 *                           node index and input are valid.
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_ENUM_NODES          (VPS_DCTRL_IOCTL_ADV_BASE + 0x1u)

/** \brief Command to enumerate nodes connected as a inputs to the given node.
 *  It enumerates inputs nodes of this node.
 *
 *  To enumerate all inputs, applications shall begin with input
 *  index zero, get the information,
 *  increment by one until the driver returns -1.
 *
 * \param   cmdargs [in/out] pointer of type vps_dcenumnodeinput, which would be
 *                           updated with input node information. provided that
 *                           node index and input are valid.
 *
 * \return  vps_sok if successful, else suitable error code
 *
 */
#define IOCTL_VPS_DCTRL_ENUM_NODE_INPUTS    (VPS_DCTRL_IOCTL_ADV_BASE + 0x2u)

/** \brief command to enable/disable the given input on the given node
 *
 *  this ioctl enables or disables input at the given index
 *  on the given node. it enables/disables given input as inputs
 *  node to this node and enables/disables given node
 *  as output node to the parent node i.e. it enables/disables edge connecting
 *  given node and input node. for example,
 *  to enable/disable cig pip output on hdcomp blender, use nodeid as
 *  vps_dc_hdcomp_blend and vps_dc_cig_pip_output as inputnodeid. this
 *  will enable/disable cig pip output in the cig and cig pip input in the
 *  hdcomp blender.
 *
 * \par caution this ioctl can not be used to enable node input
 *       if streaming on that input.
 *
 * \param   cmdargs [in] pointer of type vps_dcnodeinput
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_NODE_INPUT          (VPS_DCTRL_IOCTL_ADV_BASE + 0x3u)

/** \brief command to get the status of the given input on the given node.
 *
 *  this command is used to get the status i.e. whether it is
 *  enabled or not, of the input on the given node.
 *
 *  for example, to get the status of cig pip on hdcomp blender, use nodeid as
 *  vps_dc_hdcomp_blend and vps_dc_cig_pip_output as inputnodeid. it
 *  will return the status of this input on the blender node.
 *
 * \param   cmdargs [in] pointer of type vps_dcnodeinput
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_NODE_INPUT_STATUS  (VPS_DCTRL_IOCTL_ADV_BASE + 0x4u)

/** \brief Command to set mode/standard in the given Venc.
 *
 *  This IOCTL is used to set the mode/standard in the given vencs
 *  if its inputs are not running. If the multiple venc is tied, then
 *  it sets mode in all the vencs if their inputs are not running.
 *  Otherwise it returns error. The IOCTL first stops venc, changes
 *  mode and restarts venc. Note that this IOCTL can break tying of
 *  vencs if used incorrectly.
 *
 * \param   cmdargs [in] pointer of type vps_dcvencinfo
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_VENC_MODE   (VPS_DCTRL_IOCTL_ADV_BASE + 0x5u)

/** \brief command to get current mode/standard set in the given venc.
 *
 * \param   cmdargs [in] pointer of type vps_dcvencinfo
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VENC_MODE   (VPS_DCTRL_IOCTL_ADV_BASE + 0x6u)

/** \brief command to stop streaming on the venc.
 *
 *  it stops streaming on the given venc or set of vencs.
 *  application should pass the bit mask of all the vencs, which
 *  needs to be stopped.
 *
 * \param   cmdargs [in] pointer of type u32, that contains bitmask of all
 *          the vencs to be disabled.
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_DISABLE_VENC    (VPS_DCTRL_IOCTL_ADV_BASE + 0x7u)


/** \brief Command for Setting Venc Clock Source
 *
 * \param   cmdArgs [IN] Pointer of type vps_dcvencclksrc
 *
 * \return  VPS_SOK if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_VENC_CLK_SRC (VPS_DCTRL_IOCTL_ADV_BASE + 0x8u)


/** \brief Command for Getting Venc Clock Source
 *
 * \param   cmdArgs [IN/OUT] Pointer of type Vps_DcVencClkSrc
 *
 * \return  VPS_SOK if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VENC_CLK_SRC (VPS_DCTRL_IOCTL_ADV_BASE + 0x9u)



/** \brief maximum number of advanced ioctl commands
 *
 *  marker used to denote the maximum number of advance ioctls supported
 *
 *  \par caution ensure that advanced ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_ADV_MAX         (VPS_DCTRL_IOCTL_ADV_BASE + 0x20u)

/* Macros, Which can be used in the setconfig API to connect different
 * modules */

/** \brief macro defining main input path */
#define VPS_DC_MAIN_INPUT_PATH              (0u)
/** \brief macro defining auxiliary input path */
#define VPS_DC_AUX_INPUT_PATH               (4u)
/** \brief macro defining bypass 0 input path */
#define VPS_DC_BP0_INPUT_PATH               (5u)
/** \brief macro defining bypass 1 input path */
#define VPS_DC_BP1_INPUT_PATH               (6u)
/** \brief macro defining transcode/secondary 2 input path */
#define VPS_DC_SEC1_INPUT_PATH              (10u)
/** \brief macro defining graphics 0 input path */
#define VPS_DC_GRPX0_INPUT_PATH             (19u)
/** \brief macro defining graphics 1 input path */
#define VPS_DC_GRPX1_INPUT_PATH             (20u)
/** \brief macro defining graphics 2 input path */
#define VPS_DC_GRPX2_INPUT_PATH             (21u)

/** \brief macro defining vcomp multiplexer */
#define VPS_DC_VCOMP_MUX                    (1u)
/** \brief macro defining hdcomp multiplexer */
#define VPS_DC_HDCOMP_MUX                   (2u)
/** \brief macro defining sdvenc multiplexer */
#define VPS_DC_SDVENC_MUX                   (3u)


/* Macros for the other VPS modules */

/** \brief Macro defining VCOMP */
#define VPS_DC_VCOMP                          (14u)

/** \brief Macro defining CIG PIP input. In CIG this is the only path
  * which can be disabled. Main input path cannot be disabled. */
#define VPS_DC_CIG_PIP_INPUT                 (16u)

/** \brief macros defining cig constrained output. cig is used for
  * constraning input video. there are three video outputs
  *  from cig, i.e. non-constrained video output (hdmi), which
  *  provides original video stream, constrained
  *  video output (hdcomp), which provides video with the reduced quality
  *  and pip video output, which can be used for pip. */
#define VPS_DC_CIG_CONSTRAINED_OUTPUT          (15u)

/** \brief macros defining cig non-contrained output. */
#define VPS_DC_CIG_NON_CONSTRAINED_OUTPUT      (17u)

/** \brief Macro defining CIG PIP output. This output cannot also be used
  * as pip in the display. Input to the pip is aux path, which can be
  * smaller than the actual display size. In this case, CIG fills rest of the
  * display area with the zero. */
#define VPS_DC_CIG_PIP_OUTPUT                 (18u)


/* macros for the vps blenders */
/** \brief macro for the hdmi blender */
#define VPS_DC_HDMI_BLEND                   (22u)
#ifdef CONFIG_ARCH_TI816X
/** \brief macro for the hdcomp blender */
#define VPS_DC_HDCOMP_BLEND                 (23u)

/** \brief macro for the dvo2 blender */
#define VPS_DC_DVO2_BLEND                   (24u)

/** \brief macro for the sdvenc blender */
#define VPS_DC_SDVENC_BLEND                 (25u)
#else
/** \brief macro for the dvo2 blender */
#define VPS_DC_DVO2_BLEND                   (23u)

/** \brief macro for the sdvenc blender */
#define VPS_DC_SDVENC_BLEND                 (24u)

/** \brief macro for the hdcomp blender */
#define VPS_DC_HDCOMP_BLEND                 (VPS_DC_MAX_EDGES)

#endif

/** \brief Maximum number of characters in the string for specifying
 *  node name */
#define VPS_DC_MAX_NODE_NAME                (20u)


/**
 *  enum vps_dcnodetype
 *  \brief Enum defining node types available in VPS. There are several
 *  modules available in VPS, which controls how and which display goes
 *  to perticulcar VENC. These path controller modules are Multiplexers,
 *  VCOMP, CIG and Blenders. There modules are known as nodes in the display
 *  controller. This enum defines the type of the node.
 */
enum vps_dcnodetype {
	VPS_DC_NODETYPE_MUX = 0,
	/**< Multiplexer or Switch, takes N selectable inputs and
	     provides one output */
	VPS_DC_NODETYPE_SPLITTER,
	/**< Splitter, takes one input and provides M identical outputs */
	VPS_DC_NODETYPE_COMP,
	/**< Compositor, takes N inputs and provides one composited output */
	VPS_DC_NODETYPE_INPUT,
	/**< Input Node, no node connected as input to this node */
	VPS_DC_NODETYPE_OUTPUT,
	/**< Output Node, no node connected as output from this node*/
	VPS_DC_NODETYPE_MAX
	/**< This must be last enum */
};

enum vps_dcvencclksrcsel {
	VPS_DC_CLKSRC_VENCD = 0,
	/**< Clk1X input and output clock from VENC are same as VENCD Clock */
	VPS_DC_CLKSRC_VENCD_DIV2,
	/**< Clk1X input clock and venc output clock are
	     sourced from VEND/2 clock */
	VPS_DC_CLKSRC_VENCD_DIV2_DIFF,
	/**< Clk1X input is sourced from VENCD/2 clock and VENC output clock
	 is from VENCD DIV2 Clock */
	VPS_DC_CLKSRC_VENCA,
	/**< Clk1X input and output clock from VENC are same as VENCA Clock */
	VPS_DC_CLKSRC_VENCA_DIV2,
	/**< Clk1X input clock and venc output clock are
	    sourced from VENA/2 clock */
	VPS_DC_CLKSRC_VENCA_DIV2_DIFF
	/**< Clk1X input is sourced from VENCA/2 clock and VENC output clock
	 is from VENCA DIV2 Clock */
};

/* ======================================================================== */
/*                         Structure Declarations                             */
/* ======================================================================== */


/**
 *	struct vps_dcenumnode
 * \brief Structure containing the properties of a the processing node. Node
 *	represents a VPS module which controls how and which display goes
 *	to perticulcar VENC. This structure is for enumerating this processing
 *	modules one by one and get the properties of it.
 */
struct vps_dcenumnode {
	u32                 nodeidx;
	/**< Index of the node. Node Index starting from 0 to maximum
	     number of nodes. Used at the time of enumerating nodes.
	     After max number of nodes, this function returns zero. */
	u32                 nodeid;
	/* ID of the node*/
	u32                 nodetype;
	/**< Type of the node, This type indicates whether this node can
	accept multiple input or output or not
	for valid values see #vps_dcnodetype*/
	char                nodename[VPS_DC_MAX_NODE_NAME];
	/**< Name of the processing node */
	u32                 numinputs;
	/**< Number of all possible inputs for this node */
	u32                 numoutputs;
	/**< Number of all possible outputs for this node */
};
/**
 *  struct vps_dcenumnodeinput
 * \brief Structure containing the properties of a input of a
 *  the processing node. This structure is for enumerating input of
 *  modules one by one and get the properties of it.
 */
struct vps_dcenumnodeinput {
	u32                 nodeid;
	/**< Id of the node. This is the ID of the node for which input
	     is to be enumerated. */
	u32                 inputidx;
	/**< Index of the input. Input Index starting from 0 to maximum
	     number of inputs of this node. Used at the time of enumerating
	     inputs of the given node. After max number of inputs,
	     this function returns errro. */
	u32                 inputid;
	/**< Input ID of the given node */
	char                inputname[VPS_DC_MAX_NODE_NAME];
	/**< Name of the input */

};

/**
 *  struct Vps_DcEnableNodeInput
 * \brief This structure will be used at the time of enabling/disabling
 *  input of the given node. It takes id of the node and id of the
 *  input for this node.
 */
struct vps_dcnodeinput {
	u32          nodeid;
	/**< Id of the node. Use macros defined above for this id. */
	u32          inputid;
	/**< Input id of the node to be enabled. Use macros defined
	     above for this id. */
	u32          isenable;
	/**< flag to indicate whether given input on the given node
	is enabled or not. 0: disabled, 1: enabled */
};

/**
 *	struct vps_dcvcompconfig
 * \brief structure containing vcomp static configuration
 */
struct vps_dcvcompconfig {
	struct vps_dcyuvcolor        bckgrndcolor;
	/**< background colcor to be displayed in YCrCb */
	struct vps_dcyuvcolor        mainaltcolor;
	/**< alternate colcor to be displayed in YCrCb. When the main source is
	     enabled, send the alternate main Y/Cb/Cr values instead of the
	     true source picture.  This bit allows the datapath to flush
	     thout outputing the actual picture. */
	struct vps_dcyuvcolor        auxaltcolor;
	/**< alternate colcor to be displayed in YCrCb. When the aux source is
	     enabled, send the alternate main Y/Cb/Cr values instead of the
	     true source picture.  This bit allows the datapath to flush
	     through without outputing the actual picture. */
	/* Precedence of the Video */
};



/**
 * struct vps_dcedeconfig
 * structure containing EDE static configuration
 */
struct vps_dcedeconfig {
	u32              ltienable;
	/**< enables/disables luminance transition improvement block */
	u32              horzpeaking;
	/**< enables/disables horizontal peaking */
	u32              ctienable;
	/**< enables/disables chrominance transition improvement block */
	u32              transadjustenable;
	/**< enables/disables transition adjustment for chroma block */
	u32              lumapeaking;
	/**< enables/disables luminance peaking */
	u32              chromapeaking;
	/**< Enables/Disables Chrominance Peaking */
	u16              minclipluma;
	/**< Minimum value of the C clipping in the clipping block */
	u16              maxclipluma;
	/**< Maximum value of the Y clipping in the clipping block */
	u16              minclipchroma;
	/**< Minimum value of the C clipping in the clipping block */
	u16              maxclipchroma;
	/**< Maximum value of the C clipping in the clipping block */
	u32              bypass;
	/**< Bypass complete EDE processing */
};

/**
 * struct vps_dccigmainconfig
 * \brief Structure containing CIG static configuration
 */
struct vps_dccigmainconfig {
	u32                 enablecontraining;
	/**< This enables contraining in the CIG module. Once enabled, it
	     provides controained video on the CIT video output. */
	u32                 nonctrinterlace;
	/**< Enables/Disables interlacing on the non-contraining video output
	     of the CIG. Enabling this will require input video of type
	     progressive and CIG will interlace it and provide interlaced
	     video to the blender */
	u32                ctrinterlace;
	/**< Enables/Disables interlacing on the contraining video output
	     of the CIG. Enabling this will require input video of type
	     progressive and CIG will interlace it and provide interlaced
	     video to the blender */
};

/**
 * struct Vps_DcCigPipConfig
 * \brief Structure containing static configuration for CIG PIP Path
 */
struct vps_dccigpipconfig {
	u32                 pipinterlace;
	/**< Enables/Disables interlacing on the pip video output
	     of the CIG. Enabling this will require input video of type
	     progressive and CIG will interlace it and provide interlaced video
	     to the blender */
};

/**
 * struct
 * \brief Structure containing COMP static configuration
 */
struct vps_dccompconfig {
	struct vps_dcrgbcolor     bckgrndcolor;
	/**< Background Color in RGB format.his backround color is common for
	     all the compositors/blenders(like HDCOMP, DVO2, SD).
	     This color will replace any pixel with RGB value of 000.*/
};

/**
 * struct
 * \brief Structure containing VENC clock source configuration
 */
struct vps_dcvencclksrc {
	u32              venc;
	/**< Venc Id. VPS_DC_VENC_HDMI, VPS_DC_VENC_DVO2 or
	     VPS_DC_VENC_HDCOMP */
	u32              clksrc;
	/**< Clock source for the given venc. HDMI can be sourced only from
	     VEND clock wheread other two vencs, HDCOMP and DVO2, can be
	     sourced either from VENCD clock or VENCA clock.
	     See #Vps_DcVencClkSrcSel for possible values */
};


/**
 * struct vps_dccreateconfig
 * \brief Structure for static configuration
 */
struct vps_dccreateconfig {
	struct vps_dcvcompconfig       *vcompconfig;
	/**< Vcomp Configuration */
	struct vps_dcedeconfig         *edeconfig;
	/**< Ede Configuration */
	struct  vps_dccigmainconfig    *cigmainconfig;
	/**< Cig Configuration for the Main Path */
	struct vps_dccigpipconfig      *cigpipconfig;
	/**< Cig Configuration for the PIP Path */
	/* Dc_CprocConfig cprocConfig; */
	/**< Cproc Configuration */
	struct vps_dccompconfig        *compconfig;
	/**< Comp Configuration */
	struct vps_cscconfig           *sdcscconfig;
	/**< CSC Configuraton for the CSC on SD path */
	struct vps_cscconfig           *hdcompcscconfig;
	/**< CSC Configuraton for the CSC on HDComp */
};

/* ========================================================================== */
/* ========================================================================== */
/*                      part-4 on-chip encoder configuration                  */
/* ========================================================================== */
/* ========================================================================== */

/* ========================================================================== */
/*                           control command                                  */
/* ========================================================================== */

/**
  \addtogroup vps_drv_fvid2_ioctl_display_ctrl
*/

/** \brief on-chip encoder control base
 *
 *  marker used to denote the begining of ioctls that would be required to
 *  configure/control on-chip encoders
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE (VPS_DCTRL_IOCTL_ADV_MAX + 0x1u)


/**
 * \brief This control command retrieves the current (basic) RF configuration.
 *
 *         When any of the configurable parameters of the RF Modulator is
 *         required to be modified, it is expected to retrieve the current
 *         configuration, modify the required parameter(s) and apply the
 *         configuration via IOCTL_VPS_DCTRL_RF_SET_CONFIG.
 *
 * \par CAUTION By default, the configurable parameters may be turned OFF.
 *      Caller is expected to turn ON the required parameter(s) and
 *      update the config with IOCTL_VPS_DCTRL_RF_SET_CONFIG command.
 *
 * \param   cmdArgs [IN] Pointer of type Vps_DcOnchipEncoderCmd, which
 *                       initializes following members
 *                       .vencId    = Paired VENC ,VPS_DC_VENC_SD in this case.
 *                       .encoderId = VPS_DC_ENCODER_RF
 *                       .cmd       = This macro
 *                       .argument  = Pointer to structure of type
 *                                     #Vps_RfConfig
 *                       .additionalArgs = NULL
 *
 * \return  VPS_SOK if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_RF_GET_CONFIG                                        \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x1u)

/**
 * \brief This control command applies the supplied (basic) RF configuration.
 *
 *        Provided the values supplied are valid and encoder is not enabled.
 *
 * \param   cmdArgs [IN] Pointer of type Vps_DcOnchipEncoderCmd, which
 *                       initializes following members
 *                       .vencId    = Paired VENC ,VPS_DC_VENC_SD in this case.
 *                       .encoderId = VPS_DC_ENCODER_RF
 *                       .cmd       = This macro
 *                       .argument  = Pointer to structure of type
 *                                     #Vps_RfConfig
 *                       .additionalArgs = NULL
 *
 * \return  VPS_SOK if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_RF_SET_CONFIG                                        \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x2u)

/**
 * \brief This control command retrieves the current expert RF configuration.
 *
 *         When any of the configurable parameters of the RF Modulator is
 *         required to be modified, it is expected to retrieve the current
 *         configuration, modify the required parameter(s) and apply the
 *         configuration via IOCTL_VPS_DCTRL_RF_SET_EXPERT_CONFIG.
 *
 * \par CAUTION By default, the configurable parameters may be turned OFF.
 *      Caller is expected to turn ON the required parameter(s) and
 *      update the config with IOCTL_VPS_DCTRL_RF_SET_EXPERT_CONFIG command.
 *
 * \param   cmdArgs [IN] Pointer of type Vps_DcOnchipEncoderCmd, which
 *                       initializes following members
 *                       .vencId    = Paired VENC ,VPS_DC_VENC_SD in this case.
 *                       .encoderId = VPS_DC_ENCODER_RF
 *                       .cmd       = This macro
 *                       .argument  = Pointer to structure of type
 *                                     #VpsHal_RfExpertConfig
 *                       .additionalArgs = NULL
 *
 * \return  VPS_SOK if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_RF_GET_EXPERT_CONFIG                                 \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x3u)

/**
 * \brief This control command applies the supplied expert RF configuration.
 *
 *        Provided the values supplied are valid and encoder is not enabled.
 *
 * \param   cmdArgs [IN] Pointer of type Vps_DcOnchipEncoderCmd, which
 *                       initializes following members
 *                       .vencId    = Paired VENC ,VPS_DC_VENC_SD in this case.
 *                       .encoderId = VPS_DC_ENCODER_RF
 *                       .cmd       = This macro
 *                       .argument  = Pointer to structure of type
 *                                     #VpsHal_RfExpertConfig
 *                       .additionalArgs = NULL
 *
 * \return  VPS_SOK if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_RF_SET_EXPERT_CONFIG                                 \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x4u)

/**
 *
 * \brief Switch ON the RF Modulator DAC.
 *
 *  This control command could be used to switch ON the RF Modulator DAC.
 *
 * \param   cmdArgs [IN] Pointer of type Vps_DcOnchipEncoderCmd, which
 *                       initializes following members
 *                       .vencId    = Paired VENC ,VPS_DC_VENC_SD in this case.
 *                       .encoderId = VPS_DC_ENCODER_RF
 *                       .cmd       = This macro
 *                       .argument  = NULL
 *                       .additionalArgs = NULL
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_RF_START                                             \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x5u)

/**
 *
 * \brief Switch OFF the RF Modulator DAC.
 *
 *  This control command could be used to switch OFF the RF Modulator DAC.
 *
 * \param   cmdArgs [IN] Pointer of type Vps_DcOnchipEncoderCmd, which
 *                       initializes following members
 *                       .vencId    = Paired VENC ,VPS_DC_VENC_SD in this case.
 *                       .encoderId = VPS_DC_ENCODER_RF
 *                       .cmd       = This macro
 *                       .argument  = NULL
 *                       .additionalArgs = NULL
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_RF_STOP                                              \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x6u)
/** \brief maximum number of on-chip encoders control/configure ioctl commands
 *
 *  marker used to denote the maximum number of on-chip ioctls supported
 *
 *  \par caution ensure that on-chip encoders ioctl value does not execeed this
 *       value
 */
#define VPS_DCTRL_IOCTL_ONCHIP_ENC_MAX (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x20u)

/* @} */



/*             structure required for on-chip encoder control                 */
/* ========================================================================== */

/**
 * \brief
 *  on-chip encoders control parameters. place holder for arguments thats would
 *  be required by the on-chip encoder/its paired venc.
 *
 * \par caution - refer the encoder specific hals header files for the types
 *                of arguments required by the encoders.
 */
struct vps_dconchipencodercmd {
	u32                 vencid;
	/**< venc identifier - venc that would drive encoder of intreset */
	u32                 encoderid;
	/**< encoder identifier - selects the encoder that would require
	     perform the requested control */
	u32                 cmd;
	/**< command for the encoder */
	void                *argument;
	/**< command arguments that would be required. */
	void                *additionalargs;
	/**< additional arguments if any. */
	u32 (*Vps_DcOnChipEnc_CbFxn) (u32 vencId,
				      u32 encoderId,
				      u32 sinkState,
				      void *appData);
	/**< Application provided callback function - Used only when cmd is
	 HDMI */
};


#endif

#endif /* End of #ifndef _VPS_DISPLAYCTRL_H */

/* @} */

