/*
 * linux/drivers/video/ti81xx/vpss/system.c
 *
 * system PLL driver for TI81XX Platform
 *
 * Copyright (C) 2010 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
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
#define VPSS_SUBMODULE_NAME   "SYSTEM"

#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include "system.h"
#include "core.h"


struct vps_system_ctrl {
	u32       *handle;
	struct vps_systemvpllclk   *pllclk;
	u32                        pllclk_phy;
	u32                        *pid;
	u32                        pid_phy;
};

static struct vps_payload_info   *system_payload_info;
static struct vps_system_ctrl   *sys_ctrl;

static inline bool isvalidpllclk(struct vps_systemvpllclk *pllclk)
{
	if (pllclk->outputclk == 0)
		return false;

	switch (pllclk->outputvenc) {
	case VPS_SYSTEM_VPLL_OUTPUT_VENC_RF:
		if (cpu_is_ti816x()) {
			if (pllclk->outputclk != 216000)
				return false;
		} else {
			if (pllclk->outputclk != 54000)
				return false;
		}
		break;
	case VPS_SYSTEM_VPLL_OUTPUT_VENC_A:
	case VPS_SYSTEM_VPLL_OUTPUT_VENC_D:
		if (cpu_is_ti814x()) {
			if (pllclk->outputclk == 54000)
				return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

int vps_system_setpll(struct vps_systemvpllclk *pll)
{
	int r;
	struct vps_system_ctrl *sctrl = sys_ctrl;

	if ((sctrl == NULL) || (sctrl->handle == NULL))
		return 0;

	VPSSDBG("enter set pll %dKHz for VENC %d\n",
		pll->outputclk, pll->outputvenc);

	/*set HDMI pll by HDMI Driver*/
	if (cpu_is_ti814x() &&
		(pll->outputvenc == VPS_SYSTEM_VPLL_OUTPUT_VENC_A))
		return 0;

	if (!isvalidpllclk(pll))
		return -EINVAL;

	*sctrl->pllclk = *pll;

	/*if it is pg1 and ti816x and digital clock source, double it*/
	if (cpu_is_ti816x() &&
	    (VPS_PLATFORM_CPU_REV_1_0 == vps_system_getcpurev())
	    && (VPS_SYSTEM_VPLL_OUTPUT_VENC_D == sctrl->pllclk->outputvenc))
		sctrl->pllclk->outputclk <<= 1;

	r = vps_fvid2_control(sctrl->handle,
			      IOCTL_VPS_VID_SYSTEM_SET_VIDEO_PLL,
			      (void *)sctrl->pllclk_phy,
			      NULL);

	if (r)
		VPSSERR("set pll failed\n");

	return r;

}

int vps_system_getpll(struct vps_systemvpllclk *pll)
{
	int r = 0;
	struct vps_system_ctrl *sctrl = sys_ctrl;

	if ((sctrl == NULL) || (sctrl->handle == NULL))
		return 0;
	VPSSDBG("getpll\n");

	sctrl->pllclk->outputvenc = pll->outputvenc;

	r = vps_fvid2_control(sctrl->handle,
			       IOCTL_VPS_VID_SYSTEM_GET_VIDEO_PLL,
			       (void *)sctrl->pllclk_phy,
			       NULL);


	if (r)
		VPSSERR("get pll failed\n");
	else {
		if (cpu_is_ti816x() &&
		    (VPS_PLATFORM_CPU_REV_1_0 == vps_system_getcpurev())
		    && (sctrl->pllclk->outputvenc ==
		    VPS_SYSTEM_VPLL_OUTPUT_VENC_D))
			sctrl->pllclk->outputclk >>= 1;

		*pll = *sctrl->pllclk;
	}
	return r;
}

int vps_system_getplatformid(u32 *pid)
{
	int r;
	struct vps_system_ctrl *sctrl = sys_ctrl;

	if ((sctrl == NULL) || (sctrl->handle == NULL))
		return 0;

	r = vps_fvid2_control(sctrl->handle,
			      IOCTL_VPS_VID_SYSTEM_GET_PLATFORM_ID,
			      (char *)sctrl->pid_phy,
			      NULL);
	if (r)
		VPSSERR("failed to get platform id\n");
	else
		*pid = *sctrl->pid;

	return r;

}
static u32 system_create(struct vps_system_ctrl *sctrl)
{
	if (sctrl == NULL)
		return -EINVAL;

	sctrl->handle = vps_fvid2_create(FVID2_VPS_VID_SYSTEM_DRV,
					 0,
					 NULL,
					 NULL,
					 NULL);

	if (sctrl->handle == NULL) {
		VPSSERR("failed to create handle\n");
		return -EINVAL;
	}

	return 0;
}

