/*
 * PCIe RC (Host) Driver for TI816X PCIe Module (PCIESS) configured as Root
 * Complex.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
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

/*
 * General TODO:
 * - All of the register accesses are through raw_read/writes which eat up lines
 *   especially when needed to mask bits before writing. Better to add
 *   read-mask-write kind of wrappers.
 * - Lots of private data to maintain - group together inside a structure and
 *   provide accessor functions?
 * - Possibility of adding hw de-initialization sequence (also, re-enumeration /
 *   PM impact)
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/msi.h>
#include <linux/pci.h>

#include <asm/irq.h>
#include <asm/signal.h>
#include <asm/mach/pci.h>

#include <plat/ti81xx.h>

#include "pcie-ti816x.h"

#define DRIVER_NAME		"ti816x_pcie"

static struct platform_device *pcie_pdev;
static int msi_irq_base;
static int msi_irq_num;

/* Details for inbound access to RAM, passed from platform data */
static u32 ram_base, ram_end;

/* Used for BAR0 translation */
static u32 reg_phys;

/* Used for register accesses */
static u32 reg_virt;

/* Interrupt resources */
static int legacy_irq;
static int msi_irq;

/*
 * Protect io accesses as it involves setting IOBASE register
 *
 * FIXME: We use differnet locks for IO and config accesses. This should be OK
 * as the h/w spec doesn't mention any restriction on config and io accesses to
 * be performed exclusively
 */
static DEFINE_SPINLOCK(ti816x_pci_io_lock);

/*
 *  Application Register Offsets
 */
#define PCISTATSET                      0x010
#define CMD_STATUS			0x004
#define CFG_SETUP			0x008
#define IOBASE				0x00c
#define OB_SIZE				0x030
#define MSI_IRQ				0x054
#define OB_OFFSET_INDEX(n)              (0x200 + (8 * n))     /* 32 Registers */
#define OB_OFFSET_HI(n)			(0x204 + (8 * n))     /* 32 Registers */
#define IB_BAR0				0x300
#define IB_START0_LO			0x304
#define IB_START0_HI			0x308
#define IB_OFFSET0			0x30c
#define ERR_IRQ_STATUS_RAW		0x1c0
#define ERR_IRQ_STATUS			0x1c4
#define MSI0_IRQ_STATUS			0x104
#define MSI0_IRQ_ENABLE_SET		0x108
#define MSI0_IRQ_ENABLE_CLR		0x10c
#define IRQ_ENABLE_SET			0x188
#define IRQ_ENABLE_CLR			0x18c

/*
 * PCIe Config Register Offsets (misc)
 */
#define DEBUG0				0x728
#define PL_GEN2				0x80c

/* Various regions in PCIESS address space */
#define SPACE0_LOCAL_CFG_OFFSET		0x1000
#define SPACE0_REMOTE_CFG_OFFSET	0x2000
#define SPACE0_IO_OFFSET		0x3000

/* Application command register values */
#define DBI_CS2_EN_VAL			BIT(5)
#define IB_XLAT_EN_VAL			BIT(2)
#define OB_XLAT_EN_VAL			BIT(1)
#define LTSSM_EN_VAL			BIT(0)

/* Link training encodings as indicated in DEBUG0 register */
#define LTSSM_STATE_MASK		0x1f
#define LTSSM_STATE_L0			0x11

/* Directed Speed Change */
#define DIR_SPD				(1 << 17)

/* Default value used for Command register */
#define CFG_PCIM_CSR_VAL         (PCI_COMMAND_SERR			\
					| PCI_COMMAND_PARITY		\
					| PCI_COMMAND_INVALIDATE	\
					| PCI_COMMAND_MASTER		\
					| PCI_COMMAND_MEMORY		\
					| PCI_COMMAND_INTX_DISABLE)

/* Error mask for errors to check on CFG/IO */
#define CFG_PCI_ERR_MASK	  ((0xf << 28) | (1 < 24))

/* Outbound window size specified as power of 2 MB */
#define CFG_PCIM_WIN_SZ_IDX	3
#define CFG_PCIM_WIN_CNT	32

/* Maximum MSIs supported by PCIESS */
#define CFG_MAX_MSI_NUM		32

static int get_and_clear_err(void)
{
	int status = __raw_readl(reg_virt + ERR_IRQ_STATUS_RAW);

	if (status) {
		/* The PCIESS interrupt status buts are "write 0 to clear" */
		__raw_writel(~status, reg_virt + ERR_IRQ_STATUS);

		/*
		 * Clear all errors. We are not worried here about individual
		 * status as no specific handling is required.
		 */
		__raw_writew(0xffff, reg_virt + SPACE0_LOCAL_CFG_OFFSET + PCI_STATUS);
	}

	return status;
}

