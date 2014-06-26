/*
 * pm.c - Common OMAP2+ power management-related code
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2010 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/opp.h>
#include <linux/delay.h>

#include <plat/omap-pm.h>
#include <plat/omap_device.h>
#include <plat/common.h>
#include <plat/voltage.h>

#include "powerdomain.h"
#include "clockdomain.h"
#include "cm-regbits-34xx.h"
#include "cm2xxx_3xxx.h"
#include "pm.h"

static struct omap_device_pm_latency *pm_lats;

static struct device *mpu_dev;
static struct device *iva_dev;
static struct device *l3_dev;
static struct device *dsp_dev;

static struct clk *dpll1_clk, *dpll2_clk, *dpll3_clk;

struct device *omap2_get_mpuss_device(void)
{
	WARN_ON_ONCE(!mpu_dev);
	return mpu_dev;
}

struct device *omap2_get_iva_device(void)
{
	WARN_ON_ONCE(!iva_dev);
	return iva_dev;
}

struct device *omap2_get_l3_device(void)
{
	WARN_ON_ONCE(!l3_dev);
	return l3_dev;
}

struct device *omap4_get_dsp_device(void)
{
	WARN_ON_ONCE(!dsp_dev);
	return dsp_dev;
}
EXPORT_SYMBOL(omap4_get_dsp_device);

#ifndef CONFIG_CPU_FREQ
static unsigned long compute_lpj(unsigned long ref, u_int div, u_int mult)
{
	unsigned long new_jiffy_l, new_jiffy_h;

	/*
	 * Recalculate loops_per_jiffy.  We do it this way to
	 * avoid math overflow on 32-bit machines.  Maybe we
	 * should make this architecture dependent?  If you have
	 * a better way of doing this, please replace!
	 *
	 *    new = old * mult / div
	 */
	new_jiffy_h = ref / div;
	new_jiffy_l = (ref % div) / 100;
	new_jiffy_h *= mult;
	new_jiffy_l = new_jiffy_l * mult / div;

	return new_jiffy_h + new_jiffy_l * 100;
}
#endif

/* static int _init_omap_device(struct omap_hwmod *oh, void *user) */
static int _init_omap_device(char *name, struct device **new_dev)
{
	struct omap_hwmod *oh;
	struct omap_device *od;

	oh = omap_hwmod_lookup(name);
	if (WARN(!oh, "%s: could not find omap_hwmod for %s\n",
		 __func__, name))
		return -ENODEV;

	od = omap_device_build(oh->name, 0, oh, NULL, 0, pm_lats, 0, false);
	if (WARN(IS_ERR(od), "%s: could not build omap_device for %s\n",
		 __func__, name))
		return -ENODEV;

	*new_dev = &od->pdev.dev;

	return 0;
}

static unsigned long omap3_mpu_get_rate(struct device *dev)
{
	return dpll1_clk->rate;
}

static int omap3_mpu_set_rate(struct device *dev, unsigned long rate)
{
	unsigned long cur_rate = omap3_mpu_get_rate(dev);
	int ret;

#ifdef CONFIG_CPU_FREQ
	struct cpufreq_freqs freqs_notify;

	freqs_notify.old = cur_rate / 1000;
	freqs_notify.new = rate / 1000;
	freqs_notify.cpu = 0;
	/* Send pre notification to CPUFreq */
	cpufreq_notify_transition(&freqs_notify, CPUFREQ_PRECHANGE);
#endif
	ret = clk_set_rate(dpll1_clk, rate);
	if (ret) {
		dev_warn(dev, "%s: Unable to set rate to %ld\n",
			__func__, rate);
		return ret;
	}

#ifdef CONFIG_CPU_FREQ
	/* Send a post notification to CPUFreq */
	cpufreq_notify_transition(&freqs_notify, CPUFREQ_POSTCHANGE);
#endif

#ifndef CONFIG_CPU_FREQ
	/*Update loops_per_jiffy if processor speed is being changed*/
	loops_per_jiffy = compute_lpj(loops_per_jiffy,
			cur_rate / 1000, rate / 1000);
#endif
	return 0;
}

