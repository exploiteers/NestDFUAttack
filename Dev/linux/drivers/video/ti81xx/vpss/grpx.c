/*
 * linux/drivers/video/ti81xx/vpss/grpx.c
 *
 * VPSS graphics driver for TI 81XX
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
#define VPSS_SUBMODULE_NAME "GRPX "

#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include "core.h"
#include "dc.h"
#include "grpx.h"

static int num_gctrl;
static struct list_head gctrl_list;

static struct platform_device *grpx_dev;
static struct vps_payload_info  *grpx_payload_info;

static struct vps_grpx_ctrl *get_grpx_ctrl_from_handle(void * handle)
{
	struct vps_grpx_ctrl *gctrl;

	list_for_each_entry(gctrl, &gctrl_list, list) {
		if (handle == gctrl->handle)
			return gctrl;

	}

	return NULL;

}

static void grpx_status_reset(struct vps_grpx_ctrl *gctrl)
{

	gctrl->frames->perframecfg = NULL;
	gctrl->framelist->perlistcfg = NULL;
	gctrl->grtlist->clutptr = NULL;
	gctrl->grtlist->scparams = NULL;
	gctrl->grtparam->stenptr = NULL;
	gctrl->grtparam->scparams = NULL;

	gctrl->gstate.regset = 0;
	gctrl->gstate.scset = 0;
	gctrl->gstate.clutSet = 0;
	gctrl->gstate.stenset = 0;
	gctrl->gstate.varset = 0;
}

static int grpx_register_vsync_cb(struct vps_grpx_ctrl *gctrl,
				vsync_callback_t cb, void *arg)
{
	int r = 0;

	gctrl->vsync_cb = cb;
	gctrl->vcb_arg = arg;
	return r;
}

static int grpx_unregister_vsync_cb(struct vps_grpx_ctrl *gctrl,
					     vsync_callback_t cb, void *arg)
{
	gctrl->vsync_cb = NULL;
	gctrl->vcb_arg = NULL;
	return 0;
}

/* this will be the call back register to the VPSS m3 for the VSYNC isr*/
static int vps_grpx_vsync_cb(void *handle, void *appdata, void * reserved)
{
	struct vps_grpx_ctrl *gctrl = get_grpx_ctrl_from_handle(handle);

	if (gctrl == NULL)
		return -EINVAL;

	if (gctrl->gstate.isstarted) {
		struct vps_isr_data *isrd;

		grpx_status_reset(gctrl);
		/*call all registered function*/
		list_for_each_entry(isrd, &gctrl->cb_list, list) {
			if (isrd->isr)
				isrd->isr(isrd->arg);
		}
		/*release the semphare to tell VYSNC is comming*/
		if (gctrl->vsync_cb)
			gctrl->vsync_cb(gctrl->vcb_arg);

	}
	return 0;
}

static int vps_grpx_apply_changes(struct vps_grpx_ctrl *gctrl)
{
	struct vps_grpx_state  *gstate = &gctrl->gstate;
	int r = 0;

	VPSSDBG("(%d)- apply changes into FVID2_FRAME.\n", gctrl->grpx_num);

	if (gstate->isstarted) {
		if (gstate->scset) {
			gctrl->grtlist->scparams =
				(struct vps_grpxscparams *)gctrl->gscp_phy;
			gctrl->framelist->perlistcfg =
				(struct vps_grpxscparams *)gctrl->grtlist_phy;
		}

		if (gstate->clutSet) {
			gctrl->framelist->perlistcfg =
				(struct vps_grpxscparams *)gctrl->grtlist_phy;
		}

		if ((gstate->regset) || (gstate->varset)) {
			gctrl->frames->perframecfg =
				(struct vps_grpxrtparams *)gctrl->grtp_phy;
		}

		if (gstate->stenset)
			gctrl->frames->perframecfg =
				(struct vps_grpxrtparams *)gctrl->grtp_phy;

	} else {
		if (gstate->scset)
			gctrl->glist->scparams =
				(struct vps_grpxscparams *)gctrl->gscp_phy;

		gctrl->glist->gparams =
			(struct vps_grpxrtparams *)gctrl->gp_phy;

		/*no runtime update so far*/
		gctrl->framelist->perlistcfg = NULL;
		gctrl->frames->perframecfg = NULL;

	}



	return r;
}

static int grpx_pre_start(struct vps_grpx_ctrl *gctrl)
{
	u32 w, h;
	u8  fmt;
	int bpp;
	u32 pitch;

	gctrl->get_resolution(gctrl, &w, &h, &fmt);
	/*set the dimension*/
	if ((gctrl->inputf->width == 0) || (gctrl->inputf->height == 0) ||
		(gctrl->inputf->width > w) || (gctrl->inputf->height > h))
		gctrl->set_input(gctrl, w, h, fmt);

	/*check the setting and reset to resolution if necessary*/
	if (gctrl->gparams->regparams.regionwidth +
	    gctrl->gparams->regparams.regionposx > w) {
		gctrl->gparams->regparams.regionwidth = w;
		gctrl->gparams->regparams.regionposx = 0;
		bpp = vps_get_bitspp(gctrl->inputf->bpp);
		pitch = (w * bpp >> 3);
		/*pitch should 16 byte boundary*/
		if (pitch & 0xF)
			pitch += 16 - (pitch & 0xF);
		gctrl->gparams->pitch[FVID2_RGB_ADDR_IDX] =
					(w * bpp >> 3);

	}
	if (gctrl->gparams->regparams.regionheight +
	    gctrl->gparams->regparams.regionposy > h) {
		gctrl->gparams->regparams.regionheight = h;
		gctrl->gparams->regparams.regionposy = 0;
	}
	return 0;
}

