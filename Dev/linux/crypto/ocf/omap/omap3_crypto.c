#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/dst.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
#error Kernel versions lower than 2.6.32 are not supported
#else
#include <plat/hardware.h>
#include <plat/dma.h>
#include <plat/tc.h>
#endif

#include "omap3_crypto.h"

/* ========================================================================= */
/*                               DEBUG                                       */
/* ========================================================================= */

static int omap3_crypto_debug = 0;

#define dprintk(a...)   if (omap3_crypto_debug) printk(a);

#define KASSERT(c,p)    if (!(c)) { printk p ; } else
#define __HEXDUMP(str, ptr, len) // \
{ \
	int __i; \
	dprintk( str ); \
	dprintk( "\n" ); \
	for (__i=0;__i<len;__i++) { \
		dprintk("%02x%s", (((uint8_t*)ptr)[__i]), (((__i&0xF) == 0xF)?"\n":"")); \
	} \
	dprintk( "\n" ); \
}

/* ========================================================================= */
/*                         ENUMERATION FOR H/W INSTANCES                     */
/* ========================================================================= */

enum {
	OMAP3_CRYPTO_DES1_IDX,
	OMAP3_CRYPTO_DES2_IDX,
	OMAP3_CRYPTO_SHA2MD5_IDX,
	OMAP3_CRYPTO_SHA1MD5_IDX,
	OMAP3_CRYPTO_AES1_IDX,
	OMAP3_CRYPTO_AES2_IDX,
	OMAP3_CRYPTO_MAX_IDX
};

/* ========================================================================= */
/*                         REGISTER STRUCTURES                               */
/* ========================================================================= */

typedef struct {
	uint32_t REVISION;
	uint32_t pad0;
	uint32_t IRQSTATUS_L[4];
	uint32_t IRQENABLE_L[4];
	uint32_t SYSSTATUS;
	uint32_t OCP_SYSCONFIG;
	uint32_t pad1[13];
	uint32_t CAPS_0;
	uint32_t CAPS_1;
	uint32_t CAPS_2;
	uint32_t CAPS_3;
	uint32_t CAPS_4;
	uint32_t GCR;
	uint32_t pad2;
	struct {
		uint32_t CCR;
		uint32_t CLNK_CTRL;
		uint32_t CICR;
		uint32_t CSR;
		uint32_t CSDP;
		uint32_t CEN;
		uint32_t CFN;
		uint32_t CSSA;
		uint32_t CDSA;
		uint32_t CSEI;
		int32_t CSFI;
		uint32_t CDEI;
		int32_t CDFI;
		uint32_t CSAC;
		uint32_t CDAC;
		uint32_t CCEN;
		uint32_t CCFN;
		uint32_t COLOR;
		uint32_t CDP;
		uint32_t CNDP;
		uint32_t CCDN;
		uint32_t pad3[3];
	} chan[32];
} omap3_sdma_regs;

typedef struct {
	uint8_t pad[0xA10];
	uint32_t CM_ICLKEN1_CORE;
	uint32_t CM_ICLKEN2_CORE;
} omap3_cm_regs;

typedef struct {
	uint32_t KEY4_L;
	uint32_t KEY4_H;
	uint32_t KEY3_L;
	uint32_t KEY3_H;
	uint32_t KEY2_L;
	uint32_t KEY2_H;
	uint32_t KEY1_L;
	uint32_t KEY1_H;
	uint32_t IV_1;
	uint32_t IV_2;
	uint32_t IV_3;
	uint32_t IV_4;
	uint32_t CTRL;
	uint32_t DATA_1;
	uint32_t DATA_2;
	uint32_t DATA_3;
	uint32_t DATA_4;
	uint32_t REV;
	uint32_t MASK;
	uint32_t SYSSTATUS;
} omap3_aes_regs;

/* AES_CTRL related definitions */
#define AES_CTRL_INPUT_READY_BIT    0x2
#define AES_CTRL_OUTPUT_READY_BIT   0x1
/*AES SYS STATUS related definitions*/
#define AES_SYS_STATUS_RESET_DONE_BIT   0x1

typedef struct {
    uint32_t DIGEST_A;
    uint32_t DIGEST_B;
    uint32_t DIGEST_C;
    uint32_t DIGEST_D;
    uint32_t DIGEST_E;
    uint32_t DIGCNT;
    uint32_t CTRL;
    uint32_t DIN_0;
    uint32_t DIN_1;
    uint32_t DIN_2;
    uint32_t DIN_3;
    uint32_t DIN_4;
    uint32_t DIN_5;
    uint32_t DIN_6;
    uint32_t DIN_7;
    uint32_t DIN_8;
    uint32_t DIN_9;
    uint32_t DIN_10;
    uint32_t DIN_11;
    uint32_t DIN_12;
    uint32_t DIN_13;
    uint32_t DIN_14;
    uint32_t DIN_15;
    uint32_t REV;
    uint32_t MASK;
    uint32_t SYSSTATUS;
} omap3_sha1md5_regs;

#define SHA1MD5_CTRL_INPUT_READY_BIT    0x2
#define SHA1MD5_CTRL_OUTPUT_READY_BIT   0x1
#define SHA1MD5_SYS_STATUS_RESET_DONE_BIT   0x1

typedef struct {
    uint32_t DIGEST_A;
    uint32_t DIGEST_B;
    uint32_t DIGEST_C;
    uint32_t DIGEST_D;
    uint32_t DIGEST_E;
    uint32_t DIGEST_F;
    uint32_t DIGEST_G;
    uint32_t DIGEST_H;
    uint32_t DIGCNT;
    uint32_t BYTE;
    uint32_t IRQSTAT;
    uint32_t CTRL;
    uint32_t DIN_0;
    uint32_t DIN_1;
    uint32_t DIN_2;
    uint32_t DIN_3;
    uint32_t DIN_4;
    uint32_t DIN_5;
    uint32_t DIN_6;
    uint32_t DIN_7;
    uint32_t DIN_8;
    uint32_t DIN_9;
    uint32_t DIN_10;
    uint32_t DIN_11;
    uint32_t DIN_12;
    uint32_t DIN_13;
    uint32_t DIN_14;
    uint32_t DIN_15;
    uint32_t REV;
    uint32_t MASK;
    uint32_t SYSSTATUS;
} omap3_sha2md5_regs;

#define SHA2MD5_SYS_STATUS_RESET_DONE_BIT   0x1
#define SHA2MD5_IRQSTAT_INPUT_READY_BIT     0x2
#define SHA2MD5_IRQSTAT_OUTPUT_READY_BIT     0x1

typedef struct {
	uint32_t KEY3_L;
	uint32_t KEY3_H;
	uint32_t KEY2_L;
	uint32_t KEY2_H;
	uint32_t KEY1_L;
	uint32_t KEY1_H;
	uint32_t IV_L;
	uint32_t IV_H;
	uint32_t CTRL;
	uint32_t DATA_L;
	uint32_t DATA_H;
	uint32_t REV;
	uint32_t MASK;
	uint32_t SYSSTATUS;
} omap3_des_regs;

/* DES_CTRL related definitions */
#define DES_CTRL_INPUT_READY_BIT    0x2
#define DES_CTRL_OUTPUT_READY_BIT   0x1
/*DES SYS STATUS related definitions*/
#define DES_SYS_STATUS_RESET_DONE_BIT   0x1

/* ========================================================================= */
/*                         BASE ADDRESSES FOR MMAP                           */
/* ========================================================================= */

#define OMAP3_CRYPTO_MMAPS_SIZE 0x1000
#define OMAP3_CRYPTO_NUM_MMAPS 8
static const uint32_t omap3_crypto_base_addresses[OMAP3_CRYPTO_NUM_MMAPS] = {
	0x480A2000, // DES_1
	0x480C1000, // DES_2
	0x480A4000, // SHA_MD5_1
	0x480C3000, // SHA_MD5_2
	0x480A6000, // AES_1
	0x480C5000, // AES_2
	0x48004000, // Control module
	0x48056000, // SDMA module
};
static void *omap3_crypto_reg_ptrs[OMAP3_CRYPTO_NUM_MMAPS] = {0};
#define SDMA_REGS    ((volatile omap3_sdma_regs *)(omap3_crypto_reg_ptrs[7]))
#define CM_REGS      ((volatile omap3_cm_regs *)(omap3_crypto_reg_ptrs[6]))
#define AES_REGS(x)  ((volatile omap3_aes_regs *)(omap3_crypto_reg_ptrs[4+x]))
#define SHA2MD5_REGS ((volatile omap3_sha2md5_regs *)(omap3_crypto_reg_ptrs[2]))
#define SHA1MD5_REGS ((volatile omap3_sha1md5_regs *)(omap3_crypto_reg_ptrs[3]))
#define DES_REGS(x)  ((volatile omap3_des_regs *)(omap3_crypto_reg_ptrs[x]))

