/*
 *
 *  FVID2 inteface header file for TI81XX VPSS
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


#ifndef __LINUX_FVID2_H__
#define __LINUX_FVID2_H__

#ifdef __KERNEL__

/*
 * =========== Error codes returned by FVID functions =============
 */

/** \brief FVID2 API call successful. */
#define FVID2_SOK                               ((int) 0)
/** \brief FVID2 API call returned with error as failed. It may be some
 *  hardware failure or software failure */
#define FVID2_EFAIL                             ((int) -1)
/** \brief FVID2 API call returned with error as bad arguments. Typically
 *  NULL pointer passed to the FVID2 API where its not expected. */
#define FVID2_EBADARGS                          ((int) -2)
/** \brief FVID2 API call returned with error as invalid parameters. Typically
 *  when parameters are not valid. */
#define FVID2_EINVALID_PARAMS                   ((int) -3)
/** \brief FVID2 API call returned with error as device already in use. Tried
 *  to open the driver maximum + 1 times.  Display and Capture driver suppports
 *  single open, while M2M driver supports multiple open. */
#define FVID2_EDEVICE_INUSE                     ((int) -4)
/** \brief FVID2 API call returned with error as timed out. Typically API is
 *  waiting for some condition and returned as condition not happened
 *  in the timeout period. */
#define FVID2_ETIMEOUT                          ((int) -5)
/** \brief FVID2 API call returned with error as allocation failure. Typically
 *  memory or resource allocation failure. */
#define FVID2_EALLOC                            ((int) -6)
/** \brief FVID2 API call returned with error as out of range. Typically when
 *  API is called with some argument that is out of range for that API like
 *  array index etc. */
#define FVID2_EOUT_OF_RANGE                     ((int) -7)
/** \brief FVID2 API call returned with error as try again. Momentarily API is
 *  not able to service request because of queue full or any other temporary
 *  reason. */
#define FVID2_EAGAIN                            ((int) -8)
/** \brief FVID2 API call returned with unsupported command. Typically when
 *  command is not supported by control API. */
#define FVID2_EUNSUPPORTED_CMD                  ((int) -9)
/** \brief FVID2 API call returned with error as no more buffers available.
 *  Typically when no buffers are available. */
#define FVID2_ENO_MORE_BUFFERS                  ((int) -10)
/** \brief FVID2 API call returned with error as unsupported operation.
 *  Typically when the specific operation is not supported by that API such
 *  as IOCTL not supporting some specific functions. */
#define FVID2_EUNSUPPORTED_OPS                  ((int) -11)
/** \brief FVID2 API call returned with error as driver already in use. */
#define FVID2_EDRIVER_INUSE                     ((int) -12)
/** \brief No Timeout. */
#define FVID2_TIMEOUT_NONE                      (0u)
/** \brief Timeout wait forever. */
#define FVID2_TIMEOUT_FOREVER                   (~(0u))

/**
 *  \brief this macro determines the maximum number of fvid2 frame pointers
 *  that can be passed per frame list.
 */
#define FVID2_MAX_FVID_FRAME_PTR                (64u)

/**
 *  \brief This macro determines the maximum number of planes/address used to
 *  represent a video buffer per field.
 *
 *  Currently this is set to 3 to support the maximum pointers required for
 *  YUV planar format - Y, Cb and Cr.
 */
#define FVID2_MAX_PLANES                        (3u)

/**
 *  \brief Number of fields - top and bottom. Used for allocating address
 *  pointers for both the fields.
 */
#define FVID2_MAX_FIELDS                        (2u)

/** \brief Number of IN/OUT frame list per process list - Used for array
  * allocation in process list structure. */
#define FVID2_MAX_IN_OUT_PROCESS_LISTS          (4u)

/*
 *  == Array index to the buffer address for various formats ====
 *  fvid2_Buffer structure contains a two dimensional array of void pointers.
 *  The below indices are used to identity the correct buffer address
 *  corresponding to the field and buffer formats.
 */

/** \brief Index for top field address in case of interlaced mode. */
#define FVID2_FIELD_TOP_ADDR_IDX                (0u)

/** \brief Index for bottom field address in case of interlaced mode. */
#define FVID2_FIELD_BOTTOM_ADDR_IDX             (1u)

/** \brief Index for frame address in case of progressive mode. */
#define FVID2_FRAME_ADDR_IDX                    (0u)

/** \brief Index for field mode address index. This is used in case of field
 *  mode of operation as in field capture or in deinterlacing mode of
 *  operation. In these cases both the even and odd field index is one and
 *  the same. */
#define FVID2_FIELD_MODE_ADDR_IDX       (0u)


/** \brief Index for even field address in case of interlaced mode. */
#define FVID2_FIELD_EVEN_ADDR_IDX               (FVID2_FIELD_TOP_ADDR_IDX)

