#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libnand.a

ifeq ($(BOARD), omap3430sdp)
COBJS-y	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap3430labrador)
COBJS-y	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap3evm)
COBJS-y	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), am3517evm)
COBJS-y	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap2420h4)
COBJS-y	+= k9k1216.o
endif

ifeq ($(BOARD), omap2430sdp)
COBJS-y	+= k9k1216.o
endif

ifeq ($(BOARD), omap1710h3)
COBJS-y	+= k9f5616.o
endif

ifeq ($(BOARD), diamond)
COBJS-y	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), j49)
COBJS-y	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), j49-usb-loader)
COBJS-y	+= k9f1g08r0a.o
endif

## Disabled for now:
##	  cs8900.o ct69000.o dataflash.o dc2114x.o ds1722.o \
##	  lan91c96.o mw_eeprom.o natsemi.o \
##	  smc91111.o smiLynxEM.o spi_eeprom.o sym53c8xx.o \
##

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
