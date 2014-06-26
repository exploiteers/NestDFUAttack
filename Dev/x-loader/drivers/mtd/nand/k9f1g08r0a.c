/*
 * (C) Copyright 2004 Texas Instruments
 * Jian Zhang <jzhang@ti.com>
 *
 *  Samsung K9F1G08R0AQ0C NAND chip driver for an OMAP2420 board
 *
 * This file is based on the following u-boot file:
 *	common/cmd_nand.c
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

#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>

#if defined(CFG_NAND_K9F1G08R0A) ||	\
    defined(CFG_NAND_K9F4G08U0C) ||	\
    defined(CFG_NAND_MT29F2G16)

#define K9F1G08R0A_MFR		0xec  /* Samsung */
#define K9F1G08R0A_ID		0xa1  /* part # */

/* Since Micron and Samsung parts are similar in geometry and bus width
 * we can use the same driver. Need to revisit to make this file independent
 * of part/manufacturer
 */
#define MT29F1G_MFR		0x2c  /* Micron */
#define MT29F1G_ID		0xa1  /* x8, 1GiB */
#define MT29F2G_ID		0xba  /* x16, 2GiB */

#define HYNIX4GiB_MFR		0xAD  /* Hynix */
#define HYNIX4GiB_ID		0xBC  /* x16, 4GiB */

#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	(ADDR_COLUMN | ADDR_PAGE)

#define ADDR_OOB		(0x4 | ADDR_COLUMN_PAGE)

#define PAGE_SIZE		2048
#define OOB_SIZE		64
#define MAX_NUM_PAGES		64

#define ECC_CHECK_ENABLE

#if CFG_NAND_ECC_1BIT
#define ECC_SIZE		24
#define ECC_STEPS		3
#define omap_enable_hwecc
#define omap_correct_data
#define omap_calculate_ecc
#elif CFG_NAND_ECC_4BIT
#define ECC_SIZE		28 
#define ECC_STEPS		28 

extern enable_hwecc_bch4(uint32_t bus_width, int32_t mode);
extern int omap_correct_data_bch4(uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc);
extern int omap_calculate_ecc_bch4(const uint8_t *dat, uint8_t *ecc_code);

#define omap_enable_hwecc	omap_enable_hwecc_bch4
#define omap_correct_data	omap_correct_data_bch4
#define omap_calculate_ecc	omap_calculate_ecc_bch4
#elif CFG_NAND_ECC_8BIT
#define ECC_SIZE		52
#define ECC_STEPS		52 

extern void omap_enable_hwecc_bch8(uint32_t bus_width, int32_t mode);
extern int omap_correct_data_bch8(uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc);
extern int omap_calculate_ecc_bch8(const uint8_t *dat, uint8_t *ecc_code);

#define omap_enable_hwecc	omap_enable_hwecc_bch8
#define omap_correct_data	omap_correct_data_bch8
#define omap_calculate_ecc	omap_calculate_ecc_bch8
#else
#error "None of CFG_NAND_ECC_{1,4,8}BIT is defined."
#endif /* CFG_NAND_ECC_1BIT */

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
					  "subs %0, %0, #1\n"
					  "bne 1b":"=r" (loops):"0" (loops));
}

static int nand_read_page(u_char *buf, ulong page_addr);
static int nand_read_oob(u_char * buf, ulong page_addr);

#ifdef CFG_NAND_ECC_1BIT
static u_char ecc_pos[] =
		{40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63};
#elif CFG_NAND_ECC_4BIT
static u_char ecc_pos[] =
		{36, 37, 38, 39, 40, 41, 42, 
		43, 44, 45, 46, 47, 48, 49, 
		50, 51, 52, 53, 54, 55, 56, 
		57, 58, 59, 60, 61, 62, 63};
#elif CFG_NAND_ECC_8BIT
static u_char ecc_pos[] =
		{12, 13, 14,
		15, 16, 17, 18, 19, 20, 21,
		22, 23, 24, 25, 26, 27, 28,
		29, 30, 31, 32, 33, 34, 35,
		36, 37, 38, 39, 40, 41, 42, 
		43, 44, 45, 46, 47, 48, 49, 
		50, 51, 52, 53, 54, 55, 56, 
		57, 58, 59, 60, 61, 62, 63};
#endif

static unsigned long chipsize = (256 << 20);

