/*
 * (C) Copyright 2004
 * Texas Instruments
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#undef	_LINUX_CONFIG_H
#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char	vu_char;

#include <config.h>
#include <linux/types.h>
#include <stdarg.h>

#ifdef CONFIG_ARM
#define asmlinkage	/* nothing */
#endif

#ifdef CONFIG_ARM
# include <asm/setup.h>
# include <asm/x-load-arm.h>	/* ARM version to be fixed! */
#endif /* CONFIG_ARM */

#ifdef	CFG_PRINTF
#define printf(fmt,args...)	serial_printf (fmt ,##args)
#define getc() serial_getc()
#else
#define printf(fmt,args...)
#define getc() ' '
#endif	/* CFG_PRINTF */

#define DUMP_REG(tag, reg, type, op, format) do {			\
        register u32 _regvar = reg;							\
		register type _datavar = op(_regvar);				\
        printf("    %-30s @ 0x%08x = " #format "\n",		\
			   tag, _regvar, _datavar);						\
} while (0)

#define DUMP_REGB(mnemonic)									\
	DUMP_REG(#mnemonic, mnemonic, u8, __raw_readb, 0x%02x)

#define DUMP_REGW(mnemonic)									\
	DUMP_REG(#mnemonic, mnemonic, u16, __raw_readw, 0x%04x)

#define DUMP_REGL(mnemonic)									\
	DUMP_REG(#mnemonic, mnemonic, u32, __raw_readl, 0x%08x)

#ifdef	DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#define debugX(level,fmt,args...) if (DEBUG>=level) printf(fmt,##args);
#else
#define debug(fmt,args...)
#define debugX(level,fmt,args...)
#endif	/* DEBUG */

#define error(fmt, args...) do {					\
		printf("ERROR: " fmt "\nat %s:%d/%s()\n",		\
			##args, __FILE__, __LINE__, __func__);		\
} while (0)

#ifndef BUG
#define BUG() do { \
	printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
	panic("BUG!"); \
} while (0)
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif /* BUG */

/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof (X) __x = (X), __y = (Y);	\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof (X) __x = (X), __y = (Y);	\
		(__x > __y) ? __x : __y; })

#define MIN(x, y)  min(x, y)
#define MAX(x, y)  max(x, y)


/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

/* */
int	display_options (void);
void	print_size(unsigned long long, const char *);
int	print_buffer (ulong addr, void* data, uint width, uint count, uint linelen);

/* board/$(BOARD)/$(BOARD).c */
int 	board_init (void);
int 	nand_init (void);
int     mmc_boot (unsigned char *buf);
void	board_hang (void);
void    do_usb(void);


/* cpu/$(CPU)/cpu.c */
int 	cpu_init (void);
#ifdef  CFG_UDELAY
void 	udelay (unsigned long usec);
#endif

/* nand driver */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_READID		0x90
#define NAND_CMD_RESET		0xff

/* Extended Commands for Large page devices */
#define NAND_CMD_READSTART	0x30

/* Return codes for nand reads */
#define NAND_READ_SUCCESS              0
#define NAND_READ_SKIPPED_BAD_BLOCK    1
#define NAND_READ_ECC_FAILURE          2

int 	nand_chip(void);
int 	nand_read_block(uchar *buf, ulong block_addr);

int 	onenand_chip(void);
int	onenand_read_block(unsigned char *buf, ulong block);


#ifdef CFG_PRINTF

/* serial driver */
int	serial_init   (void);
void	serial_setbrg (void);
void	serial_putc   (const char);
void	serial_puts   (const char *);
int	serial_getc   (void);
int	serial_tstc   (void);

/* lib/printf.c */
void	serial_printf (const char *fmt, ...);
#endif

/* lib/crc.c */
void 	nand_calculate_ecc (const u_char *dat, u_char *ecc_code);

/* lib/vsprintf.c */

int 	nand_correct_data (u_char *dat, u_char *read_ecc, u_char *calc_ecc);

/* lib/board.c */
void	hang		(void) __attribute__ ((noreturn));

/* stdout */
void	puts(const char *s);

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
void show_boot_progress(int val);

#endif /* __ASSEMBLY__ */

/* Put only stuff here that the assembler can digest */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define ROUND(a,b)		(((a) + (b)) & ~((b) - 1))
#define DIV_ROUND(n,d)		(((n) + ((d)/2)) / (d))
#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))
#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))

#define ALIGN(x,a)		__ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))

#endif	/* __COMMON_H_ */
