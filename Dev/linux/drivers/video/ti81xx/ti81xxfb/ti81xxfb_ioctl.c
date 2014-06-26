/*
 * linux/drivers/video/ti81xx/ti81xxfb/ti81xxfb_ioctl.c
 *
 * Copyright (C) 2009 Texas Instruments
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * Some codes and ideals are from TI OMAP2 by Tomi Valkeinen
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

#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/ti81xxfb.h>
#include <linux/omapfb.h>
#include <plat/ti81xx_ram.h>

#include "fbpriv.h"

static void set_trans_key(struct fb_info *fbi,
			  struct ti81xxfb_region_params *rp,
			  struct vps_grpxregionparams *regp)
{

	regp->transenable = 1;
	if (rp->transen == TI81XXFB_FEATURE_DISABLE)
		regp->transenable = 0;

	if (rp->transen) {
		if (rp->transcolor > 0xFFFFFF)
			regp->transcolorrgb24 = 0xFFFFFF;
		else
			regp->transcolorrgb24 = rp->transcolor;

		regp->transtype = rp->transtype;
	}
}

static void set_boundbox(struct fb_info *fbi,
			 struct ti81xxfb_region_params *rp,
			 struct vps_grpxregionparams *regp)
{

	if (rp->bben)
		regp->bbalpha = rp->bbalpha;

	regp->bbenable = 1;
	if (rp->bben == TI81XXFB_FEATURE_DISABLE)
		regp->bbenable = 0;
}


static void set_blend(struct fb_info *fbi,
		      struct ti81xxfb_region_params *rp,
		      struct vps_grpxregionparams *regp)
{

	if (rp->blendtype == TI81XXFB_BLENDING_GLOBAL)
		regp->blendalpha = rp->blendalpha;

	regp->blendtype = (u32)rp->blendtype;

}

static int ti81xxfb_get_vram_info(struct fb_info *fbi,
					struct omapfb_vram_info *vinfo)
{
		unsigned long vram, free, largest;

		ti81xx_vram_get_info(&vram, &free, &largest);
		vinfo->total = vram;
		vinfo->free = free;
		vinfo->largest_free_block = largest;

		return 0;

}
/*This ioctl is to compatible with omap*/
static int ti81xxfb_setup_plane(struct fb_info *fbi,
				       struct omapfb_plane_info *opinfo)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct ti81xxfb_device *fbdev = tfbi->fbdev;
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct vps_grpxregionparams regp;
	struct vps_grpxscparams scp;
	int r = 0;
	bool regupdate = 0, scupdate = 0;
	ti81xxfb_lock(tfbi);
	if (!opinfo->enabled) {
		r = gctrl->stop(gctrl);
		goto exit;
	}

	gctrl->get_regparams(gctrl, &regp);
	gctrl->get_scparams(gctrl, &scp);

	/*reg parameter check*/
	if ((opinfo->pos_x != regp.regionposx) ||
		(opinfo->pos_y != regp.regionposy)) {
		regp.regionposx = opinfo->pos_x;
		regp.regionposy = opinfo->pos_y;
		regupdate = true;
	}
	/*output size check*/
	if (((opinfo->out_height == 0) && (opinfo->out_width == 0)) ||
		((opinfo->out_height == regp.regionheight) &&
			(opinfo->out_width == regp.regionwidth))) {
		if (regp.scenable) {
			regp.scenable = 0;
			regupdate = true;
		}
	} else if ((opinfo->out_height != scp.outheight) ||
			(opinfo->out_width != scp.outwidth)) {
			scp.inwidth = regp.regionwidth;
			scp.inheight = regp.regionheight;
			scp.outheight = opinfo->out_height;
			scp.outwidth = opinfo->out_width;
			scp.sccoeff = NULL;
			if (!regp.scenable) {
				regp.scenable = 1;
				regupdate = true;
			}
			scupdate = true;
	}


	/*FIXME how to handle the output size in the plane info,
	 treat it as scaling??*/
	if (scupdate)
		r = gctrl->set_scparams(gctrl, &scp);


	/*update position*/
	if (regupdate && !r) {
		r = gctrl->check_params(gctrl, &regp, 0);
		if (!r)
			gctrl->set_regparams(gctrl, &regp);
	}

	if ((regupdate || scupdate) && (!r))
		if (gctrl->gstate.isstarted)
			r = vps_fvid2_queue(gctrl->handle,
					    (struct fvid2_framelist *)
						gctrl->frmls_phy,
					    0);
	if (!r)
		r = gctrl->start(gctrl);

