/*
 * linux/arch/arm/plat-omap/dmtimer.c
 *
 * OMAP Dual-Mode Timers
 *
 * Copyright (C) 2005 Nokia Corporation
 * OMAP2 support by Juha Yrjola
 * API improvements and OMAP2 clock framework support by Timo Teras
 *
 * Copyright (C) 2009 Texas Instruments
 * Added OMAP4 support - Santosh Shilimkar <santosh.shilimkar@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <mach/hardware.h>
#include <plat/dmtimer.h>
#include <mach/irqs.h>

#define		OMAP_TIMER_CTRL_GPOCFG		(1 << 14)
#define		OMAP_TIMER_CTRL_CAPTMODE	(1 << 13)
#define		OMAP_TIMER_CTRL_PT		(1 << 12)
#define		OMAP_TIMER_CTRL_TCM_LOWTOHIGH	(0x1 << 8)
#define		OMAP_TIMER_CTRL_TCM_HIGHTOLOW	(0x2 << 8)
#define		OMAP_TIMER_CTRL_TCM_BOTHEDGES	(0x3 << 8)
#define		OMAP_TIMER_CTRL_SCPWM		(1 << 7)
#define		OMAP_TIMER_CTRL_CE		(1 << 6) /* compare enable */
#define		OMAP_TIMER_CTRL_PRE		(1 << 5) /* prescaler enable */
#define		OMAP_TIMER_CTRL_PTV_SHIFT	2 /* prescaler value shift */
#define		OMAP_TIMER_CTRL_POSTED		(1 << 2)
#define		OMAP_TIMER_CTRL_AR		(1 << 1) /* auto-reload enable */
#define		OMAP_TIMER_CTRL_ST		(1 << 0) /* start timer */

#define		WP_NONE			0	/* no write pending bit */
#define		WP_TCLR			(1 << 0)
#define		WP_TCRR			(1 << 1)
#define		WP_TLDR			(1 << 2)
#define		WP_TTGR			(1 << 3)
#define		WP_TMAR			(1 << 4)
#define		WP_TPIR			(1 << 5)
#define		WP_TNIR			(1 << 6)
#define		WP_TCVR			(1 << 7)
#define		WP_TOCR			(1 << 8)
#define		WP_TOWR			(1 << 9)

enum {
	OMAP_TIMER_ID_REG = 0,
	OMAP_TIMER_OCP_CFG_REG,
	OMAP_TIMER_SYS_STAT_REG,
	OMAP_TIMER_STAT_REG,
	OMAP_TIMER_INT_EN_REG,
	OMAP_TIMER_WAKEUP_EN_REG,
	OMAP_TIMER_CTRL_REG,
	OMAP_TIMER_COUNTER_REG,
	OMAP_TIMER_LOAD_REG,
	OMAP_TIMER_TRIGGER_REG,
	OMAP_TIMER_WRITE_PEND_REG,
	OMAP_TIMER_MATCH_REG,
	OMAP_TIMER_CAPTURE_REG,
	OMAP_TIMER_IF_CTRL_REG,
	OMAP_TIMER_CAPTURE2_REG,		/* TCAR2, 34xx only */
	OMAP_TIMER_TICK_POS_REG,		/* TPIR, 34xx only */
	OMAP_TIMER_TICK_NEG_REG,		/* TNIR, 34xx only */
	OMAP_TIMER_TICK_COUNT_REG,		/* TCVR, 34xx only */
	OMAP_TIMER_TICK_INT_MASK_SET_REG,	/* TOCR, 34xx only */
	OMAP_TIMER_TICK_INT_MASK_COUNT_REG,	/* TOWR, 34xx only */
	OMAP_TIMER_IRQ_EOI_REG,			/* ti81xx only */
	OMAP_TIMER_IRQSTATUS_RAW_REG,		/* ti81xx only */
	OMAP_TIMER_IRQSTATUS_REG,		/* ti81xx only */
	OMAP_TIMER_IRQSTATUS_SET_REG,		/* ti81xx only */
	OMAP_TIMER_IRQSTATUS_CLR_REG,		/* ti81xx only */
};

/* register offsets with the write pending bit encoded */
#define	WPSHIFT					16