/* ========================================================================= */
/*                         SCRATCH MEMORY MANAGEMENT                         */
/* ========================================================================= */

/* TODO : make this as module arguments */
static const uint32_t omap3_crypto_scratch_mem_len = (64*1024);
static const uint32_t omap3_crypto_scratch_mem_address = 0x40200000;

static uint8_t *omap3_crypto_scratch_mem_base_ptr;
typedef struct omap3_crypto_scratch_chunk {
    struct omap3_crypto_scratch_chunk *next;
	int len;
} omap3_crypto_scratch_chunk;

static omap3_crypto_scratch_chunk * omap3_crypto_scratch_free_head;

DEFINE_SPINLOCK(omap3_crypto_scratch_spinlock);

static int omap3_crypto_scratch_init(void)
{
	omap3_crypto_scratch_mem_base_ptr = (uint8_t*)ioremap_nocache(
			omap3_crypto_scratch_mem_address, omap3_crypto_scratch_mem_len);
	if (NULL == omap3_crypto_scratch_mem_base_ptr) return -EINVAL;

	omap3_crypto_scratch_free_head = 
        (omap3_crypto_scratch_chunk*)omap3_crypto_scratch_mem_base_ptr;

    omap3_crypto_scratch_free_head->next = NULL;
    omap3_crypto_scratch_free_head->len = omap3_crypto_scratch_mem_len;

	return 0;
}

static void omap3_crypto_scratch_deinit(void)
{
	iounmap(omap3_crypto_scratch_mem_base_ptr);
}

void *omap3_crypto_scratch_alloc(int len)
{
    void *ret;
    int more;
    unsigned long flags;
    omap3_crypto_scratch_chunk *cur, *prev, *new;

	len = ((len+15)&(~15));
	spin_lock_irqsave(&omap3_crypto_scratch_spinlock, flags);
    
    if (NULL == omap3_crypto_scratch_free_head) { ret = NULL; }
    else {
        cur = omap3_crypto_scratch_free_head;
        prev = NULL;
        while (cur) {
            if (cur->len >= len) break;
            prev = cur;
            cur = cur->next;
        }
        ret = (void*)cur;
        more = cur->len - len;
        if (more) {
            new = (omap3_crypto_scratch_chunk*)(((uint8_t*)cur)+len);
            new->len = more;
            new->next = cur->next;
        } else new = cur->next;
        if (prev) prev->next = new;
        else omap3_crypto_scratch_free_head = new;
    }
	spin_unlock_irqrestore(&omap3_crypto_scratch_spinlock, flags);
	return ret;
}

void omap3_crypto_scratch_free(void *ptr, int len)
{
    omap3_crypto_scratch_chunk *cur, *prev, *new;
    unsigned long flags;

	len = ((len+15)&(~15));

	spin_lock_irqsave(&omap3_crypto_scratch_spinlock, flags);

    prev = omap3_crypto_scratch_free_head;
    new = (omap3_crypto_scratch_chunk*)ptr;
    new->len = len;
    if (prev == NULL || ((uint32_t)prev > (uint32_t)new)) {
        new->next = prev;
        omap3_crypto_scratch_free_head = new;
        prev = new;
    } else {
        cur = prev->next;
        while (cur && ((uint32_t)cur < (uint32_t)new)) {
            prev = cur;
            cur = cur->next;
        }
        prev->next = new;
        new->next = cur;
        /* Merge if contiguous */
        if (((uint32_t)prev + prev->len) == (uint32_t)new) {
            prev->next = new->next;
            prev->len += new->len;
        }
    }
    cur = prev->next;
    /* Merge if contiguous */
    if (cur && (((uint32_t)prev + prev->len) == (uint32_t)cur)) {
        prev->next = cur->next;
        prev->len += cur->len;
    }

	spin_unlock_irqrestore(&omap3_crypto_scratch_spinlock, flags);
}

uint32_t omap3_crypto_scratch_get_phys(uint8_t *ptr)
{
	uint32_t ret;

	ret = ((uint32_t)ptr) - ((uint32_t)omap3_crypto_scratch_mem_base_ptr);

	if (ret > omap3_crypto_scratch_mem_len) return 0;

	return (omap3_crypto_scratch_mem_address + ret);
}


/* ========================================================================= */
/*                         SDMA MANAGEMENT                                   */
/* ========================================================================= */

static const uint32_t omap3_crypto_sdma_rx_num[OMAP3_CRYPTO_MAX_IDX] = {
	11 + 1,  // OMAP3_CRYPTO_DES1_IDX
	67 + 1, // OMAP3_CRYPTO_DES2_IDX
	0,  // OMAP3_CRYPTO_SHA2MD5_IDX
	0, // OMAP3_CRYPTO_SHA1MD5_IDX
	9 + 1,  // OMAP3_CRYPTO_AES1_IDX
	65 + 1  // OMAP3_CRYPTO_AES2_IDX
};
static const uint32_t omap3_crypto_sdma_tx_num[OMAP3_CRYPTO_MAX_IDX] = {
	10 + 1,  // OMAP3_CRYPTO_DES1_IDX
	66 + 1, // OMAP3_CRYPTO_DES2_IDX
	12 + 1,  // OMAP3_CRYPTO_SHA2MD5_IDX
	68 + 1, // OMAP3_CRYPTO_SHA1MD5_IDX
	8 + 1,  // OMAP3_CRYPTO_AES1_IDX
	64 + 1  // OMAP3_CRYPTO_AES2_IDX
};

static int omap3_crypto_sdma_channel_rx[OMAP3_CRYPTO_MAX_IDX];
static int omap3_crypto_sdma_channel_tx[OMAP3_CRYPTO_MAX_IDX];

#define OMAP3_CRYPTO_USE_TASKLETS 1

#ifdef OMAP3_CRYPTO_USE_TASKLETS
struct tasklet_struct omap3_crypto_sdma_tasklets[OMAP3_CRYPTO_MAX_IDX] = {
    { NULL, 0, ATOMIC_INIT(0), NULL, 0xFFFFFFFF },
    { NULL, 0, ATOMIC_INIT(0), NULL, 0xFFFFFFFF },
    { NULL, 0, ATOMIC_INIT(0), NULL, 0xFFFFFFFF },
    { NULL, 0, ATOMIC_INIT(0), NULL, 0xFFFFFFFF },
    { NULL, 0, ATOMIC_INIT(0), NULL, 0xFFFFFFFF },
    { NULL, 0, ATOMIC_INIT(0), NULL, 0xFFFFFFFF },
};
#define omap3_crypto_sdma_cbs(idx) omap3_crypto_sdma_tasklets[idx].func
#define omap3_crypto_sdma_cb_data(idx) omap3_crypto_sdma_tasklets[idx].data
#else
static omap3_crypto_dma_callback _omap3_crypto_sdma_cbs[OMAP3_CRYPTO_MAX_IDX];
static unsigned long _omap3_crypto_sdma_cb_data[OMAP3_CRYPTO_MAX_IDX];
#define omap3_crypto_sdma_cbs(idx) _omap3_crypto_sdma_cbs[idx]
#define omap3_crypto_sdma_cb_data(idx) _omap3_crypto_sdma_cb_data[idx]
#endif

