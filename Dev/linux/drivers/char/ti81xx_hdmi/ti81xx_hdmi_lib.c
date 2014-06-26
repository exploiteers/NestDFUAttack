/*
 * linux/drivers/char/ti81xx_hdmi/hdmi_lib.c
 *
 * Copyright (C) 2010 Texas Instruments
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
 */

/**
 * Key notes
 * 1. Wrapper doesn't generate interrupts for all the events, generates for HPD.
 *	  Using core interrupt instead.
 * 2. DVI mode is not configurable, operates in HDMI mode only, control in
 *	  HDMI_CTRL
 * 3. The Core system should operate as a SLAVE. MASTER/SLAVE mode depends on
 *	  core/wrapper integration.
 *
 */

/*
 * Open items
 * 1. Handle DDC bus hangups / lockups during EDID Read [Done]
 * 2. use copy to user and copy from user
 */


/* ========================================================================== */
/*	Include Files							      */
/* ========================================================================== */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/ti81xxhdmi.h>

#include "ti81xx_hdmi_cfg.h"
#include "ti81xx_hdmi_regoffsets.h"
#include <asm/io.h>

/* ========================================================================== */
/*	Local Configurations						      */
/* ========================================================================== */
#define HDMI_DDC_CMD_TIMEOUT (0xFFFFFu)
/* Timeout periods used to wait for a DDC operation to complete */
#define HDMI_WP_RESET_TIMEOUT (0xFFFFFu)
/* Timeout periods used to wait for a DDC opeation to complete */
#define HDMI_PHY_2_WP_PLL_LOCK_TIMEOUT (0xFFFFFu)
/* Timeout periods used to wait TCLK to stabilize - TCLK would be generated
   by PHY to operate wrapper */

/* ========================================================================== */
/*	Local Defines							      */
/* ========================================================================== */

#define HDMI_CTRL_PACKET_MODE_24BITS_PIXEL	(0x4u)
/* Defines used to configure the number of bits/pixel that would sent to
   packetizer */
#define HDMI_CTRL_PACKET_MODE_30BITS_PIXEL	(0x5u)
/* Defines used to configure the number of bits/pixel that would sent to
   packetizer */
#define HDMI_CTRL_PACKET_MODE_36BITS_PIXEL	(0x6u)
/* Defines used to configure the number of bits/pixel that would sent to
   packetizer */
#define HDMI_VID_MODE_DITHER_TO_24_BITS_MODE (0x0u)
/* Defines to used to determine the dithering width */
#define HDMI_VID_MODE_DITHER_TO_30_BITS_MODE (0x1u)
/* Defines to used to determine the dithering width */
#define HDMI_VID_MODE_DITHER_TO_36_BITS_MODE (0x2u)
/* Defines to used to determine the dithering width */
#define HDMI_TMDS_CTRL_IP_CLOCK_MULTIPLIER_AUDIO	(0x1u)
/* Defines the multiplier value used to multiply the input clock IDCK, in order
   to support higher sampling rates / channels audio */
#define HDMI_AVI_INFOFRAME_PKT_TYPE 	(0x82u)
/* AVI Info frame header - packet type - defined by standard */
#define HDMI_AVI_INFOFRAME_PKT_VER		(0x02)
/* AVI Info frame header - packet version - defined by standard */
#define HDMI_AVI_INFOFRAME_PKT_LEN		(0x0D)
/* AVI Info frame header - packet version - defined by standard */
#define HDMI_AVI_INFOFRAME_Y0_Y1_MASK		(0x60u)
/* Mask to set/extract Y0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_A0_MASK		(0x10u)
/* Mask to set/extract A0 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_B0_B1_MASK		(0x0Cu)
/* Mask to set/extract B0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_S0_S1_MASK		(0x03u)
/* Mask to set/extract S0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_C0_C1_MASK		(0xC0u)
/* Mask to set/extract C0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_M0_M1_MASK		(0x30u)
/* Mask to set/extract M0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_R0_R3_MASK		(0x0Fu)
/* Mask to set/extract R0-3 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_ITC_MASK 	(0x80u)
/* Mask to set/extract ITC bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_EC2_EC0_MASK 	(0x70u)
/* Mask to set/extract EC0-3 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_Q1_Q0_MASK		(0x0Cu)
/* Mask to set/extract Q0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_SC1_SC0_MASK 	(0x03u)
/* Mask to set/extract SC0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_VIC6_VIC0_MASK	(0x7Fu)
/* Mask to set/extract VIC6-0 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_PR3_PR0_MASK 	(0x0Fu)
/* Mask to set/extract PR3-0 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_CONST_0x100		(0x100u)
/* Constant used to calculate AVI info frame checksum */
#define HDMI_MINIMUM_PIXELS_SEC 		(25000000u)
/* HDMI standard Mandates that at a minimum there should be 25 MPixels/sec. */
#define HDMI_PIXEL_REPLECATED_ONCE		(0x2)
/* Each pixel would be sent twice */
#define HDMI_PIXEL_REPLECATED_FOUR_TIMES	(0x4)
/* Each pixel would be sent four times */

/* Standard resolutions column X row X FPS */
#define HDMI_VIDEO_STAND_NTSC			(858 * 525 * 30)
#define HDMI_VIDEO_STAND_PAL			(858 * 625 * 25)
#define HDMI_VIDEO_STAND_720P60 		(1650 * 750 * 60)
#define HDMI_VIDEO_STAND_1080P60		(2200 * 1125 * 60)
#define HDMI_VIDEO_STAND_1080I60		(2200 * 1125 * 30)
#define HDMI_VIDEO_STAND_1080P30		(2200 * 1125 * 30)
/* Undef this to test HDMI */
//#define HDMI_TEST				(1)
/* ========================================================================== */
/*				Local Structure 			      */
/* ========================================================================== */

struct instance_cfg {
	u32 instance;
	u32 core_base_addr;
	u32 wp_base_addr;
	u32 phy_base_addr;
	u32 prcm_base_addr;
	u32 venc_base_addr;
	bool is_recvr_sensed;
	bool is_scl_clocked;
	bool is_streaming;
	struct hdmi_cfg_params config;
	u32 vSync_counter;
	bool is_interlaced;
	enum ti81xxhdmi_mode hdmi_mode;
	u32 hdmi_pll_base_addr;
};

/* ========================================================================== */
/*			   Local Function Declarations			      */
/* ========================================================================== */
static int configure_phy(struct instance_cfg *inst_context);
static int configure_wrapper(struct instance_cfg *inst_context);
static int configure_core_input(struct instance_cfg *inst_context);
static int configure_core_data_path(struct instance_cfg *inst_context);
static int configure_core(struct instance_cfg *inst_context);
static int configure_policies(struct instance_cfg *inst_context);
static int configure_avi_info_frame(struct instance_cfg *inst_context);
static int configure_ctrl_packets(struct instance_cfg *inst_context);
static int configure_csc_ycbcr_rgb(struct instance_cfg *inst_context);

static int validate_info_frame_cfg(struct hdmi_info_frame_cfg *config);
static int validate_core_config(struct hdmi_core_input_cfg *config);
static int validate_wp_cfg(struct hdmi_wp_config *config);
static int validate_path_config(struct hdmi_core_data_path *config);
static int check_copy_config(struct instance_cfg *inst_cntxt,
		struct hdmi_cfg_params *config);
int ti81xx_hdmi_set_mode(enum ti81xxhdmi_mode hdmi_mode,
		struct instance_cfg *cfg);
int ti81xx_hdmi_copy_mode_config(enum ti81xxhdmi_mode mode,
		struct instance_cfg *cfg);
static int determine_pixel_repeatation(struct instance_cfg *inst_context);


static int ti81xx_hdmi_lib_read_edid(void *handle,
		struct ti81xxdhmi_edid_params *r_params,
		void *args);
static int get_phy_status(struct instance_cfg *inst_context,
		struct ti81xxhdmi_phy_status *stat);
#if 0
static int ti81xx_hdmi_lib_get_cfg(void *handle,
		struct hdmi_cfg_params *config,
		void *args);
#endif
static void HDMI_ARGS_CHECK(u32 condition);
static int ti81xx_hdmi_lib_config(struct hdmi_cfg_params *config);

/* ========================================================================== */
/*				Global Variables			      */
/* ========================================================================== */
static struct instance_cfg hdmi_config;
/* Pool of HDMI objects */
static struct hdmi_cfg_params default_config =
TI81XX_HDMI_8BIT_1080p_60_16_9_HD;
/* Default configuration to start with */

struct hdmi_cfg_params config_1080p60 = TI81XX_HDMI_8BIT_1080p_60_16_9_HD;
struct hdmi_cfg_params config_720p60 = TI81XX_HDMI_8BIT_720_60_16_9_HD;
struct hdmi_cfg_params config_1080i60 = TI81XX_HDMI_8BIT_1080i_60_16_9_HD;
struct hdmi_cfg_params config_1080p30 = TI81XX_HDMI_8BIT_1080p_30_16_9_HD;

struct hdmi_pll_ctrl gpll_ctrl[] = {
	{19, 1485, 10, 0x20021001},
	{19, 745,   10, 0x20021001}
};

/* ========================================================================== */
/*				Local Functions 			      */
/* ========================================================================== */

#ifndef CONFIG_ARCH_TI816X
/* command
 * 0x0: Command to change LDO to OFF state
 * 0x1:	Command to change LDO to ON state
 * 0x2:	Command to go to LDO TXON Power
 */
static int wp_phy_pwr_ctrl(int wp_pwr_ctrl_addr, int command)
{
	volatile u32 reg_value;
	u32 cnt = 0;
	u32 max_count = 10000;
	int ret_val = 0;
	switch (command)
	{
		case 0x0:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PHY_PWR_CMD_MASK);
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0 &&  (cnt < max_count));
			if (reg_value != 0)
			{
				ret_val = -1;
			}
			break;
		case 0x1:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PHY_PWR_CMD_MASK);
			reg_value |= 0x1 << HDMI_WP_PWR_CTRL_PHY_PWR_CMD_SHIFT;
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while((reg_value >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x1 &&
					(cnt < max_count));
			if ((reg_value  >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x1)
			{
				ret_val = -1;
			}
			break;
		case  0x2:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PHY_PWR_CMD_MASK);
			reg_value |= 0x2 << HDMI_WP_PWR_CTRL_PHY_PWR_CMD_SHIFT;
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while((reg_value  >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x2 &&  (cnt < max_count));
			if ((reg_value >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x2)
			{
				ret_val = -1;
			}
			break;
		default:
			ret_val = -1;
	}
	return ret_val;
}

/* Command
 * 0x0: Command to change to OFF state
 * 0x1: Command to change to ON state for  PLL only (HSDIVISER is OFF)
 * 0x2: Command to change to ON state for both PLL and HSDIVISER
 * 0x3: Command to change to ON state for both PLL and HSDIVISER
 (no clock output to the DSI complex IO)
 */
static int wp_pll_pwr_ctrl(int wp_pwr_ctrl_addr, int command)
{
	volatile u32 reg_value;
	u32 cnt = 0;
	u32 max_count = 10000;
	int ret_val = 0;
	switch (command)
	{
		case 0x0:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0 &&  (cnt < max_count));
			if (reg_value != 0)
			{
				ret_val = -1;
			}
			break;
		case 0x1:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			reg_value |= 0x1 << HDMI_WP_PWR_CTRL_PLL_PWR_CMD_SHIFT;
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0x1 &&  (cnt < max_count));
			if (reg_value != 0x1)
			{
				ret_val = -1;
			}
			break;
		case  0x2:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			reg_value |= 0x2 << HDMI_WP_PWR_CTRL_PLL_PWR_CMD_SHIFT;
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0x2 &&  (cnt < max_count));
			if (reg_value != 0x2)
			{
				ret_val = -1;
			}
			break;
		case  0x3:
			reg_value = __raw_readl(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			reg_value |= 0x3 << HDMI_WP_PWR_CTRL_PLL_PWR_CMD_SHIFT;
			__raw_writel(reg_value, wp_pwr_ctrl_addr);
			cnt = 0;
			do
			{
				reg_value = __raw_readl(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0x3 &&  (cnt < max_count));
			if (reg_value != 0x3)
			{
				ret_val = -1;
			}
			break;

		default:
			ret_val = -1;
	}
	return ret_val;
}
#endif

#ifdef CONFIG_ARCH_TI816X
/*
 *	   This function is expected to be called when initializing or
 *	   when re-configuring. After re-configuration its recomended to reset the
 *	   core and wrapper. To stabilize the clocks, it recomended to wait for a
 *	   period of time.
 */