const static u32 reg_map[] = {
	[OMAP_TIMER_ID_REG]			= 0x00 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_OCP_CFG_REG]		= 0x10 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_SYS_STAT_REG]		= 0x14 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_STAT_REG]			= 0x18 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_INT_EN_REG]			= 0x1c | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_WAKEUP_EN_REG]		= 0x20 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_CTRL_REG]			= 0x24 | (WP_TCLR << WPSHIFT),
	[OMAP_TIMER_COUNTER_REG]		= 0x28 | (WP_TCRR << WPSHIFT),
	[OMAP_TIMER_LOAD_REG]			= 0x2c | (WP_TLDR << WPSHIFT),
	[OMAP_TIMER_TRIGGER_REG]		= 0x30 | (WP_TTGR << WPSHIFT),
	[OMAP_TIMER_WRITE_PEND_REG]		= 0x34 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_MATCH_REG]			= 0x38 | (WP_TMAR << WPSHIFT),
	[OMAP_TIMER_CAPTURE_REG]		= 0x3c | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_IF_CTRL_REG]		= 0x40 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_CAPTURE2_REG]		= 0x44 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_TICK_POS_REG]		= 0x48 | (WP_TPIR << WPSHIFT),
	[OMAP_TIMER_TICK_NEG_REG]		= 0x4c | (WP_TNIR << WPSHIFT),
	[OMAP_TIMER_TICK_COUNT_REG]		= 0x50 | (WP_TCVR << WPSHIFT),
	[OMAP_TIMER_TICK_INT_MASK_SET_REG]	= 0x54 | (WP_TOCR << WPSHIFT),
	[OMAP_TIMER_TICK_INT_MASK_COUNT_REG]	= 0x58 | (WP_TOWR << WPSHIFT),
};

/* Few register offsets in TI81XX are different */
const static u32 ti81xx_reg_map[] = {
	[OMAP_TIMER_ID_REG]			= 0x00 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_OCP_CFG_REG]		= 0x10 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_STAT_REG]			= 0x28 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_INT_EN_REG]			= 0x2c | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_WAKEUP_EN_REG]		= 0x34 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_CTRL_REG]			= 0x38 | (WP_TCLR << WPSHIFT),
	[OMAP_TIMER_COUNTER_REG]		= 0x3c | (WP_TCRR << WPSHIFT),
	[OMAP_TIMER_LOAD_REG]			= 0x40 | (WP_TLDR << WPSHIFT),
	[OMAP_TIMER_TRIGGER_REG]		= 0x44 | (WP_TTGR << WPSHIFT),
	[OMAP_TIMER_WRITE_PEND_REG]		= 0x48 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_MATCH_REG]			= 0x4c | (WP_TMAR << WPSHIFT),
	[OMAP_TIMER_CAPTURE_REG]		= 0x50 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_IF_CTRL_REG]		= 0x54 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_CAPTURE2_REG]		= 0x58 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_IRQ_EOI_REG]		= 0x20 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_IRQSTATUS_RAW_REG]		= 0x24 | (WP_NONE << WPSHIFT),
	[OMAP_TIMER_IRQSTATUS_CLR_REG]		= 0x30 | (WP_NONE << WPSHIFT),
};

struct omap_dm_timer {
	unsigned long phys_base;
	int irq;
#ifdef CONFIG_ARCH_OMAP2PLUS
	struct clk *iclk, *fclk;
#endif
	void __iomem *io_base;
	unsigned reserved:1;
	unsigned enabled:1;
	unsigned posted:1;
};

static int dm_timer_count;

#ifdef CONFIG_ARCH_OMAP1
static struct omap_dm_timer omap1_dm_timers[] = {
	{ .phys_base = 0xfffb1400, .irq = INT_1610_GPTIMER1 },
	{ .phys_base = 0xfffb1c00, .irq = INT_1610_GPTIMER2 },
	{ .phys_base = 0xfffb2400, .irq = INT_1610_GPTIMER3 },
	{ .phys_base = 0xfffb2c00, .irq = INT_1610_GPTIMER4 },
	{ .phys_base = 0xfffb3400, .irq = INT_1610_GPTIMER5 },
	{ .phys_base = 0xfffb3c00, .irq = INT_1610_GPTIMER6 },
	{ .phys_base = 0xfffb7400, .irq = INT_1610_GPTIMER7 },
	{ .phys_base = 0xfffbd400, .irq = INT_1610_GPTIMER8 },
};