static void omap3_crypto_dma_tx_cb(int lch, u16 ch_status, void *data)
{
	int idx = (int)data;
#ifndef OMAP3_CRYPTO_USE_TASKLETS
	omap3_crypto_dma_callback cb = omap3_crypto_sdma_cbs(idx);
	omap3_crypto_sdma_cbs(idx) = NULL;
	if (NULL == cb) return;
#endif
 
	// Clear the DMA start bit
	switch (idx) {
		case OMAP3_CRYPTO_DES1_IDX:
			DES_REGS(0)->MASK = DES_REGS(0)->MASK & (~0x20);
			break;
		case OMAP3_CRYPTO_DES2_IDX:
			DES_REGS(1)->MASK = DES_REGS(1)->MASK & (~0x20);
			break;
		case OMAP3_CRYPTO_SHA2MD5_IDX:
            break;
		case OMAP3_CRYPTO_SHA1MD5_IDX:
	        SHA1MD5_REGS->MASK = SHA1MD5_REGS->MASK & (~(1 << 3));
			break;
		case OMAP3_CRYPTO_AES1_IDX:
			AES_REGS(0)->MASK = AES_REGS(0)->MASK & (~0x20);
			break;
		case OMAP3_CRYPTO_AES2_IDX:
			AES_REGS(1)->MASK = AES_REGS(1)->MASK & (~0x20);
			break;
	}
#ifndef OMAP3_CRYPTO_USE_TASKLETS
	cb(omap3_crypto_sdma_cb_data(idx));
#else
    tasklet_schedule(&omap3_crypto_sdma_tasklets[idx]);
#endif
}

static void omap3_crypto_dma_init(void)
{
	int i;
	for (i=0; i<OMAP3_CRYPTO_MAX_IDX; i++) {
		omap3_crypto_sdma_channel_rx[i] = -1;
		omap3_crypto_sdma_channel_tx[i] = -1;
	}
}

static int omap3_crypto_dma_alloc(int idx)
{
	if (omap3_crypto_sdma_channel_rx[idx] >= 0) return 0;
	if (omap3_crypto_sdma_channel_tx[idx] >= 0) return 0;

    if (0 != omap3_crypto_sdma_rx_num[idx]) {
        if (0 != omap_request_dma(OMAP_DMA_NO_DEVICE, "Crypto Driver",
                    NULL, NULL, &omap3_crypto_sdma_channel_rx[idx] ))	{
            return -ENOMEM;
        }
    }
	if (0 != omap_request_dma(OMAP_DMA_NO_DEVICE, "Crypto Driver",
				omap3_crypto_dma_tx_cb, (void *)idx, &omap3_crypto_sdma_channel_tx[idx] ))	{
        if (omap3_crypto_sdma_channel_rx[idx] >= 0)
            omap_free_dma(omap3_crypto_sdma_channel_rx[idx]);
        omap3_crypto_sdma_channel_rx[idx] = -1;
		return -ENOMEM;
	}
	return 0;
}

static void omap3_crypto_dma_free(int idx)
{
	if (omap3_crypto_sdma_channel_rx[idx] >= 0)
	    omap_free_dma(omap3_crypto_sdma_channel_rx[idx]);
	if (omap3_crypto_sdma_channel_tx[idx] >= 0)
	    omap_free_dma(omap3_crypto_sdma_channel_tx[idx]);
	omap3_crypto_sdma_channel_rx[idx] = -1;
	omap3_crypto_sdma_channel_tx[idx] = -1;
}

/* ========================================================================= */
/*                            DES FUNCTIONS                                  */
/* ========================================================================= */
static void omap3_crypto_des_reset(int id)
{
	int des_inst = id - OMAP3_CRYPTO_DES1_IDX;
	// Software reset
	DES_REGS(des_inst)->MASK = 0x2;
	// Wait for reset to complete
	while (0 == (DES_REGS(des_inst)->SYSSTATUS & DES_SYS_STATUS_RESET_DONE_BIT));
}

void omap3_crypto_des_init(int id, uint8_t *key, int key_len, uint32_t mode)
{
	int des_inst = id - OMAP3_CRYPTO_DES1_IDX;
	uint32_t tmp_buf[6], *tmp;

	DES_REGS(des_inst)->MASK = 0xC;
	DES_REGS(des_inst)->CTRL &= 0x3;
	DES_REGS(des_inst)->CTRL |= (mode << 3);

	dprintk("keylen: %d, mode: %d\n", key_len, mode);

	// Load key
	if (((uint32_t)key)&3) {
		tmp = tmp_buf;
		memcpy(tmp, key, key_len);
	} else tmp = (uint32_t*)key;

	__HEXDUMP("Key", tmp, key_len);

	DES_REGS(des_inst)->KEY1_L = tmp[0];
	DES_REGS(des_inst)->KEY1_H = tmp[1];
	if (key_len >= 24) {
		DES_REGS(des_inst)->KEY2_L = tmp[2];
		DES_REGS(des_inst)->KEY2_H = tmp[3];
		DES_REGS(des_inst)->KEY3_L = tmp[4];
		DES_REGS(des_inst)->KEY3_H = tmp[5];
	}

};


void omap3_crypto_des_process(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv)
{
	int des_inst = id - OMAP3_CRYPTO_DES1_IDX;
	uint32_t tmp_buf[2], *tmp;

	KASSERT(((buf_len & 0x7) == 0), ("%s: buf_len [%d] is not a multiple of 8", __func__, buf_len));

	encryption <<= 2;
	if ((DES_REGS(des_inst)->CTRL & 0x4) != encryption) 
		DES_REGS(des_inst)->CTRL ^= 0x4;

	if (iv) {
		// Load iv
		if (((uint32_t)iv)&3) {
			tmp = tmp_buf;
			memcpy(tmp, iv, 8);
		} else tmp = (uint32_t*)iv;
		__HEXDUMP("IV", tmp, 8);

		DES_REGS(des_inst)->IV_L = tmp[0];
		DES_REGS(des_inst)->IV_H = tmp[1];
	}

	if (((uint32_t)buf)&3) {
		tmp = tmp_buf;
		for (;buf_len>0;buf_len-=8,buf+=8) {
			memcpy(tmp, buf, 8);
			__HEXDUMP("INPUT", tmp, 8);
			while (0 == (DES_REGS(des_inst)->CTRL & DES_CTRL_INPUT_READY_BIT));
			DES_REGS(des_inst)->DATA_L = tmp[0];
			DES_REGS(des_inst)->DATA_H = tmp[1];
			while (0 == (DES_REGS(des_inst)->CTRL & DES_CTRL_OUTPUT_READY_BIT));
			tmp[0] = DES_REGS(des_inst)->DATA_L;
			tmp[1] = DES_REGS(des_inst)->DATA_H;
			__HEXDUMP("OUTPUT", tmp, 8);
			memcpy(buf, tmp, 8);
		}
	} else {
		tmp = (uint32_t*)buf;
		for (;buf_len>0;buf_len-=8,tmp+=2) {
			__HEXDUMP("INPUT", tmp, 8);
			while (0 == (DES_REGS(des_inst)->CTRL & DES_CTRL_INPUT_READY_BIT));
			DES_REGS(des_inst)->DATA_L = tmp[0];
			DES_REGS(des_inst)->DATA_H = tmp[1];
			while (0 == (DES_REGS(des_inst)->CTRL & DES_CTRL_OUTPUT_READY_BIT));
			tmp[0] = DES_REGS(des_inst)->DATA_L;
			tmp[1] = DES_REGS(des_inst)->DATA_H;
			__HEXDUMP("OUTPUT", tmp, 8);
		}
	}
}

