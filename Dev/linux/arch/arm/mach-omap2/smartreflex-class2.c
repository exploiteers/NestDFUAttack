/*
 * SmartReflex Voltage Control driver
 *
 * Copyright (C) 2011 Texas Instruments, Inc. - http://www.ti.com/
 * Author: AnilKumar Ch <anilkumar@ti.com>
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/debugfs.h>
#include <linux/slab.h>

#include <plat/ti81xx.h>
#include <plat/irqs-ti81xx.h>
#include <plat/smartreflex.h>
#include <plat/gpio.h>

#define GPIO_VAL_INIT		(0xFF)
#define NUM_GPIO_BITS		(4)
#define SENS_PER_VDOMAIN	(2)
#define SEN_NAME_LEN		(40)

#define MARGIN			(0)
#define STEP_SIZE		(80) /* (1/12.5mv)*/
#define MIN_VOLT		(0x2F5)
#define MAX_VOLT		(0x4AE)
#define	THOU_MVOLT		(1000)

#define gpio_num(bank_num, pin_num) (bank_num * 32 + pin_num)

#ifdef CONFIG_DEBUG_SR_CLASS2
static int sr_debugfs_enable = 1;
#else
static int sr_debugfs_enable;
#endif

struct tps40041_volt_data {
	u8 gpio_val;
	u32 mvolt;
};
static struct tps40041_volt_data volt_sd;

/*
 * TODO: Populate this table based on silicon characterization data
 */
static struct tps40041_volt_data tps_volt_data[NUM_VOLT_LEVELS] = {
	{0x0, 0x2F4}, {0x8, 0X32B}, {0x4, 0x330}, {0x2, 0x344},
	{0xC, 0x367}, {0xA, 0x37B}, {0x6, 0x380}, {0xE, 0x3B7},
	{0x1, 0x3EB}, {0x9, 0x422}, {0x5, 0x427}, {0x3, 0x43B},
	{0xD, 0x45E}, {0xB, 0x472}, {0x7, 0x477}, {0xF, 0x4AE},
};

struct ti816x_sr_sensors {
	const char			*name;
	struct clk			*fck;
	int				irq_processed;
	u32				nvalue;
	s32				e2v_gain;
	u32				base_addr;
};

struct ti816x_sr {
	u8				init_gpio_val;
	u8				curr_gpio_val;
	u32				curr_volt;
	int				is_autocomp_active;
	spinlock_t			lock;
	int				nvalue_count;
	struct ti816x_sr_sensors	sen[SENS_PER_VDOMAIN];
	struct timer_list		timer;
	int				sr_intdelay;
};

static void sr_start_coreautocomp(struct ti816x_sr *sr);
static void sr_stop_coreautocomp(struct ti816x_sr *sr);

static struct ti816x_sr srcore = {
	.sen[0] = {
		.name		= "hvt",
		.irq_processed	= 0,
		.nvalue		= 0,
		.e2v_gain	= (15),
		.base_addr	= TI816X_SR0_BASE,
	},
	.sen[1] = {
		.name		= "svt",
		.irq_processed	= 0,
		.nvalue		= 0,
		.e2v_gain	= (22),
		.base_addr	= TI816X_SR1_BASE,
	},
	.nvalue_count		= 2,
	.is_autocomp_active	= 0,
	.sr_intdelay		= 2000, /* msec */
};

static inline void sr_write_reg(u32 srbase, int offset, u32 value)
{
	omap_writel(value, srbase + offset);
}

static inline void sr_modify_reg(u32 srbase, int offset, u32 mask,
				 u32 value)
{
	u32 reg_val;

	reg_val = omap_readl(srbase + offset);
	reg_val &= ~mask;
	reg_val |= (value&mask);

	omap_writel(reg_val, srbase + offset);
}

static inline u32 sr_read_reg(u32 srbase, int offset)
{
	return omap_readl(srbase + offset);
}

static u8 sr_gpio_getn(void)
{
	int i = 0;
	int gpio_numb;
	int gpio_val;
	u8 cur_gpio_val = 0;

	while (i < NUM_GPIO_BITS) {
		gpio_numb = gpio_num(0, i);
		gpio_request(gpio_numb, "srdrv");
		gpio_val = gpio_get_value(gpio_numb);
		cur_gpio_val = cur_gpio_val | (gpio_val << i);
		i++;
	}
	srcore.curr_gpio_val = cur_gpio_val;

	return cur_gpio_val;
}