/**
 * ti816x_pcie_fault() - ARM abort handler for PCIe non-posted completion aborts
 * @addr: Address target on which the fault generated
 * @fsr: CP15 fault status register value
 * @regs: Pointer to register structure on abort
 *
 * Handles precise abort caused due to PCIe operation.
 *
 * Note that we are relying on virtual address filtering to determine if the
 * target of the precise aborts was a PCIe module access (i.e., config, I/O,
 * register) and only handle such aborts. We could check PCIe error status to
 * confirm if the abort was caused due to non-posted completion status received
 * by PCIESS, but this may not always be true and aborts from some downstream
 * devices, such as PCI-PCI bridges etc may not result into error status bit
 * getting set.
 *
 * Ignores and returns abort as unhandled otherwise.
 *
 * Also note that, using error status check (as was done in earlier
 * implementation) would also handle failed memory accesses (non-posted), but
 * address filerting based handling will cause aborts for memory accesses as the
 * addresses will be outside the PCIESS module space. This seems OK, as any
 * memory access after enumeration is sole responsibility of the driver and the
 * system integrator (e.g., access failures due to hotplug, suspend etc). If
 * using error check based handling, we also need to clear PCIe error status on
 * detecting errors.
 *
 * Note: Due to specific h/w implementation, we can't be sure of what kind of
 * error occurred (UR Completion, CA etc) and all we get is raw error IRQ status
 * and probably SERR which indicate 'some kind of' error - fatal or non-fatal is
 * received/happened.
 */
static int
ti816x_pcie_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	unsigned long instr = *(unsigned long *)regs->ARM_pc;

	pr_debug(DRIVER_NAME ": Data abort: address = 0x%08lx "
			"fsr = 0x%03x PC = 0x%08lx LR = 0x%08lx",
			addr, fsr, regs->ARM_pc, regs->ARM_lr);

#if 0
	if (!get_and_clear_err())
		return -1;
#endif

	/* Note: Only handle PCIESS module space access */
	if ((addr < reg_virt) || (addr >= (reg_virt + SZ_16K)))
		return -1;

	/*
	 * Mimic aborted read of all 1's as required to detect device/function
	 * absence - check if the instruction that caused abort was a LOAD,
	 */
	if ((instr & 0x0c100000) == 0x04100000) {
		int reg = (instr >> 12) & 15;
		unsigned long val;

		if (instr & 0x00400000)
			val = 255;
		else
			val = -1;

		regs->uregs[reg] = val;
	}

	regs->ARM_pc += 4;

	pr_debug(DRIVER_NAME ": Handled PCIe abort\n");

	return 0;
}

/**
 * set_outbound_trans() - Set PHYADDR <-> BUSADDR mapping for outbound
 */
static void set_outbound_trans(u32 start, u32 end)
{
	int i, tr_size;

	pr_debug(DRIVER_NAME ": Setting outbound translation for %#x-%#x\n",
			start, end);

	/* Set outbound translation size per window division */
	__raw_writel(CFG_PCIM_WIN_SZ_IDX & 0x7, reg_virt + OB_SIZE);

	tr_size = (1 << (CFG_PCIM_WIN_SZ_IDX & 0x7)) * SZ_1M;

	/* Using Direct 1:1 mapping of RC <-> PCI memory space */
	for (i = 0; (i < CFG_PCIM_WIN_CNT) && (start < end); i++) {
		__raw_writel(start | 1, reg_virt + OB_OFFSET_INDEX(i));
		__raw_writel(0,	reg_virt + OB_OFFSET_HI(i));
		start += tr_size;
	}

	/* TODO: ensure unused translation regions are disabled */

	/* Enable OB translation */
	 __raw_writel(OB_XLAT_EN_VAL | __raw_readl(reg_virt + CMD_STATUS),
			 reg_virt + CMD_STATUS);
}

/**
 * set_dbi_mode() - Set DBI mode to access overlaid BAR mask registers
 *
 * Since modification of dbi_cs2 involves different clock domain, read the
 * status back to ensure the transition is complete.
 */
static inline void set_dbi_mode(void)
{
	 __raw_writel(DBI_CS2_EN_VAL | __raw_readl(reg_virt + CMD_STATUS),
			 reg_virt + CMD_STATUS);

	  /* FIXME: Need loop to check bit5 = 1? */
	__raw_readl(reg_virt + CMD_STATUS);
}

/**
 * clear_dbi_mode() - Disable DBI mode
 *
 * Since modification of dbi_cs2 involves different clock domain, read the
 * status back to ensure the transition is complete.
 */
