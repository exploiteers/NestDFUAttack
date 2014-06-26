/*
 * (C) Copyright 2005 Samsung Electronis
 * Kyungmin Park <kyungmin.park@samsung.com>
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

#include <asm/string.h>

#include "onenand_regs.h"

#if defined(CFG_MMC_ONENAND) || defined(CFG_GPMC_ONENAND)
#define onenand_readw(a)	(*(volatile unsigned short *)(a))
#define onenand_writew(v, a)	((*(volatile unsigned short *)(a)) = (u16) (v))

#define SAMSUNG_MFR_ID		0xEC
#define KFM1G16Q2A_DEV_ID	0x30
#define KFN2G16Q2A_DEV_ID	0x40


#define THIS_ONENAND(a)		(ONENAND_ADDR + (a))

#define READ_INTERRUPT()						\
	onenand_readw(THIS_ONENAND(ONENAND_REG_INTERRUPT))

#define READ_CTRL_STATUS()						\
	onenand_readw(THIS_ONENAND(ONENAND_REG_CTRL_STATUS))

#define READ_ECC_STATUS()						\
	onenand_readw(THIS_ONENAND(ONENAND_REG_ECC_STATUS))
	
#define SET_EMIFS_CS_CONFIG(v)					\
	(*(volatile unsigned long *)(OMAP_EMIFS_CS_CONFIG) = (v))

#define onenand_block_address(block)		(block)
#define onenand_sector_address(page)		(page << 2)
#define onenand_buffer_address()		((1 << 3) << 8)
#define onenand_bufferram_address(block)	(0)

#if defined(CFG_SYNC_BURST_READ) && defined(CONFIG_OMAP1610)
static inline void set_sync_burst_read(void)
{
	unsigned int value;
	value = 0
		| (0x1 << 15)		/* Read Mode: Synchronous */
		| (0x4 << 12)		/* Burst Read Latency: 4 cycles */
		| (0x4 << 9)		/* Burst Length: 8 word */
		| (0x1 << 7)		/* RDY signal plarity */
		| (0x1 << 6)		/* INT signal plarity */
		| (0x1 << 5)		/* I/O buffer enable */
		;
	onenand_writew(value, THIS_ONENAND(ONENAND_REG_SYS_CFG1));

	value = 0
		| (4 << 16)		/* Synchronous Burst Read */
		| (1 << 12)		/* PGWST/WELEN */
		| (1 << 8)		/* WRWST */
		| (4 << 4)		/* RDWST */
		| (1 << 0)		/* FCLKDIV => 48MHz */
		;
	SET_EMIFS_CS_CONFIG(value);
}

static inline void set_async_read(void)
{
	unsigned int value;
	value = 0
		| (0x0 << 15)		/* Read Mode: Asynchronous */
		| (0x4 << 12)		/* Burst Read Latency: 4 cycles */
		| (0x0 << 9)		/* Burst Length: continuous */
		| (0x1 << 7)		/* RDY signal plarity */
		| (0x1 << 6)		/* INT signal plarity */
		| (0x0 << 5)		/* I/O buffer disable */
		;
	onenand_writew(value, THIS_ONENAND(ONENAND_REG_SYS_CFG1));

	value = 0
		| (0 << 16)		/* Asynchronous Read */
		| (1 << 12)		/* PGWST/WELEN */
		| (1 << 8)		/* WRWST */
		| (3 << 4)		/* RDWST */
		| (1 << 0)		/* FCLKDIV => 48MHz */
		;
	SET_EMIFS_CS_CONFIG(value);
}
#else
#define set_sync_burst_read(...)	do { } while (0)
#define set_async_read(...)		do { } while (0)
#endif

