/*
 * linux/drivers/char/ti81xx_hdmi/hdmi_cfg.h
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
 */

#ifndef _HDMI_CFG_H
#define _HDMI_CFG_H

#define TRUE	1
#define FALSE	0

/* ========================================================================== */
/*			  HDMI Silicon dependecy Do Not Alter		      */
/* ========================================================================== */
#define HDMI_PER_CNT		(1u)
#define HDMI_CORE_0_REGS	(0x46c00400u)
#define HDMI_WP_0_REGS		(0x46c00000u)
#ifndef CONFIG_ARCH_TI816X
#define HDMI_PHY_0_REGS 	(0x46c00300u)
#else
#define HDMI_PHY_0_REGS 	(0x48122000u)
#endif
#define PRCM_0_REGS 		(0x48180000u)


/* TODO Get the exact default value.  Will get from the IP Team.*/
#define HDMI_PHY_DEF_DE_EMPHASIS_VAL	(0x5)

/* TODO Get the exact values based on frequency from IP team */
#define HDMI_PHY_DEF_LDO_VOLTAGE_VAL	(0xB)

/* TODO Get the exact values for the threshold detection */
#define HDMI_PHY_DEF_VTHRESHPU_CNTL_VAL  (0x0)

#define VPS_PRCM_MAX_REP_CNT                    (10u)

/* Width of R/G/B or Y/Cb/Cr channels */
enum hdmi_bits_per_chan {
	hdmi_8_bits_chan_width = 0x0,
	hdmi_10_bits_chan_width,
	hdmi_12_bits_chan_width,
	hdmi_max_bits_chan_width
};

/* Syncs configurations */
enum hdmi_core_sync_gen {
	hdmi_extract_syncs = 0x0,
	/* Use extracted hSync, vSync and DE from input */
	hdmi_generate_de,
	/* Use incoming (from wrapper/hdvenc) vSync and hSync but generate DE */
	hdmi_source_syncs,
	/* Use incoming (from wrapper/hdvenc) vSync, hSync and DE */
	hdmi_max_syncs
};

/* Packing mode that wrappers supports */
enum hdmi_wp_packmode {
	hdmi_wp_30bit_RGB_YUV444 = 0x0,
	hdmi_wp_24bit_RGB_YUV444_YUV422,
	hdmi_wp_20bit_YUV444,
	hdmi_wp_16bit_YUV422,
	hdmi_wp_no_pack = 0x7
};

/* \brief structure to keep track of pll configurations for a video mode */
struct hdmi_pll_ctrl
{
	u32                  __n;
	/**< Divider N for the PLL.*/
	u32                  __m;
	/**< Multiplier M for the PLL.*/
	u32                  __m2;
	/**< Divider M2 for the PLL.*/
	u32                  clk_ctrl_value;
	/**< For comparison based on the clkOut used */
};

/*	YCbCr to RGB CSC coefficients */
struct hdmi_csc_YCbCr_2_RGB_coeff {
	u32 Y2RCOEFF_L;
	u32 Y2RCOEFF_H;
	u32 CR2RCOEFF_L;
	u32 CR2RCOEFF_H;
	u32 CB2BCOEFF_L;
	u32 CB2BCOEFF_H;
	u32 CR2GCOEFF_L;
	u32 CR2GCOEFF_H;
	u32 YOFFS1_L;
	u32 YOFFS1_U;
	u32 OFFS1_L;
	u32 OFFS1_M;
	u32 OFFS1_H;
	u32 OFFS2_L;
	u32 OFFS2_H;
	u32 DC_LEV_L;
	u32 DC_LEV_H;
};

/* Control options for YCbCr to RGB CSC */
struct hdmi_csc_YCbCr_2_RGB_ctrl {
	u32 enableRngExp;
	u32 enableFullRngExp;
	u32 customCoEff;
	u32 srcCsSel;
	/* Select source color space - xvYCC if non-zero, YCbCr otherwise */
	struct hdmi_csc_YCbCr_2_RGB_coeff coEff;
};

/* Dithering control options */
struct hdmi_dither_cfg {
	u32 M_D2;
	u32 UP2;
	u32 STR_422_EN;
	u32 D_BC_EN;
	u32 D_GC_EN;
	u32 D_RC_EN;
	u32 DRD;
};

/*
 * Defines the configurable parameters of in data path of HDMI block.
 * For enabled sub-block might require additional configurations
 */
