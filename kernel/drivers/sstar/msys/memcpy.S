/*
* memcpy.S- Sigmastar
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

#include "arm.include"
#include "macro.include"

#if 1 //defined(__ARM_NEON__) && !defined(ARCH_ARM_USE_NON_NEON_MEMCPY)

.text
.fpu    neon

#ifdef HAVE_32_BYTE_CACHE_LINE
/* a prefetch distance of 2 cache-lines */
#define CACHE_LINE_SIZE     32
#else
/* a prefetch distance of 4 cache-lines works best experimentally */
#define CACHE_LINE_SIZE     64
#endif

ENTRY(_bench_neon_memcpy)
        .save       {r0, lr}
        /* start preloading as early as possible */
        pld         [r1, #(CACHE_LINE_SIZE * 0)]
        stmfd       sp!, {r0, lr}
        pld         [r1, #(CACHE_LINE_SIZE * 1)]

/* If Neon supports unaligned access then remove the align code,
 * unless a size limit has been specified.
 */
#ifndef NEON_UNALIGNED_ACCESS
        /* do we have at least 16-bytes to copy (needed for alignment below) */
        cmp         r2, #16
        blo         5f

        /* check if buffers are aligned. If so, run arm-only version */
        eor         r3, r0, r1
        ands        r3, r3, #0x3
        beq         11f

        /* align destination to cache-line for the write-buffer */
        rsb         r3, r0, #0
        ands        r3, r3, #0xF
        beq         2f

        /* copy up to 15-bytes (count in r3) */
        sub         r2, r2, r3
        movs        ip, r3, lsl #31
        ldrmib      lr, [r1], #1
        strmib      lr, [r0], #1
        ldrcsb      ip, [r1], #1
        ldrcsb      lr, [r1], #1
        strcsb      ip, [r0], #1
        strcsb      lr, [r0], #1
        movs        ip, r3, lsl #29
        bge         1f
        // copies 4 bytes, destination 32-bits aligned
        vld4.8      {d0[0], d1[0], d2[0], d3[0]}, [r1]!
        vst4.8      {d0[0], d1[0], d2[0], d3[0]}, [r0, :32]!
1:      bcc         2f
        // copies 8 bytes, destination 64-bits aligned
        vld1.8      {d0}, [r1]!
        vst1.8      {d0}, [r0, :64]!
2:
        /* preload immediately the next cache line, which we may need */
        pld         [r1, #(CACHE_LINE_SIZE * 0)]
        pld         [r1, #(CACHE_LINE_SIZE * 1)]

#ifdef HAVE_32_BYTE_CACHE_LINE
        /* make sure we have at least 32 bytes to copy */
        subs        r2, r2, #32
        blo         4f

        /* preload all the cache lines we need.
         * NOTE: the number of pld below depends on PREFETCH_DISTANCE,
         * ideally would would increase the distance in the main loop to
         * avoid the goofy code below. In practice this doesn't seem to make
         * a big difference.
         */
        pld         [r1, #(PREFETCH_DISTANCE)]

1:      /* The main loop copies 32 bytes at a time */
        vld1.8      {d0  - d3},   [r1]!
        pld         [r1, #(PREFETCH_DISTANCE)]
        subs        r2, r2, #32
        vst1.8      {d0  - d3},   [r0, :128]!
        bhs         1b
#else
        /* make sure we have at least 64 bytes to copy */
        subs        r2, r2, #64
        blo         2f

        /* preload all the cache lines we need. */
        pld         [r1, #(CACHE_LINE_SIZE * 2)]
        pld         [r1, #(CACHE_LINE_SIZE * 3)]

1:      /* The main loop copies 64 bytes at a time */
        vld1.8      {d0 - d3}, [r1]!
        vld1.8      {d4 - d7}, [r1]!
#ifdef  HAVE_32_BYTE_CACHE_LINE
        pld         [r1, #(CACHE_LINE_SIZE * 2)]
        pld         [r1, #(CACHE_LINE_SIZE * 3)]
#else
        pld         [r1, #(CACHE_LINE_SIZE * 3)]
#endif
        subs        r2, r2, #64
        vst1.8      {d0 - d3}, [r0, :128]!
        vst1.8      {d4 - d7}, [r0, :128]!
        bhs         1b

2:      /* fix-up the remaining count and make sure we have >= 32 bytes left */
        add         r2, r2, #64
        subs        r2, r2, #32
        blo         4f

3:      /* 32 bytes at a time. These cache lines were already preloaded */
        vld1.8      {d0 - d3}, [r1]!
        subs        r2, r2, #32
        vst1.8      {d0 - d3}, [r0, :128]!
        bhs         3b
#endif
4:      /* less than 32 left */
        add         r2, r2, #32
        tst         r2, #0x10
        beq         5f
        // copies 16 bytes, 128-bits aligned
        vld1.8      {d0, d1}, [r1]!
        vst1.8      {d0, d1}, [r0, :128]!
5:      /* copy up to 15-bytes (count in r2) */
        movs        ip, r2, lsl #29
        bcc         1f
        vld1.8      {d0}, [r1]!
        vst1.8      {d0}, [r0]!
1:      bge         2f
        vld4.8      {d0[0], d1[0], d2[0], d3[0]}, [r1]!
        vst4.8      {d0[0], d1[0], d2[0], d3[0]}, [r0]!
2:      movs        ip, r2, lsl #31
        ldrmib      r3, [r1], #1
        ldrcsb      ip, [r1], #1
        ldrcsb      lr, [r1], #1
        strmib      r3, [r0], #1
        strcsb      ip, [r0], #1
        strcsb      lr, [r0], #1

        ldmfd       sp!, {r0, lr}
        bx          lr

#else   /* NEON_UNALIGNED_ACCESS */

        // Check so divider is at least 16 bytes, needed for alignment code.
        cmp         r2, #16
        blo         5f

#ifdef NEON_MEMCPY_ALIGNMENT_DIVIDER
        /* Check the upper size limit for Neon unaligned memory access in memcpy */
#if NEON_MEMCPY_ALIGNMENT_DIVIDER >= 16
        cmp         r2, #NEON_MEMCPY_ALIGNMENT_DIVIDER
        blo         3f
#endif
        /* check if buffers are aligned. If so, run arm-only version */
        eor         r3, r0, r1
        ands        r3, r3, #0x3
        beq         11f

        /* align destination to 16 bytes for the write-buffer */
        rsb         r3, r0, #0
        ands        r3, r3, #0xF
        beq         3f

        /* copy up to 15-bytes (count in r3) */
        sub         r2, r2, r3
        movs        ip, r3, lsl #31
        ldrmib      lr, [r1], #1
        strmib      lr, [r0], #1
        ldrcsb      ip, [r1], #1
        ldrcsb      lr, [r1], #1
        strcsb      ip, [r0], #1
        strcsb      lr, [r0], #1
        movs        ip, r3, lsl #29
        bge         1f
        // copies 4 bytes, destination 32-bits aligned
        vld1.32     {d0[0]}, [r1]!
        vst1.32     {d0[0]}, [r0, :32]!
1:      bcc         2f
        // copies 8 bytes, destination 64-bits aligned
        vld1.8      {d0}, [r1]!
        vst1.8      {d0}, [r0, :64]!
2:
        /* preload immediately the next cache line, which we may need */
        pld         [r1, #(CACHE_LINE_SIZE * 0)]
        pld         [r1, #(CACHE_LINE_SIZE * 1)]
3:
#endif
        /* make sure we have at least 64 bytes to copy */
        subs        r2, r2, #64
        blo         2f

        /* preload all the cache lines we need */
        pld         [r1, #(CACHE_LINE_SIZE * 2)]
        pld         [r1, #(CACHE_LINE_SIZE * 3)]

1:      /* The main loop copies 64 bytes at a time */
        vld1.8      {d0 - d3}, [r1]!
        vld1.8      {d4 - d7}, [r1]!
#ifdef  HAVE_32_BYTE_CACHE_LINE
        pld         [r1, #(CACHE_LINE_SIZE * 2)]
        pld         [r1, #(CACHE_LINE_SIZE * 3)]
#else
        pld         [r1, #(CACHE_LINE_SIZE * 3)]
#endif
        subs        r2, r2, #64
        vst1.8      {d0 - d3}, [r0]!
        vst1.8      {d4 - d7}, [r0]!
        bhs         1b

2:      /* fix-up the remaining count and make sure we have >= 32 bytes left */
        add         r2, r2, #64
        subs        r2, r2, #32
        blo         4f

3:      /* 32 bytes at a time. These cache lines were already preloaded */
        vld1.8      {d0 - d3}, [r1]!
        subs        r2, r2, #32
        vst1.8      {d0 - d3}, [r0]!
        bhs         3b

4:      /* less than 32 left */
        add         r2, r2, #32
        tst         r2, #0x10
        beq         5f
        // copies 16 bytes, 128-bits aligned
        vld1.8      {d0, d1}, [r1]!
        vst1.8      {d0, d1}, [r0]!
5:      /* copy up to 15-bytes (count in r2) */
        movs        ip, r2, lsl #29
        bcc         1f
        vld1.8      {d0}, [r1]!
        vst1.8      {d0}, [r0]!
1:      bge         2f
        vld1.32     {d0[0]}, [r1]!
        vst1.32     {d0[0]}, [r0]!
2:      movs        ip, r2, lsl #31
        ldrmib      r3, [r1], #1
        ldrcsb      ip, [r1], #1
        ldrcsb      lr, [r1], #1
        strmib      r3, [r0], #1
        strcsb      ip, [r0], #1
        strcsb      lr, [r0], #1

        ldmfd       sp!, {r0, lr}
        bx          lr
#endif  /* NEON_UNALIGNED_ACCESS */
11:
        /* Simple arm-only copy loop to handle aligned copy operations */
        stmfd       sp!, {r4, r5, r6, r7, r8}
        pld         [r1, #(CACHE_LINE_SIZE * 2)]

        /* Check alignment */
        rsb         r3, r1, #0
        ands        r3, #3
        beq         2f

        /* align source to 32 bits. We need to insert 2 instructions between
         * a ldr[b|h] and str[b|h] because byte and half-word instructions
         * stall 2 cycles.
         */
        movs        r12, r3, lsl #31
        sub         r2, r2, r3      /* we know that r3 <= r2 because r2 >= 4 */
        ldrmib      r3, [r1], #1
        ldrcsb      r4, [r1], #1
        ldrcsb      r5, [r1], #1
        strmib      r3, [r0], #1
        strcsb      r4, [r0], #1
        strcsb      r5, [r0], #1
2:
        subs        r2, #32
        blt         5f
        pld         [r1, #(CACHE_LINE_SIZE * 3)]
3:      /* Main copy loop, copying 32 bytes at a time */
        pld         [r1, #(CACHE_LINE_SIZE * 4)]
        ldmia       r1!, {r3, r4, r5, r6, r7, r8, r12, lr}
        subs        r2, r2, #32
        stmia       r0!, {r3, r4, r5, r6, r7, r8, r12, lr}
        bge         3b
5:      /* Handle any remaining bytes */
        adds        r2, #32
        beq         6f

        movs        r12, r2, lsl #28
        ldmcsia     r1!, {r3, r4, r5, r6}   /* 16 bytes */
        ldmmiia     r1!, {r7, r8}           /*  8 bytes */
        stmcsia     r0!, {r3, r4, r5, r6}
        stmmiia     r0!, {r7, r8}
        movs        r12, r2, lsl #30
        ldrcs       r3, [r1], #4            /*  4 bytes */
        ldrmih      r4, [r1], #2            /*  2 bytes */
        strcs       r3, [r0], #4
        strmih      r4, [r0], #2
        tst         r2, #0x1
        ldrneb      r3, [r1]                /*  last byte  */
        strneb      r3, [r0]
6:
        ldmfd       sp!, {r4, r5, r6, r7, r8}
        ldmfd       sp!, {r0, pc}
END(_bench_neon_memcpy)


		/*
		 * Optimized memcpy() for ARM.
         *
		 * note that memcpy() always returns the destination pointer,
		 * so we have to preserve R0.
		 */

ENTRY(_bench_memcpy)
		/* The stack must always be 64-bits aligned to be compliant with the
		 * ARM ABI. Since we have to save R0, we might as well save R4
		 * which we can use for better pipelining of the reads below
		 */
        .save       {r0, r4, lr}
        stmfd       sp!, {r0, r4, lr}
        /* Making room for r5-r11 which will be spilled later */
        .pad        #28
        sub         sp, sp, #28

        /* preload the destination because we'll align it to a cache line
           with small writes. Also start the source "pump". */
        pld         [r0, #0]
        pld         [r1, #0]
        pld         [r1, #32]

		/* it simplifies things to take care of len<4 early */
		cmp			r2, #4
		blo			copy_last_3_and_return

		/* compute the offset to align the source
		 * offset = (4-(src&3))&3 = -src & 3
		 */
		rsb			r3, r1, #0
		ands		r3, r3, #3
		beq			src_aligned

		/* align source to 32 bits. We need to insert 2 instructions between
		 * a ldr[b|h] and str[b|h] because byte and half-word instructions
		 * stall 2 cycles.
		 */
		movs		r12, r3, lsl #31
		sub			r2, r2, r3		/* we know that r3 <= r2 because r2 >= 4 */
		ldrmib		r3, [r1], #1
		ldrcsb		r4, [r1], #1
		ldrcsb		r12,[r1], #1
        strmib		r3, [r0], #1
		strcsb		r4, [r0], #1
		strcsb		r12,[r0], #1

src_aligned:

		/* see if src and dst are aligned together (congruent) */
		eor			r12, r0, r1
		tst			r12, #3
		bne			non_congruent

        /* Use post-incriment mode for stm to spill r5-r11 to reserved stack
         * frame. Don't update sp.
         */
        stmea		sp, {r5-r11}

		/* align the destination to a cache-line */
		rsb         r3, r0, #0
		ands		r3, r3, #0x1C
		beq         congruent_aligned32
		cmp         r3, r2
		andhi		r3, r2, #0x1C

		/* conditionnaly copies 0 to 7 words (length in r3) */
		movs		r12, r3, lsl #28
		ldmcsia		r1!, {r4, r5, r6, r7}	/* 16 bytes */
		ldmmiia		r1!, {r8, r9}			/*  8 bytes */
		stmcsia		r0!, {r4, r5, r6, r7}
		stmmiia		r0!, {r8, r9}
		tst         r3, #0x4
		ldrne		r10,[r1], #4			/*  4 bytes */
		strne		r10,[r0], #4
		sub         r2, r2, r3

congruent_aligned32:
		/*
		 * here source is aligned to 32 bytes.
		 */

cached_aligned32:
        subs        r2, r2, #32
        blo         less_than_32_left

        /*
         * We preload a cache-line up to 64 bytes ahead. On the 926, this will
         * stall only until the requested world is fetched, but the linefill
         * continues in the the background.
         * While the linefill is going, we write our previous cache-line
         * into the write-buffer (which should have some free space).
         * When the linefill is done, the writebuffer will
         * start dumping its content into memory
         *
         * While all this is going, we then load a full cache line into
         * 8 registers, this cache line should be in the cache by now
         * (or partly in the cache).
         *
         * This code should work well regardless of the source/dest alignment.
         *
         */

        // Align the preload register to a cache-line because the cpu does
        // "critical word first" (the first word requested is loaded first).
        bic         r12, r1, #0x1F
        add         r12, r12, #64

1:      ldmia       r1!, { r4-r11 }
        pld         [r12, #64]
        subs        r2, r2, #32

        // NOTE: if r12 is more than 64 ahead of r1, the following ldrhi
        // for ARM9 preload will not be safely guarded by the preceding subs.
        // When it is safely guarded the only possibility to have SIGSEGV here
        // is because the caller overstates the length.
        ldrhi       r3, [r12], #32      /* cheap ARM9 preload */
        stmia       r0!, { r4-r11 }
		bhs         1b

        add         r2, r2, #32




less_than_32_left:
		/*
		 * less than 32 bytes left at this point (length in r2)
		 */

		/* skip all this if there is nothing to do, which should
		 * be a common case (if not executed the code below takes
		 * about 16 cycles)
		 */
		tst			r2, #0x1F
		beq			1f

		/* conditionnaly copies 0 to 31 bytes */
		movs		r12, r2, lsl #28
		ldmcsia		r1!, {r4, r5, r6, r7}	/* 16 bytes */
		ldmmiia		r1!, {r8, r9}			/*  8 bytes */
		stmcsia		r0!, {r4, r5, r6, r7}
		stmmiia		r0!, {r8, r9}
		movs		r12, r2, lsl #30
		ldrcs		r3, [r1], #4			/*  4 bytes */
		ldrmih		r4, [r1], #2			/*  2 bytes */
		strcs		r3, [r0], #4
		strmih		r4, [r0], #2
		tst         r2, #0x1
		ldrneb		r3, [r1]				/*  last byte  */
		strneb		r3, [r0]

		/* we're done! restore everything and return */
1:		ldmfd		sp!, {r5-r11}
		ldmfd		sp!, {r0, r4, lr}
		bx			lr

		/********************************************************************/

non_congruent:
		/*
		 * here source is aligned to 4 bytes
		 * but destination is not.
		 *
		 * in the code below r2 is the number of bytes read
		 * (the number of bytes written is always smaller, because we have
		 * partial words in the shift queue)
		 */
		cmp			r2, #4
		blo			copy_last_3_and_return

        /* Use post-incriment mode for stm to spill r5-r11 to reserved stack
         * frame. Don't update sp.
         */
        stmea		sp, {r5-r11}

		/* compute shifts needed to align src to dest */
		rsb			r5, r0, #0
		and			r5, r5, #3			/* r5 = # bytes in partial words */
		mov			r12, r5, lsl #3		/* r12 = right */
		rsb			lr, r12, #32		/* lr = left  */

		/* read the first word */
		ldr			r3, [r1], #4
		sub			r2, r2, #4

		/* write a partial word (0 to 3 bytes), such that destination
		 * becomes aligned to 32 bits (r5 = nb of words to copy for alignment)
		 */
		movs		r5, r5, lsl #31
		strmib		r3, [r0], #1
		movmi		r3, r3, lsr #8
		strcsb		r3, [r0], #1
		movcs		r3, r3, lsr #8
		strcsb		r3, [r0], #1
		movcs		r3, r3, lsr #8

		cmp			r2, #4
		blo			partial_word_tail

		/* Align destination to 32 bytes (cache line boundary) */
1:		tst			r0, #0x1c
		beq			2f
		ldr			r5, [r1], #4
		sub         r2, r2, #4
		orr			r4, r3, r5,		lsl lr
		mov			r3, r5,			lsr r12
		str			r4, [r0], #4
        cmp         r2, #4
		bhs			1b
		blo			partial_word_tail

		/* copy 32 bytes at a time */
2:		subs		r2, r2, #32
		blo			less_than_thirtytwo

		/* Use immediate mode for the shifts, because there is an extra cycle
		 * for register shifts, which could account for up to 50% of
		 * performance hit.
		 */

        cmp			r12, #24
		beq			loop24
		cmp			r12, #8
		beq			loop8

loop16:
        ldr         r12, [r1], #4
1:      mov         r4, r12
		ldmia		r1!, {   r5,r6,r7,  r8,r9,r10,r11}
        pld         [r1, #64]
        subs        r2, r2, #32
        ldrhs       r12, [r1], #4
		orr			r3, r3, r4,		lsl #16
		mov			r4, r4,			lsr #16
		orr			r4, r4, r5,		lsl #16
		mov			r5, r5,			lsr #16
		orr			r5, r5, r6,		lsl #16
		mov			r6, r6,			lsr #16
		orr			r6, r6, r7,		lsl #16
		mov			r7, r7,			lsr #16
		orr			r7, r7, r8,		lsl #16
		mov			r8, r8,			lsr #16
		orr			r8, r8, r9,		lsl #16
		mov			r9, r9,			lsr #16
		orr			r9, r9, r10,	lsl #16
		mov			r10, r10,		lsr #16
		orr			r10, r10, r11,	lsl #16
		stmia		r0!, {r3,r4,r5,r6, r7,r8,r9,r10}
		mov			r3, r11,		lsr #16
		bhs			1b
		b			less_than_thirtytwo

loop8:
        ldr         r12, [r1], #4
1:      mov         r4, r12
		ldmia		r1!, {   r5,r6,r7,  r8,r9,r10,r11}
        pld         [r1, #64]
		subs		r2, r2, #32
        ldrhs       r12, [r1], #4
		orr			r3, r3, r4,		lsl #24
		mov			r4, r4,			lsr #8
		orr			r4, r4, r5,		lsl #24
		mov			r5, r5,			lsr #8
		orr			r5, r5, r6,		lsl #24
		mov			r6, r6,			lsr #8
		orr			r6, r6, r7,		lsl #24
		mov			r7, r7,			lsr #8
		orr			r7, r7, r8,		lsl #24
		mov			r8, r8,			lsr #8
		orr			r8, r8, r9,		lsl #24
		mov			r9, r9,			lsr #8
		orr			r9, r9, r10,	lsl #24
		mov			r10, r10,		lsr #8
		orr			r10, r10, r11,	lsl #24
		stmia		r0!, {r3,r4,r5,r6, r7,r8,r9,r10}
		mov			r3, r11,		lsr #8
		bhs			1b
		b			less_than_thirtytwo

loop24:
        ldr         r12, [r1], #4
1:      mov         r4, r12
		ldmia		r1!, {   r5,r6,r7,  r8,r9,r10,r11}
        pld         [r1, #64]
		subs		r2, r2, #32
        ldrhs       r12, [r1], #4
		orr			r3, r3, r4,		lsl #8
		mov			r4, r4,			lsr #24
		orr			r4, r4, r5,		lsl #8
		mov			r5, r5,			lsr #24
		orr			r5, r5, r6,		lsl #8
		mov			r6, r6,			lsr #24
		orr			r6, r6, r7,		lsl #8
		mov			r7, r7,			lsr #24
		orr			r7, r7, r8,		lsl #8
		mov			r8, r8,			lsr #24
		orr			r8, r8, r9,		lsl #8
		mov			r9, r9,			lsr #24
		orr			r9, r9, r10,	lsl #8
		mov			r10, r10,		lsr #24
		orr			r10, r10, r11,	lsl #8
		stmia		r0!, {r3,r4,r5,r6, r7,r8,r9,r10}
		mov			r3, r11,		lsr #24
		bhs			1b


less_than_thirtytwo:
		/* copy the last 0 to 31 bytes of the source */
		rsb			r12, lr, #32		/* we corrupted r12, recompute it  */
		add			r2, r2, #32
		cmp			r2, #4
		blo			partial_word_tail

1:		ldr			r5, [r1], #4
		sub         r2, r2, #4
		orr			r4, r3, r5,		lsl lr
		mov			r3,	r5,			lsr r12
		str			r4, [r0], #4
        cmp         r2, #4
		bhs			1b

partial_word_tail:
		/* we have a partial word in the input buffer */
		movs		r5, lr, lsl #(31-3)
		strmib		r3, [r0], #1
		movmi		r3, r3, lsr #8
		strcsb		r3, [r0], #1
		movcs		r3, r3, lsr #8
		strcsb		r3, [r0], #1

		/* Refill spilled registers from the stack. Don't update sp. */
		ldmfd		sp, {r5-r11}

copy_last_3_and_return:
		movs		r2, r2, lsl #31	/* copy remaining 0, 1, 2 or 3 bytes */
		ldrmib		r2, [r1], #1
		ldrcsb		r3, [r1], #1
		ldrcsb		r12,[r1]
		strmib		r2, [r0], #1
		strcsb		r3, [r0], #1
		strcsb		r12,[r0]

        /* we're done! restore sp and spilled registers and return */
        add         sp,  sp, #28
		ldmfd		sp!, {r0, r4, lr}
		bx			lr
END(_bench_memcpy)


#endif    /* __ARM_ARCH__ < 7 */