static int grpx_scparams_check(struct vps_grpx_ctrl *gctrl,
			       struct vps_grpxscparams *scparam)
{
	u16 fw, fh;
	u16 xend, yend;
	u8 vscaled, hscaled;
	struct vps_grpxregionparams regp;

	gctrl->get_regparams(gctrl, &regp);

	fw = gctrl->framewidth;
	fh = gctrl->frameheight;

	if ((scparam->inheight == 0) || (scparam->inwidth == 0) ||
	(scparam->outheight == 0) || (scparam->outwidth == 0)) {
		VPSSERR("(%d)- please config Scaler first.\n",
			gctrl->grpx_num);
		return -1;
	}

	/*get the current region scaled type*/
	if (scparam->inheight > scparam->outheight)
		vscaled = 2;
	else if (scparam->inheight < scparam->outheight)
		vscaled = 1;
	else
		/*anti-flicker*/
		vscaled = 0;

	if (scparam->inwidth != scparam->outwidth)
		hscaled = 1;
	else
		hscaled = 0;

	/*output at most 2 line and 2 pixes when scaled*/
	if (0 != vscaled)
		yend = regp.regionposy + scparam->outheight +
				GRPX_SCALED_REGION_EXTRA_LINES;

	else
		yend = regp.regionposy + regp.regionheight;

	if (0 != hscaled)
		xend = regp.regionposx + scparam->outwidth +
				GRPX_SCALED_REGION_EXTRA_PIXES;
	else
		xend = regp.regionposx + regp.regionwidth;

	/*make sure the size is not out of the frame*/
	if ((xend > fw) ||
	   (yend > fh)) {
		VPSSERR("(%d)- scaled region(%dx%d) out of frame(%dx%d).\n",
			gctrl->grpx_num, xend, yend,
			gctrl->framewidth, gctrl->frameheight);
		return -1;
	}

	return 0;


}
static int vps_grpx_set_input(struct vps_grpx_ctrl *gctrl,
			u32 width, u32 height, u8 scfmt)
{
	u32 bpp, pitch;

	VPSSDBG("(%d)- set input %d x %d P:%d.\n",
		gctrl->grpx_num, width, height, scfmt);
	gctrl->inputf->height = height;
	gctrl->inputf->width = width;
	gctrl->inputf->scanformat = FVID2_SF_PROGRESSIVE;
	bpp = vps_get_bitspp(gctrl->inputf->bpp);
	pitch = (width * bpp >> 3);
	/*pitch should 16 byte boundary*/
	if (pitch & 0xF)
		pitch += 16 - (pitch & 0xF);
	gctrl->inputf->pitch[FVID2_RGB_ADDR_IDX] = pitch;
	return 0;
}

static int vps_grpx_get_resolution(struct vps_grpx_ctrl *gctrl,
			 u32 *fwidth, u32 *fheight, u8 *scfmt)
{
	VPSSDBG("(%d)- get resolution.\n", gctrl->grpx_num);


	vps_dc_get_outpfmt(
		gctrl->enodes[0],
		&gctrl->framewidth,
		&gctrl->frameheight,
		&gctrl->scformat,
		DC_BLEND_ID);


	*fwidth = gctrl->framewidth;
	*fheight = gctrl->frameheight;
	*scfmt = gctrl->scformat;

	return 0;
}

static int vps_grpx_get_timing(struct vps_grpx_ctrl *gctrl,
		struct fvid2_modeinfo *tinfo)
{
	int r;
	VPSSDBG("(%d) - get timing\n", gctrl->grpx_num);

	r = vps_dc_get_timing(gctrl->enodes[0], tinfo);
	return r;
}

static int vps_grpx_set_format(struct vps_grpx_ctrl *gctrl,
	u8 bpp, u32 df, u32 pitch)
{
	enum fvid2_bitsperpixel fbpp;


	fbpp = vps_get_fbpp(bpp);

	gctrl->gparams->format = df;
	gctrl->gparams->pitch[FVID2_RGB_ADDR_IDX] = pitch;

	gctrl->inputf->bpp = fbpp;
	gctrl->inputf->channelnum = 0;
	gctrl->inputf->dataformat = df;
	gctrl->inputf->pitch[FVID2_RGB_ADDR_IDX] = pitch;

	if (gctrl->gstate.isstarted == false) {
		gctrl->gstate.varset = true;

		VPSSDBG("(%d)- set format bpp %d df %d, pitch %d.\n",
			gctrl->grpx_num, bpp, df, pitch);

	} else {
		if (df != gctrl->grtparam->format) {
			gctrl->grtparam->format = df;
			gctrl->gstate.varset = true;
			VPSSDBG("(%d)- set format df %d.\n",
				gctrl->grpx_num, df);
		}
		if (pitch != gctrl->grtparam->pitch[FVID2_RGB_ADDR_IDX]) {
			gctrl->grtparam->pitch[FVID2_RGB_ADDR_IDX] = pitch;
			gctrl->gstate.varset = true;
			VPSSDBG("(%d)- set format pitch %d.\n",
				gctrl->grpx_num, pitch);
		}
	}


	vps_grpx_apply_changes(gctrl);
	return 0;
}

static int vps_grpx_check_regparams(struct vps_grpx_ctrl *gctrl,
			      struct vps_grpxregionparams *regp, u16 ridx)
{
	u16 fw, fh;
	u16 xend, yend;
	u8 vscaled, hscaled;
	struct vps_grpxscparams scparam;

