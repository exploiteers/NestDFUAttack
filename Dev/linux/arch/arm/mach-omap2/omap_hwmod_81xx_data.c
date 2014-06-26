/*
 * omap_hwmod_ti81xx_data.c - hardware modules data for TI81XX chips
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
 *
 */
#include <plat/omap_hwmod.h>
#include <mach/irqs.h>
#include <plat/cpu.h>
#include <plat/dma.h>
#include <plat/serial.h>
#include <plat/l4_3xxx.h>
#include <plat/ti81xx.h>
#include <plat/gpio.h>

#include "omap_hwmod_common_data.h"

#include "control.h"
#include "cm81xx.h"
#include "cm-regbits-81xx.h"

/*
 * TI816X hardware modules integration data
 *
 * Note: This is incomplete and at present, not generated from h/w database.
 *
 * TODO: Add EDMA in the 'user' field wherever applicable.
 */

static struct omap_hwmod ti816x_mpu_hwmod;
static struct omap_hwmod ti816x_l3_slow_hwmod;
static struct omap_hwmod ti816x_l4_slow_hwmod;

/* L3 SLOW -> L4_SLOW Peripheral interface */
static struct omap_hwmod_ocp_if ti816x_l3_slow__l4_slow = {
	.master	= &ti816x_l3_slow_hwmod,
	.slave	= &ti816x_l4_slow_hwmod,
	.user	= OCP_USER_MPU,
};

/* MPU -> L3 SLOW interface */
static struct omap_hwmod_ocp_if ti816x_mpu__l3_slow = {
	.master = &ti816x_mpu_hwmod,
	.slave	= &ti816x_l3_slow_hwmod,
	.user	= OCP_USER_MPU,
};

/* Slave interfaces on the L3 SLOW interconnect */
static struct omap_hwmod_ocp_if *ti816x_l3_slow_slaves[] = {
	&ti816x_mpu__l3_slow,
};

/* Master interfaces on the L3 SLOW interconnect */
static struct omap_hwmod_ocp_if *ti816x_l3_slow_masters[] = {
	&ti816x_l3_slow__l4_slow,
};

/* L3 SLOW */
static struct omap_hwmod ti816x_l3_slow_hwmod = {
	.name		= "l3_slow",
	.class		= &l3_hwmod_class,
	.masters	= ti816x_l3_slow_masters,
	.masters_cnt	= ARRAY_SIZE(ti816x_l3_slow_masters),
	.slaves		= ti816x_l3_slow_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_l3_slow_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
	.flags		= HWMOD_NO_IDLEST,
};

static struct omap_hwmod ti816x_uart1_hwmod;
static struct omap_hwmod ti816x_uart2_hwmod;
static struct omap_hwmod ti816x_uart3_hwmod;
static struct omap_hwmod ti814x_uart4_hwmod;
static struct omap_hwmod ti814x_uart5_hwmod;
static struct omap_hwmod ti814x_uart6_hwmod;
static struct omap_hwmod ti816x_wd_timer2_hwmod;
static struct omap_hwmod ti816x_i2c1_hwmod;
static struct omap_hwmod ti816x_i2c2_hwmod;
static struct omap_hwmod ti816x_gpio1_hwmod;
static struct omap_hwmod ti816x_gpio2_hwmod;