static inline void clear_dbi_mode(void)
{
	__raw_writel(~DBI_CS2_EN_VAL & __raw_readl(reg_virt + CMD_STATUS),
			reg_virt + CMD_STATUS);

	  /* FIXME: Need loop to check bit5 = 1? */
	__raw_readl(reg_virt + CMD_STATUS);
}

static void disable_bars(void)
{
	set_dbi_mode();

	__raw_writel(0, reg_virt + SPACE0_LOCAL_CFG_OFFSET +
			PCI_BASE_ADDRESS_0);
	__raw_writel(0, reg_virt + SPACE0_LOCAL_CFG_OFFSET +
			PCI_BASE_ADDRESS_1);

	clear_dbi_mode();
}

/**
 * set_inbound_trans() - Setup inbound access
 *
 * Configure BAR0 and BAR1 for inbound access. BAR0 is set up in h/w to have
 * access to PCIESS application register space and just needs to set up inbound
 * address (mainly used for MSI). While BAR1 is set up to provide translation
 * into specified (SoC/Board level) internal address space.
 *
 * Note: 1:1 mapping for internal addresses is used.
 *
 * TODO: Add 64-bit support
 */
static void set_inbound_trans(void)
{
	/* Configure and set up BAR0 */
	set_dbi_mode();

	/* Enable BAR0 */
	__raw_writel(1, reg_virt + SPACE0_LOCAL_CFG_OFFSET +
			PCI_BASE_ADDRESS_0);

	__raw_writel(SZ_4K - 1, reg_virt +
			SPACE0_LOCAL_CFG_OFFSET + PCI_BASE_ADDRESS_0);

	clear_dbi_mode();

	 /*
	  * For BAR0, just setting bus address for inbound writes (MSI) should
	  * be sufficient. Use physical address to avoid any conflicts.
	  */
	__raw_writel(reg_phys, reg_virt + SPACE0_LOCAL_CFG_OFFSET
				+ PCI_BASE_ADDRESS_0);

	/* Configure BAR1 only if inbound window is specified */
	if (ram_base != ram_end) {
		/*
		 * Set Inbound translation. Skip BAR0 as it will have h/w
		 * default set to open application register space.
		 *
		 * The value programmed in IB_STARTXX registers must be same as
		 * the one set in corresponding BAR from PCI config space.
		 *
		 * We use translation 'offset' value to yield 1:1 mapping so as
		 * to have physical address on RC side = Inbound PCIe link
		 * address. This also ensures no overlapping with base/limit
		 * regions (outbound).
		 */
		__raw_writel(ram_base, reg_virt + IB_START0_LO);
		__raw_writel(0, reg_virt + IB_START0_HI);
		__raw_writel(1, reg_virt + IB_BAR0);
		__raw_writel(ram_base, reg_virt + IB_OFFSET0);

		/*
		 * Set BAR1 mask to accomodate inbound window
		 */

		set_dbi_mode();

		__raw_writel(1, reg_virt + SPACE0_LOCAL_CFG_OFFSET +
				PCI_BASE_ADDRESS_1);

		__raw_writel(ram_end - ram_base, reg_virt +
			SPACE0_LOCAL_CFG_OFFSET + PCI_BASE_ADDRESS_1);

		clear_dbi_mode();

		/* Set BAR1 attributes and value in config space */
		__raw_writel(ram_base | PCI_BASE_ADDRESS_MEM_PREFETCH,
				reg_virt + SPACE0_LOCAL_CFG_OFFSET
				+ PCI_BASE_ADDRESS_1);

		/*
		 * Enable IB translation only if BAR1 is set. BAR0 doesn't
		 * require enabling IB translation as it is set up in h/w
		 */
		__raw_writel(IB_XLAT_EN_VAL | __raw_readl(reg_virt + CMD_STATUS),
				reg_virt + CMD_STATUS);
	}
}

static DECLARE_BITMAP(msi_irq_bits, CFG_MAX_MSI_NUM);

/**
 * ti816x_msi_handler() - Handle MSI interrupt
 * @irq: IRQ line for MSI interrupts
 * @desc: Pointer to irq descriptor
 *
 * Traverse through pending MSI interrupts and invoke handler for each. Also
 * takes care of interrupt controller level mask/ack operation.
 */
