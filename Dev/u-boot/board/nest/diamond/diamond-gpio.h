/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
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
 *      This file defines mnemonics for Nest Learning Thermostat-
 *      specific GPIOs.
 */

#ifndef _NEST_DIAMOND_GPIO_H_
#define _NEST_DIAMOND_GPIO_H_

/*
 * Diamond Backplate GPIO Control Signals
 */
#define DIAMOND_GPIO_BACKPLATE_DETECT		65		// 3.1
#define DIAMOND_GPIO_BACKPLATE_3V3_ENABLE	104		// 4.8  (Development)
#define DIAMOND_GPIO_BACKPLATE_RESET		104		// 4.8  (Prototype + EVT)

/*
 * Diamond Ethernet GPIO control signals
 */
#define DIAMOND_GPIO_ENET_NRESET			157		// 5.29 (Development)
#define DIAMOND_GPIO_ENET_IRQ				162		// 6.2  (Development)

/*
 * Diamond LCD Panel GPIO Control Signals
 *
 *   - 3.3V LCM Power Enable (active high)
 *   - 1.8V to 3.3V LCM Signal Buffer Enable (active low)
 *   - LCM Reset (active low)
 *   - LCD ID
 */
#define DIAMOND_GPIO_LCD_NENABLE_BUFFER		43		// 2.11 (Development)
#define DIAMOND_GPIO_LCD_ENABLE_VDD			101		// 4.5  (Development)
#define	DIAMOND_GPIO_LCD_ID					105		// 4.9  (Prototype + EVT)
#define DIAMOND_GPIO_LCD_RESETB				109		// 4.13

/*
 * Diamond Charging and Power Management GPIO Control Signals
 */
#define DIAMOND_GPIO_PMU_CHARGER_SUSP		97		// 4.1  (Development)
#define	DIAMOND_GPIO_PMU_DOUBLE_CURRENT		126		// 4.30 (Development)
#define	DIAMOND_GPIO_PMU_CHARGER_NCHARGE	99		// 4.3  (Development)

/*
 * Diamond Proximity / Ambient Light Sensor GPIO Control Signal
 */
#define DIAMOND_GPIO_PROX_ALS_IRQ			161		// 6.1  (Development)

/*
 * Diamond Rotary Control Input GPIO Control Signals
 */
#define DIAMOND_GPIO_ROTARY_DIR				105		// 4.9  (Development)
#define DIAMOND_GPIO_ROTARY_VALID			106		// 4.10 (Development)
#define DIAMOND_GPIO_ROTARY_LAST_DIR		107		// 4.11 (Development)
#define DIAMOND_GPIO_ROTARY_LAST_VALID		108		// 4.12 (Development)
#define DIAMOND_GPIO_ROTARY_CLEAR			110		// 4.14 (Development)

/*
 * Diamond Avago ADBS A320 Optical Finger Navigation (OFN) Sensor Signals
 */
#define DIAMOND_EVT_GPIO_OFN_RESET_L		110		// 4.14 (EVT)
#define DIAMOND_EVT_GPIO_OFN_SHUTDOWN		101		// 4.5  (EVT)
#define DIAMOND_PROTOTYPE_GPIO_OFN_RESET_L	184		// 6.24 (Prototype)
#define DIAMOND_PROTOTYPE_GPIO_OFN_SHUTDOWN	185		// 6.25 (Prototype)
#define DIAMOND_GPIO_OFN_MOTION_L			108		// 4.12 (Prototype + EVT)

/*
 * Diamond USB GPIO Control Signals
 */
#define DIAMOND_GPIO_INTUSB_SUSPEND			42		// 2.10

/*
 * Diamond Wireless Ethernet GPIO Control Signals
 *
 *   - Chip Enable
 *   - Interrupt
 */
#define DIAMOND_GPIO_WIFI_ENABLE			103		// 4.7
#define	DIAMOND_GPIO_WIFI_IRQ				102		// 4.6

/*
 * Diamond ZigBee GPIO Control Signals
 */
#define DIAMOND_GPIO_ZIGBEE_CPU_OUT			58		// 2.26 (Development)
#define DIAMOND_GPIO_ZIGBEE_CPU_IN			163		// 6.3  (Development)
#define DIAMOND_GPIO_ZIGBEE_RESET_L			61		// 2.29
#define DIAMOND_GPIO_ZIGBEE_PWR_ENABLE		158		// 5.30

/*
 * Diamond Piezo GPIO Control Signals
 */
#define DIAMOND_GPIO_PIEZO_NENABLE			58		// 2.26 (Prototype + EVT)

/*
 * Diamond Battery Disconnect Control Signal
 */
#define DIAMOND_GPIO_BATT_DISCONNECT		161		// 6.1  (EVT)

#endif /* _NEST_DIAMOND_GPIO_H_ */