static const int omap1_dm_timer_count = ARRAY_SIZE(omap1_dm_timers);

#else
#define omap1_dm_timers			NULL
#define omap1_dm_timer_count		0
#endif	/* CONFIG_ARCH_OMAP1 */

#ifdef CONFIG_ARCH_OMAP2
static struct omap_dm_timer omap2_dm_timers[] = {
	{ .phys_base = 0x48028000, .irq = INT_24XX_GPTIMER1 },
	{ .phys_base = 0x4802a000, .irq = INT_24XX_GPTIMER2 },
	{ .phys_base = 0x48078000, .irq = INT_24XX_GPTIMER3 },
	{ .phys_base = 0x4807a000, .irq = INT_24XX_GPTIMER4 },
	{ .phys_base = 0x4807c000, .irq = INT_24XX_GPTIMER5 },
	{ .phys_base = 0x4807e000, .irq = INT_24XX_GPTIMER6 },
	{ .phys_base = 0x48080000, .irq = INT_24XX_GPTIMER7 },
	{ .phys_base = 0x48082000, .irq = INT_24XX_GPTIMER8 },
	{ .phys_base = 0x48084000, .irq = INT_24XX_GPTIMER9 },
	{ .phys_base = 0x48086000, .irq = INT_24XX_GPTIMER10 },
	{ .phys_base = 0x48088000, .irq = INT_24XX_GPTIMER11 },
	{ .phys_base = 0x4808a000, .irq = INT_24XX_GPTIMER12 },
};

static const char *omap2_dm_source_names[] __initdata = {
	"sys_ck",
	"func_32k_ck",
	"alt_ck",
	NULL
};

static struct clk *omap2_dm_source_clocks[3];
static const int omap2_dm_timer_count = ARRAY_SIZE(omap2_dm_timers);

#else
#define omap2_dm_timers			NULL
#define omap2_dm_timer_count		0
#define omap2_dm_source_names		NULL
#define omap2_dm_source_clocks		NULL
#endif	/* CONFIG_ARCH_OMAP2 */

#ifdef CONFIG_ARCH_OMAP3
static struct omap_dm_timer omap3_dm_timers[] = {
	{ .phys_base = 0x48318000, .irq = INT_24XX_GPTIMER1 },
	{ .phys_base = 0x49032000, .irq = INT_24XX_GPTIMER2 },
	{ .phys_base = 0x49034000, .irq = INT_24XX_GPTIMER3 },
	{ .phys_base = 0x49036000, .irq = INT_24XX_GPTIMER4 },
	{ .phys_base = 0x49038000, .irq = INT_24XX_GPTIMER5 },
	{ .phys_base = 0x4903A000, .irq = INT_24XX_GPTIMER6 },
	{ .phys_base = 0x4903C000, .irq = INT_24XX_GPTIMER7 },
	{ .phys_base = 0x4903E000, .irq = INT_24XX_GPTIMER8 },
	{ .phys_base = 0x49040000, .irq = INT_24XX_GPTIMER9 },
	{ .phys_base = 0x48086000, .irq = INT_24XX_GPTIMER10 },
	{ .phys_base = 0x48088000, .irq = INT_24XX_GPTIMER11 },
	{ .phys_base = 0x48304000, .irq = INT_34XX_GPT12_IRQ },
};

static const char *omap3_dm_source_names[] __initdata = {
	"sys_ck",
	"omap_32k_fck",
	NULL
};

static struct clk *omap3_dm_source_clocks[2];
static const int omap3_dm_timer_count = ARRAY_SIZE(omap3_dm_timers);

#else
#define omap3_dm_timers			NULL
#define omap3_dm_timer_count		0
#define omap3_dm_source_names		NULL
#define omap3_dm_source_clocks		NULL
#endif	/* CONFIG_ARCH_OMAP3 */

