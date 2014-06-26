/*
 * linux/drivers/video/ti81xx/vpss/grpx.h
 *
 * Graphics internal header file for TI 81XX VPSS
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

#ifndef __DRIVRS_VIDEO_TI81XX_VPSS_GRPX_H__
#define __DRIVRS_VIDEO_TI81XX_VPSS_GRPX_H__

#ifdef __KERNEL__

/* 2 extra lines output from scaler*/
#define GRPX_SCALED_REGION_EXTRA_LINES      0x02
/* 2 extra pixel output from scaler*/
#define GRPX_SCALED_REGION_EXTRA_PIXES      0x02
/* one line gap for the up scaled except first region*/
#define GRPX_REGION_UP_SCALED_GAP           0x01
/* two lines gap for the downscaled except first region*/
#define GRPX_REGION_DOWN_SCALED_GAP         0x02



static inline void grpx_lock(struct vps_grpx_ctrl *gctrl)
{
	mutex_lock(&gctrl->gmutex);
}

static inline void grpx_unlock(struct vps_grpx_ctrl *gctrl)
{
	mutex_unlock(&gctrl->gmutex);
}


#endif
#endif
