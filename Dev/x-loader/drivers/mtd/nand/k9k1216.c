/*
 * (C) Copyright 2004 Texas Instruments
 * Jian Zhang <jzhang@ti.com>
 *
 *  Samsung K9K1216Q0C NAND chip driver for an OMAP2420 board
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

#ifdef CFG_NAND_K9K1216

#define K9K1216_MFR		0xec
#define K9K1216_ID		0x46

#define ADDR_COLUMN		1          
#define ADDR_PAGE		2             
#define ADDR_COLUMN_PAGE	3

#define PAGE_SIZE		512
/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
					  "subs %0, %1, #1\n"
					  "bne 1b":"=r" (loops):"0" (loops));
}

static int nand_read_page(u_char *buf, ulong page_addr);
static int nand_read_oob(u_char * buf, ulong page_addr);

/* JFFS2 512-byte-page ECC layout */
static u_char ecc_pos[] = {0,1,2,3,6,7};
static u_char eccvalid_pos = 4;

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
static int NanD_Address(int numbytes, unsigned long ofs)
{
 	int i;

 	NAND_CTL_SETALE(NAND_ADDR);
 
	if (numbytes == ADDR_COLUMN || numbytes == ADDR_COLUMN_PAGE)
		WRITE_NAND_ADDRESS(ofs, NAND_ADDR);

	ofs = ofs >> 9;

	if (numbytes == ADDR_PAGE || numbytes == ADDR_COLUMN_PAGE)
		for (i = 0; i < 3; i++, ofs = ofs >> 8)
			WRITE_NAND_ADDRESS(ofs, NAND_ADDR);

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
	int mfr, id;

 	NAND_ENABLE_CE();

 	if (NanD_Command(NAND_CMD_RESET)) {
 		printf("Err: RESET\n");
 		NAND_DISABLE_CE();   
		return 1;
	}
 
 	if (NanD_Command(NAND_CMD_READID)) {
 		printf("Err: READID\n");
 		NAND_DISABLE_CE();
		return 1;
 	}
 
 	NanD_Address(ADDR_COLUMN, 0);

 	mfr = READ_NAND(NAND_ADDR);
	id = READ_NAND(NAND_ADDR);

	NAND_DISABLE_CE();

  	return (mfr != K9K1216_MFR || id != K9K1216_ID);
}

/* read a block data to buf
 * return 1 if the block is bad or ECC error can't be corrected for any page
 * return 0 on sucess
 */ 
int nand_read_block(unsigned char *buf, ulong block_addr)
{
	int i, offset = 0;
	uchar oob_buf[16];
 	
	/* check bad block */
	/* 0th and 5th words need be 0xffff */
	if (nand_read_oob(oob_buf, block_addr) ||
//		oob_buf[0] != 0xff || oob_buf[1] != 0xff ||
//		oob_buf[10] != 0xff || oob_buf[11] != 0xff ){
		oob_buf[5] != 0xff){
		printf("Skipped bad block at 0x%x\n", block_addr);
 		return NAND_READ_SKIPPED_BAD_BLOCK;    /* skip bad block */
	}

	/* read the block page by page*/
	for (i=0; i<32; i++){
		if (nand_read_page(buf+offset, block_addr + offset))
			return NAND_READ_ECC_FAILURE;
		offset += PAGE_SIZE;
	}

	return NAND_READ_SUCCESS;
}
static count = 0;
/* read a page with ECC */
static int nand_read_page(u_char *buf, ulong page_addr)
{
 	u_char ecc_code[6];
	u_char ecc_calc[3];
 	u_char oob_buf[16];
	u_char *p;
	u16 val;
	int cntr;

	NAND_ENABLE_CE();   
	NanD_Command(NAND_CMD_READ0);
	NanD_Address(ADDR_COLUMN_PAGE, page_addr);
	NAND_WAIT_READY();

	delay(500); /* we go through NFC need a bigger delay. 200 is not enough */
 
  	p = buf;
	for (cntr = 0; cntr < 256; cntr++){
		val = READ_NAND(NAND_ADDR);
 		*p++ = val & 0xff;
 		*p++ = val >> 8;
   	}

	p = oob_buf;
	for (cntr = 0; cntr < 8; cntr++){
 		val = READ_NAND(NAND_ADDR);
		*p++ = val & 0xff;
		*p++ = val >> 8;
 	}
	count = 1;
 	NAND_DISABLE_CE();  /* set pin high */

 	/* Pick the ECC bytes out of the oob data */
	for (cntr = 0; cntr < 6; cntr++)
		ecc_code[cntr] =  oob_buf[ecc_pos[cntr]];

	if ((oob_buf[eccvalid_pos] & 0x0f) != 0x0f) {
 		nand_calculate_ecc (buf, &ecc_calc[0]);
		if (nand_correct_data (buf, &ecc_code[0], &ecc_calc[0]) == -1) {
 			printf ("ECC Failed, page 0x%08x\n", page_addr);
			for (val=0; val <256; val++)
				printf("%x ", buf[val]);
			printf("\n");
  			return 1;
 		}
	}
	
	if ((oob_buf[eccvalid_pos] & 0xf0) != 0xf0) {
 		nand_calculate_ecc (buf + 256, &ecc_calc[0]);
		if (nand_correct_data (buf + 256, &ecc_code[3], &ecc_calc[0]) == -1) {
 			printf ("ECC Failed, page 0x%08x\n", page_addr+0x100);
			for (val=0; val <256; val++)
				printf("%x ", buf[val+256]);
			printf("\n");
  			return 1;
 		}
	}

	return 0;
}

/* read from the 16 bytes of oob data that correspond to a 512 byte page.
 */
static int nand_read_oob(u_char *buf, ulong page_addr)
{
	u16 val;
	int cntr;
    	
  	NAND_ENABLE_CE();  /* set pin low */
	NanD_Command(NAND_CMD_READOOB);
 	NanD_Address(ADDR_COLUMN_PAGE, page_addr);
	NAND_WAIT_READY();

/*	do {
		*(volatile u32 *)(0x6800A07c) = NAND_CMD_STATUS;
	  val = *(volatile u32 *)(0x6800A084);
	  printf("val = %x\n", val);	
	} while ((val & 0x40) != 0x40);
*/
	/* the above code from OSTBoot doesn't work, we use delay */
	delay(500); /* we go through NFC need a bigger delay. 200 is not enough */
 
	for (cntr = 0; cntr < 8; cntr++){
		val = READ_NAND(NAND_ADDR);
		*buf++ = val & 0xff;
		*buf++ = val >> 8;
  	}
  
	NAND_WAIT_READY();
	NAND_DISABLE_CE();  /* set pin high */

	return 0;
}

#endif