#ifdef CONFIG_ARCH_OMAP4
static struct omap_dm_timer omap4_dm_timers[] = {
	{ .phys_base = 0x4a318000, .irq = OMAP44XX_IRQ_GPT1 },
	{ .phys_base = 0x48032000, .irq = OMAP44XX_IRQ_GPT2 },
	{ .phys_base = 0x48034000, .irq = OMAP44XX_IRQ_GPT3 },
	{ .phys_base = 0x48036000, .irq = OMAP44XX_IRQ_GPT4 },
	{ .phys_base = 0x40138000, .irq = OMAP44XX_IRQ_GPT5 },
	{ .phys_base = 0x4013a000, .irq = OMAP44XX_IRQ_GPT6 },
	{ .phys_base = 0x4013a000, .irq = OMAP44XX_IRQ_GPT7 },
	{ .phys_base = 0x4013e000, .irq = OMAP44XX_IRQ_GPT8 },
	{ .phys_base = 0x4803e000, .irq = OMAP44XX_IRQ_GPT9 },
	{ .phys_base = 0x48086000, .irq = OMAP44XX_IRQ_GPT10 },
	{ .phys_base = 0x48088000, .irq = OMAP44XX_IRQ_GPT11 },
	{ .phys_base = 0x4a320000, .irq = OMAP44XX_IRQ_GPT12 },
};
static const char *omap4_dm_source_names[] __initdata = {
	"sys_clkin_ck",
	"sys_32k_ck",
	NULL
};
static struct clk *omap4_dm_source_clocks[2];
static const int omap4_dm_timer_count = ARRAY_SIZE(omap4_dm_timers);

#else
#define omap4_dm_timers			NULL
#define omap4_dm_timer_count		0
#define omap4_dm_source_names		NULL
#define omap4_dm_source_clocks		NULL
#endif	/* CONFIG_ARCH_OMAP4 */

#ifdef CONFIG_ARCH_TI81XX
static struct omap_dm_timer ti81xx_dm_timers[] = {
	{ .phys_base = 0x4802E000, .irq = TI81XX_IRQ_GPT1 },
	{ .phys_base = 0x48040000, .irq = TI81XX_IRQ_GPT2 },
	{ .phys_base = 0x48042000, .irq = TI81XX_IRQ_GPT3 },
	{ .phys_base = 0x48044000, .irq = TI81XX_IRQ_GPT4 },
	{ .phys_base = 0x48046000, .irq = TI81XX_IRQ_GPT5 },
	{ .phys_base = 0x48048000, .irq = TI81XX_IRQ_GPT6 },
	{ .phys_base = 0x4804A000, .irq = TI81XX_IRQ_GPT7 },
	{ .phys_base = 0x481C1000, .irq = TI814X_IRQ_GPT8 },
};
static const char *ti816x_dm_source_names[] __initdata = {
	"sys_clkin_ck",
	"sysclk18_ck",
	"tclkin_ck",
	NULL
};
/* FIXME: Currently set_source only allows selecting form first three sources */
static const char *ti814x_dm_source_names[] __initdata = {
	"osc0_clkin_ck",
	"sysclk18_ck",
	"tclkin_ck",
	"osc1_clkin_ck",
	"xref0_ck",
	"xref1_ck",
	"xref2_ck",
	NULL
};
static struct clk *ti816x_dm_source_clocks[3];
static struct clk *ti814x_dm_source_clocks[7];
static const int ti81xx_dm_timer_count = ARRAY_SIZE(ti81xx_dm_timers);

#else
#define ti81xx_dm_timers		NULL
#define ti81xx_dm_timer_count		0
#define ti816x_dm_source_names		NULL
#define ti816x_dm_source_clocks		NULL
#define ti814x_dm_source_names		NULL
#define ti814x_dm_source_clocks		NULL
#endif	/* CONFIG_ARCH_TI81XX */

static struct omap_dm_timer *dm_timers;
static const char **dm_source_names;
static struct clk **dm_source_clocks;
static u32 *dm_regs;

static spinlock_t dm_timer_lock;

/*
 * Reads timer registers in posted and non-posted mode. The posted mode bit
 * is encoded in reg. Note that in posted mode write pending bit must be
 * checked. Otherwise a read of a non completed write will produce an error.
 */
static inline u32 omap_dm_timer_read_reg(struct omap_dm_timer *timer, u32 reg)
{
	if (timer->posted)
		while (readl(timer->io_base +
				(dm_regs[OMAP_TIMER_WRITE_PEND_REG] & 0xff)) &
				(dm_regs[reg] >> WPSHIFT))
			cpu_relax();
	return readl(timer->io_base + (dm_regs[reg] & 0xff));
}