	gctrl->get_scparams(gctrl, &scparam);
	/*FIX ME fw and fh should be get from the display controller,
	 by know we just put a default number here*/

	fw = gctrl->framewidth;
	fh = gctrl->frameheight;

	/* does not support stencling until stenciling buffer is set*/
	if (regp->stencilingenable == true) {
		if (gctrl->gparams->stenptr == NULL)  {
			VPSSERR("(%d)- Set stenciling pointer first.\n",
				gctrl->grpx_num);
			return -1;
		}

	}
	/**
	 * due to the hardware scaler limitation, the scaler will output more lines
	 * and pixes than what asked, this should be taken into
	 * consideration when doing the frame boundary check and regions
	 * overlap
	 */
	if (regp->scenable) {

		if ((scparam.inheight == 0) || (scparam.inwidth == 0) ||
		(scparam.outheight == 0) || (scparam.outwidth == 0)) {
			VPSSERR("(%d)- please config Scaler first.\n",
				gctrl->grpx_num);
			return -1;
		}

		/*get the current region scaled type*/
		if (scparam.inheight > scparam.outheight)
			vscaled = 2;
		else if (scparam.inheight < scparam.outheight)
			vscaled = 1;
		else
			/*anti-flicker*/
			vscaled = 0;

		if (scparam.inwidth != scparam.outwidth)
			hscaled = 1;
		else
			hscaled = 0;
	} else {
		hscaled = 0;
		vscaled = 0;
	}
	/*output at most 2 line and 2 pixes when scaled*/
	if (0 != vscaled)
		yend = regp->regionposy + scparam.outheight +
				GRPX_SCALED_REGION_EXTRA_LINES;

	else
		yend = regp->regionposy + regp->regionheight;

	if (0 != hscaled)
		xend = regp->regionposx + scparam.outwidth +
				GRPX_SCALED_REGION_EXTRA_PIXES;
	else
		xend = regp->regionposx + regp->regionwidth;

	/*make sure the size is not out of the frame*/
	if ((xend > fw) ||
	   (yend > fh)) {
		VPSSERR("(%d)- region(%dx%d) out of frame(%dx%d).\n",
			gctrl->grpx_num, xend, yend,
			gctrl->framewidth, gctrl->frameheight);
		return -1;
	}
	/* we are expecting both first region and last
	 * region for single region case
	 */
	if (!((ridx == 0) && (regp->firstregion == true))) {
		VPSSERR("(%d)- first region wrong.\n", gctrl->grpx_num);
		return -1;
	}


	if (!((ridx == (gctrl->glist->numregions - 1)) &&
		(true == regp->lastregion))) {
		VPSSERR("(%d)- last region wrong.\n", gctrl->grpx_num);
		return -1;
	}
	return 0;
}

static int vps_grpx_set_stenparams(struct vps_grpx_ctrl *gctrl,
			  u32 stenptr, u32 pitch)
{
	int r = 0;

	gctrl->gparams->stenptr = (void *)stenptr;
	gctrl->grtparam->stenptr = (void *)stenptr;
	gctrl->gparams->stenpitch = pitch;
	gctrl->grtparam->stenpitch = pitch;
	if (stenptr == 0)
		gctrl->gstate.stenset = false;
	else
		gctrl->gstate.stenset = true;


	VPSSDBG("(%d)-) set stenciling %#x\n",
		gctrl->grpx_num, (u32)gctrl->gparams->stenptr);
	r = vps_grpx_apply_changes(gctrl);
	return r;
}

static int vps_grpx_get_stenparams(struct vps_grpx_ctrl *gctrl,
			  u32 *stenaddr,
			  u32 *pitch)
{
	int r = 0;
	*stenaddr = (u32)gctrl->gparams->stenptr;
	*pitch = gctrl->gparams->stenpitch;
	VPSSDBG("(%d)- get stenciling %#x with stride 0x%x\n",
		gctrl->grpx_num, (u32)*stenaddr, (u32)*pitch);
	return r;
}

static int vps_grpx_set_scparams(struct vps_grpx_ctrl *gctrl,
		struct vps_grpxscparams *sci)
{
	int r = 0;

	/*need make sure that out_widht and out_height is inside the frame*/
	if (grpx_scparams_check(gctrl, sci))
		return -1;

	VPSSDBG("(%d)- set sc params %dx%d->%dx%d\n",
		gctrl->grpx_num, sci->inwidth,
		sci->inheight, sci->outwidth, sci->outheight);
	/*set the scaling information*/
	memcpy(gctrl->gscparams, sci, sizeof(struct vps_grpxscparams));
	/*load app's own coefficients if available*/
	if (sci->sccoeff) {
		memcpy(gctrl->gsccoeff,
		       sci->sccoeff,
		       sizeof(struct vps_grpxsccoeff));
		gctrl->gscparams->sccoeff = (void *)gctrl->gsccoff_phy;
	}
	gctrl->gstate.scset = true;

	r = vps_grpx_apply_changes(gctrl);
	return r;
}

static int vps_grpx_get_scparams(struct vps_grpx_ctrl *gctrl,
			   struct vps_grpxscparams *sci)
{
	VPSSDBG("(%d)- get sc params.\n", gctrl->grpx_num);

	memcpy(sci, gctrl->gscparams, sizeof(struct vps_grpxscparams));
	return 0;

}

