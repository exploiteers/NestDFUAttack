INCS =	-I$(IPCDIR)/ \
	-I$(SYSLINK_ROOT)

EXTRA_CFLAGS += $(INCS)

obj-$(CONFIG_TI81XX_VPSS) += vpss.o
vpss-y := core.o fvid2.o dctrl.o grpx.o sbuf.o system.o video.o