void omap3_crypto_des_process_dma(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv,
		omap3_crypto_dma_callback cb, unsigned long data)
{
	int des_inst = id - OMAP3_CRYPTO_DES1_IDX;
	uint32_t tmp_buf[2], *tmp, phys;
	int i;

	KASSERT(((buf_len & 0x7) == 0), ("%s: buf_len [%d] is not a multiple of 8", __func__, buf_len));

	phys = omap3_crypto_scratch_get_phys(buf);
	if (0 == phys) {
		omap3_crypto_des_process(id, encryption, buf, buf_len, iv);
		cb(data);
		return;
	}

	encryption <<= 2;
	if ((DES_REGS(des_inst)->CTRL & 0x4) != encryption) 
		DES_REGS(des_inst)->CTRL ^= 0x4;

	if (iv) {
		// Load iv
		if (((uint32_t)iv)&3) {
			tmp = tmp_buf;
			memcpy(tmp, iv, 8);
		} else tmp = (uint32_t*)iv;
		__HEXDUMP("IV", tmp, 8);

		DES_REGS(des_inst)->IV_L = tmp[0];
		DES_REGS(des_inst)->IV_H = tmp[1];
	}
	
	i = omap3_crypto_sdma_channel_rx[id];
	// setup SDMA for input and output
	SDMA_REGS->chan[i].CCR &= 0xFC030FC0;
	SDMA_REGS->chan[i].CCR |= ((1 << 25) | (1 << 5) | (3 << 12) | (3 << 14) | 
			((omap3_crypto_sdma_rx_num[id] & 0x60) << 14) | (omap3_crypto_sdma_rx_num[id] & 0x1F));
	SDMA_REGS->chan[i].CSDP = 2;
	SDMA_REGS->chan[i].CEN = 2;
	SDMA_REGS->chan[i].CFN = (buf_len >> 3);
	SDMA_REGS->chan[i].CSSA = phys;
	SDMA_REGS->chan[i].CSEI = 1;
	SDMA_REGS->chan[i].CSFI = 1;
	SDMA_REGS->chan[i].CDSA = omap3_crypto_base_addresses[id] + 0x24;
	SDMA_REGS->chan[i].CDEI = 1;
	SDMA_REGS->chan[i].CDFI = -7;
	SDMA_REGS->chan[i].CLNK_CTRL = 0;
	SDMA_REGS->chan[i].CCR |= (1 << 7);

	i = omap3_crypto_sdma_channel_tx[id];

	SDMA_REGS->chan[i].CCR &= 0xFC030FC0;
	SDMA_REGS->chan[i].CCR |= ((1 << 25) | (1 << 24) | (1 << 5) | (3 << 12) | (3 << 14) | 
			((omap3_crypto_sdma_tx_num[id] & 0x60) << 14) | (omap3_crypto_sdma_tx_num[id] & 0x1F));
	SDMA_REGS->chan[i].CSDP = 2;
	SDMA_REGS->chan[i].CEN = 2;
	SDMA_REGS->chan[i].CFN = (buf_len >> 3);
	SDMA_REGS->chan[i].CSSA = omap3_crypto_base_addresses[id] + 0x24;
	SDMA_REGS->chan[i].CSEI = 1;
	SDMA_REGS->chan[i].CSFI = -7;
	SDMA_REGS->chan[i].CDSA = phys;
	SDMA_REGS->chan[i].CDEI = 1;
	SDMA_REGS->chan[i].CDFI = 1;
	SDMA_REGS->chan[i].CLNK_CTRL = 0;
	SDMA_REGS->chan[i].CCR |= (1 << 7);

	omap3_crypto_sdma_cbs(id) = cb;
	omap3_crypto_sdma_cb_data(id) = data;

	// set the START bit in mask register
	DES_REGS(des_inst)->MASK = DES_REGS(des_inst)->MASK | 0x20;
}

/* ========================================================================= */
/*                            AES FUNCTIONS                                  */
/* ========================================================================= */
static void omap3_crypto_aes_reset(int id)
{
	int aes_inst = id - OMAP3_CRYPTO_AES1_IDX;

	// Software reset
	AES_REGS(aes_inst)->MASK = 0x2;
	// Wait for reset to complete
	while (0 == (AES_REGS(aes_inst)->SYSSTATUS & AES_SYS_STATUS_RESET_DONE_BIT));
}

void omap3_crypto_aes_init(int id, uint8_t *key, int key_len, uint32_t mode)
{
	int aes_inst = id - OMAP3_CRYPTO_AES1_IDX;
	uint32_t tmp_buf[8], *tmp;

	AES_REGS(aes_inst)->MASK = 0xC;
	AES_REGS(aes_inst)->CTRL &= 0x3;
	AES_REGS(aes_inst)->CTRL |= ((mode << 5) | ((((key_len>>3)-1)&3) << 3));

	dprintk("keylen: %d, mode: %d\n", key_len, mode);

	// Load key
	if (((uint32_t)key)&3) {
		tmp = tmp_buf;
		memcpy(tmp, key, key_len);
	} else tmp = (uint32_t*)key;

	__HEXDUMP("Key", tmp, key_len);

	AES_REGS(aes_inst)->KEY1_L = tmp[0];
	AES_REGS(aes_inst)->KEY1_H = tmp[1];
	AES_REGS(aes_inst)->KEY2_L = tmp[2];
	AES_REGS(aes_inst)->KEY2_H = tmp[3];
	if (key_len >= 24) {
		AES_REGS(aes_inst)->KEY3_L = tmp[4];
		AES_REGS(aes_inst)->KEY3_H = tmp[5];
	}
	if (key_len >= 32) {
		AES_REGS(aes_inst)->KEY4_L = tmp[6];
		AES_REGS(aes_inst)->KEY4_H = tmp[7];
	}
};


void omap3_crypto_aes_process(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv)
{
	int aes_inst = id - OMAP3_CRYPTO_AES1_IDX;
	uint32_t tmp_buf[4], *tmp;

	KASSERT(((buf_len & 0xF) == 0), ("%s: buf_len [%d] is not a multiple of 16", __func__, buf_len));       

	encryption <<= 2;
	if ((AES_REGS(aes_inst)->CTRL & 0x4) != encryption) 
		AES_REGS(aes_inst)->CTRL ^= 0x4;

	if (iv) {
		// Load iv
		if (((uint32_t)iv)&3) {
			tmp = tmp_buf;
			memcpy(tmp, iv, 16);
		} else tmp = (uint32_t*)iv;
		__HEXDUMP("IV", tmp, 16);

		AES_REGS(aes_inst)->IV_1 = tmp[0];
		AES_REGS(aes_inst)->IV_2 = tmp[1];
		AES_REGS(aes_inst)->IV_3 = tmp[2];
		AES_REGS(aes_inst)->IV_4 = tmp[3];
	}

	if (((uint32_t)buf)&3) {
		tmp = tmp_buf;
		for (;buf_len>0;buf_len-=16,buf+=16) {
			memcpy(tmp, buf, 16);
			__HEXDUMP("INPUT", tmp, 16);
			while (0 == (AES_REGS(aes_inst)->CTRL & AES_CTRL_INPUT_READY_BIT));
			AES_REGS(aes_inst)->DATA_1 = tmp[0];
			AES_REGS(aes_inst)->DATA_2 = tmp[1];
			AES_REGS(aes_inst)->DATA_3 = tmp[2];
			AES_REGS(aes_inst)->DATA_4 = tmp[3];
			while (0 == (AES_REGS(aes_inst)->CTRL & AES_CTRL_OUTPUT_READY_BIT));
			tmp[0] = AES_REGS(aes_inst)->DATA_1;
			tmp[1] = AES_REGS(aes_inst)->DATA_2;
			tmp[2] = AES_REGS(aes_inst)->DATA_3;
			tmp[3] = AES_REGS(aes_inst)->DATA_4;
			__HEXDUMP("OUTPUT", tmp, 16);
			memcpy(buf, tmp, 16);
		}
	} else {
		tmp = (uint32_t*)buf;
		for (;buf_len>0;buf_len-=16,tmp+=4) {
			__HEXDUMP("INPUT", tmp, 16);
			while (0 == (AES_REGS(aes_inst)->CTRL & AES_CTRL_INPUT_READY_BIT));
			AES_REGS(aes_inst)->DATA_1 = tmp[0];
			AES_REGS(aes_inst)->DATA_2 = tmp[1];
			AES_REGS(aes_inst)->DATA_3 = tmp[2];
			AES_REGS(aes_inst)->DATA_4 = tmp[3];
			while (0 == (AES_REGS(aes_inst)->CTRL & AES_CTRL_OUTPUT_READY_BIT));
			tmp[0] = AES_REGS(aes_inst)->DATA_1;
			tmp[1] = AES_REGS(aes_inst)->DATA_2;
			tmp[2] = AES_REGS(aes_inst)->DATA_3;
			tmp[3] = AES_REGS(aes_inst)->DATA_4;
			__HEXDUMP("OUTPUT", tmp, 16);
		}
	}
}