static int configure_phy(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	int phy_base;
	volatile u32 temp;

	THDBG(">>>>configure_phy\n");

	phy_base = inst_context->phy_base_addr;
	/* Steps
	 * 0. Power up if powered down
	 * 1. Determine the TCLK based in Deep color mode (Dither mode is used
	 *	  to get depth of the color) and Pixel repeatation (depends on deep
	 *	  color / resolution and audio). Turn OFF BIST pattern generator
	 * 2. Turn OFF BIST and DVI Encoder
	 * 3. Configure the source termination determination - we would require
	 *	  when the sink terminates the source - recomended by HDMI Spec 1.3A
	 *	  when operating higer frequencies
	 * 4. Enable the PHY
	 */
	temp = __raw_readl((phy_base + PHY_TMDS_CNTL3_OFFSET));
	if ((temp & HDMI_PHY_TMDS_CNTL3_PDB_MASK) !=
			HDMI_PHY_TMDS_CNTL3_PDB_MASK) {
		temp |= HDMI_PHY_TMDS_CNTL3_PDB_MASK;
		__raw_writel(temp, (phy_base + PHY_TMDS_CNTL3_OFFSET));
	}
	/* BIST Pattern generator is disabled - leave it at that */
	temp = __raw_readl((phy_base + PHY_TMDS_CNTL3_OFFSET));
	temp &= (~((HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_MASK) |
				(HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_MASK) |
				(HDMI_PHY_TMDS_CNTL3_BIST_SEL_MASK)));

	/* Step 1.1 - Output width of the dither module in core, determines
	 *			  deep color or not
	 */
	if (inst_context->config.core_path_config.output_width ==
			hdmi_10_bits_chan_width) {
		temp |= (HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_10BITCHANNEL <<
				HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_SHIFT);

	} else if (inst_context->config.core_path_config.output_width ==
			hdmi_8_bits_chan_width) {
		temp |= (HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_NO <<
				HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_SHIFT);
	} else {
		temp |= (HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_12BITCHANNEL <<
				HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_SHIFT);
	}

	rtn_value = determine_pixel_repeatation(inst_context);
	if (rtn_value == HDMI_PIXEL_REPLECATED_ONCE) {
		temp |= (HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_2_0X <<
				HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_SHIFT);
	} else if (rtn_value == HDMI_PIXEL_REPLECATED_FOUR_TIMES) {
		temp |= (HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_4_0X <<
				HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_SHIFT);
	} else if (rtn_value == 0x0) {
		temp |= (HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_1_0X <<
				HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_SHIFT);
	} else {
		THDBG("Could not calc pixel repeatation\n");
		THDBG("that would be required.\n");
		goto exit_this_func;
	}
	rtn_value = 0x0;
	__raw_writel(temp, (phy_base + PHY_TMDS_CNTL3_OFFSET));

	temp = __raw_readl((phy_base + PHY_BIST_CNTL_OFFSET));
	temp &= ~HDMI_PHY_BIST_CNTL_BIST_EN_MASK;
	temp |= HDMI_PHY_BIST_CNTL_ENC_BYP_MASK;
	__raw_writel(temp, (phy_base + PHY_BIST_CNTL_OFFSET));

	/* Since the 10bit encode is done by the core, we would require to
	   disable 10bit encode in the PHY. Do So */
	__raw_writel(0xE0, (phy_base + PHY_TMDS_CNTL9_OFFSET));

	/************************ PHY BIST Test @ half clock rate *********************/

#ifdef TEST_PHY_SEND_OUT_0xAA_AT_HALF_CLOCK_RATE_ON_ALL_DATA_LINES
	__raw_writel(0x40, (phy_base + PHY_BIST_CNTL_OFFSET));
	__raw_writel(0xE9, (phy_base + PHY_TMDS_CNTL3_OFFSET));
	__raw_writel(0x00, (phy_base + PHY_BIST_PATTERN_OFFSET));
	/* Program the instruction, pattern, configuration registers */
	__raw_writel(0x81, (phy_base + PHY_BIST_INST0_OFFSET));
	__raw_writel(0x00, (phy_base + PHY_BIST_CONF0_OFFSET));
	__raw_writel(0x20, (phy_base + PHY_BIST_INST1_OFFSET));
	temp = 0xFF;
	/* Wait for few clocks (say 20 TMDS clocks) would require this. */
	while (temp)
		temp--;
	__raw_writel(0x41, (phy_base + PHY_BIST_CNTL_OFFSET));
#endif	/* TEST_PHY_SEND_OUT_0xAA_AT_HALF_CLOCK_RATE_ON_ALL_DATA_LINES */
	/************************PHY BIST Test @ half clock rate***********************/

	/* Step 3 and 4 */
	temp = __raw_readl((phy_base + PHY_TMDS_CNTL2_OFFSET));
	temp |=
		(HDMI_PHY_TMDS_CNTL2_TERM_EN_MASK |
		 HDMI_PHY_TMDS_CNTL2_OE_MASK);
	__raw_writel(temp, (phy_base + PHY_TMDS_CNTL2_OFFSET));

exit_this_func:
	THDBG("configure_phy<<<<");
	return (rtn_value);
}
#else
static int configure_phy(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	int phy_base, wp_base;
	volatile u32 temp;
	int cmd, count;

	THDBG(">>>>configure_phy\n");

	/* Steps
	 * LDOOn and TX Power ON
	 * Set the Transmit control register based on the pixel clock setting.
	 * Set the digital control register
	 * Set the power control
	 * Set the pad control register
	 * Disable Trim and Test Control
	 * Analog interface Control
	 * Digital interface control
	 * disable bist test.
	 */
	phy_base = inst_context->phy_base_addr;
	wp_base = inst_context->wp_base_addr;

	/* Power on the PLL and HSDivider */
	cmd = 0x2;
	rtn_value = wp_pll_pwr_ctrl(wp_base + HDMI_WP_PWR_CTRL_OFFSET, cmd);
	if (rtn_value)
	{
		rtn_value = -1;
		goto exit;
	}
	/* change LDO to on state */
	cmd = 1;
	rtn_value = wp_phy_pwr_ctrl(wp_base + HDMI_WP_PWR_CTRL_OFFSET, cmd);
	if (rtn_value)
	{
		rtn_value = -1;
		goto exit;
	}
	/* TXPower ON */
	cmd = 2;
	rtn_value = wp_phy_pwr_ctrl(wp_base + HDMI_WP_PWR_CTRL_OFFSET, cmd);
	if (rtn_value)
	{
		rtn_value = -1;
		goto exit;
	}
	/* read address 0 in order to get the SCPreset done completed */
	/* Dummy access performed to solve resetdone issue */
	__raw_readl(phy_base + HDMI_PHY_TX_CTRL_OFF);

	/* TX Control bit 30 set according to pixel clock frequencies*/
	temp = __raw_readl(phy_base + HDMI_PHY_TX_CTRL_OFF);
	switch (inst_context->config.display_mode)
	{
		case hdmi_1080P_30_mode:
		case hdmi_720P_60_mode:
		case hdmi_1080I_60_mode:
			temp |= 0x1 << 30;
			break;
		case hdmi_1080P_60_mode:
			temp |= 0x2 << 30;
			break;
		default:
			return -1;
	}
	/* Not programmed in OMA4 */
#if 0
	/* Enable de-emphasis on all the links D0, D1, D2 and CLK */
	temp |= 0x1 << 27;
	temp |= 0x1 << 26;
	temp |= 0x1 << 25;
	temp |= 0x1 << 24;
	/* Set the default de-emphasis value for all the links
	 * TODO: Get the proper de-emphasis value
	 */
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 21;
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 18;
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 15;
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 12;
	/* Configure the slow edge for the normal setting */
	temp |= 0x0 << 10;
	temp |= 0x0 << 8;
	temp |= 0x0 << 6;
	temp |= 0x0 << 4;
	/* Set the TMDS level for normal I/O of 3.3V */
	temp |= 0x0 << 3;
	/* Nominal current of 10ma used for signalling */
	temp |= 0x0 << 1;
#endif
	__raw_writel(temp, phy_base + HDMI_PHY_TX_CTRL_OFF);

	/* According to OMAP4 */
	/* Power Control */
	temp = __raw_readl(phy_base + HDMI_PHY_PWR_CTRL_OFF);
	/* setup max LDO voltage */
	temp |= HDMI_PHY_DEF_LDO_VOLTAGE_VAL << 0;
	__raw_writel(temp, phy_base + HDMI_PHY_PWR_CTRL_OFF);

	/* Pad configuration Control */
	temp = __raw_readl(phy_base + HDMI_PHY_PAD_CFG_CTRL_OFF);
	/* Normal polarity for all the links */
	temp |= 0x1 << 31;
	temp |= 0x0 << 30;
	temp |= 0x0 << 29;
	temp |= 0x0 << 28;
	temp |= 0x0 << 27;

	/* Channel assignement is 10101 – D2- D1 –D0-CLK */
	temp |=  0x21 << 22;
	__raw_writel(temp, phy_base + HDMI_PHY_PAD_CFG_CTRL_OFF);

	/* Digital control */
	temp = __raw_readl(phy_base + HDMI_PHY_DIGITAL_CTRL_OFF);;
	/* Use bit 30 from this register as the enable signal for the TMDS */
	temp |= 1 << 31;
	/* Enable TMDS signal. TODO*/
	temp |= 1 << 30;
	/* Use 28 pin as the TX valid from this register */
	temp |= 1  << 29;
	/* Tx Valid enable TODO*/
	temp |= 1 << 28;
	__raw_writel(temp, phy_base + HDMI_PHY_DIGITAL_CTRL_OFF);
#if 0
	/* Trim and Test Control */
	/* TODO Don't use the Bandgap values */
	temp |= 0x0 << 31;
	/* TODO Dont use cap trim settings */
	temp |= 0x0 << 15;
	/* TODO Dont enable the bandgap and switched cap current */
	temp |= 0x0 << 7;
	temp |= 0x0 << 6;
	__raw_writel(temp, phy_base + HDMI_PHY_TRIM_TEST_CTRL_OFF);

	/* Analog Interface control */
	temp = 0;
	/* TODO: Don't put AFE in debug mode */
	temp |= 0x0 << 16;
	/* TODO: Don't use the LDO prog register */
	temp |= 0x0 << 15;
	/* TODO: Don't override the value of the analog signal LDOPGD*/
	temp |= 0x0 << 14;
	/* TODO: Don't override the value of the analog signal BGON */
	temp |= 0x0 << 13;
	/* TODO: Don't override the value of the analog signal TXON */
	temp |= 0x0 << 12;
	/* TODO: Dont use the register to override the clock lane pos */
	temp |= 0x0 << 10;
	/* TODO: Analog characterization For now putting it to 0*/
	temp |= 0x0 << 0;
	__raw_writel(temp, phy_base + HDMI_PHY_ANG_INT_CTRL_OFF);

	/* Digital Interface Control */
	temp = 0;
	/* TODO: Don't use this register for data output */
	temp |= 0x0 << 31;
	__raw_writel(temp, phy_base + HDMI_PHY_DATA_INT_CTRL_OFF);

	/* BIST register */
	temp = 0;
	/* TODO: Don't use the LDO bist conrtol */
	temp |= 0x0 << 31;
	/* TODO: Don't use LB mode */
	temp |= 0x0 << 27;
	/* TODO: Don't use  the LB LANE SEL */
	temp |= 0x0 << 24;
	__raw_writel(temp, phy_base + HDMI_PHY_BIST_OFF);
#endif
exit:
	count = 0;
	while (count++ < 1000)
		;
	return rtn_value;
}

#endif

/*
 * Configure the wrapper with debouce data packing modes, timming
 *	parameters if operating as a master require timming generator also
 */