/** \brief Index for odd field address in case of interlaced mode. */
#define FVID2_FIELD_ODD_ADDR_IDX                (FVID2_FIELD_BOTTOM_ADDR_IDX)

/** \brief Index for frame address in case of progressive mode. */
#define FVID2_FIELD_NONE_ADDR_IDX               (FVID2_FRAME_ADDR_IDX)

/** \brief Index for YUV444/YUV422 interleaved formats. */
#define FVID2_YUV_INT_ADDR_IDX                  (0u)

/** \brief Y Index for YUV444/YUV422/YUV420 planar formats. */
#define FVID2_YUV_PL_Y_ADDR_IDX                 (0u)

/** \brief CB Index for YUV444/YUV422/YUV420 planar formats. */
#define FVID2_YUV_PL_CB_ADDR_IDX                (1u)

/** \brief CR Index for YUV444/YUV422/YUV420 planar formats. */
#define FVID2_YUV_PL_CR_ADDR_IDX                (2u)

/** \brief Y Index for YUV semi planar formats. */
#define FVID2_YUV_SP_Y_ADDR_IDX                 (0u)

/** \brief CB Index for semi planar formats. */
#define FVID2_YUV_SP_CBCR_ADDR_IDX              (1u)

/** \brief Index for RGB888/RGB565/ARGB32 formats. */
#define FVID2_RGB_ADDR_IDX                      (0u)

/** \brief Index for RAW formats. */
#define FVID2_RAW_ADDR_IDX                      (0u)

/*
 * =========== command codes for submit call =============
 */
/** \brief Control command base address. */
#define FVID2_CTRL_BASE                         (0x00000000u)
/** \brief User command base address. */
#define FVID2_USER_BASE                         (0x10000000u)

/**
 *	\brief Command for fvid2_allocBuffer to request the video device driver
 *	to allocate one video buffer.
 *
 *  \param cmdargs       [in]  fvid2_frame *
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define FVID2_ALLOC_BUFFER                     (FVID2_CTRL_BASE + 1)

/** \brief Command for fvid2_freeBuffer to request the video device driver to
 * free memory of given video buffer.
 *
 *  \param cmdargs       [in]  fvid2_frame *
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define FVID2_FREE_BUFFER                      (FVID2_CTRL_BASE + 2)

/**
 *	\brief Command for fvid2_setFormat to request the video driver to
 *	configure itself with the provided format information.
 *
 *  \param cmdargs       [in]  const fvid2_format *
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define FVID2_SET_FORMAT                      (FVID2_CTRL_BASE + 3)

/**
 *	\brief Command for fvid2_getFormat to request the video driver to
 *	give back the current format configuration.
 *
 *  \param cmdargs       [in]  fvid2_format *
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define FVID2_GET_FORMAT                      (FVID2_CTRL_BASE + 4)

/**
 *	\brief Command for FVID2_start to request the video driver to start the
 *	video display or capture operation.
 *
 *  \param cmdargs       [in]  driver specific
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define FVID2_START                           (FVID2_CTRL_BASE + 5)

/**
 *	\brief Command for FVID2_stop to request the video driver to stop the
 *	video display or capture operation.
 *
 *  \param cmdargs       [in]  driver specific
 *  \param cmdstatusargs [out] null
 *
 *  \return fvid_sok on success, else failure
 *
 */
#define FVID2_STOP                           (FVID2_CTRL_BASE + 6)

/**
 *	\brief Data format.
 */