static int sr_gpio_setn(u8 gpio_val)
{
	int i = 0;
	int gpio_numb;

	while (i < NUM_GPIO_BITS) {
		gpio_numb = gpio_num(0, i);
		gpio_request(gpio_numb, "srdrv");
		gpio_direction_output(gpio_numb, (gpio_val & (1 << i)));
		i++;
	}

	return 0;
}

/*
 * The routine reads the efuse register for getting current
 * gpio voltage value
 */
static int get_core_voltage(void)
{
	u8 gpio_val;
	u32 mvolt = 0;
	int loopcnt;

	gpio_val = sr_gpio_getn();

	/* Get the voltage from the table based on gpio_val */
	for (loopcnt = 0; loopcnt < NUM_VOLT_LEVELS; loopcnt++) {
		if (tps_volt_data[loopcnt].gpio_val == gpio_val) {
			mvolt = tps_volt_data[loopcnt].mvolt;
			break;
		}
	}
	srcore.curr_volt = mvolt;

	return mvolt;
}

static void get_near_gpio_val(s32 curr_volt)
{
	int loopcnt, ncnt = 0;
	s32 temp[NUM_VOLT_LEVELS], volt_greater[NUM_VOLT_LEVELS], swap_var;

	volt_sd.gpio_val = GPIO_VAL_INIT;

	/* Nearest GPIO val, nothing but picking the minimum diff
	 * gpio values from a group of values
	 */
	for (loopcnt = 0; loopcnt < NUM_VOLT_LEVELS; loopcnt++) {
		temp[loopcnt] = tps_volt_data[loopcnt].mvolt - curr_volt;
		if (temp[loopcnt] >= 0) {
			volt_greater[ncnt] = temp[loopcnt];
			ncnt++;
		}
		if (ncnt > 1) {
			if (volt_greater[ncnt-2] < volt_greater[ncnt-1]) {
				swap_var = volt_greater[ncnt-2];
				volt_greater[ncnt-2] = volt_greater[ncnt-1];
				volt_greater[ncnt-1] = swap_var;
			}
		}
	}

	for (loopcnt = 0; loopcnt < NUM_VOLT_LEVELS; loopcnt++) {
		if (temp[loopcnt] == volt_greater[ncnt - 1]) {
			volt_sd.mvolt = tps_volt_data[loopcnt].mvolt;
			volt_sd.gpio_val = tps_volt_data[loopcnt].gpio_val;
			break;
		}
	}
}

/*
 * The routine get the error value and adjust the TPS40041 core
 * voltage based on the decision from two sensors
 * Assumptions:
 * 1. The efuse data is programmed into HVT and SVT control regs
 * 2. The efuse and gpio clocks are already enabled
 */
static int set_core_voltage(s32 sr1error, s32 sr2error)
{
	int ret;
	int current_volt;
	int prev_volt;
	unsigned long flags;

	/* Get the current voltage from GPIO */
	prev_volt = get_core_voltage();
	if ((srcore.sen[0].irq_processed == 0) ||
			(srcore.sen[1].irq_processed == 0))
		return (volt_sd.mvolt == prev_volt);

	if (sr1error > sr2error)
		current_volt = prev_volt + sr1error;
	else
		current_volt = prev_volt + sr2error;

	get_near_gpio_val(current_volt);
	if (volt_sd.gpio_val == GPIO_VAL_INIT)
		printk(KERN_ERR "Failed to get the gpio val = %d\n",
				volt_sd.gpio_val);

	spin_lock_irqsave(&srcore.lock, flags);
	ret = sr_gpio_setn(volt_sd.gpio_val);
	spin_unlock_irqrestore(&srcore.lock, flags);
	if (ret)
		printk(KERN_ERR "Failed to set the core voltage with GPIO\n");

	srcore.sen[0].irq_processed = 0;
	srcore.sen[1].irq_processed = 0;

	return (volt_sd.mvolt == prev_volt);
}

