/*
 * cam_os_util_list.h- Sigmastar
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

#ifndef __CAM_OS_UTIL_LIST_H__
#define __CAM_OS_UTIL_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// List API
struct CamOsListHead_t
{
    struct CamOsListHead_t *pNext, *pPrev;
};

#define CAM_OS_POISON_POINTER_DELTA 0
#define CAM_OS_LIST_POISON1         (void *)(0x100 + CAM_OS_POISON_POINTER_DELTA)
#define CAM_OS_LIST_POISON2         (void *)(0x200 + CAM_OS_POISON_POINTER_DELTA)

#define CAM_OS_LIST_HEAD_INIT(name) \
    {                               \
        &(name), &(name)            \
    }

#define CAM_OS_LIST_HEAD(name) struct CamOsListHead_t name = CAM_OS_LIST_HEAD_INIT(name)

#define CAM_OS_LIST_ENTRY(ptr, type, member) CAM_OS_CONTAINER_OF(ptr, type, member)

#define CAM_OS_LIST_FOR_EACH(pos, head) for (pos = (head)->pNext; pos != (head); pos = pos->pNext)

#define CAM_OS_LIST_FOR_EACH_SAFE(pos, n, head) \
    for (pos = (head)->pNext, n = pos->pNext; pos != (head); pos = n, n = pos->pNext)

#define CAM_OS_LIST_FOR_EACH_SAFE_INVERSE(pos, n, head) \
    for (pos = (head)->pPrev, n = pos->pPrev; pos != (head); pos = n, n = pos->pPrev)

#define CAM_OS_LIST_FOR_EACH_PREV(pos, head) for (pos = (head)->pPrev; pos != (head); pos = pos->pPrev)

#define CAM_OS_LIST_FOR_EACH_PREV_SAFE(pos, n, head) \
    for (pos = (head)->pPrev, n = pos->pPrev; pos != (head); pos = n, n = pos->pPrev)

#define CAM_OS_LIST_PREV_ENTRY(pos, member) CAM_OS_LIST_ENTRY((pos)->member.pPrev, typeof(*(pos)), member)

#define CAM_OS_LIST_FIRST_ENTRY(ptr, type, member) CAM_OS_LIST_ENTRY((ptr)->pNext, type, member)

#define CAM_OS_LIST_LAST_ENTRY(ptr, type, member) CAM_OS_LIST_ENTRY((ptr)->pPrev, type, member)

#define CAM_OS_LIST_NEXT_ENTRY(pos, member) CAM_OS_LIST_ENTRY((pos)->member.pNext, __typeof__(*(pos)), member)

#define CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)                                                    \
    for (pos = CAM_OS_LIST_FIRST_ENTRY(head, __typeof__(*pos), member), n = CAM_OS_LIST_NEXT_ENTRY(pos, member); \
         &pos->member != (head); pos = n, n = CAM_OS_LIST_NEXT_ENTRY(n, member))

#define CAM_OS_LIST_FOR_EACH_ENTRY(pos, head, member)                                           \
    for (pos = CAM_OS_LIST_FIRST_ENTRY(head, __typeof__(*pos), member); &pos->member != (head); \
         pos = CAM_OS_LIST_NEXT_ENTRY(pos, member))

#define CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)                                                    \
    for (pos = CAM_OS_LIST_FIRST_ENTRY(head, __typeof__(*pos), member), n = CAM_OS_LIST_NEXT_ENTRY(pos, member); \
         &pos->member != (head); pos = n, n = CAM_OS_LIST_NEXT_ENTRY(n, member))

#define CAM_OS_LIST_FOR_EACH_ENTRY_REVERSE(pos, head, member)                              \
    for (pos = CAM_OS_LIST_LAST_ENTRY(head, typeof(*pos), member); &pos->member != (head); \
         pos = CAM_OS_LIST_PREV_ENTRY(pos, member))

#define CAM_OS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, n, head, member)                                       \
    for (pos = CAM_OS_LIST_LAST_ENTRY(head, typeof(*pos), member), n = CAM_OS_LIST_PREV_ENTRY(pos, member); \
         &pos->member != (head); pos = n, n = CAM_OS_LIST_PREV_ENTRY(n, member))

static inline void CAM_OS_INIT_LIST_HEAD(struct CamOsListHead_t *pList)
{
    pList->pNext = pList;
    pList->pPrev = pList;
}

static inline void _CAM_OS_LIST_ADD(struct CamOsListHead_t *pNew, struct CamOsListHead_t *pPrev,
                                    struct CamOsListHead_t *pNext)
{
    pNext->pPrev = pNew;
    pNew->pNext  = pNext;
    pNew->pPrev  = pPrev;
    pPrev->pNext = pNew;
}

static inline void CAM_OS_LIST_ADD(struct CamOsListHead_t *pNew, struct CamOsListHead_t *head)
{
    _CAM_OS_LIST_ADD(pNew, head, head->pNext);
}

static inline void CAM_OS_LIST_ADD_TAIL(struct CamOsListHead_t *pNew, struct CamOsListHead_t *head)
{
    _CAM_OS_LIST_ADD(pNew, head->pPrev, head);
}

static inline void _CAM_OS_LIST_DEL(struct CamOsListHead_t *pPrev, struct CamOsListHead_t *pNext)
{
    pNext->pPrev = pPrev;
    pPrev->pNext = pNext;
}

static inline void _CAM_OS_LIST_DEL_ENTRY(struct CamOsListHead_t *entry)
{
    _CAM_OS_LIST_DEL(entry->pPrev, entry->pNext);
}

static inline void CAM_OS_LIST_DEL(struct CamOsListHead_t *pEntry)
{
    _CAM_OS_LIST_DEL(pEntry->pPrev, pEntry->pNext);
    pEntry->pNext = (struct CamOsListHead_t *)CAM_OS_LIST_POISON1;
    pEntry->pPrev = (struct CamOsListHead_t *)CAM_OS_LIST_POISON2;
}

static inline void CAM_OS_LIST_DEL_INIT(struct CamOsListHead_t *entry)
{
    _CAM_OS_LIST_DEL_ENTRY(entry);
    CAM_OS_INIT_LIST_HEAD(entry);
}

static inline int CAM_OS_LIST_IS_LAST(const struct CamOsListHead_t *list, const struct CamOsListHead_t *head)
{
    return list->pNext == head;
}

static inline int CAM_OS_LIST_EMPTY(const struct CamOsListHead_t *head)
{
    return head->pNext == head;
}

static inline int CAM_OS_LIST_EMPTY_CAREFUL(const struct CamOsListHead_t *head)
{
    struct CamOsListHead_t *pNext = head->pNext;
    return (pNext == head) && (pNext == head->pPrev);
}

void CamOsListSort(void *priv, struct CamOsListHead_t *head,
                   int (*cmp)(void *priv, struct CamOsListHead_t *a, struct CamOsListHead_t *b));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
// HList API
static FORCE_INLINE void _CAM_OS_READ_ONCE_SIZE(const volatile void *p, void *res, int size)
{
    switch (size)
    {
        case 1:
            *(u8 *)res = *(volatile u8 *)p;
            break;
        case 2:
            *(u16 *)res = *(volatile u16 *)p;
            break;
        case 4:
            *(u32 *)res = *(volatile u32 *)p;
            break;
        case 8:
            *(u64 *)res = *(volatile u64 *)p;
            break;
        default:
            __sync_synchronize();
            __builtin_memcpy((void *)res, (const void *)p, size);
            __sync_synchronize();
    }
}
#pragma GCC diagnostic pop

#define CAM_OS_READ_ONCE(x)                                   \
    (                                                         \
        {                                                     \
            union                                             \
            {                                                 \
                __typeof__(x) __val;                          \
                char          __c[1];                         \
            } __u = {0};                                      \
            _CAM_OS_READ_ONCE_SIZE(&(x), __u.__c, sizeof(x)); \
            __u.__val;                                        \
        })

#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wcast-qual"
static FORCE_INLINE void _CAM_OS_WRITE_ONCE_SIZE(volatile void *p, void *res, int size)
{
    switch (size)
    {
        case 1:
            *(volatile u8 *)p = *(u8 *)res;
            break;
        case 2:
            *(volatile u16 *)p = *(u16 *)res;
            break;
        case 4:
            *(volatile u32 *)p = *(u32 *)res;
            break;
        case 8:
            *(volatile u64 *)p = *(u64 *)res;
            break;
        default:
            __sync_synchronize();
            __builtin_memcpy((void *)p, (const void *)res, size);
            __sync_synchronize();
    }
}
#pragma GCC diagnostic pop

#define CAM_OS_WRITE_ONCE(x, val)                                \
    (                                                            \
        {                                                        \
            union                                                \
            {                                                    \
                struct CamOsHListNode_t *__val;                  \
                char                     __c[1];                 \
            } __u = {.__val = (struct CamOsHListNode_t *)(val)}; \
            _CAM_OS_WRITE_ONCE_SIZE(&(x), __u.__c, sizeof(x));   \
            __u.__val;                                           \
        })

#define CAM_OS_GOLDEN_RATIO_32 0x61C88647
#define CAM_OS_GOLDEN_RATIO_64 0x61C8864680B583EBull

#if CAM_OS_BITS_PER_LONG == 32
#define CAM_OS_GOLDEN_RATIO_PRIME   CAM_OS_GOLDEN_RATIO_32
#define CAM_OS_HASH_LONG(val, bits) CAM_OS_HASH_32(val, bits)
#elif CAM_OS_BITS_PER_LONG == 64
#define CAM_OS_HASH_LONG(val, bits) CAM_OS_HASH_64(val, bits)
#define CAM_OS_GOLDEN_RATIO_PRIME   CAM_OS_GOLDEN_RATIO_64
#else
#error CAM_OS_BITS_PER_LONG not 32 or 64
#endif

static inline u32 _CAM_OS_HASH_32(u32 val)
{
    return val * CAM_OS_GOLDEN_RATIO_32;
}

static inline u32 CAM_OS_HASH_32(u32 val, u32 bits)
{
    /* High bits are more random, so use them. */
    return _CAM_OS_HASH_32(val) >> (32 - bits);
}

