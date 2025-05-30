/*
 * cam_os_util.h- Sigmastar
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

#ifndef __CAM_OS_UTIL_H__
#define __CAM_OS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FORCE_INLINE __attribute__((always_inline)) inline

#ifdef __compiler_offsetof
#define CAM_OS_OFFSETOF(TYPE, MEMBER) __compiler_offsetof(TYPE, MEMBER)
#else
#ifdef size_t
#define CAM_OS_OFFSETOF(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#else
#define CAM_OS_OFFSETOF(TYPE, MEMBER) ((unsigned long)&((TYPE *)0)->MEMBER)
#endif
#endif

#define CAM_OS_CONTAINER_OF(ptr, type, member)                  \
    (                                                           \
        {                                                       \
            void *__mptr = (void *)(ptr);                       \
            ((type *)(__mptr - CAM_OS_OFFSETOF(type, member))); \
        })

#ifndef likely
#define CAM_OS_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define CAM_OS_LIKELY(x) likely(x)
#endif

#ifndef unlikely
#define CAM_OS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CAM_OS_UNLIKELY(x) unlikely(x)
#endif

static FORCE_INLINE s32 CAM_OS_FLS(s32 x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }
    return r;
}

#if CAM_OS_BITS_PER_LONG == 32
static FORCE_INLINE s32 CAM_OS_FLS64(u64 x)
{
    u32 h = x >> 32;
    if (h)
        return CAM_OS_FLS(h) + 32;
    return CAM_OS_FLS(x);
}
#elif CAM_OS_BITS_PER_LONG == 64
static FORCE_INLINE s32 _CAM_OS_FLS(u64 word)
{
    s32 num = CAM_OS_BITS_PER_LONG - 1;

#if CAM_OS_BITS_PER_LONG == 64
    if (!(word & (~0ul << 32)))
    {
        num -= 32;
        word <<= 32;
    }
#endif
    if (!(word & (~0ul << (CAM_OS_BITS_PER_LONG - 16))))
    {
        num -= 16;
        word <<= 16;
    }
    if (!(word & (~0ul << (CAM_OS_BITS_PER_LONG - 8))))
    {
        num -= 8;
        word <<= 8;
    }
    if (!(word & (~0ul << (CAM_OS_BITS_PER_LONG - 4))))
    {
        num -= 4;
        word <<= 4;
    }
    if (!(word & (~0ul << (CAM_OS_BITS_PER_LONG - 2))))
    {
        num -= 2;
        word <<= 2;
    }
    if (!(word & (~0ul << (CAM_OS_BITS_PER_LONG - 1))))
        num -= 1;
    return num;
}

static FORCE_INLINE s32 CAM_OS_FLS64(u64 x)
{
    if (x == 0)
        return 0;
    return _CAM_OS_FLS(x) + 1;
}
#else
#error CAM_OS_BITS_PER_LONG not 32 or 64
#endif

#define CAM_OS_ARRAY_SIZE(arr)      \
    (sizeof(arr) / sizeof((arr)[0]) \
     + ((int)(sizeof(struct { int : (-!!(__builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0])))); }))))

#define CAM_OS_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CAM_OS_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define CAM_OS_ALIGN_DOWN(val, alignment) (((val) / (alignment)) * (alignment))
#define CAM_OS_ALIGN_UP(val, alignment)   ((((val) + (alignment)-1) / (alignment)) * (alignment))

#define TYPECHECK(type, x)        \
    (                             \
        {                         \
            type      __a;        \
            typeof(x) __b;        \
            (void)(&__a == &__b); \
            1;                    \
        })

#define CAM_OS_TIME_AFTER(a, b)  (TYPECHECK(u64, a) && TYPECHECK(u64, b) && ((s64)((b) - (a)) < 0))
#define CAM_OS_TIME_BEFORE(a, b) CAM_OS_TIME_AFTER(b, a)

#define CAM_OS_TIME_AFTER_EQ(a, b)  (TYPECHECK(u64, a) && TYPECHECK(u64, b) && ((s64)((a) - (b)) >= 0))
#define CAM_OS_TIME_BEFORE_EQ(a, b) CAM_OS_TIME_AFTER_EQ(b, a)

//=============================================================================
// Description:
//      Perform arithmetic and check for overflow
//      Support type: u8/s8/u16/s16/u32/s32/u64/s64
//      Example: i=i+1  ->  CAM_OS_ADD_CHK(i, 1, &i)
//                              u8: 255 + 1 = 0
//                              s8: 127 + 1 = -128
//                              u16: 65535 + 1 = 0
//                              s16: 32767 + 1 = -32768
//                              u32: 4294967295 + 1 = 0
//                              s32: 2147483647 + 1 = -2147483648
//                              u64: 18446744073709551615 + 1 = 0
//                              s64: 9223372036854775807 + 1 = -9223372036854775808
//               j=i-1  ->  CAM_OS_SUB_CHK(i, 1, &j)
//                              u8: 0 - 1 = 255
//                              s8: -128 - 1 = 127
//                              u16: 0 - 1 = 65535
//                              s16: -32768 - 1 = 32767
//                              u32: 0 - 1 = 4294967295
//                              s32: -2147483648 - 1 = 2147483647
//                              u64: 0 - 1 = 18446744073709551615
//                              s64: -9223372036854775808 - 1 = 9223372036854775807
//               k=i*2  ->  CAM_OS_MUL_CHK(i, 2, &k)
// Parameters:
//      [in]  A: First operands
//      [in]  B: Second operands
//      [in]  R: Pointer of result
// Return:
//      Return true if overflow occurs, otherwise return false.
//=============================================================================
#define CAM_OS_ADD_CHK(A, B, R) __builtin_add_overflow((A), (B), (R))
#define CAM_OS_SUB_CHK(A, B, R) __builtin_sub_overflow((A), (B), (R))
#define CAM_OS_MUL_CHK(A, B, R) __builtin_mul_overflow((A), (B), (R))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_UTIL_H__