static int get_errvolt(s32 srid)
{
	u32 srbase;
	s32 e2vgain;
	s8 terror;
	u32 senerror_reg;
	s32 error, delta;
	s32 steps, mvoltage;

	if (srid == SRHVT) {
		srbase	= srcore.sen[0].base_addr;
		e2vgain = srcore.sen[0].e2v_gain;
	} else {
		srbase	= srcore.sen[1].base_addr;
		e2vgain = srcore.sen[1].e2v_gain;
	}
	senerror_reg = sr_read_reg(srbase, SENERROR_V2);
	senerror_reg = (senerror_reg & 0x0000FF00);
	senerror_reg = senerror_reg >> 8;

	terror = senerror_reg & 0x000000FF;

	/* convert from binary to % error x 1000mv */
	error = terror * 25 * THOU_MVOLT;
	delta = ((error + MARGIN) * e2vgain) >> 5;

	/* compute the (steps * 1000mv) */
	steps = delta/STEP_SIZE;

	/* Steps to volatge correction based on step size in mV*/
	mvoltage = steps/STEP_SIZE;

	return mvoltage;
}

static void irq_sr_timer(unsigned long data)
{
	/* Enable both the interrupts */
	sr_modify_reg(srcore.sen[0].base_addr, IRQENABLE_SET,
		IRQENABLE_MCUBOUNDSINT,
		IRQENABLE_MCUBOUNDSINT);
	sr_modify_reg(srcore.sen[1].base_addr, IRQENABLE_SET,
		IRQENABLE_MCUBOUNDSINT,
		IRQENABLE_MCUBOUNDSINT);
}

static irqreturn_t srcore_class2_irq(int irq, void *dev_id)
{
	int sr_irqdis;
	s32 srhvt_delta;
	s32 srsvt_delta;

	srhvt_delta = get_errvolt(SRHVT);
	srsvt_delta = get_errvolt(SRSVT);

	if (irq == TI81XX_IRQ_SMRFLX0)
		srcore.sen[0].irq_processed = 1;
	else
		srcore.sen[1].irq_processed = 1;

	sr_irqdis = set_core_voltage(srhvt_delta, srsvt_delta);

	if (sr_irqdis == 1) {

		if (!timer_pending(&srcore.timer)) {
			srcore.timer.data = irq;
			srcore.timer.function = irq_sr_timer;
			srcore.timer.expires = jiffies +
					msecs_to_jiffies(srcore.sr_intdelay);
			add_timer(&srcore.timer);
		}

		sr_modify_reg(srcore.sen[0].base_addr, IRQSTATUS,
				IRQSTATUS_MCBOUNDSINT,
				IRQSTATUS_MCBOUNDSINT);
		sr_modify_reg(srcore.sen[1].base_addr, IRQSTATUS,
				IRQSTATUS_MCBOUNDSINT,
				IRQSTATUS_MCBOUNDSINT);
		/* Disable the interrupt */
		sr_modify_reg(srcore.sen[0].base_addr,
				IRQENABLE_CLR,
				IRQENABLE_MCUBOUNDSINT,
				IRQENABLE_MCUBOUNDSINT);
		sr_modify_reg(srcore.sen[1].base_addr,
				IRQENABLE_CLR,
				IRQENABLE_MCUBOUNDSINT,
				IRQENABLE_MCUBOUNDSINT);
	} else {
		if (irq == TI81XX_IRQ_SMRFLX0) {
			/* Clear MCUDisableAck Interrupt */
			sr_modify_reg(srcore.sen[0].base_addr, IRQSTATUS,
				IRQSTATUS_MCBOUNDSINT,
				IRQSTATUS_MCBOUNDSINT);
		} else {
			/* Clear MCUDisableAck Interrupt */
			sr_modify_reg(srcore.sen[1].base_addr, IRQSTATUS,
				IRQSTATUS_MCBOUNDSINT,
				IRQSTATUS_MCBOUNDSINT);
		}
	}

	return IRQ_HANDLED;
}

static int sr_clk_enable(struct clk *fck)
{
	if (clk_enable(fck) != 0) {
		printk(KERN_ERR "Could not enable sr_fck\n");
		return -1;
	}

	return 0;
}

static int sr_clk_disable(struct clk *fck)
{
	clk_disable(fck);

	return 0;
}