static FORCE_INLINE u64 CAM_OS_HASH_64(u64 val, u32 bits)
{
#if CAM_OS_BITS_PER_LONG == 64
    /* 64x64-bit multiply is efficient on all 64-bit processors */
    return val * CAM_OS_GOLDEN_RATIO_64 >> (64 - bits);
#else
    /* Hash 64 bits using only 32x32-bit multiply. */
    return CAM_OS_HASH_32((u32)val ^ _CAM_OS_HASH_32(val >> 32), bits);
#endif
}

struct CamOsHListHead_t
{
    struct CamOsHListNode_t *pFirst;
};

struct CamOsHListNode_t
{
    struct CamOsHListNode_t *pNext, **ppPrev;
};

#define CAM_OS_HLIST_HEAD_INIT \
    {                          \
        .pFirst = NULL         \
    }
#define CAM_OS_HLIST_HEAD(name)     struct CamOsHListHead_t name = {.pFirst = NULL}
#define CAM_OS_INIT_HLIST_HEAD(ptr) ((ptr)->pFirst = NULL)

#define CAM_OS_HASH_MIN(val, bits) (sizeof(val) <= 4 ? CAM_OS_HASH_32(val, bits) : CAM_OS_HASH_LONG(val, bits))

static inline void CAM_OS_INIT_HLIST_NODE(struct CamOsHListNode_t *h)
{
    h->pNext  = NULL;
    h->ppPrev = NULL;
}