/* get the region parameters*/
static int vps_grpx_get_regparams(struct vps_grpx_ctrl *gctrl,
			    struct vps_grpxregionparams *gparams)
{
	VPSSDBG("(%d)- get region params.\n", gctrl->grpx_num);
	if (gctrl->gstate.isstarted)
		memcpy(gparams, &gctrl->grtparam->regparams,
		       sizeof(struct vps_grpxregionparams));
	else
		memcpy(gparams, &gctrl->gparams->regparams,
		       sizeof(struct vps_grpxregionparams));

	return 0;
}

/* set the region parameters*/
static int vps_grpx_set_regparams(struct vps_grpx_ctrl *gctrl,
			    struct vps_grpxregionparams *gparams)
{
	int r = 0;

	VPSSDBG("(%d)- set region params.\n", gctrl->grpx_num);
	memcpy(&gctrl->grtparam->regparams, gparams,
	       sizeof(struct vps_grpxregionparams));
	memcpy(&gctrl->gparams->regparams, gparams,
		sizeof(struct vps_grpxregionparams));

	/*store this is for the reopen the fb usage*/
	gctrl->inputf->width = gparams->regionwidth;
	gctrl->inputf->height = gparams->regionheight;

	gctrl->gstate.regset = true;

	r = vps_grpx_apply_changes(gctrl);
	return r;
}

/*set the clut pointer*/
static int vps_grpx_set_clutptr(struct vps_grpx_ctrl  *gctrl, u32 pclut)
{
	int r = 0;
	VPSSDBG("(%d)- set clut %#x\n", gctrl->grpx_num, pclut);

	gctrl->glist->clutptr = (void *)pclut;
	gctrl->grtlist->clutptr = (void *)pclut;

	gctrl->gstate.clutSet = true;
	r = vps_grpx_apply_changes(gctrl);
	return 0;
}

static void vps_grpx_add_ctrl(struct vps_grpx_ctrl *gctrl)
{
	++num_gctrl;
	list_add_tail(&gctrl->list, &gctrl_list);
}

static int vps_grpx_set_buffer(struct vps_grpx_ctrl *gctrl,
			 u32 buffer_addr)
{
	int r = 0;

	VPSSDBG("(%d)- add buffer %#x\n", gctrl->grpx_num, buffer_addr);


	gctrl->buffer_addr = buffer_addr;
	gctrl->frames->addr[FVID2_RGB_ADDR_IDX]
		[FVID2_RGB_ADDR_IDX] = (void *)buffer_addr;

	r = vps_grpx_apply_changes(gctrl);
	return r;
}



static int vps_grpx_create(struct vps_grpx_ctrl *gctrl)
{
	u32				grpxinstid;
	int r = 0;

	VPSSDBG("create grpx%d\n", gctrl->grpx_num);
	gctrl->cbparams->cbfxn = vps_grpx_vsync_cb;
	gctrl->cbparams->appdata = NULL;
	gctrl->cbparams->errlist = NULL;
	gctrl->cbparams->errcbfnx = NULL;

	if (gctrl->grpx_num == 0)
		grpxinstid = VPS_DISP_INST_GRPX0;
	else if (gctrl->grpx_num == 1)
		grpxinstid = VPS_DISP_INST_GRPX1;
	else
		grpxinstid = VPS_DISP_INST_GRPX2;

	gctrl->handle = vps_fvid2_create(
		FVID2_VPS_DISP_GRPX_DRV,
		grpxinstid,
		(void *)gctrl->gcp_phy,
		(void *)gctrl->gcs_phy,
		(struct fvid2_cbparams *)gctrl->cbp_phy);

	if (gctrl->handle == NULL)
		r = -EINVAL;

	return r;
}

static int vps_grpx_delete(struct vps_grpx_ctrl *gctrl)
{
	int r = 0;

	if ((gctrl == NULL) || (gctrl->handle == NULL))
		return -EINVAL;

	VPSSDBG("delete GRPX%d\n", gctrl->grpx_num);
	r = vps_fvid2_delete(gctrl->handle, NULL);
	if (!r) {

		/*set all state value back to default*/
		gctrl->gstate.clutSet = 0;
		gctrl->gstate.regset = 0;
		gctrl->gstate.scset = 0;
		gctrl->gstate.varset = 0;
		gctrl->gstate.stenset = 0;

		gctrl->grtlist->scparams = NULL;
		gctrl->grtlist->clutptr = NULL;
		gctrl->grtparam->scparams = NULL;
		gctrl->glist->clutptr = NULL;

		gctrl->framelist->perlistcfg = NULL;
		gctrl->frames->perframecfg = NULL;

		gctrl->handle = NULL;
	}
	return r;
}

static int vps_grpx_start(struct vps_grpx_ctrl *gctrl)
{
	int r = 0;
	VPSSDBG("start grpx%d\n", gctrl->grpx_num);
	if ((gctrl == NULL) || (gctrl->handle == NULL))
		return -EINVAL;

	if (!gctrl->gstate.isstarted) {
		grpx_pre_start(gctrl);
		/*start everything over, set format,
		  params, queue buffer*/
		r = vps_fvid2_setformat(
			gctrl->handle,
			(struct fvid2_format *)gctrl->inputf_phy);

		if (r == 0) {
			if (gctrl->gparams->regparams.scenable)
				gctrl->glist->scparams =
				    (struct vps_grpxscparams *)gctrl->gscp_phy;

			r = vps_fvid2_control(gctrl->handle,
					  IOCTL_VPS_SET_GRPX_PARAMS,
					  (struct vps_grpxparamlist *)
						gctrl->glist_phy,
					   NULL);
		}

		if (r == 0)
			r = vps_fvid2_queue(gctrl->handle,
					  (struct fvid2_framelist *)
					     gctrl->frmls_phy,
					  0);

		if (r == 0)
			r = vps_fvid2_start(gctrl->handle, NULL);


		if (!r)
			gctrl->gstate.isstarted = true;
	}
	return r;
}
static int vps_grpx_stop(struct vps_grpx_ctrl *gctrl)
{
	int r = 0;

	VPSSDBG("stop grpx%d\n", gctrl->grpx_num);
	if ((gctrl == NULL) || (gctrl->handle == NULL))
		return -EINVAL;

	if (gctrl->gstate.isstarted) {
		r = vps_fvid2_stop(gctrl->handle, NULL);
		if (!r)
			gctrl->gstate.isstarted = false;
	}
	return r;
}