void omap3_crypto_aes_process_dma(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv,
		omap3_crypto_dma_callback cb, unsigned long data)
{
	int aes_inst = id - OMAP3_CRYPTO_AES1_IDX;
	uint32_t tmp_buf[4], *tmp, phys;
	int i;

	KASSERT(((buf_len & 0xF) == 0), ("%s: buf_len [%d] is not a multiple of 16", __func__, buf_len));       

	phys = omap3_crypto_scratch_get_phys(buf);
	if (0 == phys) {
		omap3_crypto_aes_process(id, encryption, buf, buf_len, iv);
		cb(data);
		return;
	}

	encryption <<= 2;
	if ((AES_REGS(aes_inst)->CTRL & 0x4) != encryption) 
		AES_REGS(aes_inst)->CTRL ^= 0x4;

	if (iv) {
		// Load iv
		if (((uint32_t)iv)&3) {
			tmp = tmp_buf;
			memcpy(tmp, iv, 16);
		} else tmp = (uint32_t*)iv;
		__HEXDUMP("IV", tmp, 16);

		AES_REGS(aes_inst)->IV_1 = tmp[0];
		AES_REGS(aes_inst)->IV_2 = tmp[1];
		AES_REGS(aes_inst)->IV_3 = tmp[2];
		AES_REGS(aes_inst)->IV_4 = tmp[3];
	}

	i = omap3_crypto_sdma_channel_rx[id];

	// setup SDMA for input and output
	SDMA_REGS->chan[i].CCR &= 0xFC030FC0;
	SDMA_REGS->chan[i].CCR |= ((1 << 25) | (1 << 5) | (3 << 12) | (3 << 14) | 
			((omap3_crypto_sdma_rx_num[id] & 0x60) << 14) | (omap3_crypto_sdma_rx_num[id] & 0x1F));
	SDMA_REGS->chan[i].CSDP = 2;
	SDMA_REGS->chan[i].CEN = 4;
	SDMA_REGS->chan[i].CFN = (buf_len >> 4);
	SDMA_REGS->chan[i].CSSA = phys;
	SDMA_REGS->chan[i].CSEI = 1;
	SDMA_REGS->chan[i].CSFI = 1;
	SDMA_REGS->chan[i].CDSA = omap3_crypto_base_addresses[id] + 0x34;
	SDMA_REGS->chan[i].CDEI = 1;
	SDMA_REGS->chan[i].CDFI = -15;
	SDMA_REGS->chan[i].CLNK_CTRL = 0;
	SDMA_REGS->chan[i].CCR |= (1 << 7);

	i = omap3_crypto_sdma_channel_tx[id];

	SDMA_REGS->chan[i].CCR &= 0xFC030FC0;
	SDMA_REGS->chan[i].CCR |= ((1 << 25) | (1 << 24) | (1 << 5) | (3 << 12) | (3 << 14) | 
			((omap3_crypto_sdma_tx_num[id] & 0x60) << 14) | (omap3_crypto_sdma_tx_num[id] & 0x1F));
	SDMA_REGS->chan[i].CSDP = 2;
	SDMA_REGS->chan[i].CEN = 4;
	SDMA_REGS->chan[i].CFN = (buf_len >> 4);
	SDMA_REGS->chan[i].CSSA = omap3_crypto_base_addresses[id] + 0x34;
	SDMA_REGS->chan[i].CSEI = 1;
	SDMA_REGS->chan[i].CSFI = -15;
	SDMA_REGS->chan[i].CDSA = phys;
	SDMA_REGS->chan[i].CDEI = 1;
	SDMA_REGS->chan[i].CDFI = 1;
	SDMA_REGS->chan[i].CLNK_CTRL = 0;
	SDMA_REGS->chan[i].CCR |= (1 << 7);

	omap3_crypto_sdma_cbs(id) = cb;
	omap3_crypto_sdma_cb_data(id) = data;

	// set the START bit in mask register
	AES_REGS(aes_inst)->MASK = AES_REGS(aes_inst)->MASK | 0x20;
}
/* ========================================================================= */
/*                            HASH    FUNCTIONS                              */
/* ========================================================================= */

static void omap3_crypto_sha1md5_reset(void)
{
	// Software reset
	SHA1MD5_REGS->MASK = 0x2;
	// Wait for reset to complete
	while (0 == (SHA1MD5_REGS->SYSSTATUS & SHA1MD5_SYS_STATUS_RESET_DONE_BIT));
}

static void omap3_crypto_sha2md5_reset(void)
{
	// Software reset
	SHA2MD5_REGS->MASK = 0x2;
	// Wait for reset to complete
	while (0 == (SHA2MD5_REGS->SYSSTATUS & SHA2MD5_SYS_STATUS_RESET_DONE_BIT));
}

/* Saved state to continue a hash across process calls */
static uint32_t omap3_crypto_sha1md5_digcnt, omap3_crypto_sha2md5_digcnt;
static uint32_t omap3_crypto_sha1md5_hash[5], omap3_crypto_sha2md5_hash[8];
static uint32_t omap3_crypto_sha1md5_data[16], omap3_crypto_sha2md5_data[16];
static uint32_t omap3_crypto_sha1md5_data_len, omap3_crypto_sha2md5_data_len;

static const char * const omap3_crypto_hash_names[] = {
"", "", "", "", "MD5", "SHA1", "", "SHA2-224", "SHA2-256" };
static const uint32_t const omap3_crypto_algo[] = { 0, 0, 0, 0, 0, 1, 0, 2, 3 };

static void omap3_crypto_sha1md5_copy(uint8_t *buf, int len)
{
    uint32_t tmp_buf[16];
    int i;
    volatile uint32_t *din = &(SHA1MD5_REGS->DIN_0);

    if (((uint32_t)buf)&3) {
        for (;len>0;len-=64,buf+=64) {
            memcpy(tmp_buf, buf, 64);
            __HEXDUMP("INPUT", tmp_buf, 64);
            while (0 == (SHA1MD5_REGS->CTRL & SHA1MD5_CTRL_INPUT_READY_BIT));
            for (i=0;i<16;i++) din[i] = tmp_buf[i];
        }
    } else {
        for (;len>0;len-=64,buf+=64) {
            __HEXDUMP("INPUT", buf, 64);
            while (0 == (SHA1MD5_REGS->CTRL & SHA1MD5_CTRL_INPUT_READY_BIT));
            for (i=0;i<16;i++) din[i] = ((uint32_t*)buf)[i];
        }
    }
}

struct semaphore omap3_crypto_sha1md5_dma_sem = 
    __SEMAPHORE_INITIALIZER(omap3_crypto_sha1md5_dma_sem, 0);

static void omap3_crypto_sha1md5_dma_callback(unsigned long data)
{
    up(&omap3_crypto_sha1md5_dma_sem);
}

static void omap3_crypto_sha1md5_dma(uint8_t *buf, int len)
{
    int i;
	uint32_t phys;

	phys = omap3_crypto_scratch_get_phys(buf);
	if (0 == phys) {
        omap3_crypto_sha1md5_copy(buf, len);
        return;
    }

	i = omap3_crypto_sdma_channel_tx[OMAP3_CRYPTO_SHA1MD5_IDX];

	omap3_crypto_sdma_cbs(OMAP3_CRYPTO_SHA1MD5_IDX) = omap3_crypto_sha1md5_dma_callback;
	omap3_crypto_sdma_cb_data(OMAP3_CRYPTO_SHA1MD5_IDX) = 0;

	SDMA_REGS->chan[i].CCR &= 0xFC030FC0;
	SDMA_REGS->chan[i].CCR |= ((1 << 25) | (1 << 5) | (3 << 12) | (3 << 14) | 
			((omap3_crypto_sdma_tx_num[OMAP3_CRYPTO_SHA1MD5_IDX] & 0x60) << 14) | 
            (omap3_crypto_sdma_tx_num[OMAP3_CRYPTO_SHA1MD5_IDX] & 0x1F));
	SDMA_REGS->chan[i].CSDP = 2;
	SDMA_REGS->chan[i].CEN = 16;
	SDMA_REGS->chan[i].CFN = (len >> 6);
	SDMA_REGS->chan[i].CSSA = phys;
	SDMA_REGS->chan[i].CSEI = 1;
	SDMA_REGS->chan[i].CSFI = 1;
	SDMA_REGS->chan[i].CDSA = omap3_crypto_base_addresses[OMAP3_CRYPTO_SHA1MD5_IDX] + 0x1c;
	SDMA_REGS->chan[i].CDEI = 1;
	SDMA_REGS->chan[i].CDFI = -63;
	SDMA_REGS->chan[i].CLNK_CTRL = 0;
	SDMA_REGS->chan[i].CCR |= (1 << 7);

    // set the DMA_EN bit in mask register
    SHA1MD5_REGS->MASK = (1 << 3);

    if (0 != down_interruptible(&omap3_crypto_sha1md5_dma_sem)) {
        printk("omap3_crypto_sha1md5_dma has hanged, bailing out!");
        omap3_crypto_sha1md5_copy(buf, len);
        // panic("omap3_crypto_sha1md5_dma wait interrupted!");
    }
}

