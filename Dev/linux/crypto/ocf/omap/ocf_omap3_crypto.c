#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>

#include <uio.h>
#include <asm/io.h>

#include "omap3_crypto.h"

#define OCF_OMAP3_CRYPTO_MAX_HASH_SIZE 32

#define debug ocf_omap3_crypto_debug
int ocf_omap3_crypto_debug = 0;
module_param(ocf_omap3_crypto_debug, int, 0644);
MODULE_PARM_DESC(ocf_omap3_crypto_debug, "Enable debug");

int ocf_omap3_crypto_dma = 1;
module_param(ocf_omap3_crypto_dma, int, 0644);
MODULE_PARM_DESC(ocf_omap3_crypto_dma, "Enable DMA");

#include <crypto/cryptodev.h>

/*
 * Generate a new software session.
 */
int ocf_omap3_crypto_newsession(device_t arg, u_int32_t *sid, struct cryptoini *cri)
{
    int ret = 0, i, hash_len;
    int alg_list[OMAP3_CRYPTO_MAX_ALG_INSTANCES+1];
    int *id_list;
    struct cryptoini *cri_init = cri;
    omap3_crypto_init_fptr init_fptr;
    uint32_t mode = 0;

	dprintk("%s()\n", __FUNCTION__);

	if (sid == NULL || cri == NULL) {
		dprintk("%s,%d - EINVAL\n", __FILE__, __LINE__);
		return EINVAL;
	}

    if (NULL == (id_list = (int*)kmalloc(
                  (OMAP3_CRYPTO_MAX_ALG_INSTANCES+1) * sizeof(int), GFP_KERNEL))) {
        return -ENOMEM;
    }

    // Look at requests
    for (i = 0; cri && ret == 0; cri = cri->cri_next, i++) {
		switch (cri->cri_alg) {
		case CRYPTO_AES_CBC:
            alg_list[i] = OMAP3_CRYPTO_ALG_AES;
            break;
        case CRYPTO_DES_CBC:
        case CRYPTO_3DES_CBC:
            alg_list[i] = OMAP3_CRYPTO_ALG_DES;
            break;
        case CRYPTO_MD5:
            alg_list[i] = OMAP3_CRYPTO_ALG_MD5;
            break;
        case CRYPTO_SHA1:
            alg_list[i] = OMAP3_CRYPTO_ALG_SHA1;
            break;
        case CRYPTO_SHA2_256:
            alg_list[i] = OMAP3_CRYPTO_ALG_SHA2;
            break;
		default:
			ret = -EINVAL;
			break;
		}
    }
    if (0 == ret) {
        alg_list[i] = -1;
        ret = omap3_crypto_alloc(alg_list, id_list);
    }

    if (0 != ret) return ret;

    // Initialize algorithms
    for (i = 0, cri = cri_init; cri && ret == 0; cri = cri->cri_next, i++) {
        switch (cri->cri_alg) {
            case CRYPTO_AES_CBC:
                init_fptr = omap3_crypto_aes_init;
                mode = OMAP3_CRYPTO_AES_MODE_CBC;
                break;
            case CRYPTO_DES_CBC:
                init_fptr = omap3_crypto_des_init;
                mode = OMAP3_CRYPTO_DES_MODE_CBC;
                break;
            case CRYPTO_3DES_CBC:
                init_fptr = omap3_crypto_des_init;
                mode = OMAP3_CRYPTO_DES_MODE_TDES_CBC;
                break;
            case CRYPTO_MD5:
                hash_len = 16;
                init_fptr = NULL;
                break;
            case CRYPTO_SHA1:
                hash_len = 20;
                init_fptr = NULL;
                break;
            case CRYPTO_SHA2_256:
                hash_len = 32;
                init_fptr = NULL;
                break;
        }
        if (init_fptr) init_fptr(id_list[i], cri->cri_key, cri->cri_klen >> 3, mode);
        else omap3_crypto_hash_init(id_list[i], alg_list[i], hash_len);
    }

    *sid = (u_int32_t)id_list;
	return ret;
}


/*
 * Free a session.
 */
int ocf_omap3_crypto_freesession(device_t arg, u_int64_t tid)
{
    int ret = 0;
	u_int32_t sid = CRYPTO_SESID2LID(tid);
    int *id_list = (int *)sid;

	dprintk("%s()\n", __FUNCTION__);

    ret = omap3_crypto_free(id_list);

    kfree(id_list);
	return ret;
}