struct hdmi_core_data_path {
	u32 up_sampler_enable;
	/*	422 to 444 */
	u32 csc_YCbCr_2_RGB_enable;
	struct hdmi_csc_YCbCr_2_RGB_ctrl csc_YCbCr_2_RGB_config;
	u32 range_exp_RGB_enable;
	u32 cscRGB_2_YCbCr_enable;
	u32 csc_convert_standard;
	/* Specifies the color space standard to be used for color space
	   conversions. non-zero value would configure to use BT.709,
	   BT.601 otherwise. */
	u32 range_comp_enable;
	u32 down_sampler_enable;
	/*	444 to 422 */
	u32 range_clip_enable;
	u32 clip_color_space;
	/* Specifies output color space of the clipper, non-zero value for
	   YCbCr, RGB otherwise */
	u32 dither_enable;
	struct hdmi_dither_cfg dither_config;
	enum hdmi_bits_per_chan output_width;
	/* Specifies the number of bits per channel that would sent out.
	   If dithering is not enabled, the output would be truncated to the
	   width specified here. */
};

/* Control DE generation */
struct hdmi_core_de_dly_cfg {
	u32 DE_DLY;
	u32 DE_TOP;
	u32 DE_CNTL;
	u32 DE_CNTH;
	u32 DE_LINL;
	u32 DE_LINH;
};

/* Core input control configurations */
struct hdmi_core_input_cfg {
	enum hdmi_bits_per_chan data_bus_width;
	enum hdmi_core_sync_gen sync_gen_cfg;
	struct hdmi_core_de_dly_cfg de_delay_cfg;
	/* when configured to generate DE - would require to provide this */
	u32 edge_pol;
	/* A non-zero value configure to latch input on rising edge, falling
	   edge otherwise */
};

/* Wrapper control configuration */
struct hdmi_wp_config {
	u32 debounce_rcv_detect;
	u32 debounce_rcv_sens;
	u32 is_slave_mode;
	/* Should be TRUE / positive for this TI 81XX device */
	enum hdmi_wp_packmode pack_mode;
	u32 is_vSync_pol_inv;
	u32 is_hSync_pol_inv;
	enum hdmi_bits_per_chan width;
	u32 vSync_pol;
	u32 hSync_pol;
	u32 hbp;
	/* Not Supported - for TI 81XX */
	u32 hfp;
	/*	Not Supported - for TI 81XX */
	u32 hsw;
	/* Not Supported - for TI 81XX */
	u32 vbp;
	/* Not Supported - for TI 81XX */
	u32 vfp;
	/* Not Supported - for TI 81XX */
	u32 vsw;
	/* Not Supported - for TI 81XX */
};

/* Transmitted color space values */
enum hdmi_avi_op_cs {
	hdmi_avi_RGB_op_cs = 0x0,
	hdmi_avi_YCbCr_422_op_cs = 0x1,
	hdmi_avi_YCbCr_444_op_cs = 0x2,
	hdmi_avi_max_op_cs
};

enum hdmi_avi_activ_ratio {
	hdmi_avi_no_aspect_ratio = 0x0,
	hdmi_avi_active_aspect_ratio,
	hdmi_avi_max_aspect_ratio
};

struct hdmi_avi_bar_info {
	u32 barInfoValid;
	/* Specify if the BAR information is valid or not.
	   0x01 - Vertical Bar Info is valid
	   0x02 - Horizontal Bar Info is valid
	   0x03 - Both Vertical & Horizontal is valid */
	u32 topBar;
	u32 bottomBar;
	u32 leftBar;
	u32 rightBar;
};

enum hdmi_avi_scan_info {
	hdmi_avi_none_scan_info = 0x0,
	hdmi_avi_over_scan,
	hdmi_avi_under_scan,
	hdmi_avi_max_scan
};

enum hdmi_avi_colorimetry {
	hdmi_avi_none_colorimetry = 0x0,
	hdmi_avi_BT601_colorimetry,
	hdmi_avi_BT709_colorimetry,
	hdmi_avi_max_colorimetry
};

enum hdmi_avi_aspectratio {
	hdmi_avi_aspect_ratio_none = 0x0,
	hdmi_avi_4_3_aspect_ratio,
	hdmi_avi_16_9_aspect_ratio,
	hdmi_avi_aspect_ratio_max
};

enum hdmi_avi_active_aspectratio {
	/* Do not modify these values - would be used to program directly */
	hdmi_avi_active_aspect_ratio_same = 0x8,
	hdmi_avi_4_3_active_aspect_ratio = 0x9,
	hdmi_avi_16_9_active_aspect_ratio = 0xA,
	hdmi_avi_14_9_active_aspect_ratio = 0xB,
	hdmi_avi_active_aspect_ratio_max = 0xC
};