/* L4 SLOW -> UART1 interface */
static struct omap_hwmod_addr_space ti816x_uart1_addr_space[] = {
	{
		.pa_start	= TI81XX_UART1_BASE,
		.pa_end		= TI81XX_UART1_BASE + SZ_8K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__uart1 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_uart1_hwmod,
	.clk		= "uart1_ick",
	.addr		= ti816x_uart1_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti816x_uart1_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> UART2 interface */
static struct omap_hwmod_addr_space ti816x_uart2_addr_space[] = {
	{
		.pa_start	= TI81XX_UART2_BASE,
		.pa_end		= TI81XX_UART2_BASE + SZ_8K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__uart2 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_uart2_hwmod,
	.clk		= "uart2_ick",
	.addr		= ti816x_uart2_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti816x_uart2_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> UART3 interface */
static struct omap_hwmod_addr_space ti816x_uart3_addr_space[] = {
	{
		.pa_start	= TI81XX_UART3_BASE,
		.pa_end		= TI81XX_UART3_BASE + SZ_8K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__uart3 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_uart3_hwmod,
	.clk		= "uart3_ick",
	.addr		= ti816x_uart3_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti816x_uart3_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> UART4 interface */
static struct omap_hwmod_addr_space ti814x_uart4_addr_space[] = {
	{
		.pa_start	= TI814X_UART4_BASE,
		.pa_end		= TI814X_UART4_BASE + SZ_8K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti814x_l4_slow__uart4 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti814x_uart4_hwmod,
	.clk		= "uart4_ick",
	.addr		= ti814x_uart4_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti814x_uart4_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> UART5 interface */
static struct omap_hwmod_addr_space ti814x_uart5_addr_space[] = {
	{
		.pa_start	= TI814X_UART5_BASE,
		.pa_end		= TI814X_UART5_BASE + SZ_8K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti814x_l4_slow__uart5 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti814x_uart5_hwmod,
	.clk		= "uart5_ick",
	.addr		= ti814x_uart5_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti814x_uart5_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> UART6 interface */
static struct omap_hwmod_addr_space ti814x_uart6_addr_space[] = {
	{
		.pa_start	= TI814X_UART6_BASE,
		.pa_end		= TI814X_UART6_BASE + SZ_8K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti814x_l4_slow__uart6 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti814x_uart6_hwmod,
	.clk		= "uart6_ick",
	.addr		= ti814x_uart6_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti814x_uart6_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> Watchdog */
static struct omap_hwmod_addr_space ti816x_wd_timer2_addrs[] = {
	{
		.pa_start	= 0x480C2000,
		.pa_end		= 0x480C2FFF,
		.flags		= ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__wd_timer2 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_wd_timer2_hwmod,
	.clk		= "wdt2_ick",
	.addr		= ti816x_wd_timer2_addrs,
	.addr_cnt	= ARRAY_SIZE(ti816x_wd_timer2_addrs),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> I2C1 */
static struct omap_hwmod_addr_space ti816x_i2c1_addr_space[] = {
	{
		.pa_start	= 0x48028000,
		.pa_end		= 0x48028000 + SZ_4K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__i2c1 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_i2c1_hwmod,
	.clk		= "i2c1_ick",
	.addr		= ti816x_i2c1_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti816x_i2c1_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> I2C2 */
static struct omap_hwmod_addr_space ti816x_i2c2_addr_space[] = {
	{
		.pa_start	= 0x4802A000,
		.pa_end		= 0x4802A000 + SZ_4K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__i2c2 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_i2c2_hwmod,
	.clk		= "i2c2_ick",
	.addr		= ti816x_i2c2_addr_space,
	.addr_cnt	= ARRAY_SIZE(ti816x_i2c2_addr_space),
	.user		= OCP_USER_MPU,
};

/* L4 SLOW -> GPIO1 */
static struct omap_hwmod_addr_space ti816x_gpio1_addrs[] = {
	{
		.pa_start	= TI816X_GPIO0_BASE,
		.pa_end		= TI816X_GPIO0_BASE + SZ_4K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__gpio1 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_gpio1_hwmod,
	.addr		= ti816x_gpio1_addrs,
	.addr_cnt	= ARRAY_SIZE(ti816x_gpio1_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* L4 SLOW -> GPIO2 */
static struct omap_hwmod_addr_space ti816x_gpio2_addrs[] = {
	{
		.pa_start	= TI816X_GPIO1_BASE,
		.pa_end		= TI816X_GPIO1_BASE + SZ_4K - 1,
		.flags		= ADDR_MAP_ON_INIT | ADDR_TYPE_RT,
	},
};

static struct omap_hwmod_ocp_if ti816x_l4_slow__gpio2 = {
	.master		= &ti816x_l4_slow_hwmod,
	.slave		= &ti816x_gpio2_hwmod,
	.addr		= ti816x_gpio2_addrs,
	.addr_cnt	= ARRAY_SIZE(ti816x_gpio2_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* Slave interfaces on the L4_SLOW interconnect */
static struct omap_hwmod_ocp_if *ti816x_l4_slow_slaves[] = {
	&ti816x_l3_slow__l4_slow,
};

/* Master interfaces on the L4_SLOW interconnect */
static struct omap_hwmod_ocp_if *ti816x_l4_slow_masters[] = {
	&ti816x_l4_slow__uart1,
	&ti816x_l4_slow__uart2,
	&ti816x_l4_slow__uart3,
	&ti814x_l4_slow__uart4,
	&ti814x_l4_slow__uart5,
	&ti814x_l4_slow__uart6,
	&ti816x_l4_slow__wd_timer2,
	&ti816x_l4_slow__i2c1,
	&ti816x_l4_slow__i2c2,
	&ti816x_l4_slow__gpio1,
	&ti816x_l4_slow__gpio2,
};

/* L4 SLOW */
static struct omap_hwmod ti816x_l4_slow_hwmod = {
	.name		= "l4_slow",
	.class		= &l4_hwmod_class,
	.masters	= ti816x_l4_slow_masters,
	.masters_cnt	= ARRAY_SIZE(ti816x_l4_slow_masters),
	.slaves		= ti816x_l4_slow_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_l4_slow_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
	.flags		= HWMOD_NO_IDLEST,
};

/* Master interfaces on the MPU device */
static struct omap_hwmod_ocp_if *ti816x_mpu_masters[] = {
	&ti816x_mpu__l3_slow,
};

/* MPU */
static struct omap_hwmod ti816x_mpu_hwmod = {
	.name		= "mpu",
	.class		= &mpu_hwmod_class,
	.main_clk	= "mpu_ck",
	.masters	= ti816x_mpu_masters,
	.masters_cnt	= ARRAY_SIZE(ti816x_mpu_masters),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
};

/* UART common */

static struct omap_hwmod_class_sysconfig uart_sysc = {
	.rev_offs	= 0x50,
	.sysc_offs	= 0x54,
	.syss_offs	= 0x58,
	.sysc_flags	= (SYSC_HAS_SIDLEMODE |
			   SYSC_HAS_ENAWAKEUP | SYSC_HAS_SOFTRESET |
			   SYSC_HAS_AUTOIDLE),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
	.sysc_fields    = &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class uart_class = {
	.name = "uart",
	.sysc = &uart_sysc,
};

/*
 * 'wd_timer' class
 * 32-bit watchdog upward counter that generates a pulse on the reset pin on
 * overflow condition
 */

static struct omap_hwmod_class_sysconfig wd_timer_sysc = {
        .rev_offs       = 0x0000,
        .sysc_offs      = 0x0010,
        .syss_offs      = 0x0014,
        .sysc_flags     = (SYSC_HAS_SIDLEMODE | SYSC_HAS_EMUFREE |
                           SYSC_HAS_ENAWAKEUP | SYSC_HAS_SOFTRESET |
                           SYSC_HAS_AUTOIDLE | SYSC_HAS_CLOCKACTIVITY),
        .idlemodes      = (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
        .sysc_fields    = &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class wd_timer_class = {
	.name = "wd_timer",
	.sysc = &wd_timer_sysc,
};

/* I2C common */
static struct omap_hwmod_class_sysconfig i2c_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x10,
	.syss_offs	= 0x90,
	.sysc_flags	= (SYSC_HAS_SIDLEMODE |
			   SYSC_HAS_ENAWAKEUP | SYSC_HAS_SOFTRESET |
			   SYSC_HAS_AUTOIDLE),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
	.sysc_fields    = &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class i2c_class = {
	.name = "i2c",
	.sysc = &i2c_sysc,
};

/*
 * 'gpio' class
 * general purpose io module
 */
static struct omap_hwmod_class_sysconfig ti816x_gpio_sysc = {
	.rev_offs	= 0x0000,
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0114,
	.sysc_flags	= (SYSC_HAS_AUTOIDLE | SYSC_HAS_ENAWAKEUP |
			   SYSC_HAS_SIDLEMODE | SYSC_HAS_SOFTRESET |
			   SYSS_HAS_RESET_STATUS),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART |
			   SIDLE_SMART_WKUP),
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class ti816x_gpio_hwmod_class = {
	.name	= "gpio",
	.sysc	= &ti816x_gpio_sysc,
	.rev	= 2,
};

/* gpio dev_attr */
static struct omap_gpio_dev_attr gpio_dev_attr = {
	.bank_width	= 32,
	.dbck_flag	= true,
};

/* UART1 */

static struct omap_hwmod_irq_info uart1_mpu_irqs[] = {
	{ .irq = TI81XX_IRQ_UART0, },
};

/*
 * There is no SDMA on TI81XX, instead we have EDMA. Presently using dummy
 * channel numbers as the omap UART driver (drivers/serial/omap-serial.c)
 * requires these values to be filled in even if we don't have DMA enabled. Same
 * applies for other UARTs below.
 */
static struct omap_hwmod_dma_info uart1_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti816x_uart1_slaves[] = {
	&ti816x_l4_slow__uart1,
};

static struct omap_hwmod ti816x_uart1_hwmod = {
	.name		= "uart1",
	.mpu_irqs	= uart1_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(uart1_mpu_irqs),
	.sdma_reqs	= uart1_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(uart1_edma_reqs),
	.main_clk	= "uart1_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI81XX_CM_ALWON_UART_0_CLKCTRL,
		},
	},
	.slaves		= ti816x_uart1_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_uart1_slaves),
	.class		= &uart_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
};

/* UART2 */

static struct omap_hwmod_irq_info uart2_mpu_irqs[] = {
	{ .irq = TI81XX_IRQ_UART1, },
};

static struct omap_hwmod_dma_info uart2_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti816x_uart2_slaves[] = {
	&ti816x_l4_slow__uart2,
};

static struct omap_hwmod ti816x_uart2_hwmod = {
	.name		= "uart2",
	.mpu_irqs	= uart2_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(uart2_mpu_irqs),
	.sdma_reqs	= uart2_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(uart2_edma_reqs),
	.main_clk	= "uart2_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI81XX_CM_ALWON_UART_1_CLKCTRL,
		},
	},
	.slaves		= ti816x_uart2_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_uart2_slaves),
	.class		= &uart_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
};

/* UART3 */

static struct omap_hwmod_irq_info uart3_mpu_irqs[] = {
	{ .irq = TI81XX_IRQ_UART2, },
};

static struct omap_hwmod_dma_info uart3_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti816x_uart3_slaves[] = {
	&ti816x_l4_slow__uart3,
};

static struct omap_hwmod ti816x_uart3_hwmod = {
	.name		= "uart3",
	.mpu_irqs	= uart3_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(uart3_mpu_irqs),
	.sdma_reqs	= uart3_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(uart3_edma_reqs),
	.main_clk	= "uart3_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI81XX_CM_ALWON_UART_2_CLKCTRL,
		},
	},
	.slaves		= ti816x_uart3_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_uart3_slaves),
	.class		= &uart_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
};

/* UART4 */

static struct omap_hwmod_irq_info uart4_mpu_irqs[] = {
	{ .irq = TI814X_IRQ_UART3, },
};

static struct omap_hwmod_dma_info uart4_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti814x_uart4_slaves[] = {
	&ti814x_l4_slow__uart4,
};

static struct omap_hwmod ti814x_uart4_hwmod = {
	.name		= "uart4",
	.mpu_irqs	= uart4_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(uart4_mpu_irqs),
	.sdma_reqs	= uart4_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(uart4_edma_reqs),
	.main_clk	= "uart4_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI814X_CM_ALWON_UART_3_CLKCTRL,
		},
	},
	.slaves		= ti814x_uart4_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti814x_uart4_slaves),
	.class		= &uart_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI814X),
};

/* UART5 */

static struct omap_hwmod_irq_info uart5_mpu_irqs[] = {
	{ .irq = TI814X_IRQ_UART4, },
};

static struct omap_hwmod_dma_info uart5_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti814x_uart5_slaves[] = {
	&ti814x_l4_slow__uart5,
};

static struct omap_hwmod ti814x_uart5_hwmod = {
	.name		= "uart5",
	.mpu_irqs	= uart5_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(uart5_mpu_irqs),
	.sdma_reqs	= uart5_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(uart5_edma_reqs),
	.main_clk	= "uart5_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI814X_CM_ALWON_UART_4_CLKCTRL,
		},
	},
	.slaves		= ti814x_uart5_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti814x_uart5_slaves),
	.class		= &uart_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI814X),
};

/* UART6 */

static struct omap_hwmod_irq_info uart6_mpu_irqs[] = {
	{ .irq = TI814X_IRQ_UART5, },
};

static struct omap_hwmod_dma_info uart6_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti814x_uart6_slaves[] = {
	&ti814x_l4_slow__uart6,
};

static struct omap_hwmod ti814x_uart6_hwmod = {
	.name		= "uart6",
	.mpu_irqs	= uart6_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(uart6_mpu_irqs),
	.sdma_reqs	= uart6_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(uart6_edma_reqs),
	.main_clk	= "uart6_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI814X_CM_ALWON_UART_5_CLKCTRL,
		},
	},
	.slaves		= ti814x_uart6_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti814x_uart6_slaves),
	.class		= &uart_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI814X),
};

/* Watchdog */

static struct omap_hwmod_ocp_if *ti816x_wd_timer2_slaves[] = {
	&ti816x_l4_slow__wd_timer2,
};

static struct omap_hwmod ti816x_wd_timer2_hwmod = {
	.name		= "wd_timer2",
	.main_clk	= "wdt2_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI81XX_CM_ALWON_WDTIMER_CLKCTRL,
		},
	},
	.slaves		= ti816x_wd_timer2_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_wd_timer2_slaves),
	.class		= &wd_timer_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X),
	.flags          = HWMOD_INIT_NO_RESET,
};

