#!/bin/bash

#Redirect output to log
exec 3>&1 1>>GTVHacker-attack.log 2>&1
set -x

#Display Ascii
echo "" 1>&3
echo "  ██████ ████████╗██╗   ██╗██╗  ██╗ █████╗  ██████╗██╗  ██╗███████╗██████╗ " 1>&3
echo " ██╔════╝╚══██╔══╝██║   ██║██║  ██║██╔══██╗██╔════╝██║ ██╔╝██╔════╝██╔══██╗" 1>&3
echo " ██║  ███╗  ██║   ██║   ██║███████║███████║██║     █████╔╝ █████╗  ██████╔╝" 1>&3
echo " ██║   ██║  ██║   ╚██╗ ██╔╝██╔══██║██╔══██║██║     ██╔═██╗ ██╔══╝  ██╔══██╗" 1>&3
echo " ╚██████╔╝  ██║    ╚████╔╝ ██║  ██║██║  ██║╚██████╗██║  ██╗███████╗██║  ██║" 1>&3
echo "  ╚═════╝   ╚═╝     ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝" 1>&3

#Check to see if the bootloader, x-loader and kernel are built.
if [! -f Nest/u-boot.bin ! -f Nest/x-load.bin ! -f Nest/uImage] 
    then
        echo "[E] - You must run build.sh script prior to running attack."
    else
        echo "[I] - Running loading script." 1>&3
        echo "[I] - Connect your nest to a usb port on your computer, then enter your password so that we can access the attached USB." 1>&3
        echo "[I] - After entering your password, you will need to hold down the screen on your nest for about 10-15 seconds (to enter DFU mode)." 1>&3
        #Just in case, lets chmod the omap3_usbload binary
        chmod +x Linux/omap3_usbload
        #Calling omap3_usbload with x-loader, bootloader, and kernel at proper load addresses.
        exec sudo Linux/omap3_usbload -f 'Nest/x-load.bin' -a 0x080100000 -f 'Nest/u-boot.bin' -a 0x080A00000 -f 'Nest/uImage' -v -j 0x080100000 |tee /dev/fd/3

        echo "[I] - All Done, Drink all the booze, Hack all the things." 1>&3
    fi