enum hdmi_avi_non_uniform_sc {
	hdmi_avi_non_uniform_scaling_none = 0x0,
	hdmi_avi_horizontal_non_uniform_scaling = 0x1,
	hdmi_avi_vertical_non_uniform_scaling = 0x2,
	hdmi_avi_non_uniform_scaling_max = 0x3
};

struct hdmi_avi_frame_cfg {
	u32 output_cs;
	enum hdmi_avi_activ_ratio use_active_aspect_ratio;
	struct hdmi_avi_bar_info bar_info;
	enum hdmi_avi_scan_info scan_info;
	enum hdmi_avi_colorimetry colorimetry_info;
	enum hdmi_avi_aspectratio aspect_ratio;
	enum hdmi_avi_active_aspectratio active_aspect_ratio;
	u32 it_content_present;
	u32 ext_colorimetry;
	u32 quantization_range;
	enum hdmi_avi_non_uniform_sc non_uniform_sc;
	u32 format_identier;
};

struct hdmi_info_frame_cfg {
	u32 use_avi_info_data;
	struct hdmi_avi_frame_cfg aviData;
	/* When audio, GAMUT is supported, add audio info packet data, GAMUT
	   packet data, SPD packet (Source Product Description ) here */
};

/* Configuration parameters for HDMI */
struct hdmi_cfg_params {
	u32 use_display_mode;
	enum ti81xxhdmi_mode display_mode;
	u32 use_wp_config;
	struct hdmi_wp_config wp_config;
	u32 use_core_config;
	struct hdmi_core_input_cfg core_config;
	u32 use_core_path_config;
	struct hdmi_core_data_path core_path_config;
	u32 use_info_frame_config;
	struct hdmi_info_frame_cfg info_frame_config;
};

struct ti81xx_hdmi_init_params {
	u32 wp_base_addr;
	u32 core_base_addr;
	u32 phy_base_addr;
	u32 prcm_base_addr;
	u32 venc_base_addr;
	u32 hdmi_pll_base_addr;
};

/* ========================================================================== */
/*			Function Declarations    			      */
/* ========================================================================== */

int ti81xx_hdmi_lib_init(struct ti81xx_hdmi_init_params *initParams,
		enum ti81xxhdmi_mode hdmi_mode);

int ti81xx_hdmi_lib_deinit(void *args);

void* ti81xx_hdmi_lib_open(u32 instance, int *status, void *args);
int ti81xx_hdmi_lib_close(void *handle, void *args);

int ti81xx_hdmi_lib_start(void *handle, void *args);

int ti81xx_hdmi_lib_stop(void *handle, void *args);

int ti81xx_hdmi_lib_control(void *handle, u32 cmd, void *cmdArgs,
		void *additionalArgs);
/* ========================================================================== */
/*		   Defaults used to initialize different modes	 	      */
/* ========================================================================== */
#define TI81xx_HDMIWPCONFIG_10BIT_IF_SALVE {\
	0x14, \
	0x14, \
	TRUE, \
	hdmi_wp_30bit_RGB_YUV444,\
	FALSE, \
	FALSE, \
	\
	hdmi_10_bits_chan_width, \
	TRUE, TRUE, 0x0, 0x0, 0x0, 0x0, 0x0,\
	0x0 }
#define TI81xx_HDMIWPCONFIG_8BIT_IF_SALVE {\
	0x14, \
	0x14, \
	TRUE, \
	hdmi_wp_24bit_RGB_YUV444_YUV422,\
	FALSE, \
	FALSE, \
	\
	hdmi_8_bits_chan_width, \
	TRUE, TRUE, 0x0, 0x0, 0x0, 0x0, 0x0,\
	0x0}

#define TI81XX_HDMICORE_IPCFG_10BIT_IF_SRCD_SYNC {\
	hdmi_10_bits_chan_width,\
	hdmi_source_syncs,\
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0},\
	TRUE}
#define TI81XX_HDMICORE_IPCFG_8BIT_IF_SRCD_SYNC {\
	hdmi_8_bits_chan_width,\
	hdmi_source_syncs,\
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0},\
	TRUE}
#define TI81XX_HDMICSC_YCBCR2RGB_COEFF {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,\
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,\
	0x0}
#define TI81XX_HDMICSC_YCBCR2RGBCTRL_DISABLED {FALSE, \
	FALSE, \
	FALSE, \
	FALSE, \
	TI81XX_HDMICSC_YCBCR2RGB_COEFF}

