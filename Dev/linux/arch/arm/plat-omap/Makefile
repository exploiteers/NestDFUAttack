#
# Makefile for the linux kernel.
#

# Common support
ifeq ($(CONFIG_ARCH_TI81XX),y)
obj-y := common.o sram.o clock.o devices.o mux.o gpio.o \
	 usb.o fb.o io.o counter_32k.o
else
obj-y := common.o sram.o clock.o devices.o dma.o mux.o gpio.o \
	 usb.o fb.o io.o counter_32k.o
endif
obj-m :=
obj-n :=
obj-  :=

# OCPI interconnect support for 1710, 1610 and 5912
obj-$(CONFIG_ARCH_OMAP16XX) += ocpi.o

# omap_device support (OMAP2+ only at the moment)
obj-$(CONFIG_ARCH_OMAP2) += omap_device.o
obj-$(CONFIG_ARCH_OMAP3) += omap_device.o
obj-$(CONFIG_ARCH_OMAP4) += omap_device.o
obj-$(CONFIG_ARCH_TI81XX) += omap_device.o sdma2edma.o

obj-$(CONFIG_OMAP_MCBSP) += mcbsp.o
obj-$(CONFIG_OMAP_IOMMU) += iommu.o iovmm.o
obj-$(CONFIG_OMAP_IOMMU_DEBUG) += iommu-debug.o

obj-$(CONFIG_CPU_FREQ) += cpu-omap.o
obj-$(CONFIG_OMAP_DM_TIMER) += dmtimer.o
obj-$(CONFIG_OMAP_DEBUG_DEVICES) += debug-devices.o
obj-$(CONFIG_OMAP_DEBUG_LEDS) += debug-leds.o
i2c-omap-$(CONFIG_I2C_OMAP) := i2c.o
obj-y += $(i2c-omap-m) $(i2c-omap-y)

# OMAP mailbox framework
obj-$(CONFIG_OMAP_MBOX_FWK) += mailbox.o

#PSP version information
obj-$(CONFIG_MACH_OMAP3517EVM) += psp-version.o
obj-$(CONFIG_MACH_OMAP3EVM) += psp-version.o
obj-$(CONFIG_MACH_OMAP3_BEAGLE) += psp-version.o

obj-$(CONFIG_OMAP_PM_NOOP) += omap-pm-noop.o
obj-$(CONFIG_HAVE_PWM) += pwm.o
