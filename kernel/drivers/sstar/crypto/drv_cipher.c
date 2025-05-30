/*
 * drv_cipher.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <crypto/aes.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <ms_platform.h>
#include <ms_msys.h>
#include "drv_aes.h"
#include "drv_rsa.h"
#include "drv_cipher.h"

/*
 * Options
 */
#define CIPHER_DEBUG (1)
#if (CIPHER_DEBUG == 1)
#define CIPHER_DBG(fmt, arg...) printk(KERN_ALERT fmt, ##arg) // KERN_DEBUG KERN_ALERT KERN_WARNING
#else
#define CIPHER_DBG(fmt, arg...)
#endif

/*
 * Data types
 */
typedef enum
{
    E_AES_DIR_DECRYPT = 0,
    E_AES_DIR_ENCRYPT,
} AES_DIR;

/*
 * External
 */
extern int  sstar_sha_init(u8 sha256_mode);
extern int  sstar_sha_update(u64 in, u32 len, u32 *state, u32 count, u8 sha_mode);
extern int  sstar_sha_final(void);
extern void enableClock(void);
extern void disableClock(void);
extern void enableRngClock(void);
extern void disableRngClock(void);
/*
 * Local
 */
static struct mutex _mtcipher_lock;
static u8           _u8AesRefcnt = 0;

/********************************************************/
/*                                                      */
/* AES functions                                        */
/*                                                      */
/********************************************************/
int cipher_aes_init(void)
{
    if (_u8AesRefcnt == 0)
    {
        mutex_init(&_mtcipher_lock);
        _u8AesRefcnt++;
    }
    return 0;
}

EXPORT_SYMBOL(cipher_aes_init);

int cipher_aes_uninit(void)
{
    if (_u8AesRefcnt)
    {
        _u8AesRefcnt--;
        if (_u8AesRefcnt == 0)
        {
            mutex_destroy(&_mtcipher_lock);
        }
    }
    return 0;
}

EXPORT_SYMBOL(cipher_aes_uninit);

static int cipher_aes_crypto(MDRV_AES_HANDLE *handle, AES_DIR dir)
{
    int                 ret = 0;
    struct sstar_aes_op op;

    enableClock();
#ifdef CONFIG_SSTAR_RNG
    enableRngClock();
#endif
    op.len = handle->len;
    op.dir = (dir == E_AES_DIR_DECRYPT) ? AES_DIR_DECRYPT : AES_DIR_ENCRYPT;
    op.iv  = handle->iv;
    memcpy(op.key, handle->key, handle->keylen);
    op.keylen = handle->keylen;
    switch (handle->mode)
    {
        case E_AES_ALGO_ECB:
            op.mode = AES_MODE_ECB;
            break;
        case E_AES_ALGO_CBC:
            op.mode = AES_MODE_CBC;
            break;
        case E_AES_ALGO_CTR:
            op.mode = AES_MODE_CTR;
            break;
    }
    Chip_Flush_Cache_Range((void *)handle->in, handle->len);
    Chip_Inv_Cache_Range((void *)handle->out, handle->len);
    ret = sstar_aes_crypt_pub(&op, (u64)handle->in_pa, (u64)handle->out_pa);
    disableClock();
#ifdef CONFIG_SSTAR_RNG
    disableRngClock();
#endif
    if (ret != handle->len)
    {
        CIPHER_DBG("aes crypto failed %d\n", ret);
    }
    return ret;
}

/* [ecb] no iv */
static int cipher_ecb_crypto(MDRV_AES_HANDLE *handle, AES_DIR dir)
{
    int ret  = 0;
    u32 left = handle->len;

    while (left)
    {
        handle->len = left;
        ret         = cipher_aes_crypto(handle, dir);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] ecb crypto err %d\n", ret);
            return ret;
        }
        left -= ret;
    }
    return 0;
}

/* [cbc] encrypt */
static int cipher_cbc_encrypt(MDRV_AES_HANDLE *handle)
{
    int ret  = 0;
    u32 left = handle->len;

    while (left)
    {
        handle->len = left;
        ret         = cipher_aes_crypto(handle, E_AES_DIR_ENCRYPT);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] cbc encrypt err %d\n", ret);
            return ret;
        }
        else if (ret > 0)
        {
            memcpy(handle->iv, (handle->out + (handle->len) - 16), 16);
            left -= ret;
        }
    }
    return 0;
}

/* [cbc] decrypt */
static int cipher_cbc_decrypt(MDRV_AES_HANDLE *handle)
{
    int ret  = 0;
    u32 left = handle->len;
    u8  temp_iv[AES_BLOCK_SIZE];

    while (left)
    {
        handle->len = left;
        memcpy(temp_iv, handle->in + (handle->len) - 16, 16);
        ret = cipher_aes_crypto(handle, E_AES_DIR_DECRYPT);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] cbc decrypt err %d\n", ret);
            return ret;
        }
        else if (ret > 0)
        {
            memcpy(handle->iv, temp_iv, 16);
            left -= ret;
        }
    }
    return 0;
}