#define TI81XX_HDMIDITHERCONFIG {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}

/*	 Defaults to initialize core - by pass all modules, sets outwidth
	 to 10 bits/channel and BT709 for TV */
#define TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_10BIT_OUTPUT_BT709 {\
	FALSE, \
	FALSE, \
	TI81XX_HDMICSC_YCBCR2RGBCTRL_DISABLED,\
	FALSE, \
	FALSE, \
	TRUE, \
	FALSE, \
	FALSE, \
	FALSE, \
	FALSE, \
	FALSE, \
	TI81XX_HDMIDITHERCONFIG, \
	hdmi_10_bits_chan_width}


#define TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_8BIT_OUTPUT_BT709 {\
	FALSE, \
	FALSE, \
	TI81XX_HDMICSC_YCBCR2RGBCTRL_DISABLED,\
	FALSE, \
	FALSE, \
	TRUE, \
	FALSE, \
	FALSE, \
	FALSE, \
	FALSE, \
	FALSE, \
	TI81XX_HDMIDITHERCONFIG, \
	hdmi_8_bits_chan_width}

#define TI81XX_HDMI_AVI_INFOFRAME_BARINFO	{0x0, 0x0, 0x0, 0x0, 0x0}

/*	 Configures HDMI AVI Info Frame with following configurations
	 output color space as RGB, Overscan - for TV, colorometry 709-HD TV
	 aspect ration as 16:9 */

#define TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT709_169  {\
	hdmi_avi_RGB_op_cs, \
	hdmi_avi_no_aspect_ratio, \
	TI81XX_HDMI_AVI_INFOFRAME_BARINFO, \
	hdmi_avi_over_scan, \
	hdmi_avi_BT709_colorimetry, \
	hdmi_avi_16_9_aspect_ratio, \
	hdmi_avi_active_aspect_ratio_same, \
	FALSE, \
	0x0, \
	0x0, \
	hdmi_avi_non_uniform_scaling_none, \
	0x0}

/*	 Configures HDMI AVI Info Frame with following configurations
	 output color space as RGB, Overscan - for TV, colorometry 709-HD TV
	 aspect ration as 4:3 */

#define TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT709_43  {\
	hdmi_avi_RGB_op_cs, \
	hdmi_avi_no_aspect_ratio, \
	TI81XX_HDMI_AVI_INFOFRAME_BARINFO, \
	hdmi_avi_over_scan, \
	hdmi_avi_BT709_colorimetry, \
	hdmi_avi_4_3_aspect_ratio, \
	hdmi_avi_active_aspect_ratio_same, \
	FALSE, \
	0x0, \
	0x0, \
	hdmi_avi_non_uniform_scaling_none, \
	0x0}
/*	 Configures HDMI AVI Info Frame with following configurations
	 output color space as RGB, Overscan - for TV, colorometry 601-SD TV
	 aspect ration as 4:3 */
#define TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT601_43  {\
	hdmi_avi_RGB_op_cs, \
	hdmi_avi_no_aspect_ratio, \
	TI81XX_HDMI_AVI_INFOFRAME_BARINFO, \
	hdmi_avi_over_scan, \
	hdmi_avi_BT601_colorimetry, \
	hdmi_avi_4_3_aspect_ratio, \
	hdmi_avi_active_aspect_ratio_same, \
	FALSE, \
	0x0, \
	0x0, \
	hdmi_avi_non_uniform_scaling_none, \
	0x0}

/*	 Configures HDMI AVI Info Frame with following configurations
	 output color space as RGB, Overscan - for TV, colorometry 601-SD TV
	 aspect ration as none */
#define TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT601_NO_ASPECT_RATIO  {\
	hdmi_avi_RGB_op_cs, \
	hdmi_avi_no_aspect_ratio, \
	TI81XX_HDMI_AVI_INFOFRAME_BARINFO, \
	hdmi_avi_over_scan, \
	hdmi_avi_BT601_colorimetry, \
	hdmi_avi_aspect_ratio_none, \
	hdmi_avi_active_aspect_ratio_same, \
	FALSE, \
	0x0, \
	0x0, \
	hdmi_avi_non_uniform_scaling_none, \
	0x0}

/*	 Configures HDMI Packets that would be sent during data island period
	 Right now, AVI Info packets are supported. For HD Display */
#define TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_43	 {TRUE, \
	TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT709_43}
#define TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169  {TRUE, \
	TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT709_169}