/* I2C1 */

static struct omap_hwmod_irq_info i2c1_mpu_irqs[] = {
	{ .irq = TI81XX_IRQ_I2C0, },
};

static struct omap_hwmod_dma_info i2c1_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti816x_i2c1_slaves[] = {
	&ti816x_l4_slow__i2c1,
};

static struct omap_hwmod ti816x_i2c1_hwmod = {
	.name		= "i2c1",
	.mpu_irqs	= i2c1_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(i2c1_mpu_irqs),
	.sdma_reqs	= i2c1_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(i2c1_edma_reqs),
	.main_clk	= "i2c1_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI816X_CM_ALWON_I2C_0_CLKCTRL,
		},
	},
	.slaves		= ti816x_i2c1_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_i2c1_slaves),
	.class		= &i2c_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X | CHIP_IS_TI814X),
};

/* I2C2 */

static struct omap_hwmod_irq_info i2c2_mpu_irqs[] = {
	{ .irq = TI81XX_IRQ_I2C1, },
};

static struct omap_hwmod_dma_info i2c2_edma_reqs[] = {
	{ .name = "tx",	.dma_req = 0, },
	{ .name = "rx",	.dma_req = 0, },
};

static struct omap_hwmod_ocp_if *ti816x_i2c2_slaves[] = {
	&ti816x_l4_slow__i2c2,
};