static int vps_grpx_wait_vsync_timeout(struct vps_grpx_ctrl *gctrl)
{
	unsigned long timeout = msecs_to_jiffies(500);


	void grpx_irq_wait_handler(void *data)
	{
		complete((struct completion *)data);
	}

	int r;
	DECLARE_COMPLETION_ONSTACK(completion);
	r = grpx_register_vsync_cb(gctrl,
				   grpx_irq_wait_handler,
				   &completion);

	if (r)
		return r;

	timeout = wait_for_completion_interruptible_timeout(&completion,
							    timeout);
	grpx_unregister_vsync_cb(gctrl,
				 grpx_irq_wait_handler,
				 &completion);
	if (timeout == 0)
		return -ETIMEDOUT;

	if (timeout == -ERESTARTSYS)
		return -ERESTARTSYS;

	return 0;


}
/*S************************** sysfs related function************************/

/*show current grpx enabled status*/
static ssize_t graphics_enabled_show(struct vps_grpx_ctrl *gctrl,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", gctrl->gstate.isstarted);
}
/*enable the current grpx*/
static ssize_t graphics_enabled_store(struct vps_grpx_ctrl *gctrl,
				      const char *buf, size_t size)
{
	bool enabled;
	int r;
	if (gctrl->handle == NULL) {
		VPSSERR("please open fb%d node first.\n", gctrl->grpx_num);
		return -EINVAL;
	}

	enabled = simple_strtoul(buf, NULL, 10);
	if (gctrl->gstate.isstarted == enabled)
		return size;

	grpx_lock(gctrl);
	if (enabled)
		r = gctrl->start(gctrl);
	else
		r = gctrl->stop(gctrl);
	grpx_unlock(gctrl);

	if (r)
		return r;

	return size;
}

/*show how many blender are connecting to this grpx*/
static ssize_t graphics_nodes_show(struct vps_grpx_ctrl *gctrl,
				char *buf)
{
	int i, r;
	int l = 0;
	char name[10];

	l += snprintf(buf + l, PAGE_SIZE - l, "%d", gctrl->numends);
	for (i = 0; i < gctrl->numends; i++)  {
		r = vps_dc_get_node_name(gctrl->enodes[i], name);
		if (i == 0)
			l += snprintf(buf + l, PAGE_SIZE - l, ":%s", name);
		else
			l += snprintf(buf + l, PAGE_SIZE - l, ",%s", name);
	}
	l += snprintf(buf + l, PAGE_SIZE - l, "\n");

	return l;

}
/*set the enodes for grpx in the format 0:XXXX,1,XXXX,2:XXXX,3:XXXX*/
static ssize_t graphics_nodes_store(struct vps_grpx_ctrl *gctrl,
				      const char *buf, size_t size)
{
	int r = 0;
	int total = 0;
	int i;
	int idx = 0;
	u8 tiedvenc = 0;
	char *input = (char *)buf, *this_opt;
	int enodes[VPS_DC_MAX_VENC];
	struct vps_dcvencinfo vinfo;

	if (gctrl->gstate.isstarted) {
		VPSSERR("please stop grpx%d before continue.\n",
			gctrl->grpx_num);
		return -EINVAL;
	}
	grpx_lock(gctrl);

	this_opt = strsep(&input, ":");
	total = simple_strtoul(this_opt, &this_opt, 10);

	if ((total == 0) || (total > vps_get_numvencs())) {
		VPSSERR("(%d)- no node to set\n", gctrl->grpx_num);
		r = -EINVAL;
		goto exit;
	}
	/*check the remaining input string*/
	if ((input == NULL) || (!(strcmp(input, "\0")))) {
		r = -EINVAL;
		VPSSERR("(%d)- wrong node information\n", gctrl->grpx_num);
		goto exit;
	}
	/*parse each end note*/
	while (!r && (this_opt = strsep(&input, ",")) != NULL) {
		int nid;

		if (idx > total)
			break;

		if (vps_dc_get_id(this_opt, &nid, DC_NODE_ID)) {
			VPSSERR("((%d)- failed to parse node name %s\n",
				gctrl->grpx_num, this_opt);
			r = -EINVAL;
			goto exit;

		}

		vps_dc_get_id(this_opt,
			      &vinfo.modeinfo[idx].vencid,
			      DC_VENC_ID);
		enodes[idx] = nid;

		VPSSDBG("(%d)- s: %d e:%d vid:%d\n",
			gctrl->grpx_num,
			gctrl->snode,
			enodes[idx],
			vinfo.modeinfo[idx].vencid);
		idx++;
		if (input == NULL)
			break;
	}
	/*the total is not match what parsed in the string*/
	if (idx != total) {
		r = -EINVAL;
		VPSSERR("(%d)- node number not match.\n",
			gctrl->grpx_num);
		goto exit;
	}
	vinfo.numvencs = total;
	for (i = 0; i < total; i++)
		tiedvenc |= vinfo.modeinfo[i].vencid;


	/*tied venc check*/
	if (total == 1)
		tiedvenc = 0;
	else {
		u8 tvencs;
		vps_dc_get_tiedvenc(&tvencs);
		if (tiedvenc & (~tvencs)) {
			VPSSERR("(%d)- nodes not match tied vencs\n",
				gctrl->grpx_num);
			r = -EINVAL;
			goto exit;
		}
	}

	/*make sure the VENCs are already running before continue*/
	vinfo.numvencs = total;
	r = vps_dc_get_vencinfo(&vinfo);
	if (r)
		goto exit;

	for (i = 0; i < vinfo.numvencs; i++) {
		if (vinfo.modeinfo[i].isvencrunning == 0) {
			r = -EINVAL;
			VPSSERR("(%d)- venc %d not running.\n",
				gctrl->grpx_num,
				vinfo.modeinfo[i].vencid);
			goto exit;
		}
	}

	/*disable the exit nodes*/
	for (i = 0; (i < gctrl->numends) && (r == 0); i++) {
		r =  vps_dc_set_node(gctrl->enodes[i],
				     gctrl->snode,
				     0);
	}
	if (r) {
		r = -EINVAL;
		VPSSERR("(%d)- failed to disable node\n",
			gctrl->grpx_num);
		goto exit;
	}
	/*enable the new nodes*/
	for (i = 0; (i < total) && (r == 0); i++) {
		r = vps_dc_set_node(enodes[i],
				    gctrl->snode,
				    1);
	}
	if (r) {
		r = -EINVAL;
		VPSSERR("(%d)- failed to enable node\n",
			gctrl->grpx_num);
		goto exit;
	}
	/*update new nodes into local database*/
	gctrl->numends = total;
	for (i = 0; i < gctrl->numends; i++)
		gctrl->enodes[i] = enodes[i];
	VPSSDBG("(%d)- numedge :%d\n",
		gctrl->grpx_num, gctrl->numends);
	r = size;

exit:
	grpx_unlock(gctrl);
	return r;
}


