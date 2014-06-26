
#ifndef _OMAP3_CRYPTO_H_
#define _OMAP3_CRYPTO_H_

#define OMAP3_CRYPTO_MAX_ALG_INSTANCES 8

enum {
    OMAP3_CRYPTO_ALG_DES,
    OMAP3_CRYPTO_ALG_AES,
    OMAP3_CRYPTO_ALG_SHA1,
    OMAP3_CRYPTO_ALG_MD5,
    OMAP3_CRYPTO_ALG_SHA2
};

typedef void (*omap3_crypto_dma_callback)(unsigned long data);
typedef void (*omap3_crypto_init_fptr)(int id, uint8_t *key, int key_len, uint32_t mode);
typedef void (*omap3_crypto_process_fptr)(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv);
typedef void (*omap3_crypto_process_dma_fptr)(int id, int encryption, uint8_t *buf, 
    int buf_len, uint8_t *iv, omap3_crypto_dma_callback cb, unsigned long data);

/*=================================== AES ==================================== */

enum {
    OMAP3_CRYPTO_AES_MODE_ECB = 0,
    OMAP3_CRYPTO_AES_MODE_CBC = 1,
    OMAP3_CRYPTO_AES_MODE_CTR_32 = 2,
    OMAP3_CRYPTO_AES_MODE_CTR_64 = 6,
    OMAP3_CRYPTO_AES_MODE_CTR_96 = 10,
    OMAP3_CRYPTO_AES_MODE_CTR_128 = 14
};
void omap3_crypto_aes_init(int id, uint8_t *key, int key_len, uint32_t mode);
void omap3_crypto_aes_process(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv);
void omap3_crypto_aes_process_dma(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv,
		omap3_crypto_dma_callback cb, unsigned long data);

/*=================================== DES ==================================== */

enum {
    OMAP3_CRYPTO_DES_MODE_ECB = 0,
    OMAP3_CRYPTO_DES_MODE_TDES_ECB = 1,
    OMAP3_CRYPTO_DES_MODE_CBC = 2,
    OMAP3_CRYPTO_DES_MODE_TDES_CBC = 3,
};
void omap3_crypto_des_init(int id, uint8_t *key, int key_len, uint32_t mode);
void omap3_crypto_des_process(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv);
void omap3_crypto_des_process_dma(int id, int encryption, uint8_t *buf, int buf_len, uint8_t *iv,
		omap3_crypto_dma_callback cb, unsigned long data);

/*================================= HASH ================================== */

void omap3_crypto_hash_init(int id, int alg, int hash_len);
void omap3_crypto_hash_process(int id, uint8_t *buf, int buf_len, uint8_t *hash, int hash_len);
void omap3_crypto_hash_process_dma(int id, uint8_t *buf, int buf_len, uint8_t *hash, int hash_len,
		omap3_crypto_dma_callback cb, unsigned long data);

/*=========================== SCRATCH ALLOCATION FOR DMA ==================== */

void *omap3_crypto_scratch_alloc(int len);
void omap3_crypto_scratch_free(void *ptr, int len);

/*============================================================================ */

/* allocate algorithms of types in array alg_list (terminated by -1)
   and returns ids in id_list (terminated by -1) */
int omap3_crypto_alloc(int *alg_list, int *id_list);

/* free algorithm ids in id_list (terminated by -1) */
int omap3_crypto_free(int *id_list);

int omap3_crypto_init(int debug_flag);

void omap3_crypto_exit(void);

#endif // _OMAP3_CRYPTO_H_