/* [ctr] decrypt = encrypt */
static int cipher_ctr_crypto(MDRV_AES_HANDLE *handle)
{
    int ret     = 0;
    int counter = 0, n = 0;
    u32 left = handle->len;
    u32 temp_iv[AES_KEYSIZE_128 >> 2];

    while (left)
    {
        handle->len = left;
        ret         = cipher_aes_crypto(handle, E_AES_DIR_ENCRYPT);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] ctr crypto err %d\n", ret);
            return ret;
        }
        else if (ret > 0)
        {
            memcpy(temp_iv, handle->iv, AES_BLOCK_SIZE);
            for (n = 0; n <= 3; n++)
            {
                temp_iv[n] = be32_to_cpu(temp_iv[n]);
            }
            counter = handle->len >> 4;
            // iv=iv+1;
            while (counter)
            {
                temp_iv[3] = temp_iv[3] + 1;
                if (temp_iv[3] == 0x0)
                {
                    temp_iv[2] = temp_iv[2] + 1;
                    if (temp_iv[2] == 0x0)
                    {
                        temp_iv[1] = temp_iv[1] + 1;
                        if (temp_iv[1] == 0x0)
                        {
                            temp_iv[0] = temp_iv[0] + 1;
                            if (temp_iv[0] == 0x0)
                            {
                                CIPHER_DBG("IV counter overflow!\n");
                            }
                        }
                    }
                }
                counter--;
            }
            for (n = 0; n <= 3; n++)
            {
                temp_iv[n] = cpu_to_be32(temp_iv[n]);
            }
            memcpy(handle->iv, temp_iv, AES_BLOCK_SIZE);
        }
        left -= ret;
    }
    return 0;
}

int cipher_aes_encrypt(MDRV_AES_HANDLE *handle)
{
    int ret = 0;

    mutex_lock(&_mtcipher_lock);
    switch (handle->mode)
    {
        case E_AES_ALGO_ECB:
            ret = cipher_ecb_crypto(handle, AES_DIR_ENCRYPT);
            break;
        case E_AES_ALGO_CBC:
            ret = cipher_cbc_encrypt(handle);
            break;
        case E_AES_ALGO_CTR:
            ret = cipher_ctr_crypto(handle);
            break;
    }
    mutex_unlock(&_mtcipher_lock);
    return ret;
}

EXPORT_SYMBOL(cipher_aes_encrypt);

int cipher_aes_decrypt(MDRV_AES_HANDLE *handle)
{
    int ret = 0;

    mutex_lock(&_mtcipher_lock);
    switch (handle->mode)
    {
        case E_AES_ALGO_ECB:
            ret = cipher_ecb_crypto(handle, AES_DIR_DECRYPT);
            break;
        case E_AES_ALGO_CBC:
            ret = cipher_cbc_decrypt(handle);
            break;
        case E_AES_ALGO_CTR:
            ret = cipher_ctr_crypto(handle);
            break;
    }
    mutex_unlock(&_mtcipher_lock);
    return ret;
}

EXPORT_SYMBOL(cipher_aes_decrypt);

/********************************************************/
/*                                                      */
/* RSA functions                                        */
/*                                                      */
/********************************************************/
int cipher_rsa_crypto(MDRV_RSA_HANDLE *handle)
{
    struct rsa_config op;
    int               ret = 0;
    if ((handle->mod_len != 0x40) && (handle->mod_len != 0x80) && (handle->mod_len != 0x100)
        && (handle->mod_len != 0x200))
    {
        CIPHER_DBG("[RSA] KenNLen %d unsupported\n", handle->mod_len);
        return -1;
    }
    enableClock();
    op.pu32RSA_KeyE   = handle->exp;
    op.pu32RSA_KeyN   = handle->modulus;
    op.u32RSA_KeyELen = handle->exp_len;
    op.u32RSA_KeyNLen = handle->mod_len;
    op.pu32RSA_Output = handle->out;
    op.pu32RSA_Sig    = handle->in;
    op.u32RSA_SigLen  = handle->len;
    op.u8RSA_pub_ekey = handle->pub_ekey;
    ret               = rsa_crypto(&op);
    disableClock();
    return ret;
}

EXPORT_SYMBOL(cipher_rsa_crypto);

/********************************************************/
/*                                                      */
/* SHA functions                                        */
/*                                                      */
/********************************************************/
int cipher_sha_init(MDRV_SHA_HANDLE *handle)
{
    u8 sha256 = 1;

    handle->ctx.count = 0;
    if ((handle->mode == E_SHA_1) || (handle->mode == E_SHA_1_ONCE))
    {
        sha256 = 0;
    }
    return sstar_sha_init(sha256);
}

EXPORT_SYMBOL(cipher_sha_init);

int cipher_sha_update(MDRV_SHA_HANDLE *handle)
{
    int ret;

    enableClock();
    ret = sstar_sha_update(Chip_Phys_to_MIU(handle->u32DataPhy), handle->u32DataLen, handle->ctx.state,
                           handle->ctx.count, handle->mode);
    disableClock();
    if (ret == 0)
    {
        handle->ctx.count += handle->u32DataLen;
    }
    return ret;
}

EXPORT_SYMBOL(cipher_sha_update);

int cipher_sha_final(MDRV_SHA_HANDLE *handle)
{
    u8  i = 0, count = 32;
    u8 *hash  = (u8 *)handle->u32ShaVal;
    u8 *state = (u8 *)handle->ctx.state;

    if ((handle->mode == E_SHA_1) || (handle->mode == E_SHA_1_ONCE))
    {
        count = 20;
    }
    for (i = 0; i < count; i++)
    {
        hash[i] = state[31 - i];
    }
    return sstar_sha_final();
}

EXPORT_SYMBOL(cipher_sha_final);

u32 cipher_random_num(void)
{
    return sstar_random();
}

EXPORT_SYMBOL(cipher_random_num);