struct graphics_attribute {
	struct attribute attr;
	ssize_t (*show)(struct vps_grpx_ctrl *, char *);
	ssize_t (*store)(struct vps_grpx_ctrl *, const char *, size_t);
};
#define GRAPHICS_ATTR(_name, _mode, _show, _store) \
	struct graphics_attribute graphics_attr_##_name = \
	__ATTR(_name, _mode, _show, _store)

static GRAPHICS_ATTR(enabled, S_IRUGO|S_IWUSR,
	graphics_enabled_show, graphics_enabled_store);

static GRAPHICS_ATTR(nodes, S_IRUGO | S_IWUSR,
	graphics_nodes_show, graphics_nodes_store);

static struct attribute *graphics_sysfs_attrs[] = {
	&graphics_attr_enabled.attr,
	&graphics_attr_nodes.attr,
	NULL
};

static ssize_t graphics_attr_show(struct kobject *kobj,
				  struct attribute *attr,
				  char *buf)
{
	struct vps_grpx_ctrl *gctrl;
	struct graphics_attribute *grpx_attr;

	gctrl = container_of(kobj, struct vps_grpx_ctrl, kobj);
	grpx_attr = container_of(attr, struct graphics_attribute, attr);

	if (!grpx_attr->show)
		return -ENOENT;

	return grpx_attr->show(gctrl, buf);
}

static ssize_t graphics_attr_store(struct kobject *kobj, struct attribute *attr,
		const char *buf, size_t size)
{
	struct vps_grpx_ctrl *gctrl;
	struct graphics_attribute *grpx_attr;

	gctrl = container_of(kobj, struct vps_grpx_ctrl, kobj);
	grpx_attr = container_of(attr, struct graphics_attribute, attr);

	if (!grpx_attr->store)
		return -ENOENT;

	return grpx_attr->store(gctrl, buf, size);
}


static const struct sysfs_ops graphics_sysfs_ops = {
	.show = graphics_attr_show,
	.store = graphics_attr_store,
};

static struct kobj_type graphics_ktype = {
	.sysfs_ops = &graphics_sysfs_ops,
	.default_attrs = graphics_sysfs_attrs,
};

/*E***********************sysfs functions *********************************/

static int vps_grpx_create_sysfs(struct vps_grpx_ctrl *gctrl)
{
	int r;
	if (gctrl == NULL)
		return -EINVAL;

	r = kobject_init_and_add(&gctrl->kobj,
				 &graphics_ktype,
				 &grpx_dev->dev.kobj,
				 "graphics%d",
				 gctrl->grpx_num);
	if (r)
		VPSSERR("failed to create grpx%d sysfs file.\n",
			gctrl->grpx_num);
	else
		gctrl->sysfs = true;
	return r;

}

static void vps_grpx_remove_sysfs(struct vps_grpx_ctrl *gctrl)
{
	if (gctrl->sysfs) {
		kobject_del(&gctrl->kobj);
		kobject_put(&gctrl->kobj);
		gctrl->sysfs = false;
	}

}
/*E*******************SYSFS Function*********************************/

int vps_grpx_register_isr(vsync_callback_t cb, void *arg, int idx)
{
	int r = 0;
	struct vps_grpx_ctrl *gctrl;
	struct vps_isr_data *isrd, *new;

	gctrl = vps_grpx_get_ctrl(idx);
	if (gctrl == NULL)
		return -EINVAL;

	grpx_lock(gctrl);
	/*make sure not duplicate */
	list_for_each_entry(isrd, &gctrl->cb_list, list) {
		if ((isrd->isr == cb) && (isrd->arg == arg)) {
			r = -EINVAL;
			goto exit;
		}
	}
	/*add to the list*/
	new = kzalloc(sizeof(*isrd), GFP_KERNEL);
	new->isr = cb;
	new->arg = arg;
	list_add_tail(&new->list, &gctrl->cb_list);

exit:
	grpx_unlock(gctrl);
	return r;

}
EXPORT_SYMBOL(vps_grpx_register_isr);

