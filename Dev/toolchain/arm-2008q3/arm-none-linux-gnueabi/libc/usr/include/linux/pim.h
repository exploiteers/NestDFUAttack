#ifndef __LINUX_PIM_H
#define __LINUX_PIM_H

#include <asm/byteorder.h>

struct pim {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	pim_type:4,		/* PIM message type */
		pim_ver:4;		/* PIM version */
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	pim_ver:4;		/* PIM version */
		pim_type:4;		/* PIM message type */
#endif
	__u8	pim_rsv;		/* Reserved */
	__be16	pim_cksum;		/* Checksum */
};

#define PIM_MINLEN		8

/* Message types - V1 */
#define PIM_V1_VERSION		__constant_htonl(0x10000000)
#define PIM_V1_REGISTER		1

/* Message types - V2 */
#define PIM_VERSION		2
#define PIM_REGISTER		1

#endif