int
onenand_chip()
{
	unsigned short mf_id, dev_id;
	mf_id = (*(volatile unsigned short *)(THIS_ONENAND(ONENAND_REG_MANUFACTURER_ID)));
	dev_id = (*(volatile unsigned short *)(THIS_ONENAND(ONENAND_REG_DEVICE_ID)));

	if(mf_id == SAMSUNG_MFR_ID) {
		if (dev_id == KFM1G16Q2A_DEV_ID) {
		printf("Detected Samsung MuxOneNAND1G Flash \r\n");
		return 0;
		} else if (dev_id == KFN2G16Q2A_DEV_ID) {
			printf("Detected Samsung MuxOneNAND2G Flash \r\n");
                        return 0;
		} else {
			printf(" ONENAND Flash unsupported\r\n");
                        return 1;
		}
	} else {
		printf("ONENAND Flash Unsupported\r\n");
		return 1;
	}
}

/* read a page with ECC */
static inline int onenand_read_page(ulong block, ulong page, u_char *buf)
{
	unsigned long *base;

#ifndef __HAVE_ARCH_MEMCPY32
	unsigned int offset, value;
	unsigned long *p;
	unsigned int ctrl, ecc;
	unsigned short bbmarker;
#endif

	onenand_writew(onenand_block_address(block),
		THIS_ONENAND(ONENAND_REG_START_ADDRESS1));

	onenand_writew(onenand_sector_address(page),
		THIS_ONENAND(ONENAND_REG_START_ADDRESS8));

	onenand_writew(onenand_buffer_address(),
		THIS_ONENAND(ONENAND_REG_START_BUFFER));

	onenand_writew(onenand_bufferram_address(block),
		THIS_ONENAND(ONENAND_REG_START_ADDRESS2));

	onenand_writew(ONENAND_INT_CLEAR, THIS_ONENAND(ONENAND_REG_INTERRUPT));

	onenand_writew(ONENAND_CMD_READ, THIS_ONENAND(ONENAND_REG_COMMAND));

#ifndef __HAVE_ARCH_MEMCPY32
 	p = (unsigned long *) buf;
#endif
	base = (unsigned long *) (ONENAND_ADDR + ONENAND_DATARAM);

	while (!(READ_INTERRUPT() & ONENAND_INT_MASTER))
		continue;
	/* Check if the block is bad. Bad block markers    */
	/* are stored in spare area of 1st or 2nd page */
	if ((page == 0) || (page == 1))
	{
	    unsigned long *spareArea = (unsigned long *) (ONENAND_ADDR + ONENAND_SPARERAM);
	    bbmarker = *spareArea;
            /* for bad block markers */
            if (bbmarker != 0xFFFF){
                return 1;
            }
	}

	ctrl = READ_CTRL_STATUS();
	
	if (ctrl & ONENAND_CTRL_ERROR) {
		hang();
	}
	
	if (READ_INTERRUPT() & ONENAND_INT_READ) {

		ecc = READ_ECC_STATUS();
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			hang();
		}
	}
	
#ifdef __HAVE_ARCH_MEMCPY32
	/* 32 bytes boundary memory copy */
	memcpy32(buf, base, ONENAND_PAGE_SIZE);
#else
	for (offset = 0; offset < (ONENAND_PAGE_SIZE >> 2); offset++) {
		value = *(base + offset);
		*p++ = value;
 	}
#endif

	return 0;
}

#define ONENAND_START_PAGE		0
#define ONENAND_PAGES_PER_BLOCK		64

/**
 * onenand_read_block - Read a block data to buf
 * @return 0 on sucess
 */ 

int onenand_read_block(unsigned char *buf, ulong block)
{
	int page, offset = 0;

	set_sync_burst_read();

	/* NOTE: you must read page from page 1 of block 0 */
	/* read the block page by page*/
	for (page = ONENAND_START_PAGE;
	    page < ONENAND_PAGES_PER_BLOCK; page++) {

		if (onenand_read_page(block, page, buf + offset)){
		    set_async_read();
		    return 1;
		}

		offset += ONENAND_PAGE_SIZE;
	}

	set_async_read();

	return 0;
}

#endif /* defined(CFG_MMC_ONENAND) || defined(CFG_GPMC_ONENAND) */