/*	 Configures HDMI Packets that would be sent during data island period
	 Right now, AVI Info packets are supported. For NTSC display*/
#define TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_NTSC   {TRUE, \
	TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT601_NO_ASPECT_RATIO}
/* Configures HDMI Packets that would be sent during data island period
   Right now, AVI Info packets are supported. For APL display */
#define TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_PAL   {TRUE, \
	TI81XX_HDMI_AVI_INFOFRAME_RGB_OVERSCAN_BT601_43}


/* ========================================================================== */
/*			  Defaults that could be used to initialize HDMI				  */
/*	Recommended to start with struct hdmi_wp_config, if reading for first time*/
/*	To understand the configurations paramters for the HDMI 				  */
/* ========================================================================== */
/*
 * Defaults that could be used initialize HDMI HAL in 1080 P, 60 FPS,
 * 4:3 Aspect Ratio, for HD TV
 * Wrapper - confiured as 10 bit interface with HDVENC and SLAVE
 * Core input configured as 10 bit interface with syncs sourced from
 * wrapper
 * Core Data path - All in bypass mode, outwidth set 10 bits/channel
 *- In case color space converter is enabled - set to BT709.
 */
#define TI81XX_HDMI_10BIT_1080p_60_16_9_HD {\
	TRUE, hdmi_1080P_60_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_10BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_10BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_10BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169}

#define TI81XX_HDMI_8BIT_1080p_60_16_9_HD {\
	TRUE, hdmi_1080P_60_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_8BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_8BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_8BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169}

/*
 * Defaults that could be used initialize HDMI HAL in 1080 P, 30 FPS,
 * 4:3 Aspect Ratio, for HD TV
 * Wrapper - confiured as 10 bit interface with HDVENC and SLAVE
 * Core input configured as 10 bit interface with syncs sourced from
 * wrapper
 * Core Data path - All in bypass mode, outwidth set 10 bits/channel
 *- In case color space converter is enabled - set to BT709.
 */
#define TI81XX_HDMI_10BIT_1080p_30_16_9_HD {\
	TRUE, hdmi_1080P_30_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_10BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_10BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_10BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169}

#define TI81XX_HDMI_8BIT_1080p_30_16_9_HD {\
	TRUE, hdmi_1080P_30_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_8BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_8BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_8BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169}

/*
 * Defaults that could be used initialize HDMI HAL in 1080I P, 60 FPS,
 * 4:3 Aspect Ratio, for HD TV
 * Wrapper - confiured as 10 bit interface with HDVENC and SLAVE
 * Core input configured as 10 bit interface with syncs sourced from
 * wrapper
 * Core Data path - All in bypass mode, outwidth set 10 bits/channel
 *- In case color space converter is enabled - set to BT709.
 */
#define TI81XX_HDMI_10BIT_1080i_60_16_9_HD {\
	TRUE, hdmi_1080I_60_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_10BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_10BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_10BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169}

#define TI81XX_HDMI_8BIT_1080i_60_16_9_HD {\
	TRUE, hdmi_1080I_60_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_8BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_8BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_8BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_169}

/*
 * Defaults that could be used initialize HDMI HAL in 720 P, 60 FPS,
 * 16:9 Aspect Ratio, for HD TV
 * Wrapper - confiured as 10 bit interface with HDVENC and SLAVE
 * Core input configured as 10 bit interface with syncs sourced from
 * wrapper
 * Core Data path - All in bypass mode, outwidth set 10 bits/channel
 *- In case color space converter is enabled - set to BT709.
 */
#define TI81XX_HDMI_10BIT_720_60_16_9_HD {\
	TRUE, hdmi_720P_60_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_10BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_10BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_10BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_43}

#define TI81XX_HDMI_8BIT_720_60_16_9_HD {\
	TRUE, hdmi_720P_60_mode,\
	TRUE, TI81xx_HDMIWPCONFIG_8BIT_IF_SALVE,\
	TRUE, TI81XX_HDMICORE_IPCFG_8BIT_IF_SRCD_SYNC,\
	TRUE, TI81XX_HDMICOREDATAPATHCONFIG_BYPS_ALL_8BIT_OUTPUT_BT709,\
	TRUE, TI81XX_HDMIINFOFRAME_CFG_RGB_OVERSCAN_BT709_43}



#ifdef DEBUG
#define THDBG(format, ...) \
	do { \
		printk(KERN_DEBUG "TI81XXHDMI : " format, \
## __VA_ARGS__); \
	} while (0)
#else
#define THDBG(format, ...)
#endif

#endif				/* _HDMI_CFG_H */

