/*
 * linux/drivers/video/ti81xx/vpss/dc.h
 *
 * Display Controller Internal Header file for TI 81XX VPSS
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

#ifndef __DRIVERS_VIDEO_TI81XX_VPSS_DC_H__
#define __DRIVERS_VIDEO_TI81XX_VPSS_DC_H__

#ifdef __KERNEL__

#define VPS_DC_MAX_NODE_NUM   26
#define HDMI 0
#define DVO2 1
#define SDVENC 2
#define HDCOMP 3

#define MAX_INPUT_NODES_BLENDER   5



enum dc_idtype {
	DC_BLEND_ID = 0,
	DC_VENC_ID,
	DC_MODE_ID,
	DC_NODE_ID
};


struct dc_vencmode_info {
	const char            *name;
	u32                   standard;
	struct fvid2_modeinfo minfo;
};

struct dc_vencname_info {
	const char *name;
	int  vid;
	int  bid;
	/*0:HDMI, 1: DVO2 , 2: SD, 3: HDCOMP*/
	int  idx;
};

struct dc_blender_info {
	char                     *name;
	u32                      idx;
	bool                     enabled;
	struct kobject           kobj;
	u32                      actnodes;
	struct vps_dispctrl      *dctrl;
	struct vps_dcvencclksrc  clksrc;
	bool                     isdeviceon;
};

struct vps_dispctrl {
	struct mutex               dcmutex;
	struct kobject             kobj;
	struct dc_blender_info     blenders[VPS_DC_MAX_VENC];
	u8                         numvencs;
	u8                         vencmask;
	/*start of shared structure*/
	struct vps_dccreateconfig  *dccreatecfg;
	u32                        dccreate_phy;
	struct vps_dcedeconfig     *dcedecfg;
	u32                        dcede_phy;
	u32                        *dcrtstatus;
	u32                        dcrtst_phy;
	struct vps_dcconfig        *dccfg;
	u32                        dccfg_phy;
	struct vps_dcvencinfo      *vinfo;
	u32                        vinfo_phy;
	struct vps_dcnodeinput     *nodeinfo;
	u32                        ninfo_phy;
	struct vps_dcoutputinfo    *opinfo;
	u32                        opinfo_phy;
	struct vps_dcvencclksrc    *clksrc;
	u32                        clksrc_phy;
	u32                        *dis_vencs;
	u32                        dis_vencsphy;
	struct vps_dccigrtconfig   *cigcfg;
	u32                         cigcfg_phy;
	struct vps_dcenumnodeinput  *dceninput;
	u32                         dceninput_phy;
	struct vps_dccomprtconfig   *comprtcfg;
	u32                          comprtcfg_phy;
	/*end of shared structure*/
	int                        tiedvenc;
	int                        enabled_venc_ids;
	void                       *fvid2_handle;
	/*automode: will set pll clock auto*/
	bool                       automode;
};

int vps_dc_get_id(char *name, int *id, enum dc_idtype type);

int vps_dc_get_node_name(int id, char *name);

int vps_dc_get_outpfmt(int id, u32 *width, u32 *height,
		      u8 *scformat, enum dc_idtype type);

int vps_dc_set_node(u8 nodeid, u8 inputid, u8 enable);

int vps_dc_get_vencinfo(struct vps_dcvencinfo *vinfo);

int vps_dc_get_tiedvenc(u8 *tiedvenc);
int vps_dc_set_color(struct vps_dccigrtconfig *cigconfig);
int vps_dc_get_color(struct vps_dccigrtconfig *cigconfig);

int vps_dc_enum_node_input(struct vps_dcenumnodeinput *eninput);
int vps_dc_get_node_status(struct vps_dcnodeinput *ninput);
int vps_dc_get_timing(u32 id, struct fvid2_modeinfo *tinfo);

#endif
#endif