static int omap3_iva_set_rate(struct device *dev, unsigned long rate)
{
	return clk_set_rate(dpll2_clk, rate);
}

static unsigned long omap3_iva_get_rate(struct device *dev)
{
	return dpll2_clk->rate;
}

static int omap3_l3_set_rate(struct device *dev, unsigned long rate)
{
	int l3_div;

	l3_div = omap2_cm_read_mod_reg(CORE_MOD, CM_CLKSEL) &
			OMAP3430_CLKSEL_L3_MASK;

	return clk_set_rate(dpll3_clk, rate * l3_div);
}

static unsigned long omap3_l3_get_rate(struct device *dev)
{
	int l3_div;

	l3_div = omap2_cm_read_mod_reg(CORE_MOD, CM_CLKSEL) &
			OMAP3430_CLKSEL_L3_MASK;
	return dpll3_clk->rate / l3_div;
}

/*
 * Build omap_devices for processors and bus.
 */
static void omap2_init_processor_devices(void)
{
	_init_omap_device("mpu", &mpu_dev);

	if (omap3_has_iva())
		_init_omap_device("iva", &iva_dev);

	if (cpu_is_omap44xx()) {
		_init_omap_device("l3_main_1", &l3_dev);
		_init_omap_device("dsp", &dsp_dev);
	} else if (cpu_is_ti81xx()) {
		_init_omap_device("l3_slow", &l3_dev);
	} else {
		_init_omap_device("l3_main", &l3_dev);
	}

	if (cpu_is_omap34xx()) {
		dpll1_clk = clk_get(NULL, "dpll1_ck");
		dpll2_clk = clk_get(NULL, "dpll2_ck");
		dpll3_clk = clk_get(NULL, "dpll3_m2_ck");

		if (mpu_dev)
			omap_device_populate_rate_fns(mpu_dev,
				omap3_mpu_set_rate, omap3_mpu_get_rate);
		if (iva_dev)
			omap_device_populate_rate_fns(iva_dev,
				omap3_iva_set_rate, omap3_iva_get_rate);
		if (l3_dev)
			omap_device_populate_rate_fns(l3_dev,
				omap3_l3_set_rate, omap3_l3_get_rate);

	}
}

/* Types of sleep_switch used in omap_set_pwrdm_state */
#define FORCEWAKEUP_SWITCH	0
#define LOWPOWERSTATE_SWITCH	1

/*
 * This sets pwrdm state (other than mpu & core. Currently only ON &
 * RET are supported.
 */
int omap_set_pwrdm_state(struct powerdomain *pwrdm, u32 state)
{
	u32 cur_state;
	int sleep_switch = 0;
	int ret = 0;

	if (pwrdm == NULL || IS_ERR(pwrdm))
		return -EINVAL;

	while (!(pwrdm->pwrsts & (1 << state))) {
		if (state == PWRDM_POWER_OFF)
			return ret;
		state--;
	}

	cur_state = pwrdm_read_next_pwrst(pwrdm);
	if (cur_state == state)
		return ret;

	if (pwrdm_read_pwrst(pwrdm) < PWRDM_POWER_ON) {
		if ((pwrdm_read_pwrst(pwrdm) > state) &&
			(pwrdm->flags & PWRDM_HAS_LOWPOWERSTATECHANGE)) {
			sleep_switch = LOWPOWERSTATE_SWITCH;
		} else {
			omap2_clkdm_wakeup(pwrdm->pwrdm_clkdms[0]);
			pwrdm_wait_transition(pwrdm);
			sleep_switch = FORCEWAKEUP_SWITCH;
		}
	}

	ret = pwrdm_set_next_pwrst(pwrdm, state);
	if (ret) {
		printk(KERN_ERR "Unable to set state of powerdomain: %s\n",
		       pwrdm->name);
		goto err;
	}

	switch (sleep_switch) {
	case FORCEWAKEUP_SWITCH:
		if (pwrdm->pwrdm_clkdms[0]->flags & CLKDM_CAN_ENABLE_AUTO)
			omap2_clkdm_allow_idle(pwrdm->pwrdm_clkdms[0]);
		else
			omap2_clkdm_sleep(pwrdm->pwrdm_clkdms[0]);
		break;
	case LOWPOWERSTATE_SWITCH:
		pwrdm_set_lowpwrstchange(pwrdm);
		break;
	default:
		return ret;
	}

	pwrdm_wait_transition(pwrdm);
	pwrdm_state_switch(pwrdm);
err:
	return ret;
}