static int configure_wrapper(struct instance_cfg *inst_context)
{
	volatile u32 temp;
	u32 wp_base_addr = 0x0;
	int rtn_value = 0x0;

	THDBG(">>>>configure_wrapper");
	HDMI_ARGS_CHECK((inst_context != NULL));

	wp_base_addr = inst_context->wp_base_addr;
	/* Step 0 - Tweak if required */
	temp = ((inst_context->config.wp_config.debounce_rcv_detect <<
				HDMI_WP_DEBOUNCE_RXDET_SHIFT) &
			HDMI_WP_DEBOUNCE_RXDET_MASK);

	temp |= ((inst_context->config.wp_config.debounce_rcv_sens <<
				HDMI_WP_DEBOUNCE_LINE5VSHORT_SHIFT) &
			HDMI_WP_DEBOUNCE_LINE5VSHORT_MASK);

	__raw_writel(temp, (wp_base_addr + HDMI_WP_DEBOUNCE_OFFSET));

	/* Dividing the 48MHz clock to 2 MHz for CEC and OCP different dividor */
	temp = __raw_readl(wp_base_addr + HDMI_WP_CLK_OFFSET);
	temp |= 0x00000218u;
	__raw_writel(temp, (wp_base_addr + HDMI_WP_CLK_OFFSET));

	/* Following steps only applicable for a master generating the timmings
	   signal to core */
	if (inst_context->config.wp_config.is_slave_mode == 0x0) {
		THDBG("Configuring as Master");
		temp =
			((inst_context->config.wp_config.
			  hbp << HDMI_WP_VIDEO_TIMING_H_HBP_SHIFT) &
			 HDMI_WP_VIDEO_TIMING_H_HBP_MASK);
		temp |=
			((inst_context->config.wp_config.
			  hfp << HDMI_WP_VIDEO_TIMING_H_HFP_SHIFT) &
			 HDMI_WP_VIDEO_TIMING_H_HFP_MASK);
		temp |=
			((inst_context->config.wp_config.
			  hsw << HDMI_WP_VIDEO_TIMING_H_HSW_SHIFT) &
			 HDMI_WP_VIDEO_TIMING_H_HSW_MASK);

		__raw_writel(temp,
				(wp_base_addr +
				 HDMI_WP_VIDEO_TIMING_H_OFFSET));

		temp = ((inst_context->config.wp_config.vbp <<
					HDMI_WP_VIDEO_TIMING_V_VBP_SHIFT) &
				HDMI_WP_VIDEO_TIMING_V_VBP_MASK);
		temp |= ((inst_context->config.wp_config.vfp <<
					HDMI_WP_VIDEO_TIMING_V_VFP_SHIFT) &
				HDMI_WP_VIDEO_TIMING_V_VFP_MASK);
		temp |= ((inst_context->config.wp_config.vsw <<
					HDMI_WP_VIDEO_TIMING_V_VSW_SHIFT) &
				HDMI_WP_VIDEO_TIMING_V_VSW_MASK);
		__raw_writel(temp,
				(wp_base_addr +
				 HDMI_WP_VIDEO_TIMING_V_OFFSET));

		temp = __raw_readl
			(wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET);
		if (inst_context->config.wp_config.vSync_pol != 0x0) {
			temp |= HDMI_WP_VIDEO_CFG_VSYNC_POL_MASK;
		} else {
			temp &= ~(HDMI_WP_VIDEO_CFG_VSYNC_POL_MASK);
		}
		if (inst_context->config.wp_config.hSync_pol != 0x0) {
			temp |= HDMI_WP_VIDEO_CFG_HSYNC_POL_MASK;
		} else {
			temp &= ~(HDMI_WP_VIDEO_CFG_HSYNC_POL_MASK);
		}
		__raw_writel(temp,
				(wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET));
	}

	temp = __raw_readl(wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET);
	temp &= (~(HDMI_WP_VIDEO_CFG_PACKING_MODE_MASK));
	temp |= ((inst_context->config.wp_config.pack_mode <<
				HDMI_WP_VIDEO_CFG_PACKING_MODE_SHIFT) &
			HDMI_WP_VIDEO_CFG_PACKING_MODE_MASK);

	/* Invert if required - follows input otherwise */
	if (inst_context->config.wp_config.is_vSync_pol_inv != 0x0) {
		temp |= HDMI_WP_VIDEO_CFG_CORE_VSYNC_INV_MASK;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_CORE_VSYNC_INV_MASK));
	}
	if (inst_context->config.wp_config.is_hSync_pol_inv != 0x0) {
		temp |= HDMI_WP_VIDEO_CFG_CORE_HSYNC_INV_MASK;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_CORE_HSYNC_INV_MASK));
	}

	if (inst_context->is_interlaced == TRUE) {
		temp |= HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK));
	}

	if (inst_context->config.wp_config.is_slave_mode == 0x0) {
		temp |= (HDMI_WP_VIDEO_CFG_MODE_MASK);
		temp |= inst_context->config.wp_config.width;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_MODE_MASK));
		THDBG("Operating as slave");
	}
	__raw_writel(temp, (wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET));

	THDBG("configure_wrapper<<<<");
	return (rtn_value);
}


/*
 * Configures the interface between the wrapper and core.
 *
 * The number of lines/channel between in the core and the wrapper is not
 * configureable option.
 */
static int configure_core_input(struct instance_cfg *inst_context)
{
	volatile u32 temp;
	volatile u32 core_addr;
	struct hdmi_core_input_cfg *cfg = NULL;

	THDBG(">>>>configure_core_input");
	HDMI_ARGS_CHECK((inst_context != NULL));
	cfg = &(inst_context->config.core_config);
	core_addr = inst_context->core_base_addr;
	/*
	 * Step 1. Configure the width of the input bus.
	 * Step 2. Configure the sources for sync signals
	 *		   if hdmi_extract_syncs - VID_MODE.SYNCEX = 1
	 *		   if hdmi_generate_de - DE_CTRL.DE_GEN = 1 and de_top
	 *		   de_dly, etc...
	 *		   if hdmi_source_syncs - SYS_CTRL1.VEN/HEN = 1
	 * Step 3. Configure the edge to latch on.
	 */
	temp = __raw_readl(core_addr + HDMI_CORE_VID_ACEN_OFFSET);
	temp &= (~(HDMI_VID_ACEN_WIDE_BUS_MASK));
	temp |= ((cfg->data_bus_width << HDMI_VID_ACEN_WIDE_BUS_SHIFT) &
			HDMI_VID_ACEN_WIDE_BUS_MASK);
	__raw_writel(temp, (core_addr + HDMI_CORE_VID_ACEN_OFFSET));

	temp = __raw_readl(core_addr + HDMI_CORE_SYS_CTRL1_OFFSET);
	temp &= (~(HDMI_SYS_CTRL1_BSEL_MASK | HDMI_SYS_CTRL1_EDGE_MASK));
	if (cfg->edge_pol != 0x0)
		temp |= HDMI_SYS_CTRL1_EDGE_MASK;

	temp |= HDMI_SYS_CTRL1_BSEL_MASK;
	__raw_writel(temp, (core_addr + HDMI_CORE_SYS_CTRL1_OFFSET));

	if (cfg->sync_gen_cfg == hdmi_extract_syncs) {
		temp = __raw_readl(core_addr + HDMI_CORE_VID_MODE_OFFSET);
		temp &= (~(HDMI_VID_MODE_SYNCEX_MASK));
		temp |= HDMI_VID_MODE_SYNCEX_MASK;
		__raw_writel(temp,
				(core_addr + HDMI_CORE_VID_MODE_OFFSET));
		THDBG("Embedded syncs \n");
	} else if (cfg->sync_gen_cfg == hdmi_generate_de) {
		temp = __raw_readl(core_addr + HDMI_CORE_DE_CTRL_OFFSET);
		temp &= (~(HDMI_DE_CTRL_DE_GEN_MASK));
		temp |= HDMI_DE_CTRL_DE_GEN_MASK;
		__raw_writel(temp, (core_addr + HDMI_CORE_DE_CTRL_OFFSET));

		__raw_writel((cfg->de_delay_cfg.
					DE_DLY & HDMI_DE_DLY_DE_DLY_MASK)
				, (core_addr + HDMI_CORE_DE_DLY_OFFSET));
		__raw_writel((cfg->de_delay_cfg.
					DE_TOP & HDMI_DE_TOP_DE_TOP_MASK)
				, (core_addr + HDMI_CORE_DE_TOP_OFFSET));
		__raw_writel((cfg->de_delay_cfg.
					DE_CNTL & HDMI_DE_CNTL_DE_CNT_MASK)
				, (core_addr + HDMI_CORE_DE_CNTL_OFFSET));
		__raw_writel((cfg->de_delay_cfg.
					DE_CNTH & HDMI_DE_CNTH_DE_CNT_MASK)
				, (core_addr + HDMI_CORE_DE_CNTH_OFFSET));
		__raw_writel((cfg->de_delay_cfg.
					DE_LINL & HDMI_DE_LINL_DE_LIN_MASK)
				, (core_addr + HDMI_CORE_DE_LINL_OFFSET));
		__raw_writel((cfg->de_delay_cfg.
					DE_LINH & HDMI_DE_LINH_DE_LIN_MASK)
				, (core_addr + HDMI_CORE_DE_LINH_OFFSET));
		THDBG("Sync being generated");
	} else {
		__raw_writel(0x1u, (core_addr + HDMI_CORE_DE_CTRL_OFFSET));
		temp = __raw_readl(core_addr + HDMI_CORE_SYS_CTRL1_OFFSET);
		temp |= HDMI_SYS_CTRL1_VEN_MASK;
		temp |= HDMI_SYS_CTRL1_HEN_MASK;
		__raw_writel(temp,
				(core_addr + HDMI_CORE_SYS_CTRL1_OFFSET));
		THDBG("Descrete syncs and being sourced");
	}
	__raw_writel(0x0u, (core_addr + HDMI_CORE_IADJUST_OFFSET));

	THDBG("configure_core_input<<<<");
	return (0x0);
}

/*
 *	Configure sub-blocks
 */