static int sr_set_nvalues(struct ti816x_sr *sr)
{
	/* Read HVT values for CORE from EFUSE */
	sr->sen[0].nvalue = omap_readl(CONTROL_FUSE_SMRT_HVT) & 0xFFFFFF;

	/* Read SVT values for CORE from EFUSE */
	sr->sen[1].nvalue = omap_readl(CONTROL_FUSE_SMRT_SVT) & 0xFFFFFF;

	if ((sr->sen[0].nvalue == 0) || (sr->sen[1].nvalue == 0)) {
		printk(KERN_ERR "SmartReflex Driver: Un-Characterized"
					" silicon found\n");
		return -1;
	}
	printk("SR NTarget value for HVT 0x%x\n", sr->sen[0].nvalue);
	printk("SR NTarget value for SVT 0x%x\n", sr->sen[1].nvalue);

	return 0;
}

static void sr_configure(u32 srbase, s32 srid)
{
	u32 sr_config;

	sr_config = SRCONFIG_ACCUM_DATA |
		SRCLKLENGTH_27MHZ_SYSCLK |
		SRCONFIG_SENENABLE | SRCONFIG_ERRGEN_EN |
		SRCONFIG_MINMAXAVG_EN |
		SRCONFIG_SENNENABLE |
		SRCONFIG_SENPENABLE;

	sr_write_reg(srbase, SRCONFIG, sr_config);

	sr_write_reg(srbase, AVGWEIGHT, AVGWEIGHT_SENPAVGWEIGHT_MASK |
			AVGWEIGHT_SENNAVGWEIGHT_MASK);

	if (srid == SRHVT)
		sr_modify_reg(srbase, ERRCONFIG_V2, (SR_ERRWEIGHT_MASK |
			SR_ERRMAXLIMIT_MASK | SR_ERRMINLIMIT_MASK),
			ERRCONFIG_ERRWEIGHT | ERRCONFIG_ERRMAXLIMIT |
			ERRCONFIG_HVT_ERRMINLIMIT);
	else
		sr_modify_reg(srbase, ERRCONFIG_V2, (SR_ERRWEIGHT_MASK |
			SR_ERRMAXLIMIT_MASK | SR_ERRMINLIMIT_MASK),
			ERRCONFIG_ERRWEIGHT | ERRCONFIG_ERRMAXLIMIT |
			ERRCONFIG_SVT_ERRMINLIMIT);
}

static void sr_enable(u32 srbase, u32 nvalue)
{
	/* Enable MCUDisableAck Interrupt */
	sr_modify_reg(srbase, IRQSTATUS_RAW,
			IRQSTATUSRAW_MCUDISABLEACKINT,
			IRQSTATUSRAW_MCUDISABLEACKINT);
	/* SRCONFIG - disable SR */
	sr_modify_reg(srbase, SRCONFIG, SRCONFIG_SRENABLE,
					~SRCONFIG_SRENABLE);

	if (nvalue == 0)
		printk(KERN_ERR "OPP doesn't support SmartReflex\n");

	sr_write_reg(srbase, NVALUERECIPROCAL, nvalue);

	/* Clear MCUDisableAck Interrupt */
	sr_modify_reg(srbase, IRQSTATUS,
			IRQSTATUS_MCUDISABLEACKINT,
			IRQSTATUS_MCUDISABLEACKINT);
	/* Disable MCUDisableAck interrupt */
	sr_modify_reg(srbase, IRQENABLE_CLR,
			IRQSTATUS_MCUDISABLEACKINT,
			IRQSTATUS_MCUDISABLEACKINT);
	/* Enable the interrupt */
	sr_modify_reg(srbase, IRQENABLE_SET,
			IRQENABLE_MCUBOUNDSINT,
			IRQENABLE_MCUBOUNDSINT);

	/* SRCONFIG - enable SR */
	sr_modify_reg(srbase, SRCONFIG, SRCONFIG_SRENABLE,
					SRCONFIG_SRENABLE);
}