static void omap3_crypto_sha1md5_do(uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
    void (*cp_fxn)(uint8_t *, int))
{
    uint32_t tmp;
    int len, l, i, one_pkt;
    volatile uint32_t *digest;
    volatile uint32_t *din;

    digest = &(SHA1MD5_REGS->DIGEST_A);
    din = &(SHA1MD5_REGS->DIN_0);
    len = omap3_crypto_sha1md5_data_len + buf_len;
    one_pkt = (len>64)?0:1;
    l = len & (~63);
    if (l == len) l = (len-64);
    if (0 == l) l = len;
    hash_len >>= 2;

    dprintk("omap3_crypto_sha1md5_do: buf_len: %d, len: %d l: %d\n", buf_len, len, l);

    if (omap3_crypto_sha1md5_digcnt) {  // are we continuing an earlier hash?
        // reload digest and counter regs
        SHA1MD5_REGS->CTRL = ((omap3_crypto_algo[hash_len] << 2) | (one_pkt << 4) | (l << 5));
        for (i=0;i<hash_len;i++) digest[i] = omap3_crypto_sha1md5_hash[i];
        SHA1MD5_REGS->DIGCNT = omap3_crypto_sha1md5_digcnt;
    } else {
        SHA1MD5_REGS->CTRL = ((omap3_crypto_algo[hash_len] << 2) | (1 << 3) | (one_pkt << 4) | (l << 5));
    }

    dprintk("Starting %s: %08x %08x\n", omap3_crypto_hash_names[hash_len], 
        SHA1MD5_REGS->CTRL, SHA1MD5_REGS->DIGCNT);

    if (omap3_crypto_sha1md5_data_len) {
        l = (!one_pkt)?64:len;
        i = (l - omap3_crypto_sha1md5_data_len);
        if (i) {
            memcpy(((uint8_t*)omap3_crypto_sha1md5_data)+omap3_crypto_sha1md5_data_len, buf, i);
            buf += i;
        }
        /* First process what is left over from the last time */
        if (!one_pkt) { 
            // Not Last packet
            len -= 64;
            while (0 == (SHA1MD5_REGS->CTRL & SHA1MD5_CTRL_INPUT_READY_BIT));
            for (i=0;i<16;i++) din[i] = omap3_crypto_sha1md5_data[i];
            omap3_crypto_sha1md5_data_len = 0;
        }
    }
    /* Now process all except the last packet of 1-64 bytes */
    l = len & (~63);
    if (l == len) l = (len-64);
    if (l > 0) { cp_fxn(buf, l); buf+=l; len-=l; }

    if (!one_pkt) {
        /* Save digest and digcnt */
        while (0 == (SHA1MD5_REGS->CTRL & SHA1MD5_CTRL_OUTPUT_READY_BIT));
        for (i=0;i<hash_len;i++) omap3_crypto_sha1md5_hash[i] = digest[i];
        omap3_crypto_sha1md5_digcnt = SHA1MD5_REGS->DIGCNT;

        dprintk("saving DIGCNT: %08x\n", omap3_crypto_sha1md5_digcnt);
        dprintk("sha1 hash output = (%08x %08x %08x %08x %08x)\n",
                omap3_crypto_sha1md5_hash[0],
                omap3_crypto_sha1md5_hash[1],
                omap3_crypto_sha1md5_hash[2],
                omap3_crypto_sha1md5_hash[3],
                omap3_crypto_sha1md5_hash[4]);

        SHA1MD5_REGS->CTRL = (((hash_len==5)?1:0) << 2) | (1 << 4) | (len << 5);
        SHA1MD5_REGS->DIGCNT = omap3_crypto_sha1md5_digcnt;
    }

    /* Last packet */
    if (!omap3_crypto_sha1md5_data_len) {
       memcpy(omap3_crypto_sha1md5_data, buf, len); 
       omap3_crypto_sha1md5_data_len = len;
    }

    while (0 == (SHA1MD5_REGS->CTRL & SHA1MD5_CTRL_INPUT_READY_BIT));
    for (i=0;len>0;i++,len-=4) din[i] = omap3_crypto_sha1md5_data[i];

    while (0 == (SHA1MD5_REGS->CTRL & SHA1MD5_CTRL_OUTPUT_READY_BIT));
    if (hash_len == 5) { // SHA1
        for (i=0;i<hash_len;i++) { 
            tmp = digest[i];
            *hash++ = (tmp >> 24);
            *hash++ = (tmp >> 16);
            *hash++ = (tmp >> 8);
            *hash++ = tmp;
        }
    }
    else { // MD5
        for (i=0;i<hash_len;i++) { 
            tmp = digest[i];
            *hash++ = tmp;
            *hash++ = (tmp >> 8);
            *hash++ = (tmp >> 16);
            *hash++ = (tmp >> 24);
        }
    }
}

static void omap3_crypto_sha1md5_process(uint8_t *buf, int buf_len, uint8_t *hash, int hash_len)
{
    omap3_crypto_sha1md5_do(buf,buf_len,hash,hash_len,omap3_crypto_sha1md5_copy);
}
static void omap3_crypto_sha1md5_process_dma(uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
		omap3_crypto_dma_callback cb, unsigned long data)
{
    omap3_crypto_sha1md5_do(buf,buf_len,hash,hash_len,omap3_crypto_sha1md5_dma);
    cb(data);
}

static void omap3_crypto_sha2md5_copy(uint8_t *buf, int len)
{
    uint32_t tmp_buf[16];
    int i;
    volatile uint32_t *din = &(SHA2MD5_REGS->DIN_0);

    if (((uint32_t)buf)&3) {
        for (;len>0;len-=64,buf+=64) {
            memcpy(tmp_buf, buf, 64);
            __HEXDUMP("INPUT", tmp_buf, 64);
            while (0 == (SHA2MD5_REGS->IRQSTAT & SHA2MD5_IRQSTAT_INPUT_READY_BIT));
            for (i=0;i<16;i++) din[i] = tmp_buf[i];
        }
    } else {
        for (;len>0;len-=64,buf+=64) {
            __HEXDUMP("INPUT", buf, 64);
            while (0 == (SHA2MD5_REGS->IRQSTAT & SHA2MD5_IRQSTAT_INPUT_READY_BIT));
            for (i=0;i<16;i++) din[i] = ((uint32_t*)buf)[i];
        }
    }
}

struct semaphore omap3_crypto_sha2md5_dma_sem = 
    __SEMAPHORE_INITIALIZER(omap3_crypto_sha2md5_dma_sem, 0);

static void omap3_crypto_sha2md5_dma_callback(unsigned long data)
{
    up(&omap3_crypto_sha2md5_dma_sem);
}

static void omap3_crypto_sha2md5_dma(uint8_t *buf, int len)
{
    int i;
	uint32_t phys;

	phys = omap3_crypto_scratch_get_phys(buf);
	if (0 == phys) {
        omap3_crypto_sha2md5_copy(buf, len);
        return;
    }

	i = omap3_crypto_sdma_channel_tx[OMAP3_CRYPTO_SHA2MD5_IDX];

	omap3_crypto_sdma_cbs(OMAP3_CRYPTO_SHA2MD5_IDX) = omap3_crypto_sha2md5_dma_callback;
	omap3_crypto_sdma_cb_data(OMAP3_CRYPTO_SHA2MD5_IDX) = 0;

	SDMA_REGS->chan[i].CCR &= 0xFC030FC0;
	SDMA_REGS->chan[i].CCR |= ((1 << 25) | (1 << 5) | (3 << 12) | (3 << 14) | 
			((omap3_crypto_sdma_tx_num[OMAP3_CRYPTO_SHA2MD5_IDX] & 0x60) << 14) | 
            (omap3_crypto_sdma_tx_num[OMAP3_CRYPTO_SHA2MD5_IDX] & 0x1F));
	SDMA_REGS->chan[i].CSDP = 2;
	SDMA_REGS->chan[i].CEN = 16;
	SDMA_REGS->chan[i].CFN = (len >> 6);
	SDMA_REGS->chan[i].CSSA = phys;
	SDMA_REGS->chan[i].CSEI = 1;
	SDMA_REGS->chan[i].CSFI = 1;
	SDMA_REGS->chan[i].CDSA = omap3_crypto_base_addresses[OMAP3_CRYPTO_SHA2MD5_IDX] + 0x30;
	SDMA_REGS->chan[i].CDEI = 1;
	SDMA_REGS->chan[i].CDFI = -63;
	SDMA_REGS->chan[i].CLNK_CTRL = 0;
	SDMA_REGS->chan[i].CCR |= (1 << 7);

    // set the DMA_EN bit in mask register
    SHA2MD5_REGS->MASK = (1 << 3);

    if (0 != down_interruptible(&omap3_crypto_sha2md5_dma_sem)) {
        printk("omap3_crypto_sha2md5_dma has hanged, bailing out!");
        omap3_crypto_sha2md5_copy(buf, len);
        // panic("omap3_crypto_sha2md5_dma wait interrupted!");
    }
}