static struct omap_hwmod ti816x_i2c2_hwmod = {
	.name		= "i2c2",
	.mpu_irqs	= i2c2_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(i2c2_mpu_irqs),
	.sdma_reqs	= i2c2_edma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(i2c2_edma_reqs),
	.main_clk	= "i2c2_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_reg = TI816X_CM_ALWON_I2C_1_CLKCTRL,
		},
	},
	.slaves		= ti816x_i2c2_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_i2c2_slaves),
	.class		= &i2c_class,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X),
};

/* GPIO1 */

static struct omap_hwmod_irq_info ti816x_gpio1_irqs[] = {
	{ .irq = TI81XX_IRQ_GPIO_0A },
	{ .irq = TI81XX_IRQ_GPIO_0B },
};

/* gpio1 slave ports */
static struct omap_hwmod_ocp_if *ti816x_gpio1_slaves[] = {
	&ti816x_l4_slow__gpio1,
};

static struct omap_hwmod_opt_clk gpio1_opt_clks[] = {
	{ .role = "dbclk", .clk = "gpio1_dbck" },
};

static struct omap_hwmod ti816x_gpio1_hwmod = {
	.name		= "gpio1",
	.class		= &ti816x_gpio_hwmod_class,
	.mpu_irqs	= ti816x_gpio1_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(ti816x_gpio1_irqs),
	.main_clk	= "gpio1_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = TI81XX_CM_ALWON_GPIO_0_CLKCTRL,
		},
	},
	.opt_clks	= gpio1_opt_clks,
	.opt_clks_cnt	= ARRAY_SIZE(gpio1_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= ti816x_gpio1_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_gpio1_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X),
};