exit:
	ti81xxfb_unlock(tfbi);
	if (r)
		dev_err(fbdev->dev, "failed to set plane info\n");

	return r;
}
/*This ioctl is to compatible with omap*/
static int ti81xxfb_query_plane(struct fb_info *fbi,
			      struct omapfb_plane_info *opinfo)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct vps_grpxregionparams regp;
	struct vps_grpxscparams scp;

	memset(opinfo, 0, sizeof(*opinfo));
	gctrl->get_regparams(gctrl, &regp);

	opinfo->enabled = gctrl->gstate.isstarted;
	opinfo->pos_x = regp.regionposx;
	opinfo->pos_y = regp.regionposy;

	if (!regp.scenable) {
		opinfo->out_height = 0;
		opinfo->out_width = 0;
	} else {
		gctrl->get_scparams(gctrl, &scp);
		opinfo->out_height = scp.outheight;
		opinfo->out_width = scp.outwidth;
	}


	return 0;

}
static int ti81xxfb_set_region_params(struct fb_info *fbi,
				     struct ti81xxfb_region_params *regparam)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct ti81xxfb_device *fbdev = tfbi->fbdev;
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct vps_grpxregionparams regp;
	struct fb_var_screeninfo *var = &fbi->var;
	int r = 0;

	TFBDBG("ti81xxfb_set_regparams\n");


	if (regparam->priority >= TI81XXFB_MAX_PRIORITY) {
		dev_err(fbdev->dev, "priority out of range");
		return -EINVAL;
	}
	ti81xxfb_lock(tfbi);
	gctrl->get_regparams(gctrl, &regp);
	/* only update if it are not same*/


	regp.regionposx = regparam->pos_x;
	regp.regionposy = regparam->pos_y;
	regp.regionwidth = var->xres;
	regp.regionheight = var->yres;

	regp.disppriority = regparam->priority;
	regp.firstregion = 1;
	if (regparam->firstregion == TI81XXFB_FEATURE_DISABLE)
		regp.firstregion = 0;

	regp.lastregion = 1;
	if (regparam->lastregion == TI81XXFB_FEATURE_DISABLE)
		regp.lastregion = 0;

	regp.scenable = 1;
	if (regparam->scalaren == TI81XXFB_FEATURE_DISABLE)
		regp.scenable = 0;

	regp.stencilingenable = 1;
	if (regparam->stencilingen == TI81XXFB_FEATURE_DISABLE)
		regp.stencilingenable = 0;

	set_boundbox(fbi, regparam, &regp);
	set_blend(fbi, regparam, &regp);
	set_trans_key(fbi, regparam, &regp);

	r = gctrl->check_params(gctrl, &regp, regparam->ridx);

	if (r == 0)
		r = gctrl->set_regparams(gctrl, &regp);

	if (0 == r) {
		TFBDBG("set params handle %x\n", (u32)gctrl->handle);
		if (gctrl->gstate.isstarted)
			r = vps_fvid2_queue(gctrl->handle,
					    (struct fvid2_framelist *)
						gctrl->frmls_phy,
					    0);
		if (r == 0) {
			fbi->var.xres = regp.regionwidth;
			fbi->var.yres = regp.regionheight;
		}
	}

	ti81xxfb_unlock(tfbi);
	if (r)
		dev_err(fbdev->dev, "setup region failed %d\n", r);
	return r;
}