static inline int CAM_OS_HLIST_UNHASHED(const struct CamOsHListNode_t *h)
{
    return !h->ppPrev;
}

static inline int CAM_OS_HLIST_EMPTY(const struct CamOsHListHead_t *h)
{
    return !CAM_OS_READ_ONCE(h->pFirst);
}

static inline void _CAM_OS_HLIST_DEL(struct CamOsHListNode_t *n)
{
    struct CamOsHListNode_t * pNext  = n->pNext;
    struct CamOsHListNode_t **ppPrev = n->ppPrev;

    CAM_OS_WRITE_ONCE(*ppPrev, pNext);
    if (pNext)
        pNext->ppPrev = ppPrev;
}

static inline void CAM_OS_HLIST_DEL(struct CamOsHListNode_t *n)
{
    _CAM_OS_HLIST_DEL(n);
    n->pNext  = (struct CamOsHListNode_t *)CAM_OS_LIST_POISON1;
    n->ppPrev = (struct CamOsHListNode_t **)CAM_OS_LIST_POISON2;
}

static inline void CAM_OS_HLIST_DEL_INIT(struct CamOsHListNode_t *n)
{
    if (!CAM_OS_HLIST_UNHASHED(n))
    {
        _CAM_OS_HLIST_DEL(n);
        CAM_OS_INIT_HLIST_NODE(n);
    }
}

static inline void CAM_OS_HLIST_ADD_HEAD(struct CamOsHListNode_t *n, struct CamOsHListHead_t *h)
{
    struct CamOsHListNode_t *pFirst = h->pFirst;
    n->pNext                        = pFirst;
    if (pFirst)
        pFirst->ppPrev = &n->pNext;
    CAM_OS_WRITE_ONCE(h->pFirst, n);
    n->ppPrev = &h->pFirst;
}

#define CAM_OS_HLIST_ENTRY(ptr, type, member) CAM_OS_CONTAINER_OF(ptr, type, member)

#define CAM_OS_HLIST_ENTRY_SAFE(ptr, type, member)                      \
    (                                                                   \
        {                                                               \
            __typeof__(ptr) ____ptr = (ptr);                            \
            ____ptr ? CAM_OS_HLIST_ENTRY(____ptr, type, member) : NULL; \
        })

#define CAM_OS_HLIST_FOR_EACH_ENTRY(pos, head, member)                                   \
    for (pos = CAM_OS_HLIST_ENTRY_SAFE((head)->pFirst, __typeof__(*(pos)), member); pos; \
         pos = CAM_OS_HLIST_ENTRY_SAFE((pos)->member.pNext, __typeof__(*(pos)), member))

#define CAM_OS_HLIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)                                                       \
    for (pos = CAM_OS_HLIST_ENTRY_SAFE((head)->pFirst, __typeof__(*(pos)), member); pos                              \
                                                                                    && (                             \
                                                                                        {                            \
                                                                                            n = (pos)->member.pNext; \
                                                                                            1;                       \
                                                                                        });                          \
         pos = CAM_OS_HLIST_ENTRY_SAFE(n, __typeof__(*(pos)), member))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_UTIL_LIST_H__