enum fvid2_dataformat {
	FVID2_DF_YUV422I_UYVY = 0x0000,
	/**< YUV 422 Interleaved format - UYVY. */
	FVID2_DF_YUV422I_YUYV,
	/**< YUV 422 Interleaved format - YUYV. */
	FVID2_DF_YUV422I_YVYU,
	/**< YUV 422 Interleaved format - YVYU. */
	FVID2_DF_YUV422I_VYUY,
	/**< YUV 422 Interleaved format - VYUY. */
	FVID2_DF_YUV422SP_UV,
	/**< YUV 422 Semi-Planar - Y separate, UV interleaved. */
	FVID2_DF_YUV422SP_VU,
	/**< YUV 422 Semi-Planar - Y separate, VU interleaved. */
	FVID2_DF_YUV422P,
	/**< YUV 422 Planar - Y, U and V separate. */
	FVID2_DF_YUV420SP_UV,
	/**< YUV 420 Semi-Planar - Y separate, UV interleaved. */
	FVID2_DF_YUV420SP_VU,
	/**< YUV 420 Semi-Planar - Y separate, VU interleaved. */
	FVID2_DF_YUV420P,
	/**< YUV 420 Planar - Y, U and V separate. */
	FVID2_DF_YUV444P,
	/**< YUV 444 Planar - Y, U and V separate. */
	FVID2_DF_YUV444I,
	/**< YUV 444 interleaved - YUVYUV... */
	FVID2_DF_RGB16_565 = 0x1000,
	/**< RGB565 16-bit - 5-bits R, 6-bits G, 5-bits B. */
	FVID2_DF_ARGB16_1555,
	/**< ARGB1555  5-bits R, 5-bits G, 5-bits B, 1-bit Alpha (MSB). */
	FVID2_DF_RGBA16_5551,
	/**< RGBA5551  5-bits R, 5-bits G, 5-bits B, 1-bit Alpha (LSB). */
	FVID2_DF_ARGB16_4444,
	/**< ARGB4444  4-bits R, 4-bits G, 4-bits B, 4-bit Alpha (MSB). */
	FVID2_DF_RGBA16_4444,
	/**< RGBA4444  4-bits R, 4-bits G, 4-bits B, 4-bit Alpha (LSB). */
	FVID2_DF_ARGB24_6666,
	/**< ARGB4444  6-bits R, 6-bits G, 6-bits B, 6-bit Alpha (MSB). */
	FVID2_DF_RGBA24_6666,
	/**< RGBA4444  6-bits R, 6-bits G, 6-bits B, 6-bit Alpha (LSB). */
	FVID2_DF_RGB24_888,
	/**< RGB24     8-bits R, 8-bits G, 8-bits B. */
	FVID2_DF_ARGB32_8888,
	/**< ARGB32   8-bits R, 8-bits G, 8-bits B, 8-bit Alpha (MSB). */
	FVID2_DF_RGBA32_8888,
	/**< RGBA32  - 8-bits R, 8-bits G, 8-bits B, 8-bit Alpha (LSB). */
	FVID2_DF_BITMAP8 = 0x2000,
	/**< BITMAP 8bpp. */
	FVID2_DF_BITMAP4_LOWER,
	/**< BITMAP 4bpp lower address in CLUT. */
	FVID2_DF_BITMAP4_UPPER,
	/**< BITMAP 4bpp upper address in CLUT. */
	FVID2_DF_BITMAP2_OFFSET0,
	/**< BITMAP 2bpp offset 0 in CLUT. */
	FVID2_DF_BITMAP2_OFFSET1,
	/**< BITMAP 2bpp offset 1 in CLUT. */
	FVID2_DF_BITMAP2_OFFSET2,
	/**< BITMAP 2bpp offset 2 in CLUT. */
	FVID2_DF_BITMAP2_OFFSET3,
	/**< BITMAP 2bpp offset 3 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET0,
	/**< BITMAP 1bpp offset 0 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET1,
	/**< BITMAP 1bpp offset 1 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET2,
	/**< BITMAP 1bpp offset 2 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET3,
	/**< BITMAP 1bpp offset 3 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET4,
	/**< BITMAP 1bpp offset 4 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET5,
	/**< BITMAP 1bpp offset 5 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET6,
	/**< BITMAP 1bpp offset 6 in CLUT. */
	FVID2_DF_BITMAP1_OFFSET7,
	/**< BITMAP 1bpp offset 7 in CLUT. */
	FVID2_DF_BAYER_RAW = 0x3000,
	/**< Bayer pattern. */
	FVID2_DF_RAW_VBI,
	/**< Raw VBI data. */
	FVID2_DF_RAW,
	/**< Raw data - Format not interpreted. */
	FVID2_DF_MISC,
	/**< For future purpose. */
	FVID2_DF_INVALID,
	/**< Invalid data format. Could be used to initialize variables. */
	FVID2_DF_MAX
	/**< Should be the last value of this enumeration.
	 Will be used by driver for validating the input parameters. */
} ;

/**
 *	\brief Scan format.
 */
enum fvid2_scanformat {
	FVID2_SF_INTERLACED = 0,
	/**< Interlaced mode. */
	FVID2_SF_PROGRESSIVE,
	/**< Progressive mode. */
	FVID2_SF_MAX
	/**< Should be the last value of this enumeration.
	  Will be used by driver for validating the input parameters. */
};

/**
 *  \brief Video standards.
 */
enum  fvid2_standard {
	FVID2_STD_NTSC = 0u,
	/**< 720x480 30FPS interlaced NTSC standard. */
	FVID2_STD_PAL,

	/**< 720x576 30FPS interlaced PAL standard. */
	FVID2_STD_480I,
	/**< 720x480 30FPS interlaced SD standard. */
	FVID2_STD_576I,
	/**< 720x576 30FPS interlaced SD standard. */

	FVID2_STD_CIF,
	/**< Interlaced, 360x120 per field NTSC, 360x144 per field PAL. */
	FVID2_STD_HALF_D1,
	/**< Interlaced, 360x240 per field NTSC, 360x288 per field PAL. */
	FVID2_STD_D1,
	/**< Interlaced, 720x240 per field NTSC, 720x288 per field PAL. */

	FVID2_STD_480P,
	/**< 720x480 60FPS progressive ED standard. */
	FVID2_STD_576P,
	/**< 720x576 60FPS progressive ED standard. */

