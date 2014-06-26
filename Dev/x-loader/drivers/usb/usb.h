/*
 * USB bootloader
 *
 * Copyright (C) 2011 Rick Bronson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _USB_H_
#define _USB_H_

#define OMAP34XX_USB_BASE     (OMAP34XX_CORE_L4_IO_BASE + 0xAB000)

#define OMAP34XX_USB_EP(n)         (OMAP34XX_USB_BASE + 0x100 + 0x10*(n))

#define MUSB_RXCSR     0x06
#define MUSB_RXCOUNT       0x08
#define MUSB_TXCSR     0x02
#define MUSB_FIFOSIZE      0x0F
#define OMAP34XX_USB_RXCSR(n)      (OMAP34XX_USB_EP(n) + MUSB_RXCSR)
#define OMAP34XX_USB_RXCOUNT(n)    (OMAP34XX_USB_EP(n) + MUSB_RXCOUNT)
#define OMAP34XX_USB_TXCSR(n)      (OMAP34XX_USB_EP(n) + MUSB_TXCSR)
#define OMAP34XX_USB_FIFOSIZE(n)   (OMAP34XX_USB_EP(n) + MUSB_FIFOSIZE)
#define OMAP34XX_USB_FIFO(n)       (OMAP34XX_USB_BASE + 0x20 + ((n) * 4))

/* memory mapped registers */
#define BULK_ENDPOINT 1
static volatile u16 *peri_rxcsr = (volatile u16 *) OMAP34XX_USB_RXCSR(BULK_ENDPOINT);
#define MUSB_RXCSR_RXPKTRDY        0x0001
static volatile u16 *rxcount    = (volatile u16 *) OMAP34XX_USB_RXCOUNT(BULK_ENDPOINT);
static volatile u16 *peri_txcsr = (volatile u16 *) OMAP34XX_USB_TXCSR(BULK_ENDPOINT);
#define MUSB_TXCSR_TXPKTRDY        0x0001
static volatile u8  *bulk_fifo  = (volatile u8  *) OMAP34XX_USB_FIFO(BULK_ENDPOINT);

/* In high speed mode packets are 512
   In full speed mode packets are 64 */
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0  (0x0200)
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1  (0x0040)

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b))

#define PACK4(a,b,c,d)     (((d)<<24) | ((c)<<16) | ((b)<<8) | (a))
#define USBLOAD_CMD_FILE PACK4('U', 'S', 'B', 's')  /* send file size */
#define USBLOAD_CMD_JUMP PACK4('U', 'S', 'B', 'j')  /* go where I tell you */
#define USBLOAD_CMD_FILE_REQ PACK4('U', 'S', 'B', 'f')  /* file request */
#define USBLOAD_CMD_ECHO_SZ PACK4('U', 'S', 'B', 'n')  /* echo file size */
#define USBLOAD_CMD_REPORT_SZ PACK4('U', 'S', 'B', 'o')  /* report file size */
#define USBLOAD_CMD_MESSAGE PACK4('U', 'S', 'B', 'm')  /* message for debug */

#endif