static int ti81xxfb_get_region_params(struct fb_info *fbi,
				     struct ti81xxfb_region_params *regparam)
{

	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct vps_grpxregionparams regp;
	int r = 0;

	ti81xxfb_lock(tfbi);
	r = gctrl->get_regparams(gctrl, &regp);

	if (0 == r) {
		regparam->ridx = 0;
		regparam->bbalpha = regp.bbalpha;

		regparam->bben = TI81XXFB_FEATURE_DISABLE;
		if (regp.bbenable == 1)
			regparam->bben = TI81XXFB_FEATURE_ENABLE;
		regparam->blendalpha = regp.blendalpha;
		regparam->blendtype = regp.blendtype;
		regparam->pos_x = regp.regionposx;
		regparam->pos_y = regp.regionposy;
		regparam->priority = regp.disppriority;

		regparam->firstregion = TI81XXFB_FEATURE_DISABLE;
		if (regp.firstregion == 1)
			regparam->firstregion = TI81XXFB_FEATURE_ENABLE;

		regparam->lastregion = TI81XXFB_FEATURE_DISABLE;
		if (regp.lastregion == 1)
			regparam->lastregion = TI81XXFB_FEATURE_ENABLE;

		regparam->scalaren = TI81XXFB_FEATURE_DISABLE;
		if (regp.scenable == 1)
			regparam->scalaren = TI81XXFB_FEATURE_ENABLE;

		regparam->stencilingen = TI81XXFB_FEATURE_DISABLE;
		if (regp.stencilingenable == 1)
			regparam->stencilingen = TI81XXFB_FEATURE_ENABLE;

		regparam->transcolor = regp.transcolorrgb24;
		regparam->transtype = regp.transtype;
		regparam->transen = TI81XXFB_FEATURE_DISABLE;
		if (regp.transenable == 1)
			regparam->transen = TI81XXFB_FEATURE_ENABLE;
	}

	ti81xxfb_unlock(tfbi);
	return r;
}

static int ti81xxfb_set_scparams(struct fb_info *fbi,
				struct ti81xxfb_scparams *scp)
{
	int r = 0;
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct vps_grpxscparams  gscp;
	ti81xxfb_lock(tfbi);
	/* check the scparamter first */

	gscp.horfineoffset = 0;
	gscp.verfineoffset = 0;
	gscp.sccoeff = NULL;
	gscp.inwidth = scp->inwidth;
	gscp.inheight = scp->inheight;
	gscp.outwidth = scp->outwidth;
	gscp.outheight = scp->outheight;
	gscp.sccoeff = scp->coeff;
	r = gctrl->set_scparams(gctrl, &gscp);
	if (0 == r) {
		if (gctrl->gstate.isstarted)
			r = vps_fvid2_queue(gctrl->handle,
					    (struct fvid2_framelist *)
						gctrl->frmls_phy,
					    0);
	}
	ti81xxfb_unlock(tfbi);

	return r;
}

static int ti81xxfb_get_scparams(struct fb_info *fbi,
				struct ti81xxfb_scparams *scp)
{
	int r = 0;
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct vps_grpxscparams gscp;
	ti81xxfb_lock(tfbi);
	r = gctrl->get_scparams(gctrl, &gscp);
	if (r == 0) {
		scp->inwidth = gscp.inwidth;
		scp->inheight = gscp.inheight;
		scp->outwidth = gscp.outwidth;
		scp->outheight = gscp.outheight;
	}
	ti81xxfb_unlock(tfbi);