static int configure_core_data_path(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	volatile u32 tempVidAcen;
	volatile u32 tempVidMode;
	volatile u32 tempVidDither;
	volatile u32 temp;
	volatile u32 core_addr;
	struct hdmi_core_data_path *pathCfg = NULL;

	THDBG(">>>>configure_core_data_path");
	HDMI_ARGS_CHECK((inst_context != NULL));
	core_addr = inst_context->core_base_addr;

	tempVidAcen = __raw_readl(core_addr + HDMI_CORE_VID_ACEN_OFFSET);
	tempVidMode = __raw_readl(core_addr + HDMI_CORE_VID_MODE_OFFSET);

	pathCfg = &(inst_context->config.core_path_config);
	tempVidMode &= (~(HDMI_VID_MODE_UPSMP_MASK |
				HDMI_VID_MODE_CSC_MASK |
				HDMI_VID_MODE_RANGE_MASK |
				HDMI_VID_MODE_DITHER_MASK));

	tempVidAcen &= (~(HDMI_VID_ACEN_RGB_2_YCBCR_MASK |
				HDMI_VID_ACEN_RANGE_CMPS_MASK |
				HDMI_VID_ACEN_DOWN_SMPL_MASK |
				HDMI_VID_ACEN_RANGE_CLIP_MASK |
				HDMI_VID_ACEN_CLIP_CS_ID_MASK));
	if (pathCfg->up_sampler_enable != 0x0)
		tempVidMode |= HDMI_VID_MODE_UPSMP_MASK;

	if (pathCfg->csc_YCbCr_2_RGB_enable != 0x0) {
		tempVidMode |= HDMI_VID_MODE_CSC_MASK;
		rtn_value = configure_csc_ycbcr_rgb(inst_context);
		if (rtn_value != 0x0)
			goto exit_this_func;
	}
	temp = __raw_readl(core_addr + HDMI_CORE_VID_CTRL_OFFSET);
	if (pathCfg->csc_convert_standard != 0x0)
		temp |= HDMI_VID_CTRL_CSCSEL_MASK;
	else
		temp &= (~(HDMI_VID_CTRL_CSCSEL_MASK));
	__raw_writel(temp, (core_addr + HDMI_CORE_VID_CTRL_OFFSET));

	if (pathCfg->range_exp_RGB_enable != 0x0)
		tempVidMode |= HDMI_VID_MODE_RANGE_MASK;

	if (pathCfg->dither_enable != 0x0) {
		tempVidDither =
			__raw_readl(core_addr + HDMI_CORE_VID_DITHER_OFFSET);
		tempVidMode |= HDMI_VID_MODE_DITHER_MASK;
		tempVidDither &= (~(HDMI_VID_DITHER_M_D2_MASK |
					HDMI_VID_DITHER_UP2_MASK |
					HDMI_VID_DITHER_STR_422_EN_MASK |
					HDMI_VID_DITHER_D_BC_EN_MASK |
					HDMI_VID_DITHER_D_GC_EN_MASK |
					HDMI_VID_DITHER_D_RC_EN_MASK |
					HDMI_VID_DITHER_DRD_MASK));
		/* Configure dithering parameters */
		if (pathCfg->dither_config.M_D2 != 0x0)
			tempVidDither |= HDMI_VID_DITHER_M_D2_MASK;
		if (pathCfg->dither_config.UP2 != 0x0)
			tempVidDither |= HDMI_VID_DITHER_UP2_MASK;
		if (pathCfg->dither_config.STR_422_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_STR_422_EN_MASK;
		if (pathCfg->dither_config.D_BC_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_D_BC_EN_MASK;
		if (pathCfg->dither_config.D_GC_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_D_GC_EN_MASK;
		if (pathCfg->dither_config.D_RC_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_D_RC_EN_MASK;
		if (pathCfg->dither_config.DRD != 0x0)
			tempVidDither |= HDMI_VID_DITHER_DRD_MASK;
		__raw_writel(tempVidDither,
				(core_addr + HDMI_CORE_VID_DITHER_OFFSET));
	}

	tempVidMode |=
		((pathCfg->output_width << HDMI_VID_MODE_DITHER_MODE_SHIFT) &
		 HDMI_VID_MODE_DITHER_MODE_MASK);
	__raw_writel(tempVidMode,
			(core_addr + HDMI_CORE_VID_MODE_OFFSET));

	if (pathCfg->cscRGB_2_YCbCr_enable != 0x0)
		tempVidAcen |= HDMI_VID_ACEN_RGB_2_YCBCR_MASK;

	if (pathCfg->range_comp_enable != 0x0)
		tempVidAcen |= HDMI_VID_ACEN_RANGE_CMPS_MASK;

	if (pathCfg->down_sampler_enable != 0x0)
		tempVidAcen |= HDMI_VID_ACEN_DOWN_SMPL_MASK;

	if (pathCfg->range_clip_enable != 0x0) {
		tempVidAcen |= HDMI_VID_ACEN_RANGE_CLIP_MASK;
		if (pathCfg->clip_color_space != 0x0) {
			tempVidAcen |= HDMI_VID_ACEN_CLIP_CS_ID_MASK;
		}
	}
	__raw_writel(tempVidAcen, (core_addr + HDMI_CORE_VID_ACEN_OFFSET));

exit_this_func:
	THDBG("configure_core_data_path<<<<");
	return (rtn_value);
}

static int configure_core(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	volatile u32 temp;
	volatile u32 core_addr;
	THDBG(">>>>configure_core");
	HDMI_ARGS_CHECK((inst_context != NULL));
	core_addr = inst_context->core_base_addr;

	temp = __raw_readl(core_addr + HDMI_CORE_TEST_TXCTRL_OFFSET);
	temp &= (~(HDMI_TEST_TXCTRL_DIV_ENC_BYP_MASK));
	__raw_writel(temp, (core_addr + HDMI_CORE_TEST_TXCTRL_OFFSET));

	if (inst_context->config.use_core_config != 0x0) {
		rtn_value = configure_core_input(inst_context);
		if (rtn_value != 0x0)
			goto exit_this_func;
	}
	if (inst_context->config.use_core_path_config != 0x0) {
		rtn_value = configure_core_data_path(inst_context);
		if (rtn_value != 0x0)
			goto exit_this_func;
	}
	rtn_value = configure_policies(inst_context);
	if (rtn_value != 0x0)
		goto exit_this_func;

	__raw_writel(0x0, (core_addr + HDMI_CORE_ACR_CTRL_OFFSET));

exit_this_func:
	THDBG("configure_core<<<<");
	return (rtn_value);
}

static int configure_policies(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	volatile u32 temp;
	volatile u32 dither_mode_val;
	volatile u32 core_addr;

	THDBG(">>>>configure_policies");
	HDMI_ARGS_CHECK((inst_context != NULL));
	core_addr = inst_context->core_base_addr;

	temp = __raw_readl(core_addr + HDMI_CORE_VID_CTRL_OFFSET);

	/* No pixel repeatation by default */
	temp &= (~(HDMI_VID_CTRL_ICLK_MASK));

	rtn_value = determine_pixel_repeatation(inst_context);
	if (rtn_value == HDMI_PIXEL_REPLECATED_ONCE) {
		temp |= (((0x01u) << HDMI_VID_CTRL_ICLK_SHIFT) &
				HDMI_VID_CTRL_ICLK_MASK);
	} else if (rtn_value == HDMI_PIXEL_REPLECATED_FOUR_TIMES) {
		temp |= HDMI_VID_CTRL_ICLK_MASK;
	} else if (rtn_value == 0x0) {
		THDBG("No Pixel repeatation required");
	} else {
		THDBG("Could not determine pixel that would be required");
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}
	__raw_writel(temp, (core_addr + HDMI_CORE_VID_CTRL_OFFSET));


	temp = __raw_readl(core_addr + HDMI_CORE_HDMI_CTRL_OFFSET);

	temp &=
		(~
		 (HDMI_HDMI_CTRL_DC_EN_MASK |
		  HDMI_HDMI_CTRL_PACKET_MODE_MASK));

	dither_mode_val =
		__raw_readl(core_addr + HDMI_CORE_VID_MODE_OFFSET);
	dither_mode_val =
		((dither_mode_val & HDMI_VID_MODE_DITHER_MODE_MASK)
		 >> HDMI_VID_MODE_DITHER_MODE_SHIFT);

	if (dither_mode_val != HDMI_VID_MODE_DITHER_TO_24_BITS_MODE) {
		temp |= HDMI_HDMI_CTRL_DC_EN_MASK;
		THDBG("Deep color mode");
	}
	temp |= ((HDMI_CTRL_PACKET_MODE_24BITS_PIXEL) <<
			HDMI_HDMI_CTRL_PACKET_MODE_SHIFT);
	if (dither_mode_val == HDMI_VID_MODE_DITHER_TO_30_BITS_MODE) {
		temp |= ((HDMI_CTRL_PACKET_MODE_30BITS_PIXEL) <<
				HDMI_HDMI_CTRL_PACKET_MODE_SHIFT);
	}
	if (dither_mode_val == HDMI_VID_MODE_DITHER_TO_36_BITS_MODE) {
		temp |= ((HDMI_CTRL_PACKET_MODE_36BITS_PIXEL) <<
				HDMI_HDMI_CTRL_PACKET_MODE_SHIFT);
	}
	/* TODO DVI mode is required - make this configureable also */
	temp |= HDMI_HDMI_CTRL_HDMI_MODE_MASK;

	__raw_writel(temp, (core_addr + HDMI_CORE_HDMI_CTRL_OFFSET));

	temp = __raw_readl(core_addr + HDMI_CORE_TMDS_CTRL_OFFSET);

	temp |= (HDMI_TMDS_CTRL_TCLKSEL_MASK &
			(HDMI_TMDS_CTRL_IP_CLOCK_MULTIPLIER_AUDIO <<
			 HDMI_TMDS_CTRL_TCLKSEL_SHIFT));
	__raw_writel(temp, (core_addr + HDMI_CORE_TMDS_CTRL_OFFSET));

exit_this_func:
	THDBG("configure_policies<<<<");
	return (rtn_value);
}


static int configure_ctrl_packets(struct instance_cfg *inst_context)
{
	volatile u32 temp;

	temp = __raw_readl((inst_context->core_base_addr) +
			HDMI_CORE_DC_HEADER_OFFSET);
	temp = 0x03;
	__raw_writel(temp,
			((inst_context->core_base_addr) +
			 HDMI_CORE_DC_HEADER_OFFSET));

	__raw_writel(HDMI_CP_BYTE1_SETAVM_MASK,
			((inst_context->core_base_addr) +
			 HDMI_CORE_CP_BYTE1_OFFSET));

	return (0x0);
}


static int configure_avi_info_frame(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	u8 check_sum = 0x0;
	u8 byte_index = 0x0;
	volatile u8 data_byte = 0x0;
	volatile u32 dbyte_base;
	struct hdmi_avi_frame_cfg *infoPkt = NULL;
	THDBG(">>>>configure_avi_info_frame\n");
	HDMI_ARGS_CHECK((inst_context != NULL));

	infoPkt = &(inst_context->config.info_frame_config.aviData);
	dbyte_base = (u32) (inst_context->core_base_addr +
			HDMI_CORE_AVI_DBYTE_BASE_OFFSET);
	data_byte = (u8)
		(HDMI_AVI_TYPE_AVI_TYPE_MASK & HDMI_AVI_INFOFRAME_PKT_TYPE);
	__raw_writel(data_byte,
			((inst_context->core_base_addr) +
			 HDMI_CORE_AVI_TYPE_OFFSET));
	check_sum = HDMI_AVI_INFOFRAME_PKT_TYPE;

	data_byte = (u8)
		(HDMI_AVI_VERS_AVI_VERS_MASK & HDMI_AVI_INFOFRAME_PKT_VER);
	__raw_writel(data_byte,
			((inst_context->core_base_addr) +
			 HDMI_CORE_AVI_VERS_OFFSET));
	check_sum += HDMI_AVI_INFOFRAME_PKT_VER;

	data_byte = (u8)
		(HDMI_AVI_LEN_AVI_LEN_MASK & HDMI_AVI_INFOFRAME_PKT_LEN);
	__raw_writel(data_byte,
			((inst_context->core_base_addr) +
			 HDMI_CORE_AVI_LEN_OFFSET));
	check_sum += HDMI_AVI_INFOFRAME_PKT_LEN;


	data_byte = (((u8) infoPkt->output_cs << 5) &
			HDMI_AVI_INFOFRAME_Y0_Y1_MASK);
	if (infoPkt->use_active_aspect_ratio == TRUE) {
		data_byte |= HDMI_AVI_INFOFRAME_A0_MASK;
	}
	/* Bar information B0 and B1 - if so require to update byte 6-13 */
	if (infoPkt->bar_info.barInfoValid != 0x0) {
		data_byte |= ((((u8) infoPkt->bar_info.barInfoValid) <<
					2) & HDMI_AVI_INFOFRAME_B0_B1_MASK);
	}
	data_byte |= (((u8) infoPkt->scan_info) &
			HDMI_AVI_INFOFRAME_S0_S1_MASK);

	/* First data byte of the packet */
	__raw_writel(data_byte, (dbyte_base + byte_index));
	byte_index += 0x4;
	check_sum += data_byte;

	data_byte = (((u8) infoPkt->colorimetry_info << 6) &
			HDMI_AVI_INFOFRAME_C0_C1_MASK);

	data_byte |= (((u8) infoPkt->aspect_ratio << 4) &
			HDMI_AVI_INFOFRAME_M0_M1_MASK);
	if (infoPkt->use_active_aspect_ratio == TRUE) {
		data_byte |= (((u8) infoPkt->active_aspect_ratio) &
				HDMI_AVI_INFOFRAME_R0_R3_MASK);
	}

	/* Second data byte of the packet */
	__raw_writel(data_byte, (dbyte_base + byte_index));
	byte_index += 0x4;
	check_sum += data_byte;

	data_byte = 0x0;
	if (infoPkt->it_content_present != 0x0) {
		data_byte = HDMI_AVI_INFOFRAME_ITC_MASK;
	}
	/* Extended colorimetry range EC3 to EC0 */
	data_byte |= (((u8) infoPkt->ext_colorimetry << 4) &
			HDMI_AVI_INFOFRAME_EC2_EC0_MASK);
	/* Quantization range range Q1 to Q0 */
	data_byte |= (((u8) infoPkt->quantization_range << 2)
			& HDMI_AVI_INFOFRAME_Q1_Q0_MASK);
	/* Non-Uniform scaling S0 and S1 */
	data_byte |= ((u8) infoPkt->non_uniform_sc &
			HDMI_AVI_INFOFRAME_SC1_SC0_MASK);
	/* Third data byte of the packet */
	__raw_writel(data_byte, (dbyte_base + byte_index));
	byte_index += 0x4;
	check_sum += data_byte;
	/* Fourth data byte of the packet */
	switch (inst_context->config.display_mode) {
		case hdmi_720P_60_mode:
			infoPkt->format_identier = 4u;
			break;
		case hdmi_1080P_30_mode:
			infoPkt->format_identier = 34u;
			break;
		case hdmi_1080I_60_mode:
			infoPkt->format_identier = 5u;
			break;
		case hdmi_1080P_60_mode:
			infoPkt->format_identier = 16u;
			break;
		default:
			rtn_value = -EINVAL ;
			goto exit_this_func;
	}

	data_byte = (u8) infoPkt->format_identier;

	__raw_writel(((u8) data_byte &
				HDMI_AVI_INFOFRAME_VIC6_VIC0_MASK),
			(dbyte_base + byte_index));
	byte_index += 0x4;
	check_sum += data_byte;

	/* Pixel Repeatation */
	data_byte = (u8) (HDMI_VID_CTRL_ICLK_MASK &
			__raw_readl(inst_context->core_base_addr +
				HDMI_CORE_VID_CTRL_OFFSET));

	/* TODO - Why do we require to up the pixel repeatation when demux is
	   is used. */
	if ((__raw_readl(inst_context->core_base_addr +
					HDMI_CORE_VID_MODE_OFFSET) &
				HDMI_VID_MODE_DEMUX_MASK) == HDMI_VID_MODE_DEMUX_MASK){
		/* Do not worry about exceeding the upper limit.
		   Pixel could be repeated a maximum of 4 times (value 0x03).
		   The pixel repeatation has 4 bit space in info packet which could
		   be a maximum of 0x0F, but limited to 0x09 */
		data_byte++;
	}
	__raw_writel((HDMI_AVI_INFOFRAME_PR3_PR0_MASK & data_byte),
			(dbyte_base + byte_index));
	byte_index += 0x4;
	check_sum += data_byte;

	if (infoPkt->bar_info.barInfoValid != 0x0) {
		data_byte = (u8) (infoPkt->bar_info.topBar & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(u8) ((infoPkt->bar_info.topBar >> 8) & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;

		data_byte = (u8) (infoPkt->bar_info.bottomBar & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(u8) ((infoPkt->bar_info.bottomBar >> 8) & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;

		data_byte = (u8) (infoPkt->bar_info.leftBar & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(u8) ((infoPkt->bar_info.leftBar >> 8) & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;

		data_byte = (u8) (infoPkt->bar_info.rightBar & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(u8) ((infoPkt->bar_info.rightBar >> 8) & 0xFF);
		__raw_writel(data_byte, (dbyte_base + byte_index));
		byte_index += 0x4;
		check_sum += data_byte;
	}

	__raw_writeb((u8) (HDMI_AVI_INFOFRAME_CONST_0x100 - (u16) check_sum),
			(inst_context->core_base_addr + HDMI_CORE_AVI_CHSUM_OFFSET));

	THDBG("AVI - Computed check sum %d", check_sum);
	THDBG("Check sum sent %d",
			__raw_readl(inst_context->core_base_addr +
				HDMI_CORE_AVI_CHSUM_OFFSET));
	THDBG("Sent check sum + all bytes should 0x0");
exit_this_func:
	THDBG("configure_avi_info_frame<<<");
	return (rtn_value);
}

static int configure_csc_ycbcr_rgb(struct instance_cfg *inst_context)
{
	struct hdmi_csc_YCbCr_2_RGB_ctrl *ctrl = NULL;
	volatile u32 temp;
	volatile u32 core_addr;

	THDBG(">>>>configure_csc_ycbcr_rgb");
	HDMI_ARGS_CHECK((inst_context != NULL));

	core_addr = inst_context->core_base_addr;
	ctrl =
		&(inst_context->config.core_path_config.
				csc_YCbCr_2_RGB_config);

	temp = __raw_readl(core_addr + HDMI_CORE_XVYCC2RGB_CTL_OFFSET);
	temp &= (~(HDMI_XVYCC2RGB_CTL_EXP_ONLY_MASK |
				HDMI_XVYCC2RGB_CTL_BYP_ALL_MASK |
				HDMI_XVYCC2RGB_CTL_SW_OVR_MASK |
				HDMI_XVYCC2RGB_CTL_FULLRANGE_MASK |
				HDMI_XVYCC2RGB_CTL_XVYCCSEL_MASK));
	if (ctrl->enableRngExp != 0x0)
		temp |= HDMI_XVYCC2RGB_CTL_EXP_ONLY_MASK;

	if (ctrl->enableFullRngExp != 0x0)
		temp |= HDMI_XVYCC2RGB_CTL_FULLRANGE_MASK;

	if (ctrl->srcCsSel != 0x0)
		temp |= HDMI_XVYCC2RGB_CTL_XVYCCSEL_MASK;

	if (ctrl->customCoEff != 0x0) {
		/* Load the custom coefficitents - using memcopy to load as the
		   structures maps to register */
		memcpy((void *) (core_addr +
					HDMI_CORE_Y2R_COEFF_LOW_OFFSET),
				((const void *) &(ctrl->coEff)),
				sizeof(struct hdmi_csc_YCbCr_2_RGB_coeff));
		THDBG("Using custom co-effs");
	}
	__raw_writel(temp, (core_addr + HDMI_CORE_XVYCC2RGB_CTL_OFFSET));

	THDBG("configure_csc_ycbcr_rgb<<<<");
	return (0x0);
}

static int validate_info_frame_cfg(struct hdmi_info_frame_cfg *config)
{
	int rtn_value = -EFAULT ;
	struct hdmi_avi_frame_cfg *aviData = NULL;
	THDBG(">>>>validate_info_frame_cfg");

	if (config == NULL)
		goto exit_this_func;

	aviData = &(config->aviData);
	if (aviData->output_cs >= hdmi_avi_max_op_cs) {
		THDBG("In correct color space");
		goto exit_this_func;
	}
	if ((aviData->use_active_aspect_ratio != hdmi_avi_no_aspect_ratio)
			&& (aviData->use_active_aspect_ratio !=
				hdmi_avi_active_aspect_ratio)) {
		THDBG("Wrong aspect ratio");
		goto exit_this_func;
	}
	if (aviData->scan_info >= hdmi_avi_max_scan) {
		THDBG("In correct scan info");
		goto exit_this_func;
	}
	if (aviData->colorimetry_info >= hdmi_avi_max_colorimetry) {
		THDBG("Wrong colorimetry info");
		goto exit_this_func;
	}
	if (aviData->aspect_ratio >= hdmi_avi_aspect_ratio_max) {
		THDBG("Wrong aspect ratio info");
		goto exit_this_func;
	}
	if ((aviData->active_aspect_ratio <
				hdmi_avi_active_aspect_ratio_same)
			&& (aviData->active_aspect_ratio >= hdmi_avi_aspect_ratio_max)) {
		THDBG("Wrong active aspect ratio info");
		goto exit_this_func;
	}
	if (aviData->non_uniform_sc >= hdmi_avi_non_uniform_scaling_max) {
		THDBG("In correct non-uniform scaling info");
		goto exit_this_func;
	}
	rtn_value = 0x0;

exit_this_func:
	THDBG("validate_info_frame_cfg<<<<");
	return (rtn_value);
}

static int validate_core_config(struct hdmi_core_input_cfg *config)
{
	THDBG(">>>>validate_core_config");
	HDMI_ARGS_CHECK((config != NULL));

	if (config->data_bus_width > hdmi_10_bits_chan_width) {
		THDBG("Bus width should be <=30 bits/pixel");
		return (-EFAULT );
	}

	if (config->sync_gen_cfg >= hdmi_max_syncs) {
		THDBG("Incorrect meathods used for synchronization");
		return (-EFAULT );
	}

	THDBG("validate_core_config<<<<");
	return (0x0);
}

static int validate_wp_cfg(struct hdmi_wp_config *config)
{
	int rtn_value = -EFAULT ;
	THDBG(">>>>validate_wp_cfg");

	if ((config->debounce_rcv_detect < 0x01) ||
			(config->debounce_rcv_detect > 0x14)) {
		THDBG("Debounce receiver detect incorrect");
		goto exit_this_func;
	}
	if ((config->debounce_rcv_sens < 0x01) ||
			(config->debounce_rcv_sens > 0x14)) {
		THDBG("Debounce receiver sens incorrect");
		goto exit_this_func;
	}
	if (config->is_slave_mode == 0x0) {
		THDBG("Warpper is not in SLAVE mode ");
		THDBG(" - Master mode cannot be supported");
		goto exit_this_func;
	}
	if (config->width >= hdmi_12_bits_chan_width) {
		THDBG("Bus width should be < 36 bits/pixel");
		THDBG(" - 8 & 10 bits/channel is valid");
		goto exit_this_func;
	}
	if (config->pack_mode >= hdmi_wp_no_pack) {
		THDBG("Incorrect data packing mode");
		goto exit_this_func;
	}
	rtn_value = 0x0;
	THDBG("validate_wp_cfg<<<<");

exit_this_func:
	return (rtn_value);
}

static int validate_path_config(struct hdmi_core_data_path *config)
{
	THDBG(">>>>validate_path_config");
	HDMI_ARGS_CHECK((config != NULL));
	if (config->output_width >= hdmi_max_bits_chan_width) {
		THDBG("In valid output channel width",
				config->output_width);
		return (-EFAULT );
	}
	THDBG("validate_path_config<<<<");
	return (0x0);
}


static int check_copy_config(struct instance_cfg *inst_cntxt,
		struct hdmi_cfg_params *config)
{
	int rtn_value = 0x0;
	THDBG(">>>>check_copy_config");

	if (config->use_display_mode != 0x0) {
		if (config->display_mode >= hdmi_max_mode) {
			THDBG("Incorrect mode id");
			rtn_value = -EINVAL ;
			goto exit_this_func;
		}
		inst_cntxt->config.display_mode = config->display_mode;
	}
	if (config->use_wp_config != 0x0) {
		rtn_value = validate_wp_cfg(&(config->wp_config));
		if (rtn_value != 0x0) {
			THDBG("Wrapper config incorrect");
			goto exit_this_func;
		}
		memcpy((void *) (&(inst_cntxt->config.wp_config)),
				((const void *) &(config->wp_config)),
				sizeof(struct hdmi_wp_config));
	}
	if (config->use_core_config != 0x0) {
		rtn_value = validate_core_config(&(config->core_config));
		if (rtn_value != 0x0) {
			THDBG("Core config incorrect");
			goto exit_this_func;
		}
		memcpy((void *) (&(inst_cntxt->config.core_config)),
				((const void *) &(config->core_config)),
				sizeof(struct hdmi_core_input_cfg));
	}
	if (config->use_core_path_config != 0x0) {
		rtn_value =
			validate_path_config(&(config->core_path_config));
		if (rtn_value != 0x0) {
			THDBG("Core data path config incorrect");
			goto exit_this_func;
		}
		memcpy((void *) (&(inst_cntxt->config.core_path_config)),
				((const void *) &(config->core_path_config)),
				sizeof(struct hdmi_core_data_path));
	}
	if (config->use_info_frame_config != 0x0) {
		if (config->info_frame_config.use_avi_info_data != 0x0) {
			rtn_value = validate_info_frame_cfg
				(&(config->info_frame_config));
			if (rtn_value != 0x0) {
				THDBG("Bad AVI Info frame data");
				goto exit_this_func;
			}
			memcpy((void
						*) (&(inst_cntxt->config.
								info_frame_config)),
					((const void *)
					 &(config->info_frame_config)),
					sizeof(struct hdmi_info_frame_cfg));
		}
	}
	THDBG("check_copy_config<<<<");

exit_this_func:
	return (rtn_value);
}

static int determine_pixel_repeatation(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	u32 mPixelPerSec = 0x0;
	THDBG(">>>>determine_pixel_repeatation");
	HDMI_ARGS_CHECK((inst_context != NULL));

	switch (inst_context->config.display_mode) {
		case hdmi_ntsc_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_NTSC;
			inst_context->is_interlaced = TRUE;
			THDBG("NTSC Standard");
			break;
		case hdmi_pal_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_PAL;
			inst_context->is_interlaced = TRUE;
			THDBG("PAL Standard");
			break;
		case hdmi_720P_60_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_720P60;
			inst_context->is_interlaced = FALSE;
			THDBG("720P60 format");
			break;
		case hdmi_1080P_60_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_1080P60;
			inst_context->is_interlaced = FALSE;
			THDBG("1080P60 format");
			break;
		case hdmi_1080P_30_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_1080P30;
			inst_context->is_interlaced = FALSE;
			THDBG("1080P30 format");
			break;
		case hdmi_1080I_60_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_1080I60;
			inst_context->is_interlaced = FALSE;
			THDBG("1080I60 format");
			break;
		default:
			/* This should not happen */
			rtn_value = -EINVAL ;
			THDBG("The display format is not supported");
			break;
	}
	if (rtn_value == 0x0) {
		if (mPixelPerSec < HDMI_MINIMUM_PIXELS_SEC) {
			if ((mPixelPerSec * HDMI_PIXEL_REPLECATED_ONCE) >=
					HDMI_MINIMUM_PIXELS_SEC) {
				rtn_value = HDMI_PIXEL_REPLECATED_ONCE;
				THDBG("Pixel Repeating 1 time");
				goto exit_this_func;
			}

			if ((mPixelPerSec *
						HDMI_PIXEL_REPLECATED_FOUR_TIMES) >=
					HDMI_MINIMUM_PIXELS_SEC) {
				rtn_value =
					HDMI_PIXEL_REPLECATED_FOUR_TIMES;
				THDBG("Pixel Repeating 4 time");
				goto exit_this_func;
			}
			/* We could not still meet the HDMI needs - let the
			   caller know */
			rtn_value = -EINVAL ;
			THDBG("Resolution too low Could not reach 25 MHz");
			goto exit_this_func;
		}
	}

exit_this_func:

	THDBG(">>>>determine_pixel_repeatation");
	return (rtn_value);
}
#ifndef CONFIG_ARCH_TI816X
int get_phy_status(struct instance_cfg *inst_context,
		struct ti81xxhdmi_phy_status *stat)
{
	int rtn_value = 0;
	int phy_base;
	u32 temp;

	THDBG(">>>>get_phy_status\n");
	phy_base = inst_context->phy_base_addr;
	if (!stat)
	{
		rtn_value = -EFAULT;
		goto exit_this_func;
	}

	temp = __raw_readl(phy_base + HDMI_PHY_PWR_CTRL_OFF);
	stat->rst_done_pclk = (temp & HDMI_PHY_RESETDONEPIXELCLK_MASK) <<
		HDMI_PHY_RESETDONEPIXELCLK_SHIFT;
	stat->rst_done_pwrclk = (temp & HDMI_PHY_RESETDONEPWRCLK_MASK) <<
		HDMI_PHY_RESETDONEPWRCLK_SHIFT;
	stat->rst_done_scpclk = (temp & HDMI_PHY_RESETDONESCPCLK_MASK) <<
		HDMI_PHY_RESETDONESCPCLK_SHIFT;
	stat->rst_done_refclk = (temp & HDMI_PHY_RESETDONEREFCLK_MASK) <<
		HDMI_PHY_RESETDONEREFCLK_SHIFT;
	temp = __raw_readl(phy_base + HDMI_PHY_PAD_CFG_CTRL_OFF);
	stat->dct_5v_short_clk = (temp & HDMI_PHY_DET5VSHT_CLK_MASK) <<
		HDMI_PHY_DET5VSHT_CLK_SHIFT;
	stat->rx_detect = (temp & HDMI_PHY_RXDET_LINE_MASK) >>
		HDMI_PHY_RXDET_LINE_SHIFT;
	stat->dct_5v_short_data = (temp & HDMI_PHY_DET5VSHT_DATA_MASK) >>
		HDMI_PHY_DET5VSHT_DATA_SHIFT;
	THDBG("get_phy_status<<<<<\n");
exit_this_func:
	return rtn_value;
}
#else
int get_phy_status(struct instance_cfg *inst_context,
		struct ti81xxhdmi_phy_status *stat)
{
	return -EINVAL;
}
#endif
#ifdef CONFIG_ARCH_TI816X
void enable_hdmi_clocks(u32 prcm_base)
{
	u32 temp, repeatCnt;
	THDBG("HDMI Clk enable in progress\n");
	temp = 2;
	/*Enable Power Domain Transition for HDMI */
	__raw_writel(temp, (prcm_base + CM_HDMI_CLKSTCTRL_OFF));
	/*Enable HDMI Clocks*/
	__raw_writel(temp, (prcm_base + CM_ACTIVE_HDMI_CLKCTRL_OFF));

	/*Check clocks are active*/
	repeatCnt = 0;
	do
	{
		temp = (__raw_readl(prcm_base + CM_HDMI_CLKSTCTRL_OFF)) >> 8;
		repeatCnt++;
	}
	while((temp != 0x3) &&(repeatCnt < VPS_PRCM_MAX_REP_CNT));

	/* Check to see module is functional */
	repeatCnt = 0;
	do
	{
		temp = ((__raw_readl(prcm_base + CM_ACTIVE_HDMI_CLKCTRL_OFF) &
					0x70000)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < VPS_PRCM_MAX_REP_CNT));
	THDBG("HDMI Clocks enabled successfully\n");
}
#else
void enable_hdmi_clocks(u32 prcm_base)
{
	u32 temp, repeatCnt;

	THDBG("HDMI Clk enable in progress\n");
	temp = 2;
	/*Enable Power Domain Transition for HDMI */
	__raw_writel(temp, (prcm_base + CM_ALWON_SDIO_CLKCTRL));

	/*Check clocks are active*/
	repeatCnt = 0;
	do
	{
		temp = (__raw_readl(prcm_base + CM_ALWON_SDIO_CLKCTRL)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < VPS_PRCM_MAX_REP_CNT));

	temp = 2;
	__raw_writel(temp, (prcm_base + CM_HDMI_CLKCTRL_OFF));
	repeatCnt = 0;
	do
	{
		temp = (__raw_readl(prcm_base + CM_HDMI_CLKCTRL_OFF)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < VPS_PRCM_MAX_REP_CNT));

	THDBG("HDMI Clocks enabled successfully\n");
}

#endif

#ifndef CONFIG_ARCH_TI816X
static void configure_hdmi_pll(volatile u32  b_addr,
		u32 __n,
		u32 __m,
		u32 __m2,
		u32 clkctrl_val)
{
	u32 m2nval, mn2val, read_clkctrl;
	u32 read_m2nval, read_mn2val;
	volatile u32 repeatCnt = 0;
	/* Put PLL in idle bypass mode */
	read_clkctrl = __raw_readl(b_addr + HDMI_PLL_CLKCTRL_OFF);
	read_clkctrl |= 0x1 << 23;
	__raw_writel(read_clkctrl, b_addr + HDMI_PLL_CLKCTRL_OFF);

	/* poll for the bypass acknowledgement */
	repeatCnt = 0u;
	while (repeatCnt < VPS_PRCM_MAX_REP_CNT)
	{
		if (((__raw_readl(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000101) == 0x00000101)
		{
			break;
		}
		/* Wait for the 100 cycles */
		udelay(100);
		repeatCnt++;
	}

	if (((__raw_readl(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000101) == 0x00000101)
	{
		;
	}
	else
	{
		printk("Not able to Keep PLL in bypass state!!!\n");
	}
	m2nval = (__m2 << 16) | __n;
	mn2val =  __m;
	/*ref_clk     = OSC_FREQ/(__n+1);
	  clkout_dco  = ref_clk*__m;
	  clk_out     = clkout_dco/__m2;
	 */

	__raw_writel(m2nval, (b_addr+HDMI_PLL_M2NDIV_OFF));
	read_m2nval = __raw_readl((b_addr+HDMI_PLL_M2NDIV_OFF));

	__raw_writel(mn2val, (b_addr+HDMI_PLL_MN2DIV_OFF));
	read_mn2val = __raw_readl((b_addr+HDMI_PLL_MN2DIV_OFF));


	__raw_writel(0x1, (b_addr+HDMI_PLL_TENABLEDIV_OFF));

	__raw_writel(0x0, (b_addr+HDMI_PLL_TENABLEDIV_OFF));

	__raw_writel(0x1, (b_addr+HDMI_PLL_TENABLE_OFF));

	__raw_writel(0x0, (b_addr+HDMI_PLL_TENABLE_OFF));

	read_clkctrl = __raw_readl(b_addr+HDMI_PLL_CLKCTRL_OFF);

	/*configure the TINITZ(bit0) and CLKDCO bits if required */
	__raw_writel((read_clkctrl & 0xff7fe3ff) | clkctrl_val, b_addr+HDMI_PLL_CLKCTRL_OFF);

	read_clkctrl = __raw_readl(b_addr+HDMI_PLL_CLKCTRL_OFF);


	/* poll for the freq,phase lock to occur */
	repeatCnt = 0u;
	while (repeatCnt < VPS_PRCM_MAX_REP_CNT)
	{
		if (((__raw_readl(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000600) == 0x00000600)
		{
			break;
		}
		/* Wait for the 100 cycles */
		udelay(100);
		repeatCnt++;
	}

	if (((__raw_readl(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000600) == 0x00000600)
	{
		/*printk("PLL Locked\n");*/
	}
	else
	{
		printk("PLL Not Getting Locked!!!\n");
	}

	/*wait fot the clocks to get stabized */
	udelay(100);
}

#endif

/* Ideally vencs should be configured from the HDVPSS drivers.	But in case
 * we want to test the HDMI this fuction can be used to generate the test
 * pattern on venc and HDMI can be tested in absence of HDVPSS drivers
 */
/*******************************************************************************
 *			Venc Configurations 				       *
 ******************************************************************************/
#ifdef HDMI_TEST
static void configure_venc_1080p30(u32 *venc_base, int useEmbeddedSync)
{
	THDBG("%s %d\n",  __func__, __LINE__);
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A033;
	}
	venc_base++;
	*venc_base = 0x003F0275;
	venc_base++;
	*venc_base = 0x1EA500BB;
	venc_base++;
	*venc_base = 0x1F9901C2;
	venc_base++;
	*venc_base = 0x1FD71E67;
	venc_base++;
	*venc_base = 0x004001C2;
	venc_base++;
	*venc_base = 0x00200200;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x84465898;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F000028;
	venc_base++;
	*venc_base = 0x587800BF;
	venc_base++;
	*venc_base = 0x00000460;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x58780118;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x58780110;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;

}

void configure_venc_1080p60(u32 *venc_base, int useEmbeddedSync)
{
	THDBG("%s %d\n",  __func__, __LINE__);
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A033;
	}
	venc_base++;
	*venc_base = 0x003F0275;
	venc_base++;
	*venc_base = 0x1EA500BB;
	venc_base++;
	*venc_base = 0x1F9901C2;
	venc_base++;
	*venc_base = 0x1FD71E67;
	venc_base++;
	*venc_base = 0x004001C2;
	venc_base++;
	*venc_base = 0x00200200;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x84465898;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F000028;
	venc_base++;
	*venc_base = 0x587800BF;
	venc_base++;
	*venc_base = 0x00000460;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x58780118;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x58780110;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;

}


void configure_venc_1080i60(u32 *venc_base, int useEmbeddedSync)
{
	THDBG("%s %d\n",  __func__, __LINE__);
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A03A;
	}

	venc_base++;
	*venc_base = 0x003F0275;
	venc_base++;
	*venc_base = 0x1EA500BB;
	venc_base++;
	*venc_base = 0x1F9901C2;
	venc_base++;
	*venc_base = 0x1FD71E67;
	venc_base++;
	*venc_base = 0x004001C2;
	venc_base++;
	*venc_base = 0x00200200;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x84465898;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F245013;
	venc_base++;
	*venc_base = 0x587800C0;
	venc_base++;
	*venc_base = 0x00000230;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x587800C1;
	venc_base++;
	*venc_base = 0x0001586D;
	venc_base++;
	*venc_base = 0x0021C247;
	venc_base++;
	*venc_base = 0x0500021C;
	venc_base++;
	*venc_base = 0x05001232;
	venc_base++;
	*venc_base = 0x00234234;
	venc_base++;
	*venc_base = 0x587800C0;
	venc_base++;
	*venc_base = 0x0001586D;
	venc_base++;
	*venc_base = 0x0021C247;
	venc_base++;
	*venc_base = 0x0500021C;
	venc_base++;
	*venc_base = 0x05001232;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;
}

void configure_venc_720p60(u32* venc_base, int useEmbeddedSync)
{
	THDBG("%s %d\n",  __func__, __LINE__);
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A033;
	}

	venc_base++;
	*venc_base = 0x1FD01E24;
	venc_base++;
	*venc_base = 0x02DC020C;
	venc_base++;
	*venc_base = 0x00DA004A;
	venc_base++;
	*venc_base = 0x020C1E6C;
	venc_base++;
	*venc_base = 0x02001F88;
	venc_base++;
	*venc_base = 0x00200000;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x842EE672;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F000018;
	venc_base++;
	*venc_base = 0x50500103;
	venc_base++;
	*venc_base = 0x000002E8;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x50500172;
	venc_base++;
	*venc_base = 0x0001A64B;
	venc_base++;
	*venc_base = 0x002D0000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x5050016A;
	venc_base++;
	*venc_base = 0x0001A64B;
	venc_base++;
	*venc_base = 0x002D0000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;
}
#endif
/* ========================================================================== */
/*			  Global Functions				      */
/* ========================================================================== */

int ti81xx_hdmi_lib_init(struct ti81xx_hdmi_init_params *init_param,
		enum ti81xxhdmi_mode hdmi_mode)
{
	int rtn_value = 0x0;
	if (init_param == NULL) {
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	THDBG("hdmi Mode passed = %d\n", hdmi_mode);
	hdmi_config.is_recvr_sensed = FALSE;
	hdmi_config.is_streaming = FALSE;
	hdmi_config.is_scl_clocked = FALSE;
	hdmi_config.vSync_counter = 0x0;
	hdmi_config.is_interlaced = FALSE;

	hdmi_config.core_base_addr = init_param->core_base_addr;
	hdmi_config.wp_base_addr = init_param->wp_base_addr;
	hdmi_config.phy_base_addr = init_param->phy_base_addr;
	hdmi_config.prcm_base_addr = init_param->prcm_base_addr;
	hdmi_config.venc_base_addr = init_param->venc_base_addr;
	hdmi_config.hdmi_pll_base_addr = init_param->hdmi_pll_base_addr;

	enable_hdmi_clocks(hdmi_config.prcm_base_addr);

	if (-1 != hdmi_mode)
	{
		ti81xx_hdmi_set_mode(hdmi_mode, &hdmi_config);
		ti81xx_hdmi_lib_config(&hdmi_config.config);
		ti81xx_hdmi_lib_start(&hdmi_config, NULL);
	}
	else
	{
		memcpy(((void *) &(hdmi_config.config)),
				((void *) &default_config),
				sizeof(struct hdmi_cfg_params));
	}
exit_this_func:
	return (rtn_value);
}


int ti81xx_hdmi_copy_mode_config(enum ti81xxhdmi_mode hdmi_mode,
		struct instance_cfg *cfg)
{
	int ret_val = 0;

	THDBG("%s %d hdmi_mode = %d\n", __func__, __LINE__, hdmi_mode);
	switch (hdmi_mode)
	{
		case hdmi_1080P_60_mode:
			memcpy(&cfg->config, &config_1080p60,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_1080p60(
					(u32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		case hdmi_720P_60_mode:
			memcpy(&cfg->config, &config_720p60,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_720p60(
					(u32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		case hdmi_1080I_60_mode:
			memcpy(&cfg->config, &config_1080i60,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_1080i60(
					(u32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		case hdmi_1080P_30_mode:
			memcpy(&cfg->config, &config_1080p30,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_1080p30(
					(u32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		default:
			ret_val = -1;;
	}
	if (!ret_val)
		cfg->hdmi_mode = hdmi_mode;
	return ret_val;
}


int ti81xx_hdmi_lib_deinit(void *args)
{
	ti81xx_hdmi_lib_stop(&hdmi_config, NULL);
	hdmi_config.is_recvr_sensed = FALSE;
	hdmi_config.is_streaming = FALSE;
	hdmi_config.is_scl_clocked = FALSE;
	hdmi_config.vSync_counter = 0x0;
	hdmi_config.is_interlaced = FALSE;
	hdmi_config.core_base_addr = 0;
	hdmi_config.wp_base_addr = 0;
	hdmi_config.phy_base_addr = 0;
	hdmi_config.prcm_base_addr = 0;
	hdmi_config.venc_base_addr = 0;
	return 0;
}

/* Open 	- Power up the clock for the DDC and keeps it ON.
 *			- Register the int, update HPD if required
 */
void *ti81xx_hdmi_lib_open(u32 instance, int *status, void *args)
{
	struct instance_cfg *inst_context = NULL;
	*status = 0;
	inst_context = &(hdmi_config);
	return inst_context;
}

int ti81xx_hdmi_lib_config(struct hdmi_cfg_params *config)
{
	struct instance_cfg *inst_context = NULL;
	int rtn_value = 0x0;
	volatile u32 reset_time_out;
	volatile u32 temp;

	inst_context = &(hdmi_config);
	if (config != NULL) {
		if (check_copy_config(&(hdmi_config), config) != 0x0) {
			rtn_value = -EFAULT ;
			goto exit_this_func;
		}
	}
	reset_time_out = HDMI_WP_RESET_TIMEOUT;
	temp =
		__raw_readl(inst_context->wp_base_addr +
				HDMI_WP_SYSCONFIG_OFFSET);
	temp |= HDMI_WP_SYSCONFIG_SOFTRESET_MASK;
	__raw_writel(temp,
			(inst_context->wp_base_addr +
			 HDMI_WP_SYSCONFIG_OFFSET));

	while (((__raw_readl
					(inst_context->wp_base_addr +
					 HDMI_WP_SYSCONFIG_OFFSET)) &
				HDMI_WP_SYSCONFIG_SOFTRESET_MASK) ==
			HDMI_WP_SYSCONFIG_SOFTRESET_MASK) {
		reset_time_out--;
		if (reset_time_out == 0x0) {
			THDBG("Could not reset wrapper\n ");
			rtn_value = -EFAULT ;
			goto exit_this_func;
		}
	}

	rtn_value = configure_phy(inst_context);
	if (rtn_value != 0x0) {
		THDBG("Could not configure PHY\n");
		goto exit_this_func;
	}

	rtn_value = configure_wrapper(inst_context);
	if (rtn_value != 0x0) {
		THDBG("Could not configure wrapper\n");
		rtn_value = -EINVAL ;
	}

	temp = __raw_readl(inst_context->wp_base_addr +
			HDMI_WP_AUDIO_CTRL_OFFSET);
	temp &= (~(HDMI_WP_AUDIO_CTRL_DISABLE_MASK));
	__raw_writel(temp, (inst_context->wp_base_addr +
				HDMI_WP_AUDIO_CTRL_OFFSET));
	__raw_writel(0x0, (inst_context->wp_base_addr +
				HDMI_WP_AUDIO_CFG_OFFSET));

	temp = __raw_readl(inst_context->core_base_addr +
			HDMI_CORE_SYS_STAT_OFFSET);
	temp = (temp & HDMI_SYS_STAT_HPD_MASK) >>
		HDMI_SYS_STAT_HPD_SHIFT;
	if (temp){
			inst_context->is_recvr_sensed = TRUE;
			THDBG("Detected a sink");
	} else {
			THDBG("Sink not detected\n");
	}
	temp = __raw_readl(inst_context->core_base_addr +
			HDMI_CORE_SRST_OFFSET);
	temp |= HDMI_SRST_SWRST_MASK;
	__raw_writel(temp,
			(inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET));

	temp = __raw_readl(inst_context->core_base_addr +
			HDMI_CORE_SYS_CTRL1_OFFSET);
	temp |= HDMI_SYS_CTRL1_PD_MASK;
	__raw_writel(temp,
			(inst_context->core_base_addr + HDMI_CORE_SYS_CTRL1_OFFSET));

	rtn_value = configure_core(inst_context);
	if (rtn_value != 0x0) {
		THDBG("Could not cfg core\n");
		goto exit_this_func;
	}
	__raw_writel(0x0,
			(inst_context->core_base_addr + HDMI_CORE_AUD_MODE_OFFSET));
exit_this_func:
	return (rtn_value);
}
int ti81xx_hdmi_set_mode(enum ti81xxhdmi_mode hdmi_mode,
		struct instance_cfg *cfg)
{
	int rtn_value = 0;
#ifndef CONFIG_ARCH_TI816X
	struct hdmi_pll_ctrl *pll_ctrl;
#endif
	if (!cfg)
	{
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	if (hdmi_mode < hdmi_ntsc_mode || hdmi_mode >= hdmi_max_mode)
	{
		rtn_value = -EINVAL;
		goto exit_this_func;
	}
	ti81xx_hdmi_copy_mode_config(hdmi_mode, cfg);
#ifndef CONFIG_ARCH_TI816X
	/* Set the PLL according to the mode selected */
	switch (hdmi_mode)
	{
		case hdmi_1080P_30_mode:
		case hdmi_1080I_60_mode:
		case hdmi_720P_60_mode:
			pll_ctrl = &gpll_ctrl[1];
			break;
		case hdmi_1080P_60_mode:
			pll_ctrl = &gpll_ctrl[0];
			break;
		default:
			printk("Mode passed is incorrect\n");
			pll_ctrl = &gpll_ctrl[1];
	}
	configure_hdmi_pll(cfg->hdmi_pll_base_addr, pll_ctrl->__n, pll_ctrl->__m,
			pll_ctrl->__m2, pll_ctrl->clk_ctrl_value);
#endif



	rtn_value = ti81xx_hdmi_lib_config(&cfg->config);
exit_this_func:
	return rtn_value;

}

int ti81xx_hdmi_lib_close(void *handle, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	THDBG(">>>>ti81xx_hdmi_lib_close\n");
	HDMI_ARGS_CHECK((args == NULL));

	if (handle == NULL) {
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	if (inst_context->is_streaming == FALSE) {
		rtn_value = -EBUSY;
		goto exit_this_func;
	}
	rtn_value = -EFAULT ;
exit_this_func:
	THDBG("ti81xx_hdmi_lib_close<<<<\n");
	return (rtn_value);
}

/* TODO Not supported for now */
#if 0
static int ti81xx_hdmi_lib_get_cfg(void *handle,
		struct hdmi_cfg_params *config, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	THDBG(">>>>ti81xx_hdmi_lib_get_cfg");
	HDMI_ARGS_CHECK((args == NULL));
	if ((handle == NULL) || (config == NULL)) {
		rtn_value = -EFAULT ;
		THDBG("Invalid handle/config pointer");
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	/* Copy the configurations */
	memcpy((void *) config,
			((const void *) &(inst_context->config)),
			sizeof(struct hdmi_cfg_params));
	/* Turn OFF the config update flags */
	config->use_display_mode = 0x0;
	config->use_wp_config = 0x0;
	config->use_core_config = 0x0;
	config->use_core_path_config = 0x0;
	config->use_info_frame_config = 0x0;

exit_this_func:
	THDBG("ti81xx_hdmi_lib_get_cfg<<<<");
	return (rtn_value);
}
static int ti81xx_hdmi_lib_set_cfg(void *handle,
		struct hdmi_cfg_params *config, void *args)
{
	struct instance_cfg *inst_context = NULL;
	int rtn_value = 0x0;
	volatile u32 temp;

	THDBG(">>>>ti81xx_hdmi_lib_set_cfg");
	HDMI_ARGS_CHECK((args == NULL));

	inst_context = (struct instance_cfg *) handle;
	HDMI_ARGS_CHECK(
			(inst_context->coreRegOvrlay != NULL));
	if (inst_context->is_streaming == TRUE) {
		THDBG("Streaming - cannot re-configure");
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}
	if (config == NULL) {
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}

	rtn_value = check_copy_config(inst_context, config);
	if (rtn_value != 0x0) {
		goto exit_this_func;
	}

	rtn_value = configure_phy(inst_context);
	if (rtn_value != 0x0) {
		goto exit_this_func;
	}

	if (config->use_wp_config != 0x0) {
		rtn_value = configure_wrapper(inst_context);
		if (rtn_value != 0x0) {
			THDBG("Could not configure wrapper");
			rtn_value = -EINVAL ;
			goto exit_this_func;
		}
	}

	inst_context->coreRegOvrlay->SRST |=
		CSL_HDMI_SRST_SWRST_MASK;
	inst_context->coreRegOvrlay->SYS_CTRL1 &=
		(~(CSL_HDMI_SYS_CTRL1_PD_MASK));

	if ((config->use_core_config != 0x0) ||
			(config->use_core_path_config != 0x0)) {
		rtn_value = configure_core(inst_context);
		if (rtn_value != 0x0) {
			goto exit_this_func;
		}
	} else {
		rtn_value =
			determine_pixel_repeatation(inst_context);
		/* No pixel repetation - by default */
		inst_context->coreRegOvrlay->VID_CTRL &=
			(~(CSL_HDMI_VID_CTRL_ICLK_MASK));
		if (rtn_value == HDMI_PIXEL_REPLECATED_ONCE) {
			/* Repeat once */
			inst_context->coreRegOvrlay->VID_CTRL |=
				(((0x01u) <<
				  CSL_HDMI_VID_CTRL_ICLK_SHIFT) &
				 CSL_HDMI_VID_CTRL_ICLK_MASK);
		} else if (rtn_value ==
				HDMI_PIXEL_REPLECATED_FOUR_TIMES) {
			inst_context->coreRegOvrlay->VID_CTRL |=
				CSL_HDMI_VID_CTRL_ICLK_MASK;
		} else if (rtn_value == 0x0) {
			THDBG("No Pixel repeatation required");
		} else {
			/* Error let the caller know */
			THDBG("Could not determine pixel ");
			THDBG(" rate that would be required.");
			goto exit_this_func;
		}
		/* Power up core and bring it out of reset. */
		inst_context->coreRegOvrlay->SYS_CTRL1 |=
			CSL_HDMI_SYS_CTRL1_PD_MASK;
		inst_context->coreRegOvrlay->SRST &=
			(~(CSL_HDMI_SRST_SWRST_MASK));
	}
	/*
	 * Step 4
	 * Re-configure the wrapper with the scan type. It might have changed.
	 */
	temp =
		__raw_readl(inst_context->wp_base_addr +
				HDMI_WP_VIDEO_CFG_OFFSET);
	if (inst_context->is_interlaced == TRUE) {
		temp |=
			HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK;
	} else {
		temp &=
			(~
			 (HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK));
	}
	__raw_writel(temp,
			(inst_context->wp_base_addr +
			 HDMI_WP_VIDEO_CFG_OFFSET));

	/* Step 4 - Configure AVI Info frame and enable them to be transmitted
	   every frame */
	if (config->use_info_frame_config != 0x0) {
		rtn_value = configure_avi_info_frame(inst_context);
		if (rtn_value != 0x0) {
			goto exit_this_func;
		}
		/*
		 * Policy
		 * 1. Enabling continious transmission of AVI Information packets
		 */
		inst_context->coreRegOvrlay->PB_CTRL1 |=
			(CSL_HDMI_PB_CTRL1_AVI_EN_MASK |
			 CSL_HDMI_PB_CTRL1_AVI_RPT_MASK);
	}
exit_this_func:
	THDBG("ti81xx_hdmi_lib_set_cfg<<<<");
	return (rtn_value);
}
#endif

int ti81xx_hdmi_lib_start(void *handle, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	volatile u32 temp;



	THDBG(">>>>ti81xx_hdmi_lib_start");

	if (handle == NULL) {
		rtn_value = -EFAULT ;
		THDBG("Invalid handle/config pointer");
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	if (inst_context->is_streaming == FALSE){

		temp  = __raw_readl(inst_context->core_base_addr
				+ HDMI_CORE_SYS_STAT_OFFSET);
		temp &= HDMI_SYS_STAT_HPD_MASK;
		if (!temp)
		{
			THDBG("Sink not detected\n");
		}

		THDBG("Trying to start the port");

		temp = __raw_readl(inst_context->core_base_addr +
				HDMI_CORE_SYS_CTRL1_OFFSET);
		temp |= HDMI_SYS_CTRL1_PD_MASK;
		__raw_writel(temp,
				(inst_context->core_base_addr + HDMI_CORE_SYS_CTRL1_OFFSET));

		temp = __raw_readl(inst_context->core_base_addr +
				HDMI_CORE_SRST_OFFSET);
		temp &= (~(HDMI_SRST_SWRST_MASK));
		__raw_writel(temp,
				(inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET));

		/*
		 * Configure core would have updated the global member to
		 * specify the scan type update the wrapper with same info
		 */
		temp =
			__raw_readl(inst_context->wp_base_addr +
					HDMI_WP_VIDEO_CFG_OFFSET);

		if (inst_context->is_interlaced == TRUE) {
			temp |= HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK;
		} else {
			temp &=
				(~(HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK));
		}
		__raw_writel(temp,
				(inst_context->wp_base_addr +
				 HDMI_WP_VIDEO_CFG_OFFSET));

		rtn_value = configure_avi_info_frame(inst_context);
		if (rtn_value != 0x0) {
			THDBG("Could not configure AVI Info frames");
			goto exit_this_func;
		}

		rtn_value = configure_ctrl_packets(inst_context);
		if (rtn_value != 0x0) {
			THDBG("Could not cfg control packets");
			goto exit_this_func;
		}

		temp = __raw_readl(inst_context->core_base_addr +
				HDMI_CORE_PB_CTRL1_OFFSET);
		temp = (HDMI_PB_CTRL1_AVI_EN_MASK
				| HDMI_PB_CTRL1_AVI_RPT_MASK);
		__raw_writel(temp,
				(inst_context->core_base_addr +
				 HDMI_CORE_PB_CTRL1_OFFSET));

		temp = __raw_readl(inst_context->core_base_addr +
				HDMI_CORE_VID_MODE_OFFSET);
		temp = ((temp & HDMI_VID_MODE_DITHER_MODE_MASK) >>
				HDMI_VID_MODE_DITHER_MODE_SHIFT);
		/* General control packets are required only in deep color mode,
		 *  as packing phase would require to be indicated,
		 *  else bypass this
		 */
		if (temp != HDMI_VID_MODE_DITHER_TO_24_BITS_MODE) {
			__raw_writel((HDMI_PB_CTRL2_GEN_EN_MASK |
						HDMI_PB_CTRL2_GEN_RPT_MASK),
					(inst_context->core_base_addr +
					 HDMI_CORE_PB_CTRL2_OFFSET));
		} else {
			__raw_writel((HDMI_PB_CTRL2_CP_EN_MASK |
						HDMI_PB_CTRL2_CP_RPT_MASK |
						HDMI_PB_CTRL2_GEN_EN_MASK |
						HDMI_PB_CTRL2_GEN_RPT_MASK),
					(inst_context->core_base_addr +
					 HDMI_CORE_PB_CTRL2_OFFSET));
		}

		temp =
			__raw_readl((inst_context->wp_base_addr +
						HDMI_WP_VIDEO_CFG_OFFSET));
		temp |= HDMI_WP_VIDEO_CFG_ENABLE_MASK;
		__raw_writel(temp,
				(inst_context->wp_base_addr +
				 HDMI_WP_VIDEO_CFG_OFFSET));
		inst_context->is_streaming = TRUE;
		THDBG("Started the port");
	} else {
		if (inst_context->is_recvr_sensed == TRUE){
			rtn_value = -EFAULT ;
			THDBG("No Sinks dected-not starting");
		}
	}
exit_this_func:
	THDBG("ti81xx_hdmi_lib_start<<<<");
	return (rtn_value);
}

int ti81xx_hdmi_lib_stop(void *handle, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	volatile u32 temp;

	THDBG(">>>>ti81xx_hdmi_lib_stop");

	if (handle == NULL) {
		rtn_value = -EFAULT ;
		THDBG("Invalid handle/config pointer");
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	if (inst_context->is_streaming == TRUE) {
		THDBG("Trying to stop the port");
		temp = __raw_readl(inst_context->core_base_addr +
				HDMI_CORE_SRST_OFFSET);
		temp |= HDMI_SRST_SWRST_MASK;
		__raw_writel(temp,
				(inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET));
		temp =
			__raw_readl((inst_context->wp_base_addr +
						HDMI_WP_VIDEO_CFG_OFFSET));
		temp &= (~(HDMI_WP_VIDEO_CFG_ENABLE_MASK));
		__raw_writel(temp,
				(inst_context->wp_base_addr +
				 HDMI_WP_VIDEO_CFG_OFFSET));
		inst_context->is_streaming = FALSE;
		THDBG("Stopped the port");
	}
exit_this_func:
	THDBG("ti81xx_hdmi_lib_stop<<<<");
	return (rtn_value);
}

int ti81xx_hdmi_lib_control(void *handle,
		u32 cmd, void *cmdArgs, void *additionalArgs)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	volatile unsigned int temp;
	struct ti81xxhdmi_status  *status;

	THDBG(">>>>ti81xx_hdmi_lib_control");
	/* Validate the handle and execute the command. */
	if (handle == NULL) {
		rtn_value = -EFAULT ;
		THDBG("Invalid handle/cmdArgs pointer");
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	switch (cmd) {
		case TI81XXHDMI_START:
			rtn_value = ti81xx_hdmi_lib_start(handle, NULL);
			break;

		case TI81XXHDMI_STOP:
			rtn_value = ti81xx_hdmi_lib_stop(handle, NULL);
			break;
		case TI81XXHDMI_GET_STATUS:
			rtn_value = -EFAULT ;
			if (cmdArgs) {
				status = (struct ti81xxhdmi_status *)cmdArgs;
				status->is_hdmi_streaming =
						inst_context->is_streaming;
				temp  = __raw_readl(inst_context->core_base_addr
					+ HDMI_CORE_SYS_STAT_OFFSET);
				temp &= HDMI_SYS_STAT_HPD_MASK;
				status->is_hpd_detected =
					temp >> HDMI_SYS_STAT_HPD_SHIFT;
				rtn_value = 0x0;
			}
			break;
		case TI81XXHDMI_READ_EDID:
			rtn_value = ti81xx_hdmi_lib_read_edid(handle,
					(struct ti81xxdhmi_edid_params *)
					cmdArgs,
					NULL);
			break;
			/* TODO Not supported for now */
#if 0
		case TI81XXHDMI_GET_CONFIG:
			rtn_value = ti81xx_hdmi_lib_get_cfg(handle,
					(struct hdmi_cfg_params *)
					cmdArgs,
					NULL);
			break;
		case TI81XXHDMI_SET_CONFIG:
			{
				if (NULL != cmdArgs)
				{
					rtn_value =
						(ti81xx_hdmi_lib_config((struct hdmi_cfg_params *)cmdArgs));
				}
				else
				{
					rtn_value = -EFAULT ;
				}

			}
#endif
		case TI81XXHDMI_SET_MODE:
			rtn_value =  (ti81xx_hdmi_set_mode((enum ti81xxhdmi_mode)cmdArgs,
						inst_context));
			break;
		case TI81XXHDMI_GET_MODE:
			return (inst_context->hdmi_mode);
		case TI81XXHDMI_TEST_HDMI:
			printk("In HDMI TEST venc_base = %d\n", inst_context->venc_base_addr);
#ifdef HDMI_TEST
			switch ((enum ti81xxhdmi_mode)cmdArgs)
			{
				case hdmi_1080P_30_mode:
					configure_venc_1080p30((u32 *)inst_context->venc_base_addr, 0);
					break;
				case hdmi_1080P_60_mode:
					configure_venc_1080p60((u32 *)inst_context->venc_base_addr, 0);
					break;
				case hdmi_1080I_60_mode:
					configure_venc_1080i60((u32 *)inst_context->venc_base_addr, 0);
					break;
				case hdmi_720P_60_mode:
					configure_venc_720p60((u32 *)inst_context->venc_base_addr, 0);
					break;
				default :
					rtn_value = -EINVAL;
			}
#endif
			rtn_value =  ti81xx_hdmi_set_mode((enum ti81xxhdmi_mode)cmdArgs,
					inst_context);
			break;
		case TI81XXHDMI_GET_PHY_STAT:
			if (cmdArgs)
			{
				get_phy_status(inst_context, cmdArgs);
			}
			else
			{
				rtn_value = -EFAULT;
			}
			break;
		default:
			rtn_value = -EINVAL;
			THDBG("Un-recoganized command");
			break;
	}
exit_this_func:
	THDBG("ti81xx_hdmi_lib_control<<<<");
	return (rtn_value);
}
static int ti81xx_hdmi_lib_read_edid(void *handle,
		struct ti81xxdhmi_edid_params *r_params,
		void *args)
{
	int rtn_value = 0x0;
	u32 r_byte_cnt = 0x0;
	volatile u32 io_timeout = 0x0;
	volatile u32 timeout;
	volatile u32 cmd_status;
	volatile u32 temp;
	u8 *buf_ptr = NULL;
	struct instance_cfg *inst_context = NULL;

	THDBG(">>>>ti81xx_hdmi_lib_read_edid");

	if ((handle == NULL) || (r_params == NULL)) {
		THDBG("Invalid params ");
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	inst_context = handle;
	buf_ptr = (u8 *) r_params->buffer_ptr;
	if (buf_ptr == NULL) {
		THDBG("Invalid buffer pointer");
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	/* 10 bits to hold the count - which would be 3FF */
	if ((r_params->no_of_bytes == 0x0)
			|| (r_params->no_of_bytes > 0x3FF)) {
		THDBG("Invalid byte count");
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	r_params->no_of_bytes_read = 0x0;
	if (inst_context->is_recvr_sensed != TRUE) {
		THDBG("HPD not detected - HAL not opened");
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}
	if (r_params->timeout == 0x0){
		THDBG("Could not read in given time");
		rtn_value = -ETIME ;
		goto exit_this_func;
	}

	temp = __raw_readl((inst_context->core_base_addr +
				HDMI_CORE_RI_STAT_OFFSET));
	if ((temp & HDMI_RI_STAT_RI_STARTED_MASK) ==
			HDMI_RI_STAT_RI_STARTED_MASK) {
		THDBG("RI Check enbled - DDC bus busy");
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}

	if (inst_context->is_scl_clocked == FALSE) {
		__raw_writel(HDMI_DDC_CMD_CLOCK_SCL,
				(inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET));

		timeout = HDMI_DDC_CMD_TIMEOUT;
		temp = __raw_readl((inst_context->core_base_addr +
					HDMI_CORE_DDC_STATUS_OFFSET));
		while ((temp & HDMI_DDC_STATUS_IN_PROG_MASK)
				== HDMI_DDC_STATUS_IN_PROG_MASK) {
			timeout--;
			temp = __raw_readl((inst_context->core_base_addr +
						HDMI_CORE_DDC_STATUS_OFFSET));
		}
		if (timeout == 0x0) {
			THDBG("Could not clock SCL before read");
			rtn_value = -ETIME ;
			goto exit_this_func;
		}
		inst_context->is_scl_clocked = TRUE;
	}

	__raw_writel((HDMI_DDC_ADDR_DDC_ADDR_MASK & r_params->slave_address),
			(inst_context->core_base_addr + HDMI_CORE_DDC_ADDR_OFFSET));

	__raw_writel((HDMI_DDC_SEGM_DDC_SEGM_MASK & r_params->segment_ptr),
			(inst_context->core_base_addr + HDMI_CORE_DDC_SEGM_OFFSET));

	__raw_writel((HDMI_DDC_OFFSET_DDC_OFFSET_MASK & r_params->offset),
			(inst_context->core_base_addr + HDMI_CORE_DDC_OFFSET_OFFSET));

	__raw_writel((HDMI_DDC_COUNT1_DDC_COUNT_MASK & r_params->no_of_bytes),
			(inst_context->core_base_addr + HDMI_CORE_DDC_COUNT1_OFFSET));

	__raw_writel((HDMI_DDC_COUNT2_DDC_COUNT_MASK &
				(r_params->no_of_bytes >> 0x08)),
			(inst_context->core_base_addr + HDMI_CORE_DDC_COUNT2_OFFSET));

	__raw_writel(HDMI_DDC_CMD_CLEAR_FIFO,
			(inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET));

	timeout = HDMI_DDC_CMD_TIMEOUT;
	temp = __raw_readl((inst_context->core_base_addr +
				HDMI_CORE_DDC_STATUS_OFFSET));
	while ((temp & HDMI_DDC_STATUS_IN_PROG_MASK)
			== HDMI_DDC_STATUS_IN_PROG_MASK) {
		timeout--;
		temp = __raw_readl((inst_context->core_base_addr +
					HDMI_CORE_DDC_STATUS_OFFSET));
	}
	if (timeout == 0x0) {
		THDBG("Could not clear FIFOs");
		rtn_value = -ETIME ;
		goto abort_exit_this_func;
	}

	io_timeout = r_params->timeout;
	if (r_params->use_eddc_read == 0x0){
		__raw_writel(HDMI_DDC_CMD_SEQ_R_NO_ACK_ON_LAST_BYTE,
				(inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET));
	}else{
		__raw_writel(HDMI_DDC_CMD_EDDC_R_NO_ACK_ON_LAST_BYTE,
				(inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET));
	}
	temp = __raw_readl((inst_context->core_base_addr +
				HDMI_CORE_DDC_FIFOCNT_OFFSET));
	while (temp == 0x0) {
		if (io_timeout == 0x0){
			rtn_value = -ETIME ;
			goto abort_exit_this_func;
		}
		temp = __raw_readl((inst_context->core_base_addr +
					HDMI_CORE_DDC_FIFOCNT_OFFSET));
		io_timeout--;
	}
	/* Check for errors */
	cmd_status = __raw_readl((inst_context->core_base_addr +
				HDMI_CORE_DDC_STATUS_OFFSET));

	if ((cmd_status & HDMI_DDC_STATUS_BUS_LOW_MASK) ==
			HDMI_DDC_STATUS_BUS_LOW_MASK) {
		/* Bus is being held by the slave / others...
		   Ultra Slow slaves? */
		THDBG("Bus being held low");
		rtn_value = -EINVAL ;
		goto abort_exit_this_func;
	}
	if ((cmd_status & HDMI_DDC_STATUS_NO_ACK_MASK) ==
			HDMI_DDC_STATUS_NO_ACK_MASK) {
		/* UnPlugged TV? */
		THDBG("No ACK from the device");
		rtn_value = -EINVAL ;
		goto abort_exit_this_func;
	}
	while (r_byte_cnt < r_params->no_of_bytes){
		if (inst_context->is_recvr_sensed != TRUE) {
			rtn_value = -ETIME ;
			goto abort_exit_this_func;
		}
		temp = __raw_readl((inst_context->core_base_addr +
					HDMI_CORE_DDC_FIFOCNT_OFFSET));
		if (temp == 0x0){
			while (temp == 0x0)
			{
				if (io_timeout == 0x0){
					rtn_value = -ETIME ;
					goto abort_exit_this_func;
				}
				io_timeout--;
				temp = __raw_readl(
						(inst_context->core_base_addr +
						 HDMI_CORE_DDC_FIFOCNT_OFFSET));
			}
		}

		*buf_ptr = (u8) ((__raw_readl((inst_context->core_base_addr
							+ HDMI_CORE_DDC_DATA_OFFSET))) &
				HDMI_DDC_DATA_DDC_DATA_MASK);
		buf_ptr++;
		r_byte_cnt++;
	}
	/*
	 * Aborting the READ command.
	 * In case we have completed as expected - no of bytes to read is read
	 *	- No issues, aborting on completion is OK
	 * If device was unplugged before read could be complete,
	 *	- Abort should leave the bus clean
	 * If any other error
	 *	- Ensure bus is clean
	 */
	r_params->no_of_bytes_read = r_byte_cnt;

abort_exit_this_func:
	__raw_writel(HDMI_DDC_CMD_ABORT,
			(inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET));

	temp = __raw_readl((inst_context->core_base_addr +
				HDMI_CORE_DDC_STATUS_OFFSET));
	while ((temp & HDMI_DDC_STATUS_IN_PROG_MASK)
			== HDMI_DDC_STATUS_IN_PROG_MASK) {
		timeout--;
		temp = __raw_readl((inst_context->core_base_addr +
					HDMI_CORE_DDC_STATUS_OFFSET));
	}
exit_this_func:
	THDBG("ti81xx_hdmi_lib_read_edid<<<<");
	return (rtn_value);
}
static void HDMI_ARGS_CHECK(u32 condition)
{
	return;
}