int vps_grpx_unregister_isr(vsync_callback_t cb , void *arg, int idx)
{
	int r = 0;
	struct vps_grpx_ctrl *gctrl;
	struct vps_isr_data  *isrd,  *next;

	gctrl = vps_grpx_get_ctrl(idx);
	if (gctrl == NULL)
		return -EINVAL;

	grpx_lock(gctrl);
	list_for_each_entry_safe(isrd,
				 next,
				 &gctrl->cb_list,
				 list) {
		if ((isrd->arg == arg) && (isrd->isr == cb)) {
			list_del(&isrd->list);
			kfree(isrd);
			break;
		}
	}

	grpx_unlock(gctrl);

	return r;

}
EXPORT_SYMBOL(vps_grpx_unregister_isr);

struct vps_grpx_ctrl *vps_grpx_get_ctrl(int num)
{
	struct vps_grpx_ctrl *gctrl;

	list_for_each_entry(gctrl, &gctrl_list, list) {
		if (gctrl->grpx_num == num)
			return gctrl;
	}

	return NULL;
}
EXPORT_SYMBOL(vps_grpx_get_ctrl);

int vps_grpx_get_num_grpx(void)
{
	return num_gctrl;
}
EXPORT_SYMBOL(vps_grpx_get_num_grpx);

void __init vps_fvid2_grpx_ctrl_init(struct vps_grpx_ctrl *gctrl)
{
	gctrl->inputf->scanformat = FVID2_SF_PROGRESSIVE;

	gctrl->gcparam->memtype = VPS_VPDMA_MT_NONTILEDMEM;
	gctrl->gcparam->drvmode = VPS_GRPX_FRAME_BUFFER_MODE;

	/* init parameters*/
	gctrl->glist->numregions = 1;

	gctrl->gparams->regparams.firstregion = true;
	gctrl->gparams->regparams.lastregion = true;
	gctrl->gparams->regparams.disppriority = 1;

	gctrl->grtparam->regparams.firstregion = true;
	gctrl->grtparam->regparams.lastregion = true;
	gctrl->grtparam->regparams.disppriority = 1;

	/* init the FVID2 related structures*/
	gctrl->framelist->numframes = 1;
	gctrl->framelist->frames[0] = (struct fvid2_frame *)gctrl->frm_phy;

	/*assign the function pointer*/
	gctrl->apply_changes = vps_grpx_apply_changes;
	gctrl->set_input = vps_grpx_set_input;
	gctrl->set_buffer = vps_grpx_set_buffer;
	gctrl->get_resolution = vps_grpx_get_resolution;
	gctrl->set_format = vps_grpx_set_format;
	gctrl->check_params = vps_grpx_check_regparams;
	gctrl->set_clutptr = vps_grpx_set_clutptr;
	gctrl->set_scparams = vps_grpx_set_scparams;
	gctrl->get_scparams = vps_grpx_get_scparams;
	gctrl->set_regparams = vps_grpx_set_regparams;
	gctrl->get_regparams = vps_grpx_get_regparams;
	gctrl->set_stenparams = vps_grpx_set_stenparams;
	gctrl->get_stenparams = vps_grpx_get_stenparams;
	gctrl->get_timing = vps_grpx_get_timing;
	gctrl->create = vps_grpx_create;
	gctrl->delete = vps_grpx_delete;
	gctrl->wait_for_vsync = vps_grpx_wait_vsync_timeout;
	gctrl->start = vps_grpx_start;
	gctrl->stop = vps_grpx_stop;

}
static inline int get_alloc_size(void)
{
	int size = 0;

	size  = sizeof(struct vps_grpxcreateparams);
	size += sizeof(struct vps_grpxcreatestatus);
	size += sizeof(struct vps_grpxscparams);
	size += sizeof(struct vps_grpxsccoeff);
	size += sizeof(struct vps_grpxrtparams);
	size += sizeof(struct vps_grpxrtparams);
	size += sizeof(struct vps_grpxrtlist);
	size += sizeof(struct vps_grpxparamlist);
	size += sizeof(struct fvid2_cbparams);
	size += sizeof(struct fvid2_framelist);
	size += sizeof(struct fvid2_format);
	size += sizeof(struct fvid2_frame);

	size *= VPS_DISP_GRPX_MAX_INST;

	return size;
}