	FVID2_STD_720P_60,
	/**< 1280x720 60FPS progressive HD standard. */
	FVID2_STD_720P_50,
	/**< 1280x720 50FPS progressive HD standard. */

	FVID2_STD_1080I_60,
	/**< 1920x1080 30FPS interlaced HD standard. */
	FVID2_STD_1080I_50,
	/**< 1920x1080 50FPS interlaced HD standard. */

	FVID2_STD_1080P_60,
	/**< 1920x1080 60FPS progressive HD standard. */
	FVID2_STD_1080P_50,
	/**< 1920x1080 50FPS progressive HD standard. */

	FVID2_STD_1080P_24,
	/**< 1920x1080 24FPS progressive HD standard. */
	FVID2_STD_1080P_30,
	/**< 1920x1080 30FPS progressive HD standard. */

	FVID2_STD_VGA_60,
	/**< 640x480 60FPS VESA standard. */
	FVID2_STD_VGA_72,
	/**< 640x480 72FPS VESA standard. */
	FVID2_STD_VGA_75,
	/**< 640x480 75FPS VESA standard. */
	FVID2_STD_VGA_85,
	/**< 640x480 85FPS VESA standard. */

	FVID2_STD_SVGA_60,
	/**< 800x600 60FPS VESA standard. */
	FVID2_STD_SVGA_72,
	/**< 800x600 72FPS VESA standard. */
	FVID2_STD_SVGA_75,
	/**< 800x600 75FPS VESA standard. */
	FVID2_STD_SVGA_85,
	/**< 800x600 85FPS VESA standard. */

	FVID2_STD_XGA_60,
	/**< 1024x768 60FPS VESA standard. */
	FVID2_STD_XGA_70,
	/**< 1024x768 72FPS VESA standard. */
	FVID2_STD_XGA_75,
	/**< 1024x768 75FPS VESA standard. */
	FVID2_STD_XGA_85,
	/**< 1024x768 85FPS VESA standard. */

	FVID2_STD_WXGA_60,
	/**< 1280x768 60FPS VESA standard. */
	FVID2_STD_WXGA_75,
	/**< 1280x768 75FPS VESA standard. */
	FVID2_STD_WXGA_85,
	/**< 1280x768 85FPS VESA standard. */

	FVID2_STD_SXGA_60,
	/**< 1280x1024 60FPS VESA standard. */
	FVID2_STD_SXGA_75,
	/**< 1280x1024 75FPS VESA standard. */
	FVID2_STD_SXGA_85,
	/**< 1280x1024 85FPS VESA standard. */

	FVID2_STD_SXGAP_60,
	/**< 1400x1050 60FPS VESA standard. */
	FVID2_STD_SXGAP_75,
	/**< 1400x1050 75FPS VESA standard. */

	FVID2_STD_UXGA_60,
	/**< 1600x1200 60FPS VESA standard. */

	FVID2_STD_MUX_2CH_D1,
	/**< Interlaced, 2Ch D1, NTSC or PAL. */
	FVID2_STD_MUX_4CH_D1,
	/**< Interlaced, 4Ch D1, NTSC or PAL. */
	FVID2_STD_MUX_4CH_CIF,
	/**< Interlaced, 4Ch CIF, NTSC or PAL. */
	FVID2_STD_MUX_4CH_HALF_D1,
	/**< Interlaced, 4Ch Half-D1, NTSC or PAL. */
	FVID2_STD_MUX_8CH_CIF,
	/**< Interlaced, 8Ch CIF, NTSC or PAL. */
	FVID2_STD_MUX_8CH_HALF_D1,
	/**< Interlaced, 8Ch Half-D1, NTSC or PAL. */

	FVID2_STD_AUTO_DETECT,
	/**< Auto-detect standard. Used in capture mode. */
	FVID2_STD_CUSTOM,
	/**< Custom standard used when connecting to external LCD etc...
	 The video timing is provided by the application.
	 Used in display mode. */

	FVID2_STD_MAX
	/**< Should be the last value of this enumeration.
	 Will be used by driver for validating the input parameters. */
};

/**
 *	\brief Field type
 */
enum fvid2_fid {
	FVID2_FID_TOP = 0,
	/**< Top field. */
	FVID2_FID_BOTTOM,
	/**< Bottom field. */
	FVID2_FID_FRAME,
	/**< Frame mode - Contains both the fields or a progressive frame. */
	FVID2_FID_MAX
	/**< Should be the last value of this enumeration.
	     Will be used by driver for validating the input parameters. */
};

/** \brief Even Field */
#define FVID2_FID_EVEN                             (FVID2_FID_TOP)
/** \brief Odd Field */
#define FVID2_FID_ODD                              (FVID2_FID_BOTTOM)
/** \brief No Field */
#define FVID2_FID_NONE                             (FVID2_FID_FRAME)

/**
 *	\brief Bits per pixel.
 */
