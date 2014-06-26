#
#    Copyright (c) 2010-2012 Nest Labs, Inc.
#    All rights reserved.
#
#    Description:
#      This file is the default root user set-up file for all Bourne- and
#      Korn-compatible shells.
#

PATH="${PATH}:/nestlabs/sbin:/nestlabs/bin"
export PATH

if [ -f $HOME/.bashrc ]; then
   source $HOME/.bashrc
fi