void __init assign_payload_addr(struct vps_grpx_ctrl *gctrl,
				 struct vps_payload_info *pinfo,
				 u32  *buf_offset)
{

	gctrl->gcparam = (struct vps_grpxcreateparams *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->gcp_phy,
				sizeof(struct vps_grpxcreateparams));

	gctrl->gcstatus = (struct vps_grpxcreatestatus *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->gcs_phy,
				sizeof(struct vps_grpxcreatestatus));

	gctrl->gscparams = (struct vps_grpxscparams *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->gscp_phy,
				sizeof(struct vps_grpxscparams));

	gctrl->gsccoeff = (struct vps_grpxsccoeff *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->gsccoff_phy,
				sizeof(struct vps_grpxsccoeff));

	gctrl->gparams = (struct vps_grpxrtparams *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->gp_phy,
				sizeof(struct vps_grpxrtparams));

	gctrl->grtparam = (struct vps_grpxrtparams *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->grtp_phy,
				sizeof(struct vps_grpxrtparams));

	gctrl->grtlist = (struct vps_grpxrtlist *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->grtlist_phy,
				sizeof(struct vps_grpxrtlist));

	gctrl->glist = (struct vps_grpxparamlist *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->glist_phy,
				sizeof(struct vps_grpxparamlist));

	gctrl->cbparams = (struct fvid2_cbparams *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->cbp_phy,
				sizeof(struct fvid2_cbparams));

	gctrl->inputf = (struct fvid2_format *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->inputf_phy,
				sizeof(struct fvid2_format));

	gctrl->framelist = (struct fvid2_framelist *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->frmls_phy,
				sizeof(struct fvid2_framelist));

	gctrl->frames = (struct fvid2_frame *)
			setaddr(pinfo,
				buf_offset,
				&gctrl->frm_phy,
				sizeof(struct fvid2_frame));
}



int __init vps_grpx_init(struct platform_device *pdev)
{
	int i, r;
	struct vps_payload_info *pinfo;
	u32 size = 0;
	u32 offset = 0;
	u8 numends = 0;
	VPSSDBG("grpx init\n");
	INIT_LIST_HEAD(&gctrl_list);

	num_gctrl = 0;
	grpx_dev = pdev;

	/*allocate payload info*/
	grpx_payload_info = kzalloc(sizeof(struct vps_payload_info),
				GFP_KERNEL);

	if (!grpx_payload_info) {
		VPSSERR("allocate payload info failed\n");
		return -ENOMEM;
	}
	pinfo = grpx_payload_info;
	/*calcuate size to allocate*/
	size = get_alloc_size();
	pinfo->vaddr = vps_sbuf_alloc(size, &pinfo->paddr);

	if (pinfo->vaddr == NULL) {
		VPSSERR("alloc grpx dma buffer failed\n");
		pinfo->paddr = 0;
		r = -ENOMEM;
		goto cleanup;
	}

	pinfo->size = PAGE_ALIGN(size);
	memset(pinfo->vaddr, 0, pinfo->size);

	for (i = 0; i < VPS_DISP_GRPX_MAX_INST; i++) {
		struct vps_grpx_ctrl *gctrl;
		gctrl = kzalloc(sizeof(*gctrl), GFP_KERNEL);

		if (gctrl == NULL) {
			VPSSERR("failed to allocate grpx%d\n", i);
			r = -ENOMEM;
			goto cleanup;
		}
		/*assign the dma buffer*/
		assign_payload_addr(gctrl,
				    pinfo,
				    &offset);
		/*init gctrl*/
		vps_fvid2_grpx_ctrl_init(gctrl);
		gctrl->grpx_num = i;
		vps_grpx_add_ctrl(gctrl);
		mutex_init(&gctrl->gmutex);
		numends = 1;
		INIT_LIST_HEAD(&gctrl->cb_list);
		switch (i) {
		case 0:

			gctrl->snode = VPS_DC_GRPX0_INPUT_PATH;
			gctrl->enodes[0] = VPS_DC_HDMI_BLEND;
			break;
		case 1:
			gctrl->snode = VPS_DC_GRPX1_INPUT_PATH;
			if (cpu_is_ti816x())
				gctrl->enodes[0] = VPS_DC_HDCOMP_BLEND;
			else
				gctrl->enodes[0] = VPS_DC_DVO2_BLEND;

			break;
		case 2:
			gctrl->snode = VPS_DC_GRPX2_INPUT_PATH;
			gctrl->enodes[0] = VPS_DC_SDVENC_BLEND;
			break;
		}
		r = vps_dc_set_node(gctrl->enodes[0],
				    gctrl->snode,
				    1);
		if (r) {
			VPSSERR("failed to set grpx%d nodes\n",
				i);
			goto cleanup;
		}
		gctrl->numends = numends;
		r = vps_grpx_create_sysfs(gctrl);
		if (r)
			goto cleanup;

	}

	return 0;
cleanup:
	vps_grpx_deinit(pdev);
	return r;
}

void __exit vps_grpx_deinit(struct platform_device *pdev)
{
	struct vps_grpx_ctrl *gctrl;

	VPSSDBG("grpx deinit\n");
	/*remove all nodes*/
	list_for_each_entry(gctrl, &gctrl_list, list) {
		int i;
		for (i = 0; i < gctrl->numends; i++)
			vps_dc_set_node(gctrl->enodes[i],
					gctrl->snode,
					0);
	}
	/*free payload memory*/
	if (grpx_payload_info) {
		if (grpx_payload_info->vaddr) {
			vps_sbuf_free(grpx_payload_info->paddr,
				      grpx_payload_info->vaddr,
				      grpx_payload_info->size);
		}
		kfree(grpx_payload_info);
		grpx_payload_info = NULL;
	}
	/*free grpx ctrl */
	while (!list_empty(&gctrl_list)) {
		struct vps_isr_data *isrd;
		gctrl = list_first_entry(&gctrl_list,
					 struct vps_grpx_ctrl, list);
		/*free all cb list*/
		while (!list_empty(&gctrl->cb_list)) {
			isrd = list_first_entry(&gctrl->cb_list,
						struct vps_isr_data,
						list);
			list_del(&isrd->list);
			kfree(isrd);
		}
		vps_grpx_remove_sysfs(gctrl);
		list_del(&gctrl->list);
		kfree(gctrl);
	}
	num_gctrl = 0;
	grpx_dev = NULL;
}

