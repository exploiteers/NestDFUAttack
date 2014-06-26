#
# (C) Copyright 2004 Texas Instruments
#
# (C) Copyright 2000-2006
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).a

SOBJS	+= _ashldi3.o
SOBJS	+= _divsi3.o
SOBJS	+= _lshrdi3.o
SOBJS	+= _modsi3.o
SOBJS	+= _udivsi3.o
SOBJS	+= _umodsi3.o

COBJS-y += crc32.o
COBJS-y += ctype.o
COBJS-y += display_options.o
COBJS-y	+= div0.o
COBJS-y	+= board.o
COBJS-y += string.o
COBJS-y	+= ecc.o
COBJS-y	+= printf.o
COBJS-y	+= omap_bch_decoder.o
COBJS-y	+= omap_bch.o

COBJS	:= $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