static void omap3_crypto_sha2md5_do(uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
    void (*cp_fxn)(uint8_t *, int))
{
    uint32_t tmp;
    int len, l, i, one_pkt;
    volatile uint32_t *digest;
    volatile uint32_t *din;

    digest = &(SHA2MD5_REGS->DIGEST_A);
    din = &(SHA2MD5_REGS->DIN_0);
    len = omap3_crypto_sha2md5_data_len + buf_len;
    one_pkt = (len>64)?0:1;
    l = len & (~63);
    if (l == len) l = (len-64);
    if (0 == l) l = len;
    hash_len >>= 2;

    dprintk("omap3_crypto_sha2md5_do: buf_len: %d, len: %d l: %d\n", buf_len, len, l);

    if (omap3_crypto_sha2md5_digcnt) {  // are we continuing an earlier hash?
        // reload digest and counter regs
        SHA2MD5_REGS->CTRL = ((omap3_crypto_algo[hash_len] << 1) | (one_pkt << 4) | (l << 5));
        for (i=0;i<hash_len;i++) digest[i] = omap3_crypto_sha2md5_hash[i];
        SHA2MD5_REGS->DIGCNT = omap3_crypto_sha2md5_digcnt;
    } else {
        SHA2MD5_REGS->CTRL = ((omap3_crypto_algo[hash_len] << 1) | (1 << 3) | (one_pkt << 4) | (l << 5));
    }

    dprintk("Starting %s: %08x %08x\n", omap3_crypto_hash_names[hash_len], 
        SHA2MD5_REGS->CTRL, SHA2MD5_REGS->DIGCNT);

    if (omap3_crypto_sha2md5_data_len) {
        l = (!one_pkt)?64:len;
        i = (l - omap3_crypto_sha2md5_data_len);
        if (i) {
            memcpy(((uint8_t*)omap3_crypto_sha2md5_data)+omap3_crypto_sha2md5_data_len, buf, i);
            buf += i;
        }
        /* First process what is left over from the last time */
        if (!one_pkt) { 
            // Not Last packet
            len -= 64;
            while (0 == (SHA2MD5_REGS->IRQSTAT & SHA2MD5_IRQSTAT_INPUT_READY_BIT));
            for (i=0;i<16;i++) din[i] = omap3_crypto_sha2md5_data[i];
            omap3_crypto_sha2md5_data_len = 0;
        }
    }
    /* Now process all except the last packet of 1-64 bytes */
    l = len & (~63);
    if (l == len) l = (len-64);
    if (l > 0) { cp_fxn(buf, l); buf+=l; len-=l; }

    if (!one_pkt) {
        /* Save digest and digcnt */
        while (0 == (SHA2MD5_REGS->IRQSTAT & SHA2MD5_IRQSTAT_OUTPUT_READY_BIT));
        for (i=0;i<hash_len;i++) omap3_crypto_sha2md5_hash[i] = digest[i];
        omap3_crypto_sha2md5_digcnt = SHA2MD5_REGS->DIGCNT;

        dprintk("saving DIGCNT: %08x\n", omap3_crypto_sha2md5_digcnt);
        dprintk("sha1 hash output = (%08x %08x %08x %08x %08x)\n",
                omap3_crypto_sha2md5_hash[0],
                omap3_crypto_sha2md5_hash[1],
                omap3_crypto_sha2md5_hash[2],
                omap3_crypto_sha2md5_hash[3],
                omap3_crypto_sha2md5_hash[4]);

        SHA2MD5_REGS->CTRL = (omap3_crypto_algo[hash_len] << 1) | (1 << 4) | (len << 5);
        SHA2MD5_REGS->DIGCNT = omap3_crypto_sha2md5_digcnt;
    }

    /* Last packet */
    if (!omap3_crypto_sha2md5_data_len) {
       memcpy(omap3_crypto_sha2md5_data, buf, len); 
       omap3_crypto_sha2md5_data_len = len;
    }

    while (0 == (SHA2MD5_REGS->IRQSTAT & SHA2MD5_IRQSTAT_INPUT_READY_BIT));
    for (i=0;len>0;i++,len-=4) din[i] = omap3_crypto_sha2md5_data[i];

    while (0 == (SHA2MD5_REGS->IRQSTAT & SHA2MD5_IRQSTAT_OUTPUT_READY_BIT));
    if (hash_len >= 5) { // SHAx works big endian
        for (i=0;i<hash_len;i++) { 
            tmp = digest[i];
            *hash++ = (tmp >> 24);
            *hash++ = (tmp >> 16);
            *hash++ = (tmp >> 8);
            *hash++ = tmp;
        }
    }
    else { // MD5
        for (i=0;i<hash_len;i++) { 
            tmp = digest[i];
            *hash++ = tmp;
            *hash++ = (tmp >> 8);
            *hash++ = (tmp >> 16);
            *hash++ = (tmp >> 24);
        }
    }
}
static void omap3_crypto_sha2md5_process(uint8_t *buf, int buf_len, uint8_t *hash, int hash_len)
{
    omap3_crypto_sha2md5_do(buf,buf_len,hash,hash_len,omap3_crypto_sha2md5_copy);
}
static void omap3_crypto_sha2md5_process_dma(uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
		omap3_crypto_dma_callback cb, unsigned long data)
{
    omap3_crypto_sha2md5_do(buf,buf_len,hash,hash_len,omap3_crypto_sha2md5_dma);
    cb(data);
}


#define NUM_HASH_INSTANCES   2
#define HASH_INST(id) (id-OMAP3_CRYPTO_SHA2MD5_IDX)
static void (*omap3_crypto_hash_process_fptr[NUM_HASH_INSTANCES])(
    uint8_t *buf, int buf_len, uint8_t *hash, int hash_len);
static void (*omap3_crypto_hash_process_dma_fptr[NUM_HASH_INSTANCES])(
    uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
    omap3_crypto_dma_callback cb, unsigned long data);

void omap3_crypto_hash_init(int id, int alg, int hash_len)
{
    if (id == OMAP3_CRYPTO_SHA2MD5_IDX) {
        omap3_crypto_hash_process_fptr[HASH_INST(id)] = omap3_crypto_sha2md5_process;
        omap3_crypto_hash_process_dma_fptr[HASH_INST(id)] = omap3_crypto_sha2md5_process_dma;
        omap3_crypto_sha2md5_digcnt = 0;
        omap3_crypto_sha2md5_data_len = 0;
    } else {
        omap3_crypto_hash_process_fptr[HASH_INST(id)] = omap3_crypto_sha1md5_process;
        omap3_crypto_hash_process_dma_fptr[HASH_INST(id)] = omap3_crypto_sha1md5_process_dma;
        omap3_crypto_sha1md5_digcnt = 0;
        omap3_crypto_sha1md5_data_len = 0;
    }
}

void omap3_crypto_hash_process(int id, uint8_t *buf, int buf_len, uint8_t *hash, int hash_len)
{
    omap3_crypto_hash_process_fptr[HASH_INST(id)](buf, buf_len, hash, hash_len);
}
void omap3_crypto_hash_process_dma(int id, uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
		omap3_crypto_dma_callback cb, unsigned long data)
{
    omap3_crypto_hash_process_dma_fptr[HASH_INST(id)](buf, buf_len, hash, hash_len, cb, data);
}

/* ========================================================================= */
/*                            H/W CHANNEL MANGEMENT                          */
/* ========================================================================= */

static const uint32_t 
	omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_MAX_IDX] = {
		(1 << 0),  // OMAP3_CRYPTO_DES1_IDX
		(1 << 26), // OMAP3_CRYPTO_DES2_IDX
		(1 << 1),  // OMAP3_CRYPTO_SHA2MD5_IDX
		(1 << 27), // OMAP3_CRYPTO_SHA1MD5_IDX
		(1 << 3),  // OMAP3_CRYPTO_AES1_IDX
		(1 << 28)  // OMAP3_CRYPTO_AES2_IDX
	};

static uint32_t omap3_crypto_mod_used[OMAP3_CRYPTO_MAX_IDX];
DECLARE_MUTEX(omap3_crypto_mutex);