#ifdef NAND_16BIT
static int bus_width = 16;
#else
static int bus_width = 8;
#endif

/* NanD_Command: Send a flash command to the flash chip */
static int NanD_Command(unsigned char command)
{
 	NAND_CTL_SETCLE(NAND_ADDR);

 	WRITE_NAND_COMMAND(command, NAND_ADDR);
 	NAND_CTL_CLRCLE(NAND_ADDR);

  	if(command == NAND_CMD_RESET){
		unsigned char ret_val;
		NanD_Command(NAND_CMD_STATUS);
		do{
			ret_val = READ_NAND(NAND_ADDR);/* wait till ready */
  		} while((ret_val & 0x40) != 0x40);
 	}

	NAND_WAIT_READY();
	return 0;
}


/* NanD_Address: Set the current address for the flash chip */
static int NanD_Address(unsigned int numbytes, unsigned long ofs)
{
	uchar u;

 	NAND_CTL_SETALE(NAND_ADDR);

	if (numbytes == ADDR_COLUMN || numbytes == ADDR_COLUMN_PAGE
				|| numbytes == ADDR_OOB)
	{
		ushort col = ofs;

		u = col  & 0xff;
		WRITE_NAND_ADDRESS(u, NAND_ADDR);

		u = (col >> 8) & 0x07;
		if (numbytes == ADDR_OOB)
			u = u | ((bus_width == 16) ? (1 << 2) : (1 << 3));
		WRITE_NAND_ADDRESS(u, NAND_ADDR);
	}

	if (numbytes == ADDR_PAGE || numbytes == ADDR_COLUMN_PAGE
				|| numbytes == ADDR_OOB)
	{
		u = (ofs >> 11) & 0xff;
		WRITE_NAND_ADDRESS(u, NAND_ADDR);
		u = (ofs >> 19) & 0xff;
		WRITE_NAND_ADDRESS(u, NAND_ADDR);

		/* One more address cycle for devices > 128MiB */
		if (chipsize > (128 << 20)) {
			u = (ofs >> 27) & 0xff;
			WRITE_NAND_ADDRESS(u, NAND_ADDR);
		}
	}

 	NAND_CTL_CLRALE(NAND_ADDR);

 	NAND_WAIT_READY();
	return 0;
}

/* read chip mfr and id
 * return 0 if they match board config
 * return 1 if not
 */
int nand_chip()
{
	const int supported = 0;
	int mfr, id;

 	NAND_ENABLE_CE();

 	if (NanD_Command(NAND_CMD_RESET)) {
 		printf("Err: RESET\n");
 		NAND_DISABLE_CE();
		return (!supported);
	}

 	if (NanD_Command(NAND_CMD_READID)) {
 		printf("Err: READID\n");
 		NAND_DISABLE_CE();
		return (!supported);
 	}

 	NanD_Address(ADDR_COLUMN, 0);

 	mfr = READ_NAND(NAND_ADDR);
	id = READ_NAND(NAND_ADDR);

	debug("NAND: %x:%x\n", mfr, id);

	NAND_DISABLE_CE();

	switch (mfr) {

	case K9F1G08R0A_MFR:
		switch (id) {

		case K9F1G08R0A_ID:		return (supported);
		default:				return (!supported);

		}
		break;

	case MT29F1G_MFR:
		switch (id) {

		case MT29F1G_ID:		
		case MT29F2G_ID:		return (supported);
		default:				return (!supported);

		}
		break;

	case HYNIX4GiB_MFR:
		switch (id) {

		case HYNIX4GiB_ID:		return (supported);
		default:				return (!supported);

		}
		break;

	default:					return (!supported);

	}

	return (!supported);
}

/* read a block data to buf
 * return 1 if the block is bad or ECC error can't be corrected for any page
 * return 0 on sucess
 */
