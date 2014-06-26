#
# (C) Copyright 2006
# Texas Instruments, <www.ti.com>
#
# SDP3430 board uses OMAP3430 (ARM-CortexA8) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# SDP3430 has 1 bank of 32MB or 128MB mDDR-SDRAM on CS0
# SDP3430 has 1 bank of 32MB or 00MB mDDR-SDRAM on CS1
# Physical Address:
# 8000'0000 (bank0)
# A000'0000 (bank1) - re-mappable below CS1

# For use if you want X-Loader to relocate from SRAM to DDR
#TEXT_BASE = 0x80e80000

# For XIP in 64K of SRAM or debug (GP device has it all availabe)
# SRAM 40200000-4020FFFF base
# initial stack at 0x4020fffc used in s_init (below xloader).
# The run time stack is (above xloader, 2k below)
# If any globals exist there needs to be room for them also
TEXT_BASE = 0x40208800