static void sr_disable(u32 srbase)
{
	/* SRCONFIG - disable SR */
	sr_modify_reg(srbase, SRCONFIG, SRCONFIG_SRENABLE,
					~SRCONFIG_SRENABLE);
	/* Disable the interrupt */
	sr_modify_reg(srbase, IRQENABLE_CLR,
			IRQENABLE_MCUBOUNDSINT,
			IRQENABLE_MCUBOUNDSINT);
}

static void sr_start_coreautocomp(struct ti816x_sr *sr)
{
	if ((sr->sen[0].nvalue == 0) || (sr->sen[1].nvalue == 0)) {
		printk(KERN_ERR "SmartReflex Driver: Un-Characterized"
					" silicon found\n");
		return;
	}

	if (sr->is_autocomp_active == 1) {
		printk(KERN_WARNING "SR VDD autocomp is already active\n");
		return;
	}

	sr_clk_enable(sr->sen[0].fck);
	sr_clk_enable(sr->sen[1].fck);
	sr_configure(sr->sen[0].base_addr, SRHVT);
	sr_configure(sr->sen[1].base_addr, SRSVT);

	sr_enable(sr->sen[0].base_addr, sr->sen[0].nvalue);
	sr_enable(sr->sen[1].base_addr, sr->sen[1].nvalue);

	sr->is_autocomp_active = 1;
	init_timer(&sr->timer);
}

static void sr_stop_coreautocomp(struct ti816x_sr *sr)
{
	unsigned long flags;
	int ret;

	if (sr->is_autocomp_active == 0) {
		printk(KERN_WARNING "SR VDD autocomp is not active\n");
		return;
	}

	del_timer(&sr->timer);
	sr->is_autocomp_active = 0;
	sr_disable(sr->sen[0].base_addr);
	sr_disable(sr->sen[1].base_addr);
	sr_clk_disable(sr->sen[0].fck);
	sr_clk_disable(sr->sen[1].fck);

	spin_lock_irqsave(&sr->lock, flags);
	ret = sr_gpio_setn(sr->init_gpio_val);
	spin_unlock_irqrestore(&sr->lock, flags);
	if (ret)
		printk(KERN_ERR "Failed to set the initial voltage\n");
	sr->curr_gpio_val = sr->init_gpio_val;
	sr->curr_volt = get_core_voltage();
}

/* Sysfs interface to select SR CORE auto compensation */
static ssize_t sr_core_autocomp_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", srcore.is_autocomp_active);
}

static ssize_t sr_core_autocomp_store(struct kobject *kobj,
					struct kobj_attribute *attr,
					const char *buf, size_t n)
{
	unsigned short value;

	if (sscanf(buf, "%hu", &value) != 1 || (value > 1)) {
		printk(KERN_ERR "sr_core_autocomp: Invalid value\n");
		return -EINVAL;
	}

	if (value == 0)
		sr_stop_coreautocomp(&srcore);
	else
		sr_start_coreautocomp(&srcore);

	return n;
}

static struct kobj_attribute sr_core_autocomp = {
	.attr = {
		 .name = __stringify(sr_core_autocomp),
		 .mode = 0644,
		 },
	.show = sr_core_autocomp_show,
	.store = sr_core_autocomp_store,
};

static int sr_debugfs_entires(struct ti816x_sr *sr_info)
{
	struct dentry *dbg_dir, *sen_dir;
	int i;

	dbg_dir = debugfs_create_dir("smartreflex", NULL);
	if (IS_ERR(dbg_dir)) {
		printk(KERN_ERR "Unable to create debugfs directory\n");
		return PTR_ERR(dbg_dir);
	}

	(void) debugfs_create_x32("sr_interrupt_delay", S_IRUGO | S_IWUGO,
				dbg_dir, &sr_info->sr_intdelay);
	(void) debugfs_create_u32("sr_current_voltage", S_IRUGO, dbg_dir,
				&sr_info->curr_volt);
	(void) debugfs_create_u8("sr_init_gpio_val", S_IRUGO, dbg_dir,
				&sr_info->init_gpio_val);
	(void) debugfs_create_u8("sr_curr_gpio_val", S_IRUGO, dbg_dir,
				&sr_info->curr_gpio_val);

	for (i = 0; i < sr_info->nvalue_count; i++) {
		char *name;

		name = kzalloc(SEN_NAME_LEN + 1, GFP_KERNEL);
		if (!name) {
			printk(KERN_ERR "Unable to allocate memory for n-value"
						"directory name\n");
			return -ENOMEM;
		}

		sprintf(name, "%s", sr_info->sen[i].name);
		sen_dir = debugfs_create_dir(name, dbg_dir);
		if (IS_ERR(sen_dir)) {
			printk(KERN_ERR "Unable to create debugfs directory\n");
			return PTR_ERR(sen_dir);
		}

		(void) debugfs_create_x32("sr_err2voltgain", S_IRUGO, sen_dir,
					&sr_info->sen[i].e2v_gain);
		(void) debugfs_create_x32("sr_nvalue", S_IRUGO | S_IWUGO,
					sen_dir, &sr_info->sen[i].nvalue);
	}
	return 0;
}