void ocf_omap3_crypto_process_hash(int id, int hash_len,
    struct cryptop *crp, struct cryptodesc *crd) 
{
    uint32_t hash[OCF_OMAP3_CRYPTO_MAX_HASH_SIZE >> 2];

    if ((crp->crp_flags & CRYPTO_F_SKBUF) != 0) {
        printk("ocf_omap3_crypto: currently not supported\n");
    } else if ((crp->crp_flags & CRYPTO_F_IOV) != 0) {
        struct iovec *iov = ((struct uio*)crp->crp_buf)->uio_iov;
        int iol = ((struct uio*)crp->crp_buf)->uio_iovcnt;
        unsigned count;
        uint8_t *buf;
        int off, len;

        off = 0; // crd->crd_skip;
        len = crd->crd_len;

        dprintk("processing hash: off: %d, len: %d\n", off, len);
#if 0
        do {                        
            KASSERT(off >= 0, ("%s: off %d < 0", __func__, off));       
            KASSERT(len >= 0, ("%s: len %d < 0", __func__, len));       
            while (off > 0) {                       
                KASSERT(iol >= 0, ("%s: empty in skip", __func__)); 
                if (off < iov->iov_len)                 
                    break;                      
                off -= iov->iov_len;                    
                iol--;                          
                iov++;                          
            }                               
        } while (0);
#endif
        for ( ;len > 0; iol--, iov++) {
            KASSERT(iol >= 0, ("%s: empty", __func__));
            count = min((int)(iov->iov_len - off), len);
            buf = ((uint8_t*)iov->iov_base) + off;
            omap3_crypto_hash_process(id, buf, count, (uint8_t*)hash, hash_len);
            len -= count;
            off = 0;
        }
        KASSERT (((iov->iov_len - count) >= hash_len), ("%s: no space to put hash", __func__)); 
        memcpy(buf + count, hash, hash_len);
        
    }
    else { 
        omap3_crypto_hash_process(id, (uint8_t*)crp->crp_buf, crd->crd_len, (uint8_t*)hash, hash_len);
        KASSERT (((crp->crp_ilen - crd->crd_len) >= hash_len), ("%s: no space to put hash", __func__)); 
        memcpy((uint8_t*)crp->crp_buf + crd->crd_len, hash, hash_len);
    }
}

void ocf_omap3_crypto_process_data(int id, 
    int encryption, uint8_t *iv,
    omap3_crypto_process_fptr process_fptr,
    struct cryptop *crp, struct cryptodesc *crd) 
{
    if ((crp->crp_flags & CRYPTO_F_SKBUF) != 0) {
        printk("ocf_omap3_crypto: currently not supported\n");
    } else if ((crp->crp_flags & CRYPTO_F_IOV) != 0) {
        struct iovec *iov = ((struct uio*)crp->crp_buf)->uio_iov;
        int iol = ((struct uio*)crp->crp_buf)->uio_iovcnt;
        unsigned count;
        int off, len;

        off = 0; // crd->crd_skip;
        len = crp->crp_ilen; // crd->crd_len;

        dprintk("processing: encryption: %d, off: %d, len: %d\n", encryption, off, len);
#if 0
        do {                        
            KASSERT(off >= 0, ("%s: off %d < 0", __func__, off));       
            KASSERT(len >= 0, ("%s: len %d < 0", __func__, len));       
            while (off > 0) {                       
                KASSERT(iol >= 0, ("%s: empty in skip", __func__)); 
                if (off < iov->iov_len)                 
                    break;                      
                off -= iov->iov_len;                    
                iol--;                          
                iov++;                          
            }                               
        } while (0);
#endif
        while (len > 0) {
            KASSERT(iol >= 0, ("%s: empty", __func__));
            count = min((int)(iov->iov_len - off), len);
            process_fptr(id,encryption,((uint8_t*)iov->iov_base) + off, count, iv);
            iv = NULL;
            len -= count;
            off = 0;
            iol--;
            iov++;
        }
    }
    else process_fptr(id, encryption, (uint8_t*)crp->crp_buf, crp->crp_ilen, iv);
}