static u32 system_delete(struct vps_system_ctrl *sctrl)
{
	int r;
	if ((sctrl == NULL) || (sctrl->handle == NULL))
		return 0;

	r = vps_fvid2_delete(sctrl->handle, NULL);

	if (r)
		VPSSERR("failed to delete handle\n");

	return r;
}

static inline int get_payload_size(void)
{
	int size = 0;

	size += sizeof(struct vps_systemvpllclk);
	size += sizeof(enum vps_platformid);

	return size;
}

static inline void assign_payload_addr(struct vps_system_ctrl *sctrl,
				       struct vps_payload_info *pinfo,
				       u32 *buf_offset)
{

	sctrl->pllclk = (struct vps_systemvpllclk *)
			setaddr(pinfo,
				buf_offset,
				&sctrl->pllclk_phy,
				sizeof(struct vps_systemvpllclk));

	sctrl->pid = (u32 *)setaddr(pinfo,
				    buf_offset,
				    &sctrl->pid_phy,
				    sizeof(u32));

}


int __init vps_system_init(struct platform_device *pdev)
{
	int size;
	int r = 0;
	u32 offset = 0;
	struct vps_payload_info  *pinfo;
	enum vps_platformid   pid = VPS_PLATFORM_ID_MAX;

	VPSSDBG("enter system init\n");
	/*allocate payload info*/
	system_payload_info = kzalloc(sizeof(struct vps_payload_info),
				       GFP_KERNEL);

	if (!system_payload_info) {
		VPSSERR("failed to allocate payload structure\n");
		return -ENOMEM;
	}
	pinfo = system_payload_info;

	/*allocate system control*/
	sys_ctrl = kzalloc(sizeof(struct vps_system_ctrl), GFP_KERNEL);
	if (sys_ctrl == NULL) {
		VPSSERR("failed to allocate control\n");
		r = -ENOMEM;
		goto exit;

	}

	/*allocate shared payload buffer*/
	size = get_payload_size();
	pinfo->vaddr = vps_sbuf_alloc(size, &pinfo->paddr);
	if (pinfo->vaddr == NULL) {
		VPSSERR("failed to allocate payload\n");
		 r = -EINVAL;
		goto exit;
	}

	pinfo->size = PAGE_ALIGN(size);
	assign_payload_addr(sys_ctrl, pinfo, &offset);
	memset(pinfo->vaddr, 0, pinfo->size);

	r = system_create(sys_ctrl);
	if (r)
		goto exit;

	r = vps_system_getplatformid(&pid);
	if (r)
		goto exit;

	if (cpu_is_ti816x()) {
		if (!((pid == VPS_PLATFORM_ID_EVM_TI816x) ||
			(pid == VPS_PLATFORM_ID_SIM_TI816x))) {
			VPSSERR("TI816X EVM with TI814X M3 firmware,"
				"please use TI816x M3 firmware\n");
			goto exit;
		}
	} else {
		if (!((pid == VPS_PLATFORM_ID_EVM_TI814x) ||
			(pid == VPS_PLATFORM_ID_SIM_TI814x))) {
			VPSSERR("TI814X EVM with TI816X M3 firmware,"
				" please use TI814x M3 firmware\n");
			goto exit;
		}
	}

	return 0;
exit:
	vps_system_deinit(pdev);
	return r;

}

int __exit vps_system_deinit(struct platform_device *pdev)
{
	int r = 0;

	VPSSDBG("enter system deinit\n");
	if (sys_ctrl) {
		system_delete(sys_ctrl);
		kfree(sys_ctrl);
		sys_ctrl = NULL;
	}

	if (system_payload_info) {
		if (system_payload_info->vaddr) {
			vps_sbuf_free(system_payload_info->paddr,
				      system_payload_info->vaddr,
				      system_payload_info->size);
		}
		kfree(system_payload_info);
		system_payload_info = NULL;
	}
	return r;
}