	return r;

}
static int ti81xxfb_allocate_mem(struct fb_info *fbi,
				     u32 size,
				     dma_addr_t *paddr)
{
	struct ti81xxfb_alloc_list *mlist;
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	mlist = kzalloc(sizeof(*mlist), GFP_KERNEL);

	if (mlist == NULL) {
		*paddr = 0;
		return -ENOMEM;
	}
	ti81xxfb_lock(tfbi);
	/* allocate the buffer from the pool*/
	mlist->size = PAGE_ALIGN(size);
	mlist->virt_addr = (void *)ti81xx_vram_alloc(TI81XXFB_MEMTYPE_SDRAM,
					     (size_t)size,
					     (unsigned long *)&mlist->phy_addr);
	if (mlist->virt_addr == NULL) {
		kfree(mlist);
		*paddr = 0;
		ti81xxfb_unlock(tfbi);
		return -ENOMEM;
	}
	/* add into the list*/
	list_add(&mlist->list, &tfbi->alloc_list);
	TFBDBG("Sten allocated %d bytes @ 0x%08X\n",
			mlist->size, mlist->phy_addr);

	*paddr = mlist->phy_addr;
	ti81xxfb_unlock(tfbi);
	return 0;

}

static int ti81xxfb_free_mem(struct fb_info *fbi, int offset)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct ti81xxfb_alloc_list *mlist;
	u32 stenaddr;
	u32 stride;
	int r = -EINVAL;

	/* check to make sure that the  sten buffer to
	be freeed is not the one being used*/
	ti81xxfb_lock(tfbi);
	r = gctrl->get_stenparams(gctrl, &stenaddr, &stride);
	if ((r == 0) && (stenaddr != 0) && (gctrl->gstate.isstarted)) {
		struct vps_grpxregionparams regp;

		r = gctrl->get_regparams(gctrl, &regp);
		if (r == 0)
			if ((stenaddr == offset) &&
				(regp.stencilingenable == 1))
				return -EINVAL;
	}
	/* loop the list to find out the offset and free it*/
	list_for_each_entry(mlist, &tfbi->alloc_list, list) {
		if (mlist->phy_addr == offset) {
			r = ti81xx_vram_free(mlist->phy_addr,
					 mlist->virt_addr,
					 mlist->size);
			if (r == 0) {
				list_del(&mlist->list);
				kfree(mlist);
			} else
				dev_err(tfbi->fbdev->dev,
				       "failed to free mem %x\n",
				       mlist->phy_addr);
			break;
		}
	}

	/* if the buffer to be free is the one used previous,
	   set the sten ptr to NULL */
	if ((r == 0) && (offset == stenaddr))
		gctrl->set_stenparams(gctrl, 0, stride);


	ti81xxfb_unlock(tfbi);
	return r;
}

static int ti81xxfb_set_sten(struct fb_info *fbi,
			    struct ti81xxfb_stenciling_params *stparams)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	struct ti81xxfb_alloc_list *mlist;
	bool found = false;
	int r = 0;
	int offset = stparams->paddr;

	/* loop the list to find out the offset*/
	list_for_each_entry(mlist, &tfbi->alloc_list, list) {
		if (mlist->phy_addr == offset) {
			found = true;
			break;
		}
	}

	if (found == false) {
		dev_err(tfbi->fbdev->dev,
			"buffer not allocated by driver.\n");
		return -EINVAL;
	}

	if (stparams->pitch & 0xF) {
		dev_err(tfbi->fbdev->dev,
			"stride should be 16 byte boundry.\n");
		return -EINVAL;
	}

	ti81xxfb_lock(tfbi);
	r = gctrl->set_stenparams(gctrl, offset, stparams->pitch);
	if ((r == 0) && (gctrl->gstate.isstarted))
		vps_fvid2_queue(gctrl->handle,
				(struct fvid2_framelist *)gctrl->frmls_phy,
				0);
	ti81xxfb_unlock(tfbi);
	return r;

}

