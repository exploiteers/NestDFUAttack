/*
 *
 * Copyright (C) 2010 Texas Instruments
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __LINUX_VPSS_VIDEO_H
#define __LINUX_VPSS_VIDEO_H


#include <linux/list.h>
#include <linux/kobject.h>
#include <linux/device.h>

#include <linux/vps_display.h>


#define VPS_DISPLAY_MAX_EDGES   2
#define  MAX_BUFFER_NUM        32

#define VPSS_VID_CAPS_SCALING               1
#define VPSS_VID_CAPS_CROPING               2
#define VPSS_VID_CAPS_POSITIONING           4
#define VPSS_VID_CAPS_COLOR                 8

#define VPSS_VPDMA_BUFFER_ALIGN 0xFFFFFFF0

enum vps_video_colormask {
	VPS_VIDEO_COLOR_NOMASK,
	VPS_VIDEO_COLOR_MASK_1BIT,
	VPS_VIDEO_COLOR_MASK_2BIT,
	VPS_VIDEO_COLOR_MASK_3BIT
};

struct vps_video_color {
	bool ben; /*blend enable*/
	bool ten; /*transparent color enable*/
	u8 alpha;
	u32 colorkey;
	enum vps_video_colormask mask;
};

struct vps_video_ctrl {
	struct kobject                  kobj;
	bool                            sysfs;
	u32                             idx;
	bool                            isstarted;
	/*capacitiy of this video plane*/
	u32                             caps;
	struct mutex                    vmutex;
	struct list_head                list;
	/*list of the all callback for vsync*/
	struct list_head                cb_list;
	/*fvid2 control handle*/
	void                            *handle;
	/*store the alpha and color key*/
	struct vps_dccigrtconfig        cigconfig;
	/*S************Payload structure**********************/
	/*display driver parameters*/
	struct vps_dispcreateparams     *vcparams;
	u32                             vcp_phy;
	struct vps_dispcreatestatus     *vcstatus;
	u32                             vcs_phy;
	/*runtime paramter changes*/
	struct vps_disprtparams         *vrtparams;
	u32                             vrt_phy;
	/*position config*/
	struct vps_posconfig            *vdmaposcfg;
	u32                             vdpc_phy;
	/*video region config*/
	struct vps_frameparams         *vfrmprm;
	u32                            vfp_phy;
	/*dispaly status*/
	struct vps_dispstatus           *vstatus;
	u32                             vs_phy;
	/* The followings  are the FVID2 varables*/
	/*fvid2 create params*/
	struct fvid2_cbparams           *cbparams;
	u32                             cbp_phy;
	/* set format params*/
	struct fvid2_format             *fmt;
	u32                             fmt_phy;
	/*frame list*/
	struct fvid2_framelist          *framelist;
	u32                             frmls_phy;
	/*frames*/
	struct fvid2_frame              *frames[MAX_BUFFER_NUM];
	u32                             frm_phy[MAX_BUFFER_NUM];

	/*E************Payload structure**********************/
	/*output format*/
	u32                             framewidth;
	u32                             frameheight;
	u8                              scformat;

	/*display nodes(not including the final blend node)*/
	int num_edges;
	struct vps_dcnodeinput          nodes[VPS_DISPLAY_MAX_EDGES];
	/*num vencs connected to the input only blender is considered here*/
	int num_outputs;
	struct vps_dcnodeinput          enodes[VPS_DC_MAX_VENC - 1];
	/*function pointer*/
	int (*create)(struct vps_video_ctrl *vctrl,
				enum vps_vpdmamemorytype mtype);
	int (*delete)(struct vps_video_ctrl *vctrl);
	int (*start)(struct vps_video_ctrl *vctrl);
	int (*stop)(struct vps_video_ctrl *vctrl);
	int (*queue)(struct vps_video_ctrl *queue);
	int (*dequeue)(struct vps_video_ctrl *dequeue);

	int (*apply_changes)(struct vps_video_ctrl *vctrl);
	int (*check_format)(struct vps_video_ctrl *vctrl,
			struct fvid2_format *fmt);
	int (*try_format)(struct vps_video_ctrl *vctrl,
			  u32 width, u32 height, u32 df,
			  u32 pitch, u8 merged,
			  struct fvid2_format *fmt);
	int (*set_format)(struct vps_video_ctrl *vctrl,
			struct fvid2_format *fmt);
	int (*get_format) (struct vps_video_ctrl *vctrl,
			   struct fvid2_format *fmt);
	int (*set_pos) (struct vps_video_ctrl *vctrl,
			   u32 posx, u32 posy);
	int (*set_crop) (struct vps_video_ctrl *vctrl,
			   u32 posx, u32 posy, u32 pitch, u32 w, u32 h);
	int (*set_buffer)(struct vps_video_ctrl *vctrl,
		u32 buffer_paddr, u8 idx);
	int (*get_resolution)(struct vps_video_ctrl *vctrl,
		u32 *fwidth, u32 *fheight, bool *isprogressive);
	int (*set_color) (struct vps_video_ctrl *vctrl,
			struct vps_video_color *color);
	int (*get_color) (struct vps_video_ctrl *vctrl,
			struct vps_video_color *color);

};

int vps_video_get_num_video(void);
struct vps_video_ctrl *vps_video_get_ctrl(int num);

int vps_video_register_isr(vsync_callback_t cb , void *arg, int idx);

int vps_video_unregister_isr(vsync_callback_t cb , void *arg, int idx);


static inline void video_lock(struct vps_video_ctrl *vctrl)
{
	mutex_lock(&vctrl->vmutex);
}

static inline void video_unlock(struct vps_video_ctrl *vctrl)
{
	mutex_unlock(&vctrl->vmutex);
}

#endif