static void ti816x_msi_handler(unsigned int irq, struct irq_desc *desc)
{
	int bit = 0;
	u32 status;

	pr_debug(DRIVER_NAME ": Handling MSI irq %d\n", irq);

	/*
	 * The chained irq handler installation would have replaced normal
	 * interrupt driver handler so we need to take care of mask/unmask and
	 * ack operation.
	 */
	desc->chip->mask(irq);
	if (desc->chip->ack)
		desc->chip->ack(irq);

	status = __raw_readl(reg_virt + MSI0_IRQ_STATUS);

	/* FIXME: Use max loops count? */
	while (status) {
		bit = find_first_bit((unsigned long *)&status, 32);
		generic_handle_irq(msi_irq_base + bit);
		status = __raw_readl(reg_virt + MSI0_IRQ_STATUS);
	}

	desc->chip->unmask(irq);
}

static void ack_msi(unsigned int irq)
{
	unsigned int msi_num = irq - msi_irq_base;
	__raw_writel(~(1 << (msi_num & 0x1f)), reg_virt + MSI0_IRQ_STATUS);
}

static void mask_msi(unsigned int irq)
{
	unsigned int msi_num = irq - msi_irq_base;
	__raw_writel(1 << (msi_num & 0x1f), reg_virt + MSI0_IRQ_ENABLE_CLR);
}

static void unmask_msi(unsigned int irq)
{
	unsigned int msi_num = irq - msi_irq_base;
	__raw_writel(1 << (msi_num & 0x1f), reg_virt + MSI0_IRQ_ENABLE_SET);
}

/*
 * TODO: Add support for mask/unmask on remote devices (mask_msi_irq and
 * unmask_msi_irq). Note that, this may not work always - requires endpoints to
 * support mask bits capability.
 */

static struct irq_chip ti816x_msi_chip = {
	.name = "PCIe-MSI",
	.ack = ack_msi,
	.mask = mask_msi,
	.unmask = unmask_msi,
};

/**
 * get_free_msi() - Get a free MSI number
 *
 * Checks for availability of MSI and returns the first available.
 */
static int get_free_msi(void)
{
	int bit;

	do {
		bit = find_first_zero_bit(msi_irq_bits, msi_irq_num);

		if (bit >= msi_irq_num)
			return -ENOSPC;

	} while (test_and_set_bit(bit, msi_irq_bits));

	pr_debug(DRIVER_NAME ": MSI %d available\n", bit);

	return bit;
}

/* Stub for legacy-interrupts-only mode. */
#ifndef CONFIG_PCI_MSI
void write_msi_msg(unsigned int irq, struct msi_msg *msg) {}
#endif

/**
 * arch_setup_msi_irq() - Set up an MSI for Endpoint
 * @pdev: Pointer to PCI device structure of requesting EP
 * @desc: Pointer to MSI descriptor data
 *
 * Assigns an MSI to endpoint and sets up corresponding irq. Also passes the MSI
 * information to the endpont.
 *
 * TODO: Add 64-bit addressing support
 */
int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	int ret, irq;
	struct msi_msg msg;

	if (msi_irq < 0) {
		pr_err(DRIVER_NAME ": MSI irq pin not specified\n");
		return msi_irq;
	}

	ret = get_free_msi();
	if (ret < 0) {
		pr_err(DRIVER_NAME ": Failed to get free MSI\n");
	} else {
		irq = msi_irq_base + ret;
		msg.data = ret;

		dynamic_irq_init(irq);
		ret = set_irq_msi(irq, desc);

		if (!ret) {
			msg.address_hi = 0;
			msg.address_lo = reg_phys + MSI_IRQ;

			pr_debug(DRIVER_NAME ": MSI %d @%#x:%#x, irq = %d\n",
					msg.data, msg.address_hi,
					msg.address_lo, irq);

			write_msi_msg(irq, &msg);

			set_irq_chip_and_handler(irq, &ti816x_msi_chip,
						handle_level_irq);
			set_irq_flags(irq, IRQF_VALID);
		}
	}

	return ret;
}

void arch_teardown_msi_irq(unsigned int irq)
{
	int pos = irq - msi_irq_base;

	dynamic_irq_cleanup(irq);

	clear_bit(pos, msi_irq_bits);
}

/**
 * ti816x_pcie_setup() - Perform PCIe system setup.
 * @nr: PCI controller index
 * @sys: PCI data for the controller
 *
 * Initializeand configure PCIe Root Complex h/w and fill up resource data to be
 * used by PCI core enumeration layer.
 *
 * H/W initializations consist mainly of:
 * - Setting up PCIESS module clock and getting out of reset.
 * - Establish link with downstream.
 * - Set up outbound access.
 * - Enable memory and IO access.
 *
 * Following resources are allotted for bios enumeration:
 * - Non-Prefetch memory
 * - 32-bit IO
 * - Legacy interrupt
 * - MSI (if enabled)
 *
 * TODO: Add
 * - Prefetchable memory
 * - 64-bit addressing support
 * - Additional delay/handshaking for EPs indulging in CRRS
 */