enum fvid2_bitsperpixel {
	FVID2_BPP_BITS1 = 0,
	/**< 1 Bits per Pixel. */
	FVID2_BPP_BITS2,
	/**< 2 Bits per Pixel. */
	FVID2_BPP_BITS4,
	/**< 4 Bits per Pixel. */
	FVID2_BPP_BITS8,
	/**< 8 Bits per Pixel. */
	FVID2_BPP_BITS12,
	/**< 12 Bits per Pixel - used for YUV420 format. */
	FVID2_BPP_BITS16,
	/**< 26 Bits per Pixel. */
	FVID2_BPP_BITS24,
	/**< 24 Bits per Pixel. */
	FVID2_BPP_BITS32,
	/**< 32 Bits per Pixel. */
	FVID2_BPP_MAX
	/**< Should be the last value of this enumeration.
	     Will be used by driver for validating the input parameters. */
};


/**
 *	\brief Structure for setting FVID2 callback functions.
 */
struct fvid2_cbparams {
	int (*cbfxn)(void *handle, void *appdata, void *reserved);
	/**< Application callback function used by the driver to intimate any
	     operation has completed or not. This is an optional parameter
	     in case application decides to use polling method and so could be
	     set to NULL. */
	int (*errcbfnx)(void *handle, void *appdata, void *errdata,
		    void *reserved);
	/**< Application error callback function used by the driver to intimate
	     any error occurs at the time of streaming. This is an optional
	     parameter in case application decides not to get any error
	     callback and so could be set to NULL. */
	void *errlist;
	/**< Pointer to a valid framelist (FVID2_FrameList) in case of capture
	     and display drivers or a pointer to a valid processlist
	     (FVID2_ProcessList) in case of M2M drivers where the driver
	     copies the aborted/error packet. The memory of this list should
	     be allocated by the application and provided to the driver at the
	     time of driver creation. When the application gets this callback,
	     it has to empty this list and taken necessary action like freeing
	     up memories etc. The driver will then reuse the same list for
	     future error callback.
	     This could be NULL if errCbFxn is NULL. Otherwise this should be
	     non-NULL. */
	void *appdata;
	/**< Application specific data which is returned in the callback
	     function as it is. This could be set to NULL if not used. */
	void *reserved;
	/**< For future use. Not used currently. Set this to NULL. */
} ;

/**
 *	\brief Structure for setting or getting the FVID2 buffer formats.
 */
struct fvid2_format {
	u32                     channelnum;
	/**< Channel Number to which this format belongs to.
	     This is used in case of multiple buffers queuing/deqeuing using a
	     single call. This is not applicable for all the drivers. When not
	     used set it to zero. */
	u32                     width;
	/**< Width in pixels. */
	u32                     height;
	/**< Number of lines per frame. For interlaced mode, this should be set
	     to the frame size and not the field size. */
	u32                     pitch[FVID2_MAX_PLANES];
	/**< Pitch in bytes for each of the sub-buffers. This represents the
	     difference between two consecutive line address.
	     This is irrespective of whether the video is interlaced or
	     progressive and whether the fields are merged or separated for
	     interlaced video. */
	u32                     fieldmerged[FVID2_MAX_PLANES];
	/**< Whether both the fields have to be merged - line interleaved or
	     not. Used only for interlaced format. The effective pitch is
	     calculated based on this information along with pitch parameter.
	     If fields are merged, effective pitch = pitch * 2 else effective
	     pitch = pitch. */
	u32                     dataformat;
	/**< Frame data Format. */
	u32                     scanformat;
	/**< Scan Format. */
	u32                     bpp;
	/**< Number of bits per pixel. */
	void                    *reserved;
	/**< For future use. Not used currently. Set this to NULL. */
};

/**
 *	\brief Slice information used in drivers supporting subframe level
 *	     processing, for application and driver interaction
 *	[IN]  sliceNumber and NoOfLines available in the frame for queue
 *	      operation. Set by application and used by driver
 *	[OUT] NoOfLines output for current subframe for Dequeue. Set by driver
 *	      and used by application
 */
struct fvid2_subframeinfo {
	u32                      subframenum;
	/**< Current subframe Number in this frame,
	     range is from 0 to (NoOfSlicesInFrame-1)  */
	u32                      numinlines;
	/**< Number of lines available in the frame at the end of this slice.
	*/
	u32                      numoutlines;
	/**< Number of lines generated in output buffer after processing
	     current subframe */
} ;
/**
 *	\brief FVID2 frame buffer structure.
 */
