/*
 * include/linux//ti81xxhdmi.c
 *
 * Copyright (C) 2009 Texas Instruments
 *
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
 * Author: Sujith Shivalingappa <sujith.s@ti.com>
 */

#ifndef __TI81XXHDMI_H__
#define __TI81XXHDMI_H__


#include <linux/ioctl.h>

/* IOCTL definitions */
#define TI81XXHDMI_MAGIC            'N'
#define TI81XXHDMI_IOW(num, dtype)  _IOW(TI81XXHDMI_MAGIC, num, dtype)
#define TI81XXHDMI_IOR(num, dtype)  _IOR(TI81XXHDMI_MAGIC, num, dtype)
#define TI81XXHDMI_IOWR(num, dtype) _IOWR(TI81XXHDMI_MAGIC, num, dtype)
#define TI81XXHDMI_IO(num)          _IO(TI81XXHDMI_MAGIC, num)

/* IOCLT Supported by this driver */
#define TI81XXHDMI_START      		TI81XXHDMI_IO(0)
#define TI81XXHDMI_STOP       		TI81XXHDMI_IO(1)
#define TI81XXHDMI_GET_STATUS 		TI81XXHDMI_IOR(2, int *)
#define TI81XXHDMI_READ_EDID  		TI81XXHDMI_IOR(3, struct ti81xxdhmi_edid_params)
/* Use this command only when hdmi is streaming video to a sink */
/* TODO Not supported for now */
#if 0
#define TI81XXHDMI_GET_CONFIG 		TI81XXHDMI_IOR(4, struct hdmi_cfg_params)
#define TI81XXHDMI_SET_CONFIG 		TI81XXHDMI_IOW(5, struct hdmi_cfg_params)
#endif
#define TI81XXHDMI_SET_MODE         	TI81XXHDMI_IOW(6, enum ti81xxhdmi_mode)
#define TI81XXHDMI_GET_MODE         	TI81XXHDMI_IOR(7, enum ti81xxhdmi_mode)
#define TI81XXHDMI_TEST_HDMI        	TI81XXHDMI_IOW(8, enum ti81xxhdmi_mode)
#define TI81XXHDMI_GET_PHY_STAT		TI81XXHDMI_IOR(9, enum ti81xxhdmi_mode)
/* Data Types */

/* Supported resolutions */
enum ti81xxhdmi_mode {
	hdmi_ntsc_mode = 0,
	hdmi_pal_mode,
	hdmi_1080P_60_mode,
	hdmi_720P_60_mode,
	hdmi_1080I_60_mode,
	hdmi_1080P_30_mode,
	hdmi_max_mode
};
struct ti81xxdhmi_edid_params {
	unsigned int slave_address;
	unsigned int segment_ptr;
	unsigned int offset;
	unsigned int no_of_bytes;
	void *buffer_ptr;
	unsigned int no_of_bytes_read;
	unsigned int timeout;
	unsigned int use_eddc_read;
};
struct ti81xxhdmi_phy_status {
	unsigned int rst_done_pclk;
	unsigned int rst_done_pwrclk;
	unsigned int rst_done_scpclk;
	unsigned int rst_done_refclk;
	unsigned int dct_5v_short_clk;
	unsigned int rx_detect;
	unsigned int dct_5v_short_data;
};
struct ti81xxhdmi_status {
	unsigned int is_hpd_detected;
	unsigned int is_hdmi_streaming;
};
#endif /* End of #ifndef __TI81XXHDMI_H__ */