/*
 * Writes timer registers in posted and non-posted mode. The posted mode bit
 * is encoded in reg. Note that in posted mode the write pending bit must be
 * checked. Otherwise a write on a register which has a pending write will be
 * lost.
 */
static void omap_dm_timer_write_reg(struct omap_dm_timer *timer, u32 reg,
						u32 value)
{
	if (timer->posted)
		while (readl(timer->io_base +
				(dm_regs[OMAP_TIMER_WRITE_PEND_REG] & 0xff)) &
				(dm_regs[reg] >> WPSHIFT))
			cpu_relax();
	writel(value, timer->io_base + (dm_regs[reg] & 0xff));
}

static inline int omap_dm_timer_is_reset_done(struct omap_dm_timer *timer)
{
	/* TI81XX timers do not have SYS_STAT register */
	if (!cpu_is_ti81xx())
		return ((omap_dm_timer_read_reg(timer,
					OMAP_TIMER_SYS_STAT_REG) & 1) == 1);
	else
		return ((omap_dm_timer_read_reg(timer,
					OMAP_TIMER_OCP_CFG_REG) & 1) == 0);
}

static void omap_dm_timer_wait_for_reset(struct omap_dm_timer *timer)
{
	int c;

	c = 0;

	while (!omap_dm_timer_is_reset_done(timer)) {
		c++;
		if (c > 100000) {
			printk(KERN_ERR "Timer failed to reset\n");
			return;
		}
	}
}

static void omap_dm_timer_reset(struct omap_dm_timer *timer)
{
	u32 l;

	if (!cpu_class_is_omap2() || timer != &dm_timers[0]) {
		omap_dm_timer_write_reg(timer, OMAP_TIMER_IF_CTRL_REG, 0x06);
		omap_dm_timer_wait_for_reset(timer);
	}
	omap_dm_timer_set_source(timer, OMAP_TIMER_SRC_32_KHZ);

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_OCP_CFG_REG);
	l |= 0x02 << 3;  /* Set to smart-idle mode */
	l |= 0x2 << 8;   /* Set clock activity to perserve f-clock on idle */

	/*
	 * Enable wake-up on OMAP2 CPUs.
	 */
	if (cpu_class_is_omap2())
		l |= 1 << 2;
	omap_dm_timer_write_reg(timer, OMAP_TIMER_OCP_CFG_REG, l);

	/* Match hardware reset default of posted mode */
	omap_dm_timer_write_reg(timer, OMAP_TIMER_IF_CTRL_REG,
			OMAP_TIMER_CTRL_POSTED);
	timer->posted = 1;
}

static void omap_dm_timer_prepare(struct omap_dm_timer *timer)
{
	omap_dm_timer_enable(timer);
	omap_dm_timer_reset(timer);
}

struct omap_dm_timer *omap_dm_timer_request(void)
{
	struct omap_dm_timer *timer = NULL;
	unsigned long flags;
	int i;

	spin_lock_irqsave(&dm_timer_lock, flags);
	for (i = 0; i < dm_timer_count; i++) {
		if (dm_timers[i].reserved)
			continue;

		timer = &dm_timers[i];
		timer->reserved = 1;
		break;
	}
	spin_unlock_irqrestore(&dm_timer_lock, flags);

	if (timer != NULL)
		omap_dm_timer_prepare(timer);

	return timer;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_request);

struct omap_dm_timer *omap_dm_timer_request_specific(int id)
{
	struct omap_dm_timer *timer;
	unsigned long flags;

	spin_lock_irqsave(&dm_timer_lock, flags);
	if (id <= 0 || id > dm_timer_count || dm_timers[id-1].reserved) {
		spin_unlock_irqrestore(&dm_timer_lock, flags);
		printk("BUG: warning at %s:%d/%s(): unable to get timer %d\n",
		       __FILE__, __LINE__, __func__, id);
		dump_stack();
		return NULL;
	}

	timer = &dm_timers[id-1];
	timer->reserved = 1;
	spin_unlock_irqrestore(&dm_timer_lock, flags);

	omap_dm_timer_prepare(timer);

	return timer;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_request_specific);

void omap_dm_timer_free(struct omap_dm_timer *timer)
{
	omap_dm_timer_enable(timer);
	omap_dm_timer_reset(timer);
	omap_dm_timer_disable(timer);

	WARN_ON(!timer->reserved);
	timer->reserved = 0;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_free);