/* allocate algorithms of types in array alg_list (terminated by -1)
	 and returns ids in id_list (terminated by -1) */
int omap3_crypto_alloc(int *alg_list, int *id_list)
{
	int ret = 0;
	int i;
	// uint32_t clken_mask = 0;

	if (0 != (ret = down_interruptible(&omap3_crypto_mutex))) return ret;
	// Look at requests
	for (i = 0; alg_list[i] != -1; i++) {
		switch (alg_list[i]) {
			case OMAP3_CRYPTO_ALG_AES:
				if (omap3_crypto_mod_used[OMAP3_CRYPTO_AES2_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_AES2_IDX;
				else if (omap3_crypto_mod_used[OMAP3_CRYPTO_AES1_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_AES1_IDX;
				else ret = -EBUSY;
				break;
			case OMAP3_CRYPTO_ALG_DES:
				if (omap3_crypto_mod_used[OMAP3_CRYPTO_DES2_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_DES2_IDX;
				else if (omap3_crypto_mod_used[OMAP3_CRYPTO_DES1_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_DES1_IDX;
				else ret = -EBUSY;
				break;
			case OMAP3_CRYPTO_ALG_SHA1:
			case OMAP3_CRYPTO_ALG_MD5:
				if (omap3_crypto_mod_used[OMAP3_CRYPTO_SHA2MD5_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_SHA2MD5_IDX;
				else if (omap3_crypto_mod_used[OMAP3_CRYPTO_SHA1MD5_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_SHA1MD5_IDX;
				else ret = -EBUSY;
				break;
			case OMAP3_CRYPTO_ALG_SHA2:
				if (omap3_crypto_mod_used[OMAP3_CRYPTO_SHA2MD5_IDX] == 0)
					id_list[i] = OMAP3_CRYPTO_SHA2MD5_IDX;
				else ret = -EBUSY;
				break;
			default:
				// TODO: rest of the algorithms
				ret = -EINVAL;
				break;
		}
		if (0 != ret) break;
		// if (0 != (ret = omap3_crypto_dma_alloc(id_list[i]))) break;
		omap3_crypto_mod_used[id_list[i]] = omap3_crypto_mod_clken_bits[id_list[i]];
		// clken_mask |= omap3_crypto_mod_clken_bits[id_list[i]];
	}
	id_list[i] = -1;
	if (0 != ret) {
		for (--i; i >= 0; i--) {
			omap3_crypto_mod_used[id_list[i]] = 0;
			// omap3_crypto_dma_free(id_list[i]);
		}
	} else {
    #if 0
		// Enable clocks to these modules
		CM_REGS->CM_ICLKEN1_CORE |= (clken_mask & 0xFFFF0000);
		CM_REGS->CM_ICLKEN2_CORE |= (clken_mask & 0xFFFF);
    #endif
	}

	up(&omap3_crypto_mutex);
	return ret;
}


/* free algorithm ids in id_list (terminated by -1) */
int omap3_crypto_free(int *id_list)
{
	int i, ret;
	uint32_t clken_mask = 0;
	if (0 != (ret = down_interruptible(&omap3_crypto_mutex))) return ret;
	for (i = 0; id_list[i] != -1; i++) {
		clken_mask |= omap3_crypto_mod_used[id_list[i]];
		omap3_crypto_mod_used[id_list[i]] = 0;
        // omap3_crypto_dma_free(id_list[i]);
	}
    #if 0
	// Disable clocks to these modules
	CM_REGS->CM_ICLKEN1_CORE &= ((~clken_mask) | 0xFFFF);
	CM_REGS->CM_ICLKEN2_CORE &= ((~clken_mask) | 0xFFFF0000);
    #endif
	up(&omap3_crypto_mutex);
	return 0;
}

/* ========================================================================= */
/*                            MODULE INIT AND DEINIT                         */
/* ========================================================================= */

int omap3_crypto_init(int debug_flag)
{
	int i;
	uint32_t clken_mask;

    omap3_crypto_debug = debug_flag;

	dprintk("%s(%p)\n", __FUNCTION__, omap3_crypto_init);

	for (i = 0; i < OMAP3_CRYPTO_NUM_MMAPS; i++) {
		omap3_crypto_reg_ptrs[i] = ioremap_nocache(omap3_crypto_base_addresses[i], 
				OMAP3_CRYPTO_MMAPS_SIZE);
		if (NULL == omap3_crypto_reg_ptrs[i]) 
			panic("omap3_crypto: ioremap_nocache() failed!");
	}

	if (0 != omap3_crypto_scratch_init())
		panic("omap3_crypto: omap3_crypto_scratch_init() failed!");

	omap3_crypto_dma_init();

	for (i = 0; i < OMAP3_CRYPTO_MAX_IDX; i++)
		omap3_crypto_mod_used[i] = 0;
 
    clken_mask = omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_AES1_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_AES2_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_DES1_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_DES2_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_SHA1MD5_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_SHA2MD5_IDX];
 
    // Enable clocks to these modules
    CM_REGS->CM_ICLKEN1_CORE |= (clken_mask & 0xFFFF0000);
    CM_REGS->CM_ICLKEN2_CORE |= (clken_mask & 0xFFFF);

    // Wait for sometime for the clocks to be enabled
    {   volatile int x = 0;
        for (x=0; x<10000; x++);  }

    omap3_crypto_aes_reset(OMAP3_CRYPTO_AES1_IDX);
    omap3_crypto_aes_reset(OMAP3_CRYPTO_AES2_IDX);
    omap3_crypto_des_reset(OMAP3_CRYPTO_DES1_IDX);
    omap3_crypto_des_reset(OMAP3_CRYPTO_DES2_IDX);
    omap3_crypto_sha1md5_reset();
    omap3_crypto_sha2md5_reset();

    if (0 != omap3_crypto_dma_alloc(OMAP3_CRYPTO_AES1_IDX)) {
		panic("omap3_crypto: omap3_crypto_dma_alloc() failed!");
    }
    if (0 != omap3_crypto_dma_alloc(OMAP3_CRYPTO_AES2_IDX)) {
		panic("omap3_crypto: omap3_crypto_dma_alloc() failed!");
    }
    if (0 != omap3_crypto_dma_alloc(OMAP3_CRYPTO_DES1_IDX)) {
		panic("omap3_crypto: omap3_crypto_dma_alloc() failed!");
    }
    if (0 != omap3_crypto_dma_alloc(OMAP3_CRYPTO_DES2_IDX)) {
		panic("omap3_crypto: omap3_crypto_dma_alloc() failed!");
    }
    if (0 != omap3_crypto_dma_alloc(OMAP3_CRYPTO_SHA1MD5_IDX)) {
		panic("omap3_crypto: omap3_crypto_dma_alloc() failed!");
    }
    if (0 != omap3_crypto_dma_alloc(OMAP3_CRYPTO_SHA2MD5_IDX)) {
		panic("omap3_crypto: omap3_crypto_dma_alloc() failed!");
    }
	return 0;
}

void omap3_crypto_exit(void)
{
	int i;
	uint32_t clken_mask;

    clken_mask = omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_AES1_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_AES2_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_DES1_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_DES2_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_SHA1MD5_IDX] |
                 omap3_crypto_mod_clken_bits[OMAP3_CRYPTO_SHA2MD5_IDX];
 
	// Disable clocks to these modules
	CM_REGS->CM_ICLKEN1_CORE &= ((~clken_mask) | 0xFFFF);
	CM_REGS->CM_ICLKEN2_CORE &= ((~clken_mask) | 0xFFFF0000);

    omap3_crypto_dma_free(OMAP3_CRYPTO_AES1_IDX);
    omap3_crypto_dma_free(OMAP3_CRYPTO_AES2_IDX);
    omap3_crypto_dma_free(OMAP3_CRYPTO_DES1_IDX);
    omap3_crypto_dma_free(OMAP3_CRYPTO_DES2_IDX);
    omap3_crypto_dma_free(OMAP3_CRYPTO_SHA1MD5_IDX);
    omap3_crypto_dma_free(OMAP3_CRYPTO_SHA2MD5_IDX);

	dprintk("%s()\n", __FUNCTION__);
	for (i=0;i<OMAP3_CRYPTO_NUM_MMAPS;i++) {
		iounmap(omap3_crypto_reg_ptrs[i]);
	}
	omap3_crypto_scratch_deinit();
}


