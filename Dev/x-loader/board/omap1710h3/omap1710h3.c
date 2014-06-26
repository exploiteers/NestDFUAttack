/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Jian Zhang <jzhang@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#if defined(CONFIG_OMAP1710)
#include <./configs/omap1510.h>
#endif
  
#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

int board_init (void)
{
#ifdef CFG_PRINTF 

  	/* setup for UART1 */
	*(volatile unsigned int *) ((unsigned int)FUNC_MUX_CTRL_0) &= ~(0x02000000);	/* bit 25 */
  	/* bit 29 for UART1 */
	*(volatile unsigned int *) ((unsigned int)MOD_CONF_CTRL_0) &= ~(0x00002000);
  
	/* Enable the power for UART1 */
#define UART1_48MHZ_ENABLE	((unsigned short)0x0200)
#define SW_CLOCK_REQUEST	0xFFFE0834
	*((volatile unsigned short *)SW_CLOCK_REQUEST) |= UART1_48MHZ_ENABLE;

#endif

 	*(volatile unsigned int *) ((unsigned int)COMP_MODE_CTRL_0) = COMP_MODE_ENABLE;
  	return 0;
}

#define GPIO1_DIRECTION		0xFFFBE434
#define FUNC_MUX_CTRL_F		0xFFFE1094
#define PU_PD_SEL_4			0xFFFE10C4
/*
 * On H3 board, Nand R/B is tied to GPIO_10
 * We setup this GPIO pin 
 */
int nand_init (void)
{
   	
 	/* GPIO_10 for input. it is in GPIO1 module */
 	*(volatile unsigned int *) ((unsigned int)GPIO1_DIRECTION) |= 0x0400; 
	
	/* GPIO10 Func_MUX_CTRL reg bit 29:27, Configure V2 to mode1 as GPIO */
	*(volatile unsigned int *) ((unsigned int)FUNC_MUX_CTRL_F) &= 0xC7FFFFFF;
	*(volatile unsigned int *) ((unsigned int)FUNC_MUX_CTRL_F) |= 0x08000000;
	  
	/* GPIO10 pullup/down register, Enable pullup on GPIO10 */
	*(volatile unsigned int *) ((unsigned int)PU_PD_SEL_4) |= 0x08;
 	 
 	if (nand_chip()){
		printf("Unsupported Chip!\n");
		return 1;
	}
	return 0;
}

/* optionally do something like blinking LED */
void board_hang (void)
{}
 
