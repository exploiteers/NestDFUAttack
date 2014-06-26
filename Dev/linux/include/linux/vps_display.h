/*
 *
 * Video Display Header file for TI81XX VPSS
 *
 * Copyright (C) 2010 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * This file is from TI81XX VPSS M3 Driver
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

#ifndef __LINUX_VPS_DISPLAY_H
#define __LINUX_VPS_DISPLAY_H

#ifdef __KERNEL__

/**
 *  \brief Get display status IOCTL.
 *
 *  This IOCTL can be used to get the display status like number of frames
 *  displayed, number of frames repeated, number of frames queued/dequeued.
 *  Note: These counters will be reset either at the time of driver create or
 *  while starting the display operation. See respective counter comments for
 *  details.
 *
 *  \param cmdArgs       [OUT] Vps_DispStatus *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID_SOK on success, else failure
 *
 */
#define IOCTL_VPS_DISP_GET_STATUS       (VPS_DISP_IOCTL_BASE + 0x0001u)

/* @} */

/*
 *  Macros for different driver instance numbers to be passed as instance ID
 *  at the time of driver create.
 *  Note: These are read only macros. Don't modify the value of these macros.
 */
/** \brief Bypass path 0 display driver instance number. */
#define VPS_DISP_INST_BP0               (0u)
/** \brief Bypass path 1 display driver instance number. */
#define VPS_DISP_INST_BP1               (1u)
/** \brief Secondary path SD display driver instance number. */
#define VPS_DISP_INST_SEC1              (2u)

/**
 *  \brief Maximum number of display driver instance.
 *  2 bypass paths and 1 SD display through secondary path 1.
 *  Note: This is a read only macro. Don't modify the value of this macro.
 */
#define VPS_DISPLAY_INST_MAX            (3u)


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  struct Vps_DispCreateParams
 *  \brief Display driver create parameter structure to be passed to the
 *  driver at the time of display driver create call.
 */
struct vps_dispcreateparams {
    u32                  memtype;
    /**< VPDMA Memory type. For valid values see #Vps_VpdmaMemoryType. */
    u32                  percben;
    /**< TRUE: User callback passed during FVID2 create is called periodically.
         For progressive display, this interval is equal to VSYNC interval.
         For interlaced display, this interval is equal to twice the VSYNC
         interval as frames (two fields) are queued to the driver.
         FALSE: User callback passed during FVID2 create is called only
         if one or more frames (requests) are available in the driver output
         queue for the application to dequeue. */
};

/**
 *  sturct vps_dispcreatestatus
 *  \brief Create status for the display driver create call. This should be
 *  passed to the create function as a create status argument while creating
 *  the driver.
 */
struct vps_dispcreatestatus {
    int                   retval;
    /**< Return value of create call. */
    u32                  standard;
    /**< VENC Standard like NTSC, 1080p etc to which the display driver
         path is connected. For valid values see #FVID2_Standard. */
    u32                  dispwidth;
    /**< Width of the display at the VENC in pixels to which the display driver
         path is connected. */
    u32                  dispheight;
    /**< Height of the display at the VENC in linesto which the display driver
         path is connected. */
    u32                  minnumprimebuf;
    /**< Minimum number of buffers to prime before starting display operation.*/
    u32                  maxreqinqueue;
    /**< Maximum number of request per driver instance that can be submitted
         for display without having to dequeue the displayed requests. */
    u32                  maxmultiwin;
    /**< Maximum number of multiple windows supported for the opened
         instance. If this value is equal to 1, then multiple window display
         is not supported by the driver instance.
         Note: The maximum number of windows/col/row is after the splitting of
         the windows according to the VPDMA requirement. So from the
         application point of view, this could be less than returned value
         depending on the layout selected. */
    u32                  maxmultiwincol;
    /**< Maximum number of columns supported in multiple window mode. */
    u32                  maxmultiwinrow;
    /**< Maximum number of rows supported in multiple window mode. */
    u32                  maxmultiwinlayout;
    /**< Maximum number of multiple window layout that could be created to
         support dynamic layout change at runtime. */
};

/**
 *  struct Vps_DispRtParams
 *  \brief Run time configuration structure for the display driver.
 *  This needs to be passed along with frame list to update any supported
 *  run time parameters.
 */
struct vps_disprtparams {
    struct vps_layoutid           *layoutid;
    /**< ID of the layout to be selected. This should be a valid layout ID
         as returned by create multi window layout IOCTL. When layout ID is
         NULL, the driver will ignore this runtime parameter
         and continue processing the submitted request. */
    struct vps_cropconfig         *vcompcropcfg;
    /**< VCOMP crop configuration to crop the PIP window.
         Pass NULL if no change is required or VCOMP is not present in the
         display path. If application passes non-NULL when VCOMP is not present
         in the display path, then this runtime parameter will be ignored. */
    struct vps_posconfig          *vcompposcfg;
    /**< VCOMP position configuration used to position the PIP window after
         cropping.
         Pass NULL if no change is required or VCOMP is not present in the
         display path. If application passes non-NULL when VCOMP is not present
         in the display path, then this runtime parameter will be ignored. */
    struct vps_frameparams        *infrmprms;
    /**< Frame params for input frame - used to change the frame width and
         height at runtime.
         Note that this is used only in non-mosaic mode when the buffer
         dimension is smaller than the VENC size.
         When changing the size, the application should ensure that the
         startX/startY + the frame size doesn't exceed the display resolution.
         Pass NULL if no change is required. */
    struct vps_posconfig          *vpdmaposcfg;
    /**< VPDMA position configuration containing startX and startY.
         Note that this is used only in non-mosaic mode when the buffer
         dimension is smaller than the VENC size.
         When changing the position, the application should ensure that the
         startX/startY + the frame size doesn't exceed the display resolution.
         Pass NULL if no change is required. */

} ;

/**
 *  struct Vps_DispStatus
 *  \brief Display status structure used to get the current status.
 */
struct vps_dispstatus{
    u32                  queueCount;
    /**< Counter to keep track of how many requests are queued to the driver.
         Note: This counter will be reset at the time of driver create. */
    u32                  dequeueCount;
    /**< Counter to keep track of how many requests are dequeued from the
         driver.
         Note: This counter will be reset at the time of driver create. */
    u32                  displayedFrameCount;
    /**< Counter to keep track of how many frames are displayed. For interlaced
         display, this is half of the actual field display.
         Note: This counter will be reset at the time of display start. */
    u32                  repeatFrameCount;
    /**< Counter to keep track of how many frames are repeated when the
         application fails to queue buffer at the display rate.
         Note: This counter will be reset at the time of display start. */
} ;


#endif

#endif /* #ifndef __LINUX_VPS_DISPLAY_H */