static int ti816x_pcie_setup(int nr, struct pci_sys_data *sys)
{
	struct resource *res, *plat_res;
	struct clk *pcie_ck;

	if (nr != 0)
		return 0;

	pr_info(DRIVER_NAME ": Setting up Host Controller...\n");

	plat_res = platform_get_resource_byname(pcie_pdev,
						IORESOURCE_MEM, "pcie-regs");
	if (!plat_res) {
		pr_err(DRIVER_NAME ": Failed to get 'regs' memory resource\n");
		return -1;
	}

	reg_phys = plat_res->start;

	/*
	 * Substitute the resources which were set up by default by
	 * pcibios_init_hw
	 */
	res = kzalloc(sizeof(*res) * 2, GFP_KERNEL);
	if (res == NULL) {
		pr_err(DRIVER_NAME ": resource structure allocation failed.\n");
		return -1;
	}

	/*
	 * Gather resources which will be used to assign BARs to targets during
	 * scanning.
	 */

	plat_res = platform_get_resource_byname(pcie_pdev,
						IORESOURCE_MEM, "pcie-nonprefetch");
	if (!plat_res) {
		pr_err(DRIVER_NAME ": no resource for non-prefetch memory\n");
		goto err_memres;
	}

	res[0].start = plat_res->start;
	res[0].end = plat_res->end;
	res[0].name = "PCI Memory";
	res[0].flags = IORESOURCE_MEM;

#if 0
	{
		int ret;
	if ((ret=insert_resource(&iomem_resource, &res[0]))) {
		pr_err(DRIVER_NAME ": Failed to reserve memory resource, got %d\n", ret);
		goto err_memres;
	}
#endif
	/* Optional: io window */
	plat_res = platform_get_resource_byname(pcie_pdev, IORESOURCE_IO, "pcie-io");
	if (!plat_res) {
		pr_warning(DRIVER_NAME ": no resource for PCI I/O\n");
	} else {
		res[1].start = plat_res->start;
		res[1].end = plat_res->end;
		res[1].name = "PCI I/O";
		res[1].flags = IORESOURCE_IO;

		if (insert_resource(&ioport_resource, &res[1])) {
			pr_err(DRIVER_NAME ": Failed to reserve io resource\n");
			goto err_iores;
		}
	}

	/*
	 * Note: Only one inbound window can be considered as BAR0 is set up for
	 * application register space in h/w.
	 */
	plat_res = platform_get_resource_byname(pcie_pdev,
						IORESOURCE_MEM, "pcie-inbound0");
	if (!plat_res) {
		pr_warning(DRIVER_NAME ": no resource for inbound PCI\n");
	} else {
		ram_base = plat_res->start;
		ram_end = plat_res->end;
	}

	sys->resource[0] = &res[0];
	sys->resource[1] = &res[1];
	sys->resource[2] = NULL;

	/* 16KB region is sufficiant for reg(4KB) + configs(8KB) + IO(4KB) */
	reg_virt = (u32)ioremap_nocache(reg_phys, SZ_16K);

	if (!reg_virt) {
		pr_err(DRIVER_NAME ": PCIESS register memory remap failed\n");
		goto err_ioremap;
	}

	pr_info(DRIVER_NAME ": Register base mapped @0x%08x\n", (int)reg_virt);

	pcie_ck = clk_get(NULL, "pcie_ck");
	if (IS_ERR(pcie_ck)) {
		pr_err(DRIVER_NAME ": Failed to get PCIESS clock\n");
		goto err_clkget;
	}

	if (clk_enable(pcie_ck))
		goto err_clken;

	/*
	 * TI81xx devices do not support h/w autonomous link up-training to GEN2
	 * form GEN1 in either EP/RC modes. The software needs to initiate speed
	 * change.
	 */
	__raw_writel(DIR_SPD | __raw_readl(
				reg_virt + SPACE0_LOCAL_CFG_OFFSET + PL_GEN2),
			reg_virt + SPACE0_LOCAL_CFG_OFFSET + PL_GEN2);

	/*
	 * Initiate Link Training. We will delay for L0 as specified by
	 * standard, but will still proceed and return success irrespective of
	 * L0 status as this will be handled by explicit L0 state checks during
	 * enumeration.
	 */
	 __raw_writel(LTSSM_EN_VAL | __raw_readl(reg_virt + CMD_STATUS),
			 reg_virt + CMD_STATUS);

	 /* 100ms */
	 msleep(100);

	/*
	 * Identify ourselves as 'Bridge' for enumeration purpose. This also
	 * avoids "Invalid class 0000 for header type 01" warnings from "lspci".
	 *
	 * If at all we want to restore the default class-subclass values, the
	 * best place would be after returning from pci_common_init ().
	 */
	__raw_writew(PCI_CLASS_BRIDGE_PCI,
			reg_virt + SPACE0_LOCAL_CFG_OFFSET + PCI_CLASS_DEVICE);

	/*
	 * Prevent the enumeration code from assigning resources to our BARs. We
	 * will set up them after the scan is complete.
	 */
	disable_bars();

	set_outbound_trans(res[0].start, res[0].end);

	/* Enable 32-bit IO addressing support */
	__raw_writew(PCI_IO_RANGE_TYPE_32 | (PCI_IO_RANGE_TYPE_32 << 8),
			reg_virt + SPACE0_LOCAL_CFG_OFFSET + PCI_IO_BASE);

	/*
	 * FIXME: The IO Decode size bits in IO base and limit registers are
	 * writable from host any time and during enumeration, the Linux PCI
	 * core clears the lower 4-bits of these registers while writing lower
	 * IO address. This makes IO upper address and limit registers to show
	 * value as '0' and not the actual value as configured by the core
	 * during enumeration. We need to re-write bits 0 of IO limit and base
	 * registers again. Need to find if a post configuration hook is
	 * possible. An easier and clear but possibly inefficient WA is to snoop
	 * each config write and restore 32-bit IO decode configuration.
	 */

	/*
	 * Setup as PCI master, also clear any pending  status bits.
	 * FIXME: Nolonger needed as post-scan fixup handles this (see below).
	 */
#if 0
	__raw_writel((__raw_readl(reg_virt + SPACE0_LOCAL_CFG_OFFSET
					+ PCI_COMMAND)
			| CFG_PCIM_CSR_VAL),
			reg_virt + SPACE0_LOCAL_CFG_OFFSET + PCI_COMMAND);
#endif

	legacy_irq = platform_get_irq_byname(pcie_pdev, "legacy_int");

	if (legacy_irq >= 0) {
		__raw_writel(0xf, reg_virt + IRQ_ENABLE_SET);
	} else {
		__raw_writel(0xf, reg_virt + IRQ_ENABLE_CLR);
		pr_warning(DRIVER_NAME ": INTx disabled since no legacy IRQ\n");
	}

	msi_irq = platform_get_irq_byname(pcie_pdev, "msi_int");

	if ((msi_irq >= 0) && msi_irq_num) {
		if (msi_irq_num > CFG_MAX_MSI_NUM) {
			msi_irq_num = max(msi_irq_num, CFG_MAX_MSI_NUM);
			pr_warning(DRIVER_NAME
				": Restricting MSI count to max supported (%d)",
				msi_irq_num);
		}

		set_irq_chained_handler(msi_irq, ti816x_msi_handler);
	} else {
		pr_warning(DRIVER_NAME ": MSI info not available, disabled\n");
		msi_irq_num = 0;
	}

	get_and_clear_err();

	/*
	 * PCIe access errors that result into OCP errors on TI816X are caught
	 * by ARM as "External aborts" (Precise).
	 */
	hook_fault_code(8, ti816x_pcie_fault, SIGBUS, 0,
			"Precise External Abort on non-linefetch");

	 return 1;

err_clken:
	 clk_put(pcie_ck);
err_clkget:
	 iounmap((void __iomem *)reg_virt);
err_ioremap:
	 if (res[1].flags == IORESOURCE_IO)
		 release_resource(&res[1]);
err_iores:
	 release_resource(&res[0]);
err_memres:
	 kfree(res);

	 return -1;
}

/**
 * check_device() - Checks device availability
 * @bus: Pointer to bus to check the device availability on
 * @dev: Device number
 *
 * Checks for the possibility of device being present. Relies on following
 * logic to indicate success:
 * - downstream link must be established to traverse PCIe fabric
 * - treating RC as virtual PCI bridge, first (and only) device on bus 1 will be
 *   numbered as 0
 * - don't check device number beyond bus 1 as device on our secondary side may
 *   as well be a PCIe-PCI bridge
 */
static int check_device(struct pci_bus *bus, int dev)
{
	if ((__raw_readl(reg_virt + SPACE0_LOCAL_CFG_OFFSET + DEBUG0) &
				LTSSM_STATE_MASK) != LTSSM_STATE_L0)
		return 0;

	if (bus->number <= 1) {
		if (dev == 0)
			return 1;
		else
			return 0;
	}

	return 1;
}

/**
 * setup_config_addr() - Set up configuration space address for a device
 * @bus: Bus number the device is residing on
 * @device: Device number
 * @function: Function number in device
 * @where: Offset of configuration register
 *
 * Forms and returns the address of configuration space mapped in PCIESS
 * address space 0. Also configures CFG_SETUP for remote configuration space
 * access.
 *
 * The address space has two regions to access configuration - local and remote.
 * We access local region for bus 0 (as RC is attached on bus 0) and remote
 * region for others with TYPE 1 access when bus > 1. As for device on bus = 1,
 * we will do TYPE 0 access as it will be on our secondary bus (logical).
 * CFG_SETUP is needed only for remote configuration access.
 *
 * _NOTE_: Currently only supports device 0 on bus = 0 which is OK as PCIESS has
 * single root port.
 */
static inline u32 setup_config_addr(u8 bus, u8 device, u8 function)
{
	u32 addr;

	if (bus == 0) {
		addr = reg_virt + SPACE0_LOCAL_CFG_OFFSET;
	} else {
		u32 regval = (bus << 16) | (device << 8) | function;

		/*
		 * Since Bus#1 will be a virtual bus, we need to have TYPE0
		 * access only.
		 */
		/* TYPE 1 */
		if (bus != 1)
			regval |= BIT(24);

		__raw_writel(regval, reg_virt + CFG_SETUP);

		addr = reg_virt + SPACE0_REMOTE_CFG_OFFSET;
	}

	return addr;
}

/**
 * ti816x_pci_read_io() - Perform PCI IO read from a device
 * @addr: IO address
 * @size: Number of bytes
 * @value: Pointer to hold the read value
 */
int ti816x_pci_io_read(u32 addr, int size, u32 *value)
{
	unsigned long flags;

	if (!IS_ALIGNED(addr, size))
		return -1;

	pr_debug(DRIVER_NAME ": IO read @%#x = ", addr);

	spin_lock_irqsave(&ti816x_pci_io_lock, flags);

	__raw_writel(addr & 0xfffff000, reg_virt + IOBASE);

	/* Get the actual address in I/O space */
	addr = reg_virt + SPACE0_IO_OFFSET + (addr & 0xffc);

	*value = __raw_readl(addr);
	*value >>= ((addr & 3)*8);

	spin_unlock_irqrestore(&ti816x_pci_io_lock, flags);

	pr_debug("%#x\n", *value);

	return 0;
}
EXPORT_SYMBOL(ti816x_pci_io_read);

/**
 * ti816x_pci_write_io() - Perform PCI IO write to a device
 * @addr: IO address
 * @size: Number of bytes
 * @value: Value to write
 */
int ti816x_pci_io_write(u32 addr, int size, u32 value)
{
	unsigned long flags;
	u32 iospace_addr;

	if (!IS_ALIGNED(addr, size))
		return -1;

	pr_debug(DRIVER_NAME ": IO write @%#x = %#x\n", addr, value);

	spin_lock_irqsave(&ti816x_pci_io_lock, flags);

	__raw_writel(addr & 0xfffff000, reg_virt + IOBASE);

	/* Get the actual address in I/O space */
	iospace_addr = reg_virt + SPACE0_IO_OFFSET + (addr & 0xffc);

	if (size != 4) {
		u32 shift = (addr & 3) * 8;
		u32 mask = (size == 1 ? 0xff : 0xffff) << shift;
		u32 readval = __raw_readl(iospace_addr);
		value = ((value << shift) & mask) | (readval & ~mask);
	}

	__raw_writel(value, iospace_addr);

	spin_unlock_irqrestore(&ti816x_pci_io_lock, flags);

	return 0;
}
EXPORT_SYMBOL(ti816x_pci_io_write);

/**
 * ti816x_pci_read_config() - Perform PCI configuration read from a device
 * @bus: Pointer to bus to access device on
 * @devfn: Device number of the bus and function number within
 * @where: Configuration space register offset
 * @size: Width of the register in bytes
 * @value: Pointer to hold the read value
 *
 * Note: We skip alignment check and locking since it is taken care by PCI
 * access wrappers.
 */
static int ti816x_pci_read_config(struct pci_bus *bus, unsigned int devfn,
				int where, int size, u32 *value)
{
	u8 bus_num = bus->number;

	pr_debug(DRIVER_NAME ": Reading config[%x] for device %04x:%02x:%02x..",
			where, bus_num, PCI_SLOT(devfn), PCI_FUNC(devfn));

	if (!check_device(bus, PCI_SLOT(devfn))) {
		*value = ~0;
		pr_debug("failed. No link/device.\n");
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	*value = __raw_readl(setup_config_addr(bus_num, PCI_SLOT(devfn),
				PCI_FUNC(devfn)) + (where & ~3));

	if (size == 1)
		*value = (*value >> (8 * (where & 3))) & 0xff;
	else if (size == 2)
		*value = (*value >> (8 * (where & 3))) & 0xffff;

	pr_debug("done. value = %#x\n", *value);
	return PCIBIOS_SUCCESSFUL;
}

/**
 * ti816x_pci_write_config() - Perform PCI configuration write to a device
 * @bus: Pointer to bus to access device on
 * @devfn: Device number of the bus and function number within
 * @where: Configuration space register offset
 * @size: Width of the register in bytes
 * @value: Value to write
 *
 * Note: We skip alignment check and locking since it is taken care by PCI
 * access wrappers.
 */
static int ti816x_pci_write_config(struct pci_bus *bus, unsigned int devfn,
				int where, int size, u32 value)
{
	u8 bus_num = bus->number;
	u32 addr;

	pr_debug(DRIVER_NAME ": Writing config[%x] = %x "
			"for device %04x:%02x:%02x ...", where, value,
			bus_num, PCI_SLOT(devfn), PCI_FUNC(devfn));

	if (!check_device(bus, PCI_SLOT(devfn))) {
		pr_debug("failed. No link/device.\n");
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	addr = setup_config_addr(bus_num, PCI_SLOT(devfn), PCI_FUNC(devfn));

	if (size == 4) {
		__raw_writel(value, addr + where);
	} else if (size == 2) {
		__raw_writew(value, addr + where);
	} else {
		__raw_writeb(value, addr + where);
	}

	/*
	 * The h/w has a limitation where Config Writes don't signal aborts to
	 * processor. Clear explicitly to avoid stale status.
	 */
	get_and_clear_err();

	pr_debug("done.\n");
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops ti816x_pci_ops = {
	.read	= ti816x_pci_read_config,
	.write	= ti816x_pci_write_config,
};

static struct pci_bus *ti816x_pcie_scan(int nr, struct pci_sys_data *sys)
{
	struct pci_bus *bus = NULL;

	pr_info(DRIVER_NAME ": Starting PCI scan...\n");
	if (nr == 0) {
		bus = pci_scan_bus(0, &ti816x_pci_ops, sys);

		/* Post enumeration fixups */
		set_inbound_trans();

		/* Bridges are not getting enabled by default! */
		pci_assign_unassigned_resources();
	}

	return bus;
}

/**
 * ti816x_pcie_map_irq() - Map a legacy interrupt to an IRQ
 * @dev: Device structure of End Point (EP) to assign IRQ to.
 * @slot: Device slot
 * @pin: Pin number for the function
 *
 * Note: Currently ignores all the parameters and only supports mapping to
 * single IRQ.
 */
static int ti816x_pcie_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	pr_debug(DRIVER_NAME "Returning Legacy irq = %d\n", legacy_irq);
	return (legacy_irq >= 0) ? legacy_irq : -1;
}

/* PCI controller setup and configuration data */
static struct hw_pci ti816x_pci = {
	.nr_controllers     = 1,
	.setup              = ti816x_pcie_setup,
	.scan               = ti816x_pcie_scan,
	.preinit            = NULL,
	.postinit           = NULL,
	.swizzle            = pci_std_swizzle,
	.map_irq            = ti816x_pcie_map_irq
};

/**
 * ti816x_pcie_probe() - Invoke PCI BIOS to perrform enumeration.
 * @pdev: Contains platform data as supplied from board level code.
 *
 * Also stores reference to platform device structure for use during PCIe
 * module initialization and configuration.
 */
static int ti816x_pcie_probe(struct platform_device *pdev)
{
	struct ti816x_pcie_data *pdata;

	pcie_pdev = pdev;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		pr_err(DRIVER_NAME ": No platform data\n");
		return -ENODEV;
	}

	msi_irq_base = pdata->msi_irq_base;
	msi_irq_num = pdata->msi_irq_num;

	pr_info(DRIVER_NAME ": Invoking PCI BIOS...\n");
	pci_common_init(&ti816x_pci);

	return 0;
}

static struct platform_driver ti816x_pcie_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe = ti816x_pcie_probe,
};

/**
 * ti816x_pcie_rc_init() - Register PCIe Root Complex node.
 *
 * Invoked as subsystem initialization.
 *
 * IMPORTANT NOTE: We are relying on SoC/Board level code to check PCIESS
 * mode setting (RC/EP) and register the RC device only in RC mode.
 */
static int __init ti816x_pcie_rc_init(void)
{
	platform_driver_register(&ti816x_pcie_driver);
	return 0;
}
subsys_initcall(ti816x_pcie_rc_init);
