#!/bin/sh
# This file attempts to use the omap3_usbload binary to push a a x-load.bin and u-boot.bin to the nest Load Address.
# GTVHacker
sudo ./omap3_usbload -f '../built/x-load.bin' -a 0x080100000 -f '../built/u-boot.bin' -a 0x080A00000 -f '../built/uImage' -v -j 0x080100000
