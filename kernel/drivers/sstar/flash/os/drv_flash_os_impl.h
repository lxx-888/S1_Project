/*
 * drv_flash_os_impl.h- Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifndef _DRV_FLASH_OS_IMPL_H_
#define _DRV_FLASH_OS_IMPL_H_

#include <linux/string.h>
#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <ms_platform.h>
#include <cam_os_wrapper.h>
#include <hal_bdma.h>

#if defined(CONFIG_SSTAR_XZDEC)
#define CONFIG_FLASH_XZDEC
#endif
#if defined(CONFIG_SSTAR_SOC_ECC)
#define CONFIG_FLASH_SOC_ECC
#endif

#if defined(CONFIG_SSTAR_FLASH_WRITE_BACK)
#define CONFIG_FLASH_WRITE_BACK
#endif

//#define CONFIG_FLASH_FIX_BDMA_ALIGN
#define CONFIG_MIU_ADDR_ALIGN       0x10
#define CONFIG_FLASH_ADDR_ALIGN     0x10
#define CONFIG_FLASH_SIZE_ALIGN     0x10
#define CONFIG_ALIGN_BIT            4
#define CONFIG_FLASH_CACHELINE_SIZE 0x40

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

#define FLASH_IMPL_CRC32(x, y, z)    (crc32(~x, y, z) ^ 0xFFFFFFFF)
#define FLASH_IMPL_USDELAY(x)        udelay(x)
#define FLASH_IMPL_IO_ADDRESS(_reg_) IO_ADDRESS(_reg_)
#define FLASH_IMPL_UNUSED_VAR(x)     (x = x)
#define FLASH_IMPL_CACHE_LINE_ALIGN_UP(x) \
    ((x + (CONFIG_FLASH_CACHELINE_SIZE - 1)) & (~(CONFIG_FLASH_CACHELINE_SIZE - 1)))
#define FLASH_IMPL_CACHE_LINE_ALIGN_DOWN(x) (x & ~(CONFIG_FLASH_CACHELINE_SIZE - 1))

typedef enum
{
    FLASH_BOOT_STORAGE_NAND = 0,
    FLASH_BOOT_STORAGE_NOR,
    FLASH_BOOT_STORAGE_OTHER,
} flash_boot_storage;

typedef enum
{
    FLASH_BDMA_SUCCESS = 0,
    FLASH_BDMA_NOSUPPORT,
    FLASH_BDMA_TIMEOUT
} flash_bdma_err;

typedef enum
{
    FLASH_BDMA_SPI2MIU = 0,
    FLASH_BDMA_MIU2SPI,
    FLASH_BDMA_SPI2XZDEC,
    FLASH_BDMA_PMSPI2MIU,
    FLASH_BDMA_MIU2PMSPI,
    FLASH_BDMA_PMSPI2XZDEC,
    FLASH_BDMA_MIU2XZDEC
} flash_bdma_path;

struct flash_bdma_param
{
    u8              interrupt_en;
    flash_bdma_path path;
    u32             size;
    u8 *            src;
    u8 *            dst;
    void *          callback;
    void *          callback_parm;
    hal_bdma_param  bdma_param;
};

u8  flash_impl_count_bits(u32 x);
u32 flash_impl_checksum(u8 *data, u32 size);

void flash_impl_printf(const char *data);
void flash_impl_printf_hex(const char *data, u32 value, const char *data2);
void flash_impl_show_id(u8 *data, u8 cnt);

u64  flash_impl_virt_to_phys(void *virt);
u64  flash_impl_phys_to_miu(u64 phys);
void flash_impl_miupipe_flush(void);
void flash_impl_mem_flush(void *data, u32 size);
void flash_impl_mem_invalidate(void *data, u32 size);

u8    flash_impl_memcmp(u8 *cs, const u8 *ct, u8 size);
void *flash_impl_memcpy(u8 *dst, const u8 *src, u32 size);
void *flash_impl_memset(u8 *s, u32 c, u32 size);
u32   flash_impl_strcmp(const u8 *cs, const u8 *ct);

u32 flash_impl_get_ipl_size(u8 *buf);
u8  flash_impl_get_boot_storage(void);

u8  flash_impl_bdma_transfer(struct flash_bdma_param *param);
u8  flash_impl_bit_change(u8 data);
u32 flash_impl_bdma_sw_crc32(u8 flag, u32 crc, u32 poly, u8 *buf, u32 size);

MODULE_LICENSE("GPL");
#endif /* _DRV_FLASH_OS_IMPL_H_ */