static int ti81xxfb_setup_mem(struct fb_info *fbi, struct ti81xxfb_mem_info *mi)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct ti81xxfb_device *fbdev = tfbi->fbdev;
	struct ti81xxfb_mem_region *rg;
	int r;
	size_t size;

	if (mi->type != TI81XXFB_MEM_NONTILER)
		return -EINVAL;

	size = PAGE_ALIGN(mi->size);

	rg = &tfbi->mreg;
	ti81xxfb_lock(tfbi);

	/*make sure that the Fb is not used */
	if (tfbi->gctrl->gstate.isstarted)
		return -EBUSY;

	if (rg->size != size || tfbi->mmode != mi->type) {
		r = ti81xxfb_realloc_fbmem(fbi, size);
		if (r) {
			dev_err(fbdev->dev, "realloc fbmem failed\n");
			goto out;
		}
	}

	r = 0;
out:
	ti81xxfb_unlock(tfbi);

	return r;
}


static int ti81xxfb_query_mem(struct fb_info *fbi, struct ti81xxfb_mem_info *mi)
{
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct ti81xxfb_mem_region *rg;

	rg = &tfbi->mreg;
	memset(mi, 0, sizeof(*mi));

	ti81xxfb_lock(tfbi);
	mi->size = rg->size;
	mi->type = tfbi->mmode;
	ti81xxfb_unlock(tfbi);

	return 0;
}