struct fvid2_frame {
	void                     *addr[FVID2_MAX_FIELDS][FVID2_MAX_PLANES];
	/**< FVID2 buffer pointers for supporting multiple addresses like
	     y, u, v etc for a given frame. The interpretation of these
	     pointers depend on the format configured.
	     The first dimension represents the field and the second dimension
	     represents the plane.
	     Not all pointers are valid for a given format.

	     Representation of YUV422 Planar Buffer:
	     Field 0 Y -> addr[0][0], Field 1 Y -> addr[1][0]
	     Field 0 U -> addr[0][1], Field 1 U -> addr[1][1]
	     Field 0 V -> addr[0][2], Field 1 V -> addr[1][2]

	     Representation of YUV422 Interleaved Buffer:
	     Field 0 YUV -> addr[0][0], Field 1 YUV -> addr[1][0]
	     Other pointers are not valid.

	     Representation of RGB888 Buffer(Assuming RGB is always progressive)
	     RGB -> addr[0][0]
	     Other pointers are not valid.

	     Instead of using numericals for accessing the buffers,
	     the application can use the macros defined for each buffer
	     formats like FVID2_YUV_INT_ADDR_IDX, FVID2_RGB_ADDR_IDX,
	     FVID2_FID_TOP etc.

	     [IN] for queue operation.
	     [OUT] for dequeue operation. */

	u32                       fid;
	/**< Indicates whether this frame belong to top or bottom field.
	     [IN] for queue operation.
	     [OUT] for dequeue operation. */
	u32                       channelnum;
	/**< Channel number to which this FVID2 frame belongs to.
	     This is used in case of multiple buffers queuing/deqeuing using a
	     single call.
	     If only one channel is supported, then this should be set to zero.
	     [IN] for queue operation.
	     [OUT] for dequeue operation. */

	u32                       timestamp;
	/**< Time Stamp for captured or displayed frame.
	     [OUT] for dequeue operation. Not valid for queue operation. */

	void                      *appdata;
	/**< Additional application parameter per frame. This is not modified
	     by driver. */
	void                      *perframecfg;
	/**< Per frame configuration parameters like scaling ratio, cropping,
	     positioning etc...
	     This could be set to NULL if not used.
	     [IN] for queue operation. Dequeue returns the same pointer back to
		 the application. */

	void                      *blankdata;
	/**< Blanking data.
	     This could be set to NULL if not used.
	     [IN] for queue operation.
	     [OUT] for dequeue operation. */
	void                      *drvdata;
	/**< Used by driver. Application should not modify this. */
	struct fvid2_subframeinfo    *subframeinfo;
	/**< Used for Slice level processing information exchange between
	     application and driver.
	     This could be set to NULL if subframe level processing is not used.
	*/
	void                      *reserved;
    /**< For future use. Not used currently. Set this to NULL. */
} ;

/**
 *  \brief FVID2 Mode information structure.
 */
struct fvid2_modeinfo {
	u32              standard;
	/**< [IN] Standard for which to get the info.
	 For valid values see #FVID2_Standard. */
	u32              width;
	/**< Active video frame width in pixels. */
	u32              height;
	/**< Active video frame height in lines. */
	u32              scanformat;
	/**< Scan format of standard. For valid values see #FVID2_ScanFormat. */
	u32              pixelclock;
	/**< Pixel clock of standard in KHz. */
	u32              fps;
	/**< Frames per second. Not Used */
	u32              hfrontporch;
	/**< Horizontal front porch. Same for both fields in case of interlaced
	display */
	u32              hbackporch;
	/**< Horizontal back porch */
	u32              hsynclen;
	/**< Horizontal sync length. Same for both fields in case of interlaced
	display */
	u32              vfrontporch;
	/**< Vertical front porch for each field or frame */
	u32              vbackporch;
	/**< Vertical back porch for each field or frame */
	u32              vsynclen;
	/**< Vertical sync length for each field */
	u32              reserved[4u];
	/**< For future use. Not used currently. */
};

/**
 *	\brief FVID2 frame buffer list used to exchange multiple FVID2
 *	frames in a single driver call.
 */
struct fvid2_framelist {
	struct fvid2_frame         *frames[FVID2_MAX_FVID_FRAME_PTR];
	/**< an array of fvid2 frame pointers.
	[in] the content of the pointer array i.e fvid2_frame pointer is input
	for queue operation
	     output for dequeue operation. */
	u32                        numframes;
	/**< Number of frames - Size of the array containing FVID2 pointers.
	     [IN] for queue operation.
	     [OUT] for dequeue operation. */
	void                       *perlistcfg;
	/**< Per list configuration parameters like scaling ratio, positioning,
	     cropping etc for all the frames together.
	     This could be set to NULL if not used. In this case, the driver
	     will use the previous configuration.
	     [IN] for queue operation. Dequeue returns the same pointer back to
		 the application. */
	void                       *drvdata;
	/**< Used by driver. Application should not modify this. */
	void                       *reserved;
	/**< For future use. Not used currently. Set this to NULL. */
	void                       *appdata;
	/**< Additional application parameter per frame. This is not modified by
	     driver. */

} ;

/**
 * \brief FVID2 process list containing frame list used to exchange multiple
 * input/output buffers in M2M (memory to memory) operation. Each of the frame
 * list in turn have multiple frames/request.
 */
