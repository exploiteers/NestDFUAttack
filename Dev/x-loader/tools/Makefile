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

TOOLSUBDIRS =

#
# toolchains targeting win32 generate .exe files
#
ifneq (,$(findstring WIN32 ,$(shell $(HOSTCC) -E -dM -xc /dev/null)))
SFX = .exe
else
SFX =
endif

#
# Include this after HOSTOS HOSTARCH check
# so that we can act intelligently.
#
include $(TOPDIR)/config.mk

# Generated executable files
BIN_FILES-y += signGP$(SFX)
OBJ_FILES-y += signGP.o

# now $(obj) is defined
HOSTSRCS += $(addprefix $(SRCTREE)/tools/,$(OBJ_FILES-y:.o=.c))
BINS	:= $(addprefix $(obj),$(sort $(BIN_FILES-y)))
HOSTOBJS := $(addprefix $(obj),$(OBJ_FILES-y))
NOPEDOBJS := $(addprefix $(obj),$(NOPED_OBJ_FILES-y))

#
# Use native tools and options
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
#
HOSTCPPFLAGS =	-idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
	        -I $(SRCTREE)/lib/libfdt \
		-I $(SRCTREE)/tools \
		-DTEXT_BASE=$(TEXT_BASE) -DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES


all:	$(obj).depend $(BINS) $(LOGO-y) subdirs

$(obj)signGP$(SFX):	$(obj)signGP.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

subdirs:
ifeq ($(TOOLSUBDIRS),)
	@:
else
	@for dir in $(TOOLSUBDIRS) ; do \
	    $(MAKE) \
		HOSTOS=$(HOSTOS) \
		HOSTARCH=$(HOSTARCH) \
		-C $$dir || exit 1 ; \
	done
endif

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