/*
 * This API is to be called during init to put the various voltage
 * domains to the voltage as per the opp table. Typically we boot up
 * at the nominal voltage. So this function finds out the rate of
 * the clock associated with the voltage domain, finds out the correct
 * opp entry and puts the voltage domain to the voltage specifies
 * in the opp entry
 */
static int __init omap2_set_init_voltage(char *vdd_name, char *clk_name,
						struct device *dev)
{
	struct voltagedomain *voltdm;
	struct clk *clk;
	struct opp *opp;
	unsigned long freq, bootup_volt;

	if (!vdd_name || !clk_name || !dev) {
		printk(KERN_ERR "%s: Invalid parameters!\n", __func__);
		goto exit;
	}

	voltdm = omap_voltage_domain_lookup(vdd_name);
	if (IS_ERR(voltdm)) {
		printk(KERN_ERR "%s: Unable to get vdd pointer for vdd_%s\n",
			__func__, vdd_name);
		goto exit;
	}

	clk =  clk_get(NULL, clk_name);
	if (IS_ERR(clk)) {
		printk(KERN_ERR "%s: unable to get clk %s\n",
			__func__, clk_name);
		goto exit;
	}

	freq = clk->rate;
	clk_put(clk);

	opp = opp_find_freq_ceil(dev, &freq);
	if (IS_ERR(opp)) {
		printk(KERN_ERR "%s: unable to find boot up OPP for vdd_%s\n",
			__func__, vdd_name);
		goto exit;
	}

	bootup_volt = opp_get_voltage(opp);
	if (!bootup_volt) {
		printk(KERN_ERR "%s: unable to find voltage corresponding"
			"to the bootup OPP for vdd_%s\n", __func__, vdd_name);
		goto exit;
	}

	omap_voltage_scale_vdd(voltdm, bootup_volt);
	return 0;

exit:
	printk(KERN_ERR "%s: Unable to put vdd_%s to its init voltage\n\n",
		__func__, vdd_name);
	return -EINVAL;
}

static void __init omap3_init_voltages(void)
{
	if (!cpu_is_omap34xx())
		return;

	omap2_set_init_voltage("mpu", "dpll1_ck", mpu_dev);
	omap2_set_init_voltage("core", "l3_ick", l3_dev);
}

static void __init omap4_init_voltages(void)
{
	if (!cpu_is_omap44xx())
		return;

	omap2_set_init_voltage("mpu", "dpll_mpu_ck", mpu_dev);
	omap2_set_init_voltage("core", "l3_div_ck", l3_dev);
	omap2_set_init_voltage("iva", "dpll_iva_m5x2_ck", iva_dev);
}

static int __init omap2_common_pm_init(void)
{
	omap2_init_processor_devices();
	omap_pm_if_init();

	return 0;
}
postcore_initcall(omap2_common_pm_init);

static int __init omap2_common_pm_late_init(void)
{
	/* Init the OMAP TWL parameters */
	omap3_twl_init();
	omap4_twl_init();
	omap3_tps65023_init();

	/* Init the voltage layer */
	omap_voltage_late_init();

	/* Initialize the voltages */
	omap3_init_voltages();
	omap4_init_voltages();

	/* Smartreflex device init */
	omap_devinit_smartreflex();

	return 0;
}
late_initcall(omap2_common_pm_late_init);