int ti81xxfb_ioctl(struct fb_info *fbi, unsigned int cmd,
		  unsigned long arg)
{
	union {
		struct ti81xxfb_region_params regparams;
		struct ti81xxfb_scparams scparams;
		struct ti81xxfb_mem_info   minfo;
		struct ti81xxfb_stenciling_params stparams;
		struct omapfb_plane_info opinfo;
		struct omapfb_vram_info   ovinfo;
		int mirror;
		int size;
		int offset;
	} param;
	struct ti81xxfb_info *tfbi = FB2TFB(fbi);
	struct ti81xxfb_device *fbdev = tfbi->fbdev;
	struct vps_grpx_ctrl *gctrl = tfbi->gctrl;
	int r = 0;

	if (!tfbi->mreg.size) {
		dev_err(fbdev->dev, "alloce fb memory first\n");
		return -EINVAL;
	}

	switch (cmd) {

	case FBIO_WAITFORVSYNC:
		TFBDBG("ioctl WAITFORVSYNC\n");
		r = gctrl->wait_for_vsync(gctrl);
		break;

	case TIFB_SET_PARAMS:
		TFBDBG("ioctl SET_PARAMS\n");

		if (copy_from_user(&param.regparams, (void __user *)arg,
				   sizeof(struct ti81xxfb_region_params)))
			r = -EFAULT;
		else
			r = ti81xxfb_set_region_params(fbi, &param.regparams);

		break;

	case TIFB_GET_PARAMS:
		TFBDBG("ioctl GET_PARAMS\n");
		r = ti81xxfb_get_region_params(fbi, &param.regparams);

		if (r < 0)
			break;

		if (copy_to_user((void __user *)arg, &param.regparams,
				 sizeof(param.regparams)))
			r = -EFAULT;
		break;

	case TIFB_SET_SCINFO:
		TFBDBG("ioctl SET_SCINFO\n");
		if (copy_from_user(&param.scparams, (void __user *)arg,
				   sizeof(param.scparams))) {
			r = -EFAULT;
			break;
		}
		if (param.scparams.coeff) {
			struct ti81xxfb_coeff *coeff =
				kzalloc(sizeof(struct ti81xxfb_coeff),
					GFP_KERNEL);

			TFBDBG("loading app's coeff\n");
			if (copy_from_user(coeff,
					   (void __user *)param.scparams.coeff,
					    sizeof(struct ti81xxfb_coeff))) {
				kfree(coeff);
				r = -EFAULT;
				break;
			}
			param.scparams.coeff = coeff;

		}
		r = ti81xxfb_set_scparams(fbi, &param.scparams);
		kfree(param.scparams.coeff);
		break;

	case  TIFB_GET_SCINFO:
		TFBDBG("ioctl GET_SCINFO");
		r = ti81xxfb_get_scparams(fbi, &param.scparams);
		if (r == 0) {
			struct ti81xxfb_scparams scp;
			/*keep the coeff pointer in the strucutre and
			  do not change it*/
			if (copy_from_user(&scp,
					(void __user *)arg,
					sizeof(param.scparams))) {
				r = -EFAULT;
				break;
			}
			/*store the coeff back to app*/
			param.scparams.coeff = scp.coeff;

			if (copy_to_user((void __user *)arg, &param.scparams,
					 sizeof(param.scparams)))
				r = -EFAULT;
		}


		break;

	case TIFB_ALLOC:
	{
		dma_addr_t paddr;
		TFBDBG("ioctl ALLOC_STEN\n");

		if (get_user(param.size, (int __user *)arg)) {
			r = -EFAULT;
			break;
		}

		r = ti81xxfb_allocate_mem(fbi, param.size, &paddr);
		if ((r == 0) && (paddr != 0)) {
			if (put_user(paddr, (int __user *)arg))
				r = -EFAULT;
		}
		break;
	}

	case TIFB_FREE:
		TFBDBG("ioctl FREE_STENC\n");
		if (get_user(param.offset, (int __user *)arg)) {
			r = -EFAULT;
			break;
		}

		r = ti81xxfb_free_mem(fbi, param.offset);
		break;

	/*compatible with OMAP*/
	case OMAPFB_SETUP_MEM:
	case TIFB_SETUP_MEM:
		TFBDBG("ioctl SETUP_MEM\n");
		if (copy_from_user(&param.minfo, (void __user *)arg,
					sizeof(param.minfo)))
			r = -EFAULT;
		else
			r = ti81xxfb_setup_mem(fbi, &param.minfo);
		break;

	/*compatible with OMAP*/
	case OMAPFB_QUERY_MEM:
	case TIFB_QUERY_MEM:
		TFBDBG("ioctl QUERY_MEM\n");
		r = ti81xxfb_query_mem(fbi, &param.minfo);
		if (r < 0)
			break;
		if (copy_to_user((void __user *)arg, &param.minfo,
					sizeof(param.minfo)))
			r = -EFAULT;

		break;
	case TIFB_SET_STENC:
		TFBDBG("ioctl SET_STEN.\n");
		if (copy_from_user(&param.stparams, (int __user *)arg,
				   sizeof(param.stparams))) {
			r = -EFAULT;
			break;
		}
		r = ti81xxfb_set_sten(fbi, &param.stparams);
		break;
	/*compatible with OMAP*/
	case OMAPFB_SETUP_PLANE:
		TFBDBG("ioctl SETUP_PLANE\n");
		if (copy_from_user(&param.opinfo, (void __user *)arg,
					sizeof(param.opinfo)))
			r = -EFAULT;
		else
			r = ti81xxfb_setup_plane(fbi, &param.opinfo);
		break;
	/*compatible with OMAP*/
	case OMAPFB_QUERY_PLANE:
		TFBDBG("ioctl QUERY_PLANE\n");
		r = ti81xxfb_query_plane(fbi, &param.opinfo);
		if (r < 0)
			break;
		if (copy_to_user((void __user *)arg, &param.opinfo,
					sizeof(param.opinfo)))
			r = -EFAULT;
		break;
	/*compatible with OMAP*/
	case OMAPFB_GET_VRAM_INFO:
		TFBDBG("ioctl GET_VRAM_INFO\n");
		r = ti81xxfb_get_vram_info(fbi, &param.ovinfo);
		if (copy_to_user((void __user *)arg, &param.ovinfo,
					sizeof(param.ovinfo)))
			r = -EFAULT;
		break;

	default:
		dev_err(FB2TFB(fbi)->fbdev->dev, "Unknown ioctl 0x%x\n", cmd);
		r = -EFAULT;
		break;
	}

	if (r < 0)
		TFBDBG("ioctl 0x%x failed: %d\n", cmd , r);

	return r;
}