struct fvid2_processlist {
	struct fvid2_framelist    *inframelist[FVID2_MAX_IN_OUT_PROCESS_LISTS];
	/**< Pointer to an array of FVID2 frame list pointers for input nodes.
	     [IN] for both queue and dequeue operation.
	     The content of the pointer array i.e FVID2_FrameList pointer is
	     input for queue operation and is output for dequeue operation. */
	struct fvid2_framelist    *outframelist[FVID2_MAX_IN_OUT_PROCESS_LISTS];
	/**< Pointer to an array of FVID2 frame list pointers for output nodes.
	     [IN] for both queue and dequeue operation.
	     The content of the pointer array i.e FVID2_FrameList pointer is
	     input for queue operation and is output for dequeue operation. */
	u32                       numinlists;
	/**< Number of input frame list valid in inFrameList.
	     [IN] for queue operation.
	     [OUT] for dequeue operation. */
	u32                       numoutlists;
	/**< Number of output frame list valid in outFrameList.
	     [IN] for queue operation.
	     [OUT] for dequeue operation. */
	void                      *drvdata;
	/**< Used by driver. Application should not modify this. */
	void                      *reserved;
	/**< For future use. Not used currently. Set this to NULL. */
} ;


/**
 *      FVID2_create
 *      \brief Opens the driver identified by the driver ID.
 *
 *      \param drvId           [IN] Used to find a matching ID in the driver
 *                                  table.
 *      \param instanceId      [IN] Instance ID of the driver to open and is
 *                                  used to differentiate multiple instance
 *                                  support on a single driver.
 *      \param createArgs      [IN] Pointer to the create argument structure.
 *                                  The type of the structure is defined by the
 *                                  specific driver.  This parameter could be
 *                                  NULL depending on whether the actual
 *                                  driver forces it or not.
 *       \param createStatusArgs [OUT] Pointer to status argument structure
 *                                     where the Driver returns any status
 *                                     information. The type of the structure
 *                                     is defined by the specific driver. This
 *                                     parameter could be NULL depending on
 *                                     whether the actual driver forces it or
 *                                     not.
 *      \param cbParams         [IN] Application callback parameters.This
 *                                   parameter could be NULL depending on
 *                                   whether the actual driver forces it or
 *                                   not.
 *
 *      \return Returns a non-NULL FVID2_Handle object on success else returns
*               NULL on error.
 */
void *vps_fvid2_create(u32 drvid,
			u32 instanceid,
			void *createargs,
			void *createstatusargs,
			struct fvid2_cbparams *cbparams);

/**
 *	FVID2_delete
 *	\brief Application calls FVID2_delete to close the logical channel
 *	associated with FVID2 handle. Driver will relinquish all the resources
 *	allocated at the time of FVID2_create once all the logical handles for
 *	the driver is closed.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param deleteArgs	[IN] Pointer to the delete argument structure.
 *	                             The type of the structure is defined by
 *                                   the specific driver. This parameter could
 *                                   be NULL depending on whether the actual
 *                                   driver forces it or not.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *	failure.
 */
int vps_fvid2_delete(void *handle, void *deleteargs);

/**
 *	\brief An application calls FVID2_control to send standard control
 *	commands or device/driver specific control commands to the video
 *	driver.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param cmd		[IN] IOCTL command. The type of command
 *	                             supported is defined by the specific
 *                                   driver.
 *	\param cmdArgs		[IN] Pointer to the command argument structure.
 *	                             The type of the structure is defined by
 *                                   the specific driver for each of the
 *                                   supported IOCTL.  This parameter could be
 *                                   NULL depending on whether the actual
 *                                   driver forces it or not.
 *	\param cmdStatusArgs    [OUT] Pointer to status argument structure
 *                                    where the driver returns any status
 *                                   information.  The type of the structure is
 *                                   defined by the specific driver for each of
 *                                   the supported IOCTL.  This parameter could
 *                                   be NULL depending on whether the actual
 *                                   driver forces it or not.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *	      failure.
 */
int vps_fvid2_control(void *handle,
		       u32 cmd,
		       void *cmdargs,
		       void *cmdstatusargs);

/**
 *	\brief An application calls FVID2_queue to submit a video buffer to the
 *	video device driver.
 *
 *  - this is used in capture/display drivers.
 *  - this function could be called from task or isr context unless the specific
 *  driver restricts from doing so.
 *  - this is a non-blocking api unless the specific driver restricts from
 *  doing so.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param frameList	[IN] Pointer to the frame list structure
 *	                             containing the information about the
 *                                   FVID2 frames that has to be queued in the
 *                                   driver.
 *	\param streamId     [IN] Stream ID to which the frames should be
 *                                  queued This is used in drivers where they
 *                                  could support multiple streams for the
 *                                  same handle. Otherwise this should be set
 *                                  to zero.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *               failure.
 */
int vps_fvid2_queue(void *handle,
		    struct fvid2_framelist *framelist,
		    u32 streamid);

