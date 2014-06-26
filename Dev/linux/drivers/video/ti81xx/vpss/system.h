/*
 * linux/drivers/video/ti81xx/vpss/system.h
 *
 * VPSS system-platform  driver for TI 81XX
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
#ifndef __DRIVERS_VIDEO_TI81XX_VPSS_SYSTEM_H__
#define __DRIVERS_VIDEO_TI81XX_VPSS_SYSTEM_H__


/**< Platform driver Id */
#define FVID2_VPS_VID_SYSTEM_DRV          (VPS_VID_SYSTEM_DRV_BASE + 0x0000)

/**
  * \addtogroup FVID2_VPS_VID_SYSTEM_DRV
  * @{
*/

/**
  * \brief ioclt for setting video pll.
  *
  *
  * \param cmdArgs       [IN]   Vps_SystemVPllClk*
  * \param cmdArgsStatus [OUT]  NULL
  *
  * \return FVID_SOK on success, else failure.
  *
*/
#define IOCTL_VPS_VID_SYSTEM_SET_VIDEO_PLL                                     \
		(VPS_VID_SYSTEM_IOCTL_BASE + 0x0000)

/**
  * \brief ioclt for getting current video settings.
  *
  *
  * \param cmdArgs       [OUT]  Vps_SystemVPllClk*
  * \param cmdArgsStatus [OUT]  NULL
  *
  * \return FVID_SOK on success, else failure.
  *
*/
#define IOCTL_VPS_VID_SYSTEM_GET_VIDEO_PLL                               \
	(VPS_VID_SYSTEM_IOCTL_BASE + 0x0001)

/**
  * \brief ioclt for getting platform Id.
  *
  *
  * \param cmdArgs       [IN]   UInt32*
  * \param cmdArgsStatus [OUT]  NULL
  *
  * \return FVID_SOK on success, else failure.
  *
*/
#define IOCTL_VPS_VID_SYSTEM_GET_PLATFORM_ID                             \
	(VPS_VID_SYSTEM_IOCTL_BASE + 0x0002)

/* @} */

/**
 *  \brief Enum for the output clock modules
 *  Caution: Do not change the enum values.
 */
enum vps_vplloutputclk {
	VPS_SYSTEM_VPLL_OUTPUT_VENC_RF = 0,
	/**< Pixel clock frequency for the RF,
	 Note: SD Pixel frequency will RF Clock Freq/4. */
	VPS_SYSTEM_VPLL_OUTPUT_VENC_D,
	/**< VencD output clock. */
	VPS_SYSTEM_VPLL_OUTPUT_VENC_A,
	/**< VencA output clock. */
	VPS_SYSTEM_VPLL_OUTPUT_MAX_VENC
	/**< This should be last Enum. */
};

/**
 *  \brief Platform ID.
 */
enum vps_platformid {
	VPS_PLATFORM_ID_UNKNOWN,
	/**< Unknown platform. */
	VPS_PLATFORM_ID_EVM_TI816x,
	/**< TI816x EVMs. */
	VPS_PLATFORM_ID_SIM_TI816x,
	/**< TI816x Simulator. */
	VPS_PLATFORM_ID_EVM_TI814x,
	/**< TI814x EVMs. */
	VPS_PLATFORM_ID_SIM_TI814x,
	/**< TI814x Simulator. */
	VPS_PLATFORM_ID_MAX
	/**< Max Platform ID. */
};

/**
 *  \brief CPU revision ID.
 */
enum  vps_platformcpurev {
	VPS_PLATFORM_CPU_REV_1_0,
	/**< CPU revision 1.0. */
	VPS_PLATFORM_CPU_REV_1_1,
	/**< CPU revision 1.1. */
	VPS_PLATFORM_CPU_REV_2_0,
	/**< CPU revision 2.0. */
	VPS_PLATFORM_CPU_REV_UNKNOWN,
	/**< Unknown/unsupported CPU revision. */
	VPS_PLATFORM_CPU_REV_MAX
	/**< Max CPU revision. */
};

/**
 *  \brief EVM Board ID.
 */
enum vps_platformboardid {
	VPS_PLATFORM_BOARD_UNKNOWN,
	/**< Unknown board */
	VPS_PLATFORM_BOARD_VS,
	/**< TVP5158 based board */
	VPS_PLATFORM_BOARD_VC,
	/**< TVP7002/SII9135 based board */
	VPS_PLATFORM_BOARD_MAX
	/**< Max board ID */
} ;



struct vps_systemvpllclk {
	u32 outputvenc;
	/**< Select output venc for which video pll is configured.
	 See #vps_vplloutputclk  See for all possible values */
	u32 outputclk;
	/**< Pixel clock for Venc.*/
} ;

int vps_system_setpll(struct vps_systemvpllclk *pll);
int vps_system_getpll(struct vps_systemvpllclk *pll);
int vps_system_getplatformid(u32 *pid);
static inline enum vps_platformcpurev  vps_system_getcpurev(void)
{
	u32 devid, cpurev;
	devid = omap_readl(0x48140600);

	cpurev = (devid >> 28) & 0xF;

	switch (cpurev) {
	case 0:
		return VPS_PLATFORM_CPU_REV_1_0;
	case 1:
		if (cpu_is_ti816x())
			return VPS_PLATFORM_CPU_REV_1_1;
		else
			return VPS_PLATFORM_CPU_REV_MAX;
	case 2:
		return VPS_PLATFORM_CPU_REV_MAX;
	default:
		return VPS_PLATFORM_CPU_REV_MAX;
	}

}

#endif