int nand_read_block(unsigned char *buf, ulong block_addr)
{
	int i, offset = 0;

#ifdef ECC_CHECK_ENABLE
	u16 oob_buf[OOB_SIZE >> 1];

	/* check bad block */
	/* 0th word in spare area needs be 0xff */
	if (nand_read_oob((unsigned char *)oob_buf, block_addr)
		|| (oob_buf[0] & 0xff) != 0xff) {
		printf("Skipped bad block at 0x%x\n", block_addr);
		return NAND_READ_SKIPPED_BAD_BLOCK;    /* skip bad block */
	}
#endif
	/* read the block page by page*/
	for (i=0; i<MAX_NUM_PAGES; i++){
		if (nand_read_page(buf+offset, block_addr + offset))
			return NAND_READ_ECC_FAILURE;
		offset += PAGE_SIZE;
	}

	return NAND_READ_SUCCESS;
}
static int count;
/* read a page with ECC */
static int nand_read_page(u_char *buf, ulong page_addr)
{
#ifdef ECC_CHECK_ENABLE
	/* increased size of ecc_code and ecc_calc to match the OOB size, 
	   as is done in the kernel */
	u_char ecc_code[OOB_SIZE];
	u_char ecc_calc[OOB_SIZE];
	u_char oob_buf[OOB_SIZE];
#endif
	u16 val;
	int cntr;
	int len;

#ifdef NAND_16BIT
	u16 *p;
#else
	u_char *p;
#endif

	NAND_ENABLE_CE();
	NanD_Command(NAND_CMD_READ0);
	NanD_Address(ADDR_COLUMN_PAGE, page_addr);
	NanD_Command(NAND_CMD_READSTART);
	NAND_WAIT_READY();

	/* A delay seems to be helping here. needs more investigation */
	delay(10000);

	omap_enable_hwecc(bus_width, 0);

	/* read the page */
	len = (bus_width == 16) ? PAGE_SIZE >> 1 : PAGE_SIZE;
	p = (u16 *)buf;
	for (cntr = 0; cntr < len; cntr++){
		*p++ = READ_NAND(NAND_ADDR);
		delay(10);
   	}

#ifdef ECC_CHECK_ENABLE
	omap_calculate_ecc(buf, &ecc_calc[0]);

	/* read the OOB area */
	p = (u16 *)oob_buf;
        len = (bus_width == 16) ? OOB_SIZE >> 1 : OOB_SIZE;
	for (cntr = 0; cntr < len; cntr++){
		*p++ = READ_NAND(NAND_ADDR);
		delay(10);
 	}
	count = 0;
 	NAND_DISABLE_CE();  /* set pin high */

	/* Need to enable HWECC for READING */

 	/* Pick the ECC bytes out of the oob data */
	for (cntr = 0; cntr < ECC_SIZE; cntr++)
		ecc_code[cntr] =  oob_buf[ecc_pos[cntr]];

	for(count = 0; count < ECC_SIZE; count += ECC_STEPS) {
#if CFG_NAND_ECC_1BIT
 		nand_calculate_ecc (buf, &ecc_calc[0]);
		if (nand_correct_data (buf, &ecc_code[count], &ecc_calc[0]) == -1) {
#else
		if (omap_correct_data(buf, &ecc_code[count], &ecc_calc[0]) == -1) {
#endif
 			printf ("ECC Failed, page 0x%08x\n", page_addr);
			for (val=0; val <256; val++)
				printf("%x ", buf[val]);
			printf("\n");
  			return 1;
 		}
		buf += 256;
		page_addr += 256;
	}
#endif
	return 0;
}

/* read from the 16 bytes of oob data that correspond to a 512 / 2048 byte page.
 */
static int nand_read_oob(u_char *buf, ulong page_addr)
{
	int cntr;
	int len;

#ifdef NAND_16BIT
	u16 *p;
#else
	u_char *p;
#endif
	p = (u16 *)buf;
        len = (bus_width == 16) ? OOB_SIZE >> 1 : OOB_SIZE;

  	NAND_ENABLE_CE();  /* set pin low */
	NanD_Command(NAND_CMD_READ0);
 	NanD_Address(ADDR_OOB, page_addr);
	NanD_Command(NAND_CMD_READSTART);
	NAND_WAIT_READY();

	/* A delay seems to be helping here. needs more investigation */
	delay(10000);
	for (cntr = 0; cntr < len; cntr++)
		*p++ = READ_NAND(NAND_ADDR);

	NAND_WAIT_READY();
	NAND_DISABLE_CE();  /* set pin high */

	return 0;
}
#endif /* defined(CFG_NAND_K9F1G08R0A) ||
		* defined(CFG_NAND_K9F4G08U0C) ||
		* defined(CFG_NAND_MT29F2G16)
		*/
