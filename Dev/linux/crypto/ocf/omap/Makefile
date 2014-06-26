# for SGlinux builds
-include $(ROOTDIR)/modules/.config

obj-$(CONFIG_OCF_OMAP)	+= ocf_omap3_crypto.o
ocf_omap3_crypto-y	:= omap3_crypto.o

obj ?= .
EXTRA_CFLAGS += -I$(obj)/.. -I$(obj)/

ifdef TOPDIR
-include $(TOPDIR)/Rules.make
endif