/* SR driver init */
static int __init sr_class2_init(void)
{
	int ret = 0;

	srcore.sen[0].fck = clk_get(NULL, "smartreflex_corehvt_fck");
	if (IS_ERR(srcore.sen[0].fck)) {
		printk(KERN_ERR "Could not get corehvt_fck\n");
		return PTR_ERR(srcore.sen[0].fck);
	}
	printk("sr1_fck HVT rate = %lu\n",
				clk_get_rate(srcore.sen[0].fck));

	srcore.sen[1].fck = clk_get(NULL, "smartreflex_coresvt_fck");
	if (IS_ERR(srcore.sen[1].fck)) {
		printk(KERN_ERR "Could not get coresvt_fck\n");
		ret = PTR_ERR(srcore.sen[1].fck);
		goto fail_svt_clk_get;
	}
	printk("sr2_fck SVT rate = %lu\n",
				clk_get_rate(srcore.sen[1].fck));

	if (sr_debugfs_enable == 1) {
		ret = sr_debugfs_entires(&srcore);
		if (ret) {
			printk(KERN_ERR "Debug FS entires are created\n");
			goto fail_hvt_req_irq;
		}
	} else {
		ret = sr_set_nvalues(&srcore);
		if (ret) {
			printk(KERN_WARNING "SmartReflex Driver is not"
						"initialized\n");
			goto fail_hvt_req_irq;
		}
	}

	/* Read current GPIO value and voltage */
	srcore.init_gpio_val = sr_gpio_getn();
	srcore.curr_volt = get_core_voltage();

	ret = request_irq(TI81XX_IRQ_SMRFLX0, srcore_class2_irq,
				IRQF_DISABLED, "sr1", NULL);
	if (ret) {
		printk(KERN_ERR "Could not install SR1 ISR\n");
		goto fail_hvt_req_irq;
	}

	ret = request_irq(TI81XX_IRQ_SMRFLX1, srcore_class2_irq,
				IRQF_DISABLED, "sr2", NULL);
	if (ret) {
		printk(KERN_ERR "Could not install SR2 ISR\n");
		goto fail_svt_req_irq;
	}

	ret = sysfs_create_file(power_kobj, &sr_core_autocomp.attr);
	if (ret) {
		printk(KERN_ERR "subsys_create_file failed: %d\n", ret);
		goto fail_sysfs;
	}

	printk("SmartReflex Driver initialized\n");
	return 0;

fail_sysfs:
	free_irq(TI81XX_IRQ_SMRFLX1, NULL);
fail_svt_req_irq:
	free_irq(TI81XX_IRQ_SMRFLX0, NULL);
fail_hvt_req_irq:
	clk_put(srcore.sen[1].fck);
fail_svt_clk_get:
	clk_put(srcore.sen[0].fck);
	return ret;
}

static void __exit sr_class2_exit(void)
{
	clk_put(srcore.sen[0].fck);
	clk_put(srcore.sen[1].fck);

	free_irq(TI81XX_IRQ_SMRFLX0, NULL);
	free_irq(TI81XX_IRQ_SMRFLX1, NULL);

	sysfs_remove_file(power_kobj, &sr_core_autocomp.attr);
}

MODULE_AUTHOR("Texas Instruments, Inc.");
MODULE_DESCRIPTION("SmartReflex Class2 driver");
MODULE_LICENSE("GPL");

module_init(sr_class2_init);
module_exit(sr_class2_exit);