static void * ocf_omap3_crypto_dma_free_ptr[OMAP3_CRYPTO_MAX_ALG_INSTANCES];
static struct cryptop * ocf_omap3_crypto_dma_crp[OMAP3_CRYPTO_MAX_ALG_INSTANCES];

static void ocf_omap3_crypto_hash_dma_cb(unsigned long data)
{
    void *ptr;
    struct cryptop *crp;
    struct cryptodesc *crd;

    if (data >= OMAP3_CRYPTO_MAX_ALG_INSTANCES) return; // spurious

    ptr = ocf_omap3_crypto_dma_free_ptr[(int)data];
    crp = ocf_omap3_crypto_dma_crp[(int)data];
    crd = crp->crp_desc;

    omap3_crypto_scratch_free(ptr, crd->crd_len);
    crypto_done(crp);
}

static void ocf_omap3_crypto_dma_cb(unsigned long data)
{
    void *ptr;
    struct cryptop *crp;

    if (data >= OMAP3_CRYPTO_MAX_ALG_INSTANCES) return; // spurious

    ptr = ocf_omap3_crypto_dma_free_ptr[(int)data];
    crp = ocf_omap3_crypto_dma_crp[(int)data];

    memcpy(((struct uio*)crp->crp_buf)->uio_iov->iov_base, ptr, crp->crp_ilen);
    omap3_crypto_scratch_free(ptr, crp->crp_ilen);
    crypto_done(crp);
}

/*
 * Process a request.
 */
int ocf_omap3_crypto_process(device_t arg, struct cryptop *crp, int hint)
{
    u_int32_t sid = CRYPTO_SESID2LID(crp->crp_sid);
    struct cryptodesc *crd = crp->crp_desc;
    int *id_list = (int *)sid, i, hash_len;
    omap3_crypto_process_fptr process_fptr;
    omap3_crypto_process_dma_fptr process_dma_fptr;

    dprintk("%s()\n", __FUNCTION__);

    crp->crp_etype = 0;

    if (crp->crp_desc == NULL || crp->crp_buf == NULL) {
        dprintk("%s,%d: EINVAL\n", __FILE__, __LINE__);
        crp->crp_etype = EINVAL;
        goto done;
    }

    for (i = 0; crd && (crp->crp_etype == 0) && (id_list[i] != -1) ; 
            crd = crd->crd_next, i++) {
        switch (crd->crd_alg) {
            case CRYPTO_AES_CBC:
                process_fptr = omap3_crypto_aes_process;
                process_dma_fptr = omap3_crypto_aes_process_dma;
                break;
            case CRYPTO_DES_CBC:
                process_fptr = omap3_crypto_des_process;
                process_dma_fptr = omap3_crypto_des_process_dma;
                break;
            case CRYPTO_3DES_CBC:
                process_fptr = omap3_crypto_des_process;
                process_dma_fptr = omap3_crypto_des_process_dma;
                break;
            case CRYPTO_MD5:
                hash_len = 16;
                process_fptr = NULL;
                break;
            case CRYPTO_SHA1:
                hash_len = 20;
                process_fptr = NULL;
                break;
            case CRYPTO_SHA2_256:
                hash_len = 32;
                process_fptr = NULL;
                break;
            default:
                crp->crp_etype = -EINVAL;
                break;
        }
        if (0 == crp->crp_etype) {
            dprintk("flags: %x\n", crd->crd_flags);
            if (process_fptr) {
                // ENCRYPTION or DECRYPTION
                if (ocf_omap3_crypto_dma &&
                        (NULL == crd->crd_next) && (crp->crp_flags & CRYPTO_F_IOV) && 
                        (((struct uio*)crp->crp_buf)->uio_iov->iov_len >= crp->crp_ilen))
                {
                    void *dma_ptr;

                    dma_ptr = omap3_crypto_scratch_alloc(crp->crp_ilen);
                    if (dma_ptr) {
                        memcpy(dma_ptr, ((struct uio*)crp->crp_buf)->uio_iov->iov_base, crp->crp_ilen);
                        ocf_omap3_crypto_dma_free_ptr[id_list[i]] = dma_ptr;
                        ocf_omap3_crypto_dma_crp[id_list[i]] = crp;

                        dprintk("DMA processing, id: %d, len: %d\n", id_list[i], crp->crp_ilen);

                        process_dma_fptr(id_list[i],(crd->crd_flags & CRD_F_ENCRYPT)?1:0,
                                dma_ptr, crp->crp_ilen,
                                (crd->crd_flags & CRD_F_IV_EXPLICIT)?crd->crd_iv:NULL,
                                ocf_omap3_crypto_dma_cb, (unsigned long)id_list[i]);
                        return 0;
                    }
                }
                ocf_omap3_crypto_process_data(id_list[i], 
                        (crd->crd_flags & CRD_F_ENCRYPT)?1:0, 
                        (crd->crd_flags & CRD_F_IV_EXPLICIT)?crd->crd_iv:NULL,
                        process_fptr, crp, crd);
            } else {
                if (ocf_omap3_crypto_dma &&
                        (NULL == crd->crd_next) && (crp->crp_flags & CRYPTO_F_IOV) && 
                        (((struct uio*)crp->crp_buf)->uio_iov->iov_len >= crp->crp_ilen))
                {
                    void *dma_ptr;

                    dma_ptr = omap3_crypto_scratch_alloc(crd->crd_len);
                    if (dma_ptr) {
                        memcpy(dma_ptr, ((struct uio*)crp->crp_buf)->uio_iov->iov_base, crd->crd_len);
                        ocf_omap3_crypto_dma_free_ptr[id_list[i]] = dma_ptr;
                        ocf_omap3_crypto_dma_crp[id_list[i]] = crp;

                        dprintk("DMA processing hash, id: %d, len: %d\n", id_list[i], crd->crd_len);

                        omap3_crypto_hash_process_dma(id_list[i], dma_ptr, crd->crd_len, 
                            (uint8_t*)(((struct uio*)crp->crp_buf)->uio_iov->iov_base) + crd->crd_len, hash_len,
                            ocf_omap3_crypto_hash_dma_cb, (unsigned long)id_list[i]);
                        return 0;
                    }
                }
                // HASH FUNCTION
                ocf_omap3_crypto_process_hash(id_list[i], hash_len, crp, crd);
            }
        }
    }
done:
    crypto_done(crp);
    return 0;
}

