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

#include <common.h>
#include <malloc.h>
#include <asm/arch/cpu.h>        /* get chip and board defs */
#include "usb.h"

extern void udelay (unsigned long usecs);

typedef int boot_os_fn (void);

/* send a buffer via USB */
int usb_send(unsigned char *buffer, unsigned int buffer_size)
   {
   int ret = 0;

   if (!(*peri_txcsr & MUSB_TXCSR_TXPKTRDY))
       {
       unsigned int cntr;

       for (cntr = 0; cntr < buffer_size; cntr++)
           *bulk_fifo = buffer[cntr];

       *peri_txcsr |= MUSB_TXCSR_TXPKTRDY;

       ret = buffer_size;
       }
   return ret;
   }

////////////////////

static int usb_recv (u8 *buffer, int size)
{
   int cntr;
   u16 count = 0;

   if (*peri_rxcsr & MUSB_RXCSR_RXPKTRDY)
       {
       count = *rxcount;
       for (cntr = 0; cntr < count; cntr++)
           {
           *buffer++ = *bulk_fifo;
           }
           /* Clear the RXPKTRDY bit */
       *peri_rxcsr &= ~MUSB_RXCSR_RXPKTRDY;
       }
   return count;  /* FIXME */
}

static unsigned char usb_outbuffer[64];

static void usb_msg (unsigned int cmd, const char *msg)
   {
   unsigned char *p_char = usb_outbuffer;

   * (int *) p_char = cmd;
   p_char += sizeof (cmd);
   if (msg)
       {
       while (*msg)
           *p_char++= *msg++;
       *p_char++= 0;
       }
   usb_send (usb_outbuffer, p_char - usb_outbuffer);
   }

static void usb_code (unsigned int cmd, u32 code)
   {
   unsigned int *p_int = (unsigned int *) usb_outbuffer;

   *p_int++ = cmd;
   *p_int++ = code;
   usb_send (usb_outbuffer, ((unsigned char *) p_int) - usb_outbuffer);
   }

void do_usb (void)
   {
   boot_os_fn *boot_fn;
   int res;
   u32 usb_inbuffer[512];
   u32 total;
   u8 *addr;
   u32 bytes;
   int size;
   int cntr = 0;

   usb_msg (USBLOAD_CMD_FILE_REQ, "file req");
   while(++cntr < 200000)  /* try for 1 second then bail out */
       {
       res = usb_recv ((u8 *) usb_inbuffer, sizeof (usb_inbuffer));
       switch (usb_inbuffer[0])
           {
           case USBLOAD_CMD_FILE:
               printf ("USBLOAD_CMD_FILE total = %d cmd = %c%c%c%c val = 0x%x val = 0x%x\n", 
                       res, 
                       ((char*)&usb_inbuffer[0])[0], ((char*)&usb_inbuffer[0])[1], ((char*)&usb_inbuffer[0])[2], ((char*)&usb_inbuffer[0])[3],
                       usb_inbuffer[1],
                       usb_inbuffer[2]);
               total = usb_inbuffer[1];  /* get size and address */
               addr = (u8 *) usb_inbuffer[2];
               usb_code (USBLOAD_CMD_ECHO_SZ, total);

               bytes = 0;
               while (bytes < total)
                   {
                   size = usb_recv ((u8 *) usb_inbuffer, sizeof (usb_inbuffer));
                   memcpy(addr, usb_inbuffer, size);
                   addr += size;
                   bytes += size;
                   }
               usb_code (USBLOAD_CMD_REPORT_SZ, total);  /* tell him we got this many bytes */
               printf ("got file addr = 0x%x counter = %d\n", addr, cntr);
               usb_msg (USBLOAD_CMD_FILE_REQ, "file req");  /* see if they have another file for us */
               cntr = 0;
               break;
           case USBLOAD_CMD_JUMP:
               printf ("USBLOAD_CMD_JUMP total = %d addr = 0x%x val = 0x%x\n", res, usb_inbuffer[0], usb_inbuffer[1]);
               boot_fn = (boot_os_fn *) usb_inbuffer[1];
               boot_fn();  /* go to u-boot and maybe kernel */
               break;
           default:
               break;
           }
       udelay(10);  /* delay 10 us */
       }
       printf("USB done\n");
       hang();
   }