/* GPIO2 */

static struct omap_hwmod_irq_info ti816x_gpio2_irqs[] = {
	{ .irq = TI81XX_IRQ_GPIO_1A },
	{ .irq = TI81XX_IRQ_GPIO_1B },
};

/* gpio2 slave ports */
static struct omap_hwmod_ocp_if *ti816x_gpio2_slaves[] = {
	&ti816x_l4_slow__gpio2,
};

static struct omap_hwmod_opt_clk gpio2_opt_clks[] = {
	{ .role = "dbclk", .clk = "gpio2_dbck" },
};

static struct omap_hwmod ti816x_gpio2_hwmod = {
	.name		= "gpio2",
	.class		= &ti816x_gpio_hwmod_class,
	.mpu_irqs	= ti816x_gpio2_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(ti816x_gpio2_irqs),
	.main_clk	= "gpio2_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = TI81XX_CM_ALWON_GPIO_1_CLKCTRL,
		},
	},
	.opt_clks	= gpio2_opt_clks,
	.opt_clks_cnt	= ARRAY_SIZE(gpio2_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= ti816x_gpio2_slaves,
	.slaves_cnt	= ARRAY_SIZE(ti816x_gpio2_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_TI816X),
};

static __initdata struct omap_hwmod *ti81xx_hwmods[] = {
	&ti816x_l3_slow_hwmod,
	&ti816x_l4_slow_hwmod,
	&ti816x_mpu_hwmod,
	&ti816x_uart1_hwmod,
	&ti816x_uart2_hwmod,
	&ti816x_uart3_hwmod,
	&ti814x_uart4_hwmod,
	&ti814x_uart5_hwmod,
	&ti814x_uart6_hwmod,
	&ti816x_wd_timer2_hwmod,
	&ti816x_i2c1_hwmod,	/* Note: In TI814X this enables I2C0/2 */
	&ti816x_i2c2_hwmod,
	&ti816x_gpio1_hwmod,
	&ti816x_gpio2_hwmod,
	NULL,
};

int __init ti81xx_hwmod_init(void)
{
	return omap_hwmod_init(ti81xx_hwmods);
}
