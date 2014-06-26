/*
 *    Copyright (c) 2012 Nest Labs, Inc.
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *      This file defines the u-boot configuration settings for the
 *      Nest Learning Thermostat J49 board.  The items below should
 *      be updated as the hardware settles.
 *
 *      SDRAM Controller (SDRC):
 *        - Samsung K4X51163PI-FCG6 64 MiB DDR SDRAM 
 *
 *      General Purpose Memory Controller (GPMC):
 *        - Micron MT29F2G16ABDHC-ET 256 MiB SLC NAND Flash (Development)
 *        - Micron MT29F2G16ABBEAH4 256 MiB SLC NAND Flash (Non-development)
 *        - Samsung K9F4G08U0C 512 MiB SLC NAND Flash (Development)
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development)
 *
 *      Multimedia Card (MMC) Interface:
 *        - Molex 500998-0900 Secure Digital (SD) Connector
 *        - TI WL1270 Wireless LAN (WLAN) Module
 *
 *      Display Subsystem (DSS):
 *        - Samsung LMS350DF03 HVGA (320 x 480) LCM (Development and Prototype)
 *        - Tianma TM025ZDZ01 (320 x 320) LCM (Production)
 *
 *      Camera Image Signal Processor:
 *        - Unused
 *
 *      Inter-Integrated Circuit (I2C) Controller:
 *        - Channel 1 (Bus 0):
 *          * Texas Instruments TPS65921 Power Management Unit (PMU)
 *
 *        - Channel 2 (Bus 1):
 *        - Si1143 Proximity / Ambient Light Sensor (Development)
 *
 *        - Channel 3 (Bus 2):
 *          * Unused
 *
 *        - Channel 4 (Bus 3):
 *          * Texas Instruments TPS65921 Power Management Unit (PMU)
 *
 *      UART Controller:
 *        - Channel 1:
 *            * Head Unit Serial Console
 *
 *        - Channel 2:
 *            * ZigBee Wireless Module
 *
 *      Multichannel Buffered Serial Port (McBSP):
 *        - Channel 1:
 *        - Channel 2:
 *        - Channel 3:
 *        - Channel 4:
 *        - Channel 5:
 *
 *      Multichannel Serial Peripheral Interface (McSPI):
 *        - Channel 1:
 *        - Channel 2:
 *        - Channel 3:
 *
 *      Interrupt Controller:
 *        - SMSC LAN9220 10/100 Mbit Ethernet MAC and PHY (Development)
 *        - Si1143 Proximity / Ambient Light Sensor (Development)
 *
 *      GPIO Controller:
 *        - Backplate Detect
 *        - ZigBee
 */

#ifndef __J49_CONFIG_H
#define __J49_CONFIG_H

#include "diamond.h"

#endif /* __J49_CONFIG_H */
