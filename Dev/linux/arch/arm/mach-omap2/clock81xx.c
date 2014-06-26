/*
 * TI81XX specific clock ops.
 *
 * Copyright (C) 2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/clk.h>

#include <plat/prcm.h>

#include "clock.h"
#include "clock81xx.h"
#include "prm.h"
#include "prm2xxx_3xxx.h"
#include "prm-regbits-81xx.h"
#include "cm.h"
#include "cm-regbits-81xx.h"

/**
 * ti81xx_dflt_wait_clk_enable() - Enable a ti816x/ti814x module clock
 * @clk: Pointer to the clock to be enabled
 *
 * This function just wraps omap2_dflt_clk_enable with a check for module idle
 * status. We loop till module goes to funcitonal state as the immediate access
 * to module space will not work otherwise.
 */
int ti81xx_dflt_wait_clk_enable(struct clk *clk)
{
	omap2_dflt_clk_enable(clk);

	omap2_cm_wait_idlest(clk->enable_reg, TI81XX_IDLEST_MASK,
			TI81XX_IDLEST_VAL, clk->name);

	return 0;
}

int ti81xx_pcie_clk_enable(struct clk *clk)
{
	omap2_dflt_clk_enable(clk);

	/* De-assert local reset after module enable */
	omap2_prm_clear_mod_reg_bits(TI81XX_PCI_LRST_MASK,
			TI81XX_PRM_DEFAULT_MOD,
			TI81XX_RM_RSTCTRL);

	omap2_cm_wait_idlest(clk->enable_reg, TI81XX_IDLEST_MASK,
			TI81XX_IDLEST_VAL, clk->name);

	return 0;
}

void ti81xx_pcie_clk_disable(struct clk *clk)
{
	/* Assert local reset */
	omap2_prm_set_mod_reg_bits(TI81XX_PCI_LRST_MASK,
			TI81XX_PRM_DEFAULT_MOD,
			TI81XX_RM_RSTCTRL);

	omap2_dflt_clk_disable(clk);
}

int ti81xx_usb_clk_enable(struct clk *clk)
{
	omap2_dflt_clk_enable(clk);

	/* De-assert local reset after module enable */
	omap2_prm_clear_mod_reg_bits(TI81XX_USB1_LRST_MASK
			| TI81XX_USB2_LRST_MASK,
			TI81XX_PRM_DEFAULT_MOD,
			TI81XX_RM_RSTCTRL);

	omap2_cm_wait_idlest(clk->enable_reg, TI81XX_IDLEST_MASK,
			TI81XX_IDLEST_VAL, clk->name);

	return 0;
}

void ti81xx_usb_clk_disable(struct clk *clk)
{
	/* Assert local reset */
	omap2_prm_set_mod_reg_bits(TI81XX_USB1_LRST_MASK
			| TI81XX_USB2_LRST_MASK,
			TI81XX_PRM_DEFAULT_MOD,
			TI81XX_RM_RSTCTRL);

	omap2_dflt_clk_disable(clk);
}


const struct clkops clkops_ti81xx_dflt_wait = {
	.enable         = ti81xx_dflt_wait_clk_enable,
	.disable	= omap2_dflt_clk_disable,
};

const struct clkops clkops_ti81xx_pcie = {
	.enable         = ti81xx_pcie_clk_enable,
	.disable        = ti81xx_pcie_clk_disable,
};

const struct clkops clkops_ti81xx_usb = {
	.enable         = ti81xx_usb_clk_enable,
	.disable        = ti81xx_usb_clk_disable,
};
