/**
 * Implements support for TPS65023
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
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

#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/i2c/twl.h>

#include <plat/voltage.h>

#include "pm.h"

#define	TPS65023_VDCDC1_MIN		800000	/* 0.8V		*/
#define	TPS65023_VDCDC1_STEP		25000	/* 0.025V	*/


/*
 * Get voltage corresponding to specified vsel value using this formula:
 *	Vout = 0.8V + (25mV x Vsel)
 */
static unsigned long tps65023_vsel_to_uv(const u8 vsel)
{
	return (TPS65023_VDCDC1_MIN + (TPS65023_VDCDC1_STEP * vsel));
}

/*
 * Get vsel value corresponding to specified voltage using this formula:
 *	Vsel = (Vout - 0.8V)/ 25mV
 */
static u8 tps65023_uv_to_vsel(unsigned long uv)
{
	return DIV_ROUND_UP(uv - TPS65023_VDCDC1_MIN, TPS65023_VDCDC1_STEP);
}

/*
 * TPS65023 is currently supported only for AM35x devices.
 * Therefore, implementation below is specific to this device pair.
 */

/**
 * Voltage information related to the MPU voltage domain of the
 * AM35x processors - in relation to the TPS65023.
 */
static struct omap_volt_pmic_info tps65023_am35xx_mpu_volt_info = {
	.step_size		= 25000,
	.on_volt                = 1200000,
	.vsel_to_uv		= tps65023_vsel_to_uv,
	.uv_to_vsel		= tps65023_uv_to_vsel,
};

int __init omap3_tps65023_init(void)
{
	struct voltagedomain *voltdm;

	if (!cpu_is_omap34xx())
		return -ENODEV;

	if (cpu_is_omap3505() || cpu_is_omap3517()) {
		voltdm = omap_voltage_domain_lookup("mpu");
		omap_voltage_register_pmic(voltdm, &tps65023_am35xx_mpu_volt_info);
		voltdm = omap_voltage_domain_lookup("core");
		omap_voltage_register_pmic(voltdm, &tps65023_am35xx_mpu_volt_info);
	} else {
		/* TODO:
		 * Support for other devices that support TPS65023
		 */
	}

	return 0;
}