void omap_dm_timer_enable(struct omap_dm_timer *timer)
{
	if (timer->enabled)
		return;

#ifdef CONFIG_ARCH_OMAP2PLUS
	if (cpu_class_is_omap2()) {
		clk_enable(timer->fclk);
		clk_enable(timer->iclk);
	}
#endif

	timer->enabled = 1;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_enable);

void omap_dm_timer_disable(struct omap_dm_timer *timer)
{
	if (!timer->enabled)
		return;

#ifdef CONFIG_ARCH_OMAP2PLUS
	if (cpu_class_is_omap2()) {
		clk_disable(timer->iclk);
		clk_disable(timer->fclk);
	}
#endif

	timer->enabled = 0;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_disable);

int omap_dm_timer_get_irq(struct omap_dm_timer *timer)
{
	return timer->irq;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_get_irq);

#if defined(CONFIG_ARCH_OMAP1)

/**
 * omap_dm_timer_modify_idlect_mask - Check if any running timers use ARMXOR
 * @inputmask: current value of idlect mask
 */
__u32 omap_dm_timer_modify_idlect_mask(__u32 inputmask)
{
	int i;

	/* If ARMXOR cannot be idled this function call is unnecessary */
	if (!(inputmask & (1 << 1)))
		return inputmask;

	/* If any active timer is using ARMXOR return modified mask */
	for (i = 0; i < dm_timer_count; i++) {
		u32 l;

		l = omap_dm_timer_read_reg(&dm_timers[i], OMAP_TIMER_CTRL_REG);
		if (l & OMAP_TIMER_CTRL_ST) {
			if (((omap_readl(MOD_CONF_CTRL_1) >> (i * 2)) & 0x03) == 0)
				inputmask &= ~(1 << 1);
			else
				inputmask &= ~(1 << 2);
		}
	}

	return inputmask;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_modify_idlect_mask);

#else

struct clk *omap_dm_timer_get_fclk(struct omap_dm_timer *timer)
{
	return timer->fclk;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_get_fclk);

__u32 omap_dm_timer_modify_idlect_mask(__u32 inputmask)
{
	BUG();

	return 0;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_modify_idlect_mask);

#endif

void omap_dm_timer_trigger(struct omap_dm_timer *timer)
{
	omap_dm_timer_write_reg(timer, OMAP_TIMER_TRIGGER_REG, 0);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_trigger);

void omap_dm_timer_start(struct omap_dm_timer *timer)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	if (!(l & OMAP_TIMER_CTRL_ST)) {
		l |= OMAP_TIMER_CTRL_ST;
		omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
	}
}
EXPORT_SYMBOL_GPL(omap_dm_timer_start);

void omap_dm_timer_stop(struct omap_dm_timer *timer)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	if (l & OMAP_TIMER_CTRL_ST) {
		l &= ~0x1;
		omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
#ifdef CONFIG_ARCH_OMAP2PLUS
		/* Readback to make sure write has completed */
		omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
		 /*
		  * Wait for functional clock period x 3.5 to make sure that
		  * timer is stopped
		  */
		udelay(3500000 / clk_get_rate(timer->fclk) + 1);
#endif
	}
	/* Ack possibly pending interrupt */
	omap_dm_timer_write_reg(timer, OMAP_TIMER_STAT_REG,
			OMAP_TIMER_INT_OVERFLOW);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_stop);

#ifdef CONFIG_ARCH_OMAP1

int omap_dm_timer_set_source(struct omap_dm_timer *timer, int source)
{
	int n = (timer - dm_timers) << 1;
	u32 l;

	l = omap_readl(MOD_CONF_CTRL_1) & ~(0x03 << n);
	l |= source << n;
	omap_writel(l, MOD_CONF_CTRL_1);

	return 0;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_source);

#else

int omap_dm_timer_set_source(struct omap_dm_timer *timer, int source)
{
	int ret = -EINVAL;

	if (source < 0 || source >= 3)
		return -EINVAL;

	clk_disable(timer->fclk);
	ret = clk_set_parent(timer->fclk, dm_source_clocks[source]);
	clk_enable(timer->fclk);

	/*
	 * When the functional clock disappears, too quick writes seem
	 * to cause an abort. XXX Is this still necessary?
	 */
	__delay(300000);

	return ret;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_source);