/**
 *	\brief An application calls FVID2_dequeue to request the video device
 *	driver to give ownership of a video buffer.
 *
 *  - this is used in capture/display drivers.
 *  - this is a non-blocking api if timeout is fvid2_timeout_none and could be
 *  called by task and isr context unless the specific driver restricts from
 *  doing so.
 *  - this is blocking api if timeout is fvid2_timeout_forever if supported by
 *  specific driver implementation.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param frameList	[OUT] Pointer to the frame list structure where
 *                            the dequeued frame pointer will be stored.
 *	\param streamId     [IN] Stream ID from where frames should be
 *	                             dequeued.  This is used in drivers where
 *	                             it could support multiple streams for the
 *	                             same handle. Otherwise this should be set
 *	                             to zero.
 *	\param timeout		[IN] FVID2 timeout in units of OS ticks. This
 *	                            will determine the timeout value till the
 *	                            driver will block for a free or completed
 *	                            buffer is available.  For non-blocking
 *	                            drivers this parameter might be ignored.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *	        failure.
 */
int vps_fvid2_dequeue(void *handle,
		       struct fvid2_framelist *framelist,
		       u32 stream_id,
		       u32 timeout);

/**
 *	\brief An application calls FVID2_processFrames to submit a video
 *	buffer to the video device driver.
 *
 *  this api is very similar to the fvid2_queue api except that this is
 *  used in m2m drivers only.
 *  - this function could be called from task or isr context unless the specific
 *  driver restricts from doing so.
 *  - this is a non-blocking api unless the specific driver restricts from
 *  doing so.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param processList	[IN] Pointer to the process list structure
 *	                            containing the information about the FVID2
 *                              frame lists and frames that has to be
 *                              queued to the driver for processing.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *             failure.
 */
int vps_fvid2_processframes(void *handle,
			    struct fvid2_processlist *processlist);

/**
 *	\brief An application calls FVID2_getProcessedFrames to request the
 *	video device driver to give ownership of a video buffer.
 *
 *  this api is very similar to the fvid2_dequeue api except that this is
 *  used in m2m drivers only.
 *  - this is a non-blocking api if timeout is fvid2_timeout_none and could be
 *  called by task and isr context unless the specific driver restricts from
 *  doing so.
 *  - this is blocking api if timeout is fvid2_timeout_forever if supported by
 *  specific driver implementation.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param processList	[OUT] Pointer to the process list structure
 *	                              where the driver will copy the references
 *	                              to the dequeued FVID2 frame lists and
 *	                              frames.
 *	\param timeout		[IN] FVID2 timeout. This will determine the
 *                                  timeout value till the driver will block
 *                                  for a free or completed buffer is
 *                                  available. For non-blocking drivers this
 *                                  parameter might be ignored.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *             failure.
 */
int vps_fvid2_getprocessedframes(void *handle,
				 struct fvid2_processlist *processlist,
				 u32 timeout);


/**
 *	\brief An application calls FVID2_start to request the video device
 *	driver to start the video display or capture operation.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param cmdArgs		[IN] Pointer to the start argument structure.
 *	                             The type of the structure is defined by
 *	                             the specific driver.  This parameter could
 *	                             be NULL depending on whether the actual
 *	                             driver forces it or not.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *	        failure.
 */
static inline int vps_fvid2_start(void *handle, void *cmdargs)
{
	return vps_fvid2_control(handle, FVID2_START, cmdargs, NULL);
}

/**
 *	\brief An application calls FVID2_stop to request the video device
 *	driver to stop the video display or capture operation.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param cmdArgs		[IN] Pointer to the stop argument structure.
 *                              The type of the structure is defined by
 *                              the specific driver.  This parameter could
 *                              be NULL depending on whether the actual
 *                              driver forces it or not.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *	        failure.
 */
static inline int vps_fvid2_stop(void *handle, void *cmdargs)
{
	return vps_fvid2_control(handle, FVID2_STOP, cmdargs, NULL);
}

/**
 *	\brief An application calls FVID2_setFormat to request the video device
 *	driver to set the format for a given channel.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param fmt		[IN] Pointer to the FVID2 format structure.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *         failure.
 */
static inline int vps_fvid2_setformat(void *handle, struct fvid2_format *fmt)
{
	return vps_fvid2_control(handle, FVID2_SET_FORMAT, fmt, NULL);
}

/**
 *	\brief An application calls FVID2_getFormat to request the video device
 *	driver to get the current format for a given channel.
 *
 *	\param handle		[IN] FVID2 handle returned by create call.
 *	\param fmt		[OUT] Pointer to the FVID2 format structure.
 *
 *	\return FVID2_SOK on success, else appropriate FVID2 error code on
 *         failure.
 */
static inline int vps_fvid2_getformat(void *handle, struct fvid2_format *fmt)
{
	return vps_fvid2_control(handle, FVID2_GET_FORMAT, fmt, NULL);
}

#endif

#endif

