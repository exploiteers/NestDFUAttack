

# (C) Copyright 2006
# Texas Instruments, <www.ti.com>
#
# OMAP3EVM board uses OMAP3430 (ARM-CortexA8) cpu
# see http://www.ti.com/ for more information on Texas Instruments#
#
# OMAP3EVM has 1 bank of 128MB mPOP-SDRAM on CS0
# Physical Address:
# 8000'0000 (bank0)


# For XIP in 64K of SRAM or debug (GP device has it all availabe)
# SRAM 40200000-4020FFFF base
# initial stack at 0x4020fffc used in s_init (below xloader).
# The run time stack is (above xloader, 2k below)
# If any globals exist there needs to be room for them also

TEXT_BASE = 0x40200000
STACK_BASE = 0x4020FCB0