static int32_t ocf_omap3_crypto_id = -1;

static struct {
	softc_device_decl	sc_dev;
} ocf_omap3_crypto_dev;

static device_method_t ocf_omap3_crypto_methods = {
	/* crypto device methods */
	DEVMETHOD(cryptodev_newsession,	ocf_omap3_crypto_newsession),
	DEVMETHOD(cryptodev_freesession,ocf_omap3_crypto_freesession),
	DEVMETHOD(cryptodev_process,	ocf_omap3_crypto_process),
};

/*
 * our driver startup and shutdown routines
 */
int ocf_omap3_crypto_init(void)
{
	dprintk("%s(%p)\n", __FUNCTION__, ocf_omap3_crypto_init);

	memset(&ocf_omap3_crypto_dev, 0, sizeof(ocf_omap3_crypto_dev));
	softc_device_init(&ocf_omap3_crypto_dev, "ocf_omap3_crypto", 0, ocf_omap3_crypto_methods);

	ocf_omap3_crypto_id = crypto_get_driverid(softc_get_device(&ocf_omap3_crypto_dev),
				CRYPTOCAP_F_HARDWARE);
	if (ocf_omap3_crypto_id < 0)
		panic("ocf_omap3_crypto: crypto device cannot initialize!");

#define	REGISTER(alg) \
	crypto_register(ocf_omap3_crypto_id,alg,0,0)
	REGISTER(CRYPTO_DES_CBC);
	REGISTER(CRYPTO_3DES_CBC);
	REGISTER(CRYPTO_AES_CBC);
	REGISTER(CRYPTO_MD5);
	REGISTER(CRYPTO_SHA1);
	REGISTER(CRYPTO_SHA2_256);
#undef REGISTER

	return omap3_crypto_init(ocf_omap3_crypto_debug);
}

void ocf_omap3_crypto_exit(void)
{
	dprintk("%s()\n", __FUNCTION__);
	crypto_unregister_all(ocf_omap3_crypto_id);
	ocf_omap3_crypto_id = -1;

    omap3_crypto_exit();
}

module_init(ocf_omap3_crypto_init);
module_exit(ocf_omap3_crypto_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Harinarayana Bhatta <harinarayan@ti.com>");
MODULE_DESCRIPTION("ocf module that uses omap3 crypto H/W accelerators");