#endif

void omap_dm_timer_set_load(struct omap_dm_timer *timer, int autoreload,
			    unsigned int load)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	if (autoreload)
		l |= OMAP_TIMER_CTRL_AR;
	else
		l &= ~OMAP_TIMER_CTRL_AR;
	omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
	omap_dm_timer_write_reg(timer, OMAP_TIMER_LOAD_REG, load);

	omap_dm_timer_write_reg(timer, OMAP_TIMER_TRIGGER_REG, 0);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_load);

/* Optimized set_load which removes costly spin wait in timer_start */
void omap_dm_timer_set_load_start(struct omap_dm_timer *timer, int autoreload,
                            unsigned int load)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	if (autoreload) {
		l |= OMAP_TIMER_CTRL_AR;
		omap_dm_timer_write_reg(timer, OMAP_TIMER_LOAD_REG, load);
	} else {
		l &= ~OMAP_TIMER_CTRL_AR;
	}
	l |= OMAP_TIMER_CTRL_ST;

	omap_dm_timer_write_reg(timer, OMAP_TIMER_COUNTER_REG, load);
	omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_load_start);

/**
 * omap_dm_timer_set_match - set the dual-mode timer match value
 * @timer:  A pointer to the OMAP dual-mode timer to set the match
 *          value for.
 * @enable: Flag indicating whether to enable the OMAP dual-mode timer
 *          compare mode functionality in which the compare (i.e. load)
 *          and match counters are continuously compared and an interrupt
 *          issued when omap_dm_timer_set_int_enable is set with
 *          OMAP_TIMER_INT_MATCH.
 * @match:  The match value to set in the timer.
 */
void omap_dm_timer_set_match(struct omap_dm_timer *timer, int enable,
			     unsigned int match)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	if (enable)
		l |= OMAP_TIMER_CTRL_CE;
	else
		l &= ~OMAP_TIMER_CTRL_CE;
	omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
	omap_dm_timer_write_reg(timer, OMAP_TIMER_MATCH_REG, match);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_match);

/**
 * omap_dm_timer_set_pwm - set the dual-mode timer into PWM mode
 * @timer:   A pointer to the OMAP dual-mode timer to place into PWM
 *           mode.
 * @def_on:  Flag indicating whether the default output value of the
 *           PWM is high or low and is effectively a PWM polarity
 *           flag.
 * @toggle:  Flag indicating whether the PWM should operate in toggle
 *           or pulse mode. The former yields traditional pulse width
 *           modulation by toggling the value of the output at each
 *           trigger event; whereas the latter yields a single source
 *           clock-width pulse at each trigger event.
 * @trigger: Flags indicating whether the PWM should generate no trigger
 *           or trigger an overflow over or both overflow and compare
 *           events.
 */
void omap_dm_timer_set_pwm(struct omap_dm_timer *timer, int def_on,
			   int toggle, int trigger)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	l &= ~(OMAP_TIMER_CTRL_GPOCFG | OMAP_TIMER_CTRL_SCPWM |
	       OMAP_TIMER_CTRL_PT | (0x03 << 10));
	if (def_on)
		l |= OMAP_TIMER_CTRL_SCPWM;
	if (toggle)
		l |= OMAP_TIMER_CTRL_PT;
	l |= trigger << 10;
	omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_pwm);

void omap_dm_timer_set_prescaler(struct omap_dm_timer *timer, int prescaler)
{
	u32 l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG);
	l &= ~(OMAP_TIMER_CTRL_PRE | (0x07 << 2));
	if (prescaler >= 0x00 && prescaler <= 0x07) {
		l |= OMAP_TIMER_CTRL_PRE;
		l |= prescaler << 2;
	}
	omap_dm_timer_write_reg(timer, OMAP_TIMER_CTRL_REG, l);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_prescaler);

void omap_dm_timer_set_int_enable(struct omap_dm_timer *timer,
				  unsigned int value)
{
	omap_dm_timer_write_reg(timer, OMAP_TIMER_INT_EN_REG, value);
	omap_dm_timer_write_reg(timer, OMAP_TIMER_WAKEUP_EN_REG, value);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_set_int_enable);

unsigned int omap_dm_timer_read_status(struct omap_dm_timer *timer)
{
	unsigned int l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_STAT_REG);

	return l;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_read_status);

void omap_dm_timer_write_status(struct omap_dm_timer *timer, unsigned int value)
{
	omap_dm_timer_write_reg(timer, OMAP_TIMER_STAT_REG, value);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_write_status);

unsigned int omap_dm_timer_read_counter(struct omap_dm_timer *timer)
{
	unsigned int l;

	l = omap_dm_timer_read_reg(timer, OMAP_TIMER_COUNTER_REG);

	return l;
}
EXPORT_SYMBOL_GPL(omap_dm_timer_read_counter);

void omap_dm_timer_write_counter(struct omap_dm_timer *timer, unsigned int value)
{
	omap_dm_timer_write_reg(timer, OMAP_TIMER_COUNTER_REG, value);
}
EXPORT_SYMBOL_GPL(omap_dm_timer_write_counter);

int omap_dm_timers_active(void)
{
	int i;

	for (i = 0; i < dm_timer_count; i++) {
		struct omap_dm_timer *timer;

		timer = &dm_timers[i];

		if (!timer->enabled)
			continue;

		if (omap_dm_timer_read_reg(timer, OMAP_TIMER_CTRL_REG) &
		    OMAP_TIMER_CTRL_ST) {
			return 1;
		}
	}
	return 0;
}
EXPORT_SYMBOL_GPL(omap_dm_timers_active);

int __init omap_dm_timer_init(void)
{
	struct omap_dm_timer *timer;
	int i, map_size = SZ_8K;	/* Module 4KB + L4 4KB except on omap1 */

	if (!(cpu_is_omap16xx() || cpu_class_is_omap2()))
		return -ENODEV;

	spin_lock_init(&dm_timer_lock);

	dm_regs = (u32 *)reg_map;

	if (cpu_class_is_omap1()) {
		dm_timers = omap1_dm_timers;
		dm_timer_count = omap1_dm_timer_count;
		map_size = SZ_2K;
	} else if (cpu_is_omap24xx()) {
		dm_timers = omap2_dm_timers;
		dm_timer_count = omap2_dm_timer_count;
		dm_source_names = omap2_dm_source_names;
		dm_source_clocks = omap2_dm_source_clocks;
	} else if (cpu_is_omap34xx()) {
		dm_timers = omap3_dm_timers;
		dm_timer_count = omap3_dm_timer_count;
		dm_source_names = omap3_dm_source_names;
		dm_source_clocks = omap3_dm_source_clocks;
	} else if (cpu_is_omap44xx()) {
		dm_timers = omap4_dm_timers;
		dm_timer_count = omap4_dm_timer_count;
		dm_source_names = omap4_dm_source_names;
		dm_source_clocks = omap4_dm_source_clocks;
	} else if (cpu_is_ti81xx()) {
		dm_timers = ti81xx_dm_timers;
		/* TI816X has difference clock sources and 1 timer less */
		if (cpu_is_ti816x()) {
			dm_timer_count = ti81xx_dm_timer_count - 1;
			dm_source_names = ti816x_dm_source_names;
			dm_source_clocks = ti816x_dm_source_clocks;
		} else {
			dm_timer_count = ti81xx_dm_timer_count;
			dm_source_names = ti814x_dm_source_names;
			dm_source_clocks = ti814x_dm_source_clocks;
		}

		/* Few registers/offsets are different */
		dm_regs = (u32 *)ti81xx_reg_map;
	}

	if (cpu_class_is_omap2())
		for (i = 0; dm_source_names[i] != NULL; i++)
			dm_source_clocks[i] = clk_get(NULL, dm_source_names[i]);

	if (cpu_is_omap243x())
		dm_timers[0].phys_base = 0x49018000;

	for (i = 0; i < dm_timer_count; i++) {
		timer = &dm_timers[i];

		/* Static mapping, never released */
		timer->io_base = ioremap(timer->phys_base, map_size);
		BUG_ON(!timer->io_base);

#ifdef CONFIG_ARCH_OMAP2PLUS
		if (cpu_class_is_omap2()) {
			char clk_name[16];
			sprintf(clk_name, "gpt%d_ick", i + 1);
			timer->iclk = clk_get(NULL, clk_name);
			sprintf(clk_name, "gpt%d_fck", i + 1);
			timer->fclk = clk_get(NULL, clk_name);
		}
#endif
	}

	return 0;
}
