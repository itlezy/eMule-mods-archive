/** 
 * @file  SFMT.c
 * @brief SIMD oriented Fast Mersenne Twister(SFMT)
 *
 * @author Mutsuo Saito (Hiroshima University)
 * @author Makoto Matsumoto (Hiroshima University)
 *
 * Copyright (C) 2006,2007 Mutsuo Saito, Makoto Matsumoto and Hiroshima
 * University. All rights reserved.
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */
#include "stdafx.h"
#include "SFMT.h"
#include "SFMT-params.h"

#if defined(__GNUC__)
#define inline __inline__
#define ALWAYSINLINE __attribute__((always_inline))
#else
#define ALWAYSINLINE
#endif

#if defined(_MSC_VER)
  #if _MSC_VER >= 1200
    #define PRE_ALWAYS __forceinline
  #else
    #define PRE_ALWAYS inline
  #endif
#else
  #define PRE_ALWAYS inline
#endif

#if defined(_WIN64) || (_M_IX86_FP == 2)
#define HAVE_SSE2
#endif

#if N != 156
#error "need MT19937 params!"
#endif
#if defined(__BIG_ENDIAN__) && !defined(__amd64) && !defined(BIG_ENDIAN64)
#define BIG_ENDIAN64 1
#endif
#if defined(HAVE_ALTIVEC) && !defined(BIG_ENDIAN64)
#define BIG_ENDIAN64 1
#endif
#if defined(ONLY64) && !defined(BIG_ENDIAN64)
  #if defined(__GNUC__)
    #error "-DONLY64 must be specified with -DBIG_ENDIAN64"
  #endif
#undef ONLY64
#endif

#if defined(HAVE_ALTIVEC)
#if !defined(__APPLE__)
#include <altivec.h>
#endif
#elif defined(HAVE_SSE2)
#include <emmintrin.h>
#endif

/*------------------------------------------------------
  128-bit SIMD data type for Altivec, SSE2 or standard C
  ------------------------------------------------------*/
union w128_t {
#if defined(HAVE_ALTIVEC)
	vector unsigned int s;
#elif defined(HAVE_SSE2)
	__m128i si;
#endif
	uint32 u[4];
};

/** a parity check vector which certificate the period of 2^{MEXP} */
const uint32 parity[4] = {PARITY1, PARITY2, PARITY3, PARITY4};

/*----------------
  STATIC FUNCTIONS
  ----------------*/
inline int idxof(int i);
inline void rshift128(w128_t *out,  w128_t const *in, int shift);
inline void lshift128(w128_t *out,  w128_t const *in, int shift);
inline uint32 func1(uint32 x);
inline uint32 func2(uint32 x);

#if defined(HAVE_ALTIVEC)
  #include "SFMT-alti.h"
#elif defined(HAVE_SSE2)
  #include "SFMT-sse2.h"
#endif

/**
 * This function simulate a 64-bit index of LITTLE ENDIAN 
 * in BIG ENDIAN machine.
 */
#ifdef ONLY64
inline int idxof(int i) {
    return i ^ 1;
}
#else
inline int idxof(int i) {
    return i;
}
#endif
/**
 * This function simulates SIMD 128-bit right shift by the standard C.
 * The 128-bit integer given in in is shifted by (shift * 8) bits.
 * This function simulates the LITTLE ENDIAN SIMD.
 * @param out the output of this function
 * @param in the 128-bit data to be shifted
 * @param shift the shift value
 */
#ifdef ONLY64
inline void rshift128(w128_t *out, w128_t const *in, int shift) {
    uint64 th, tl, oh, ol;

    th = ((uint64)in->u[2] << 32) | ((uint64)in->u[3]);
    tl = ((uint64)in->u[0] << 32) | ((uint64)in->u[1]);

    oh = th >> (shift * 8);
    ol = tl >> (shift * 8);
    ol |= th << (64 - shift * 8);
    out->u[0] = (uint32)(ol >> 32);
    out->u[1] = (uint32)ol;
    out->u[2] = (uint32)(oh >> 32);
    out->u[3] = (uint32)oh;
}
#else
inline void rshift128(w128_t *out, w128_t const *in, int shift) {
    uint64 th, tl, oh, ol;

    th = ((uint64)in->u[3] << 32) | ((uint64)in->u[2]);
    tl = ((uint64)in->u[1] << 32) | ((uint64)in->u[0]);

    oh = th >> (shift * 8);
    ol = tl >> (shift * 8);
    ol |= th << (64 - shift * 8);
    out->u[1] = (uint32)(ol >> 32);
    out->u[0] = (uint32)ol;
    out->u[3] = (uint32)(oh >> 32);
    out->u[2] = (uint32)oh;
}
#endif
/**
 * This function simulates SIMD 128-bit left shift by the standard C.
 * The 128-bit integer given in in is shifted by (shift * 8) bits.
 * This function simulates the LITTLE ENDIAN SIMD.
 * @param out the output of this function
 * @param in the 128-bit data to be shifted
 * @param shift the shift value
 */
#ifdef ONLY64
inline void lshift128(w128_t *out, w128_t const *in, int shift) {
    uint64 th, tl, oh, ol;

    th = ((uint64)in->u[2] << 32) | ((uint64)in->u[3]);
    tl = ((uint64)in->u[0] << 32) | ((uint64)in->u[1]);

    oh = th << (shift * 8);
    ol = tl << (shift * 8);
    oh |= tl >> (64 - shift * 8);
    out->u[0] = (uint32)(ol >> 32);
    out->u[1] = (uint32)ol;
    out->u[2] = (uint32)(oh >> 32);
    out->u[3] = (uint32)oh;
}
#else
inline void lshift128(w128_t *out, w128_t const *in, int shift) {
    uint64 th, tl, oh, ol;

    th = ((uint64)in->u[3] << 32) | ((uint64)in->u[2]);
    tl = ((uint64)in->u[1] << 32) | ((uint64)in->u[0]);

    oh = th << (shift * 8);
    ol = tl << (shift * 8);
    oh |= tl >> (64 - shift * 8);
    out->u[1] = (uint32)(ol >> 32);
    out->u[0] = (uint32)ol;
    out->u[3] = (uint32)(oh >> 32);
    out->u[2] = (uint32)oh;
}
#endif

/**
 * This function represents the recursion formula.
 * @param r output
 * @param a a 128-bit part of the internal state array
 * @param b a 128-bit part of the internal state array
 * @param c a 128-bit part of the internal state array
 * @param d a 128-bit part of the internal state array
 */
#if (!defined(HAVE_ALTIVEC)) && (!defined(HAVE_SSE2))
#ifdef ONLY64
inline void do_recursion(w128_t *r, w128_t *a, w128_t *b, w128_t *c,
				w128_t *d) {
    w128_t x;
    w128_t y;

    lshift128(&x, a, SL2);
    rshift128(&y, c, SR2);
    r->u[0] = a->u[0] ^ x.u[0] ^ ((b->u[0] >> SR1) & MSK2) ^ y.u[0] 
	^ (d->u[0] << SL1);
    r->u[1] = a->u[1] ^ x.u[1] ^ ((b->u[1] >> SR1) & MSK1) ^ y.u[1] 
	^ (d->u[1] << SL1);
    r->u[2] = a->u[2] ^ x.u[2] ^ ((b->u[2] >> SR1) & MSK4) ^ y.u[2] 
	^ (d->u[2] << SL1);
    r->u[3] = a->u[3] ^ x.u[3] ^ ((b->u[3] >> SR1) & MSK3) ^ y.u[3] 
	^ (d->u[3] << SL1);
}
#else
inline void do_recursion(w128_t *r, w128_t *a, w128_t *b, w128_t *c,
				w128_t *d) {
    w128_t x;
    w128_t y;

    lshift128(&x, a, SL2);
    rshift128(&y, c, SR2);
    r->u[0] = a->u[0] ^ x.u[0] ^ ((b->u[0] >> SR1) & MSK1) ^ y.u[0] 
	^ (d->u[0] << SL1);
    r->u[1] = a->u[1] ^ x.u[1] ^ ((b->u[1] >> SR1) & MSK2) ^ y.u[1] 
	^ (d->u[1] << SL1);
    r->u[2] = a->u[2] ^ x.u[2] ^ ((b->u[2] >> SR1) & MSK3) ^ y.u[2] 
	^ (d->u[2] << SL1);
    r->u[3] = a->u[3] ^ x.u[3] ^ ((b->u[3] >> SR1) & MSK4) ^ y.u[3] 
	^ (d->u[3] << SL1);
}
#endif
#endif

#if (!defined(HAVE_ALTIVEC)) && (!defined(HAVE_SSE2))
/**
 * This function fills the internal state array with pseudorandom
 * integers.
 */
inline void SFMT::gen_rand_all() {
    int i;
    w128_t *r1, *r2;

    r1 = &sfmt[N - 2];
    r2 = &sfmt[N - 1];
    for (i = 0; i < N - POS1; i++) {
	do_recursion(&sfmt[i], &sfmt[i], &sfmt[i + POS1], r1, r2);
	r1 = r2;
	r2 = &sfmt[i];
    }
    for (; i < N; i++) {
	do_recursion(&sfmt[i], &sfmt[i], &sfmt[i + POS1 - N], r1, r2);
	r1 = r2;
	r2 = &sfmt[i];
    }
}
#endif

/**
 * This function represents a function used in the initialization
 * by init_by_array
 * @param x 32-bit integer
 * @return 32-bit integer
 */
uint32 func1(uint32 x) {
    return (x ^ (x >> 27)) * (uint32)1664525UL;
}

/**
 * This function represents a function used in the initialization
 * by init_by_array
 * @param x 32-bit integer
 * @return 32-bit integer
 */
uint32 func2(uint32 x) {
    return (x ^ (x >> 27)) * (uint32)1566083941UL;
}
/**
 * This function certificate the period of 2^{MEXP}
 */
void SFMT::period_certification() {
    int inner = 0;
    int i, j;
    uint32 work;
	uint32 *psfmt32 = &sfmt[0].u[0];
    for (i = 0; i < 4; i++)
	inner ^= psfmt32[idxof(i)] & parity[i];
    for (i = 16; i > 0; i >>= 1)
	inner ^= inner >> i;
    inner &= 1;
    /* check OK */
    if (inner == 1) {
	return;
    }
    /* check NG, and modification */
    for (i = 0; i < 4; i++) {
	work = 1;
	for (j = 0; j < 32; j++) {
	    if ((work & parity[i]) != 0) {
		psfmt32[idxof(i)] ^= work;
		return;
	    }
	    work = work << 1;
	}
    }
}

SFMT::SFMT()
{
	sfmt = (w128_t*)(_sfmt + 16 - ((size_t)_sfmt & 0xf));
#ifndef _WIN64
	uint32 seed;
	__asm
	{
		rdtsc
		mov DWORD PTR seed, eax
	}
#else
	uint32 seed = (uint32)time(NULL);
#endif
	init(seed);
}
uint8 SFMT::getUInt8(void)
{
    if (idx > N8 - 1) {
		gen_rand_all();
		idx = 0;
	}
    uint8 r = *(((uint8*)&sfmt[0]) + idx);
	++idx;
    return r;
}

uint16 SFMT::getUInt16(void)
{
    if (idx > N8 - 2) {
		gen_rand_all();
		idx = 0;
	}
    uint16 r = *(uint16*)(((uint8*)&sfmt[0]) + idx);
	idx += 2;
    return r;
}

uint32 SFMT::getUInt32(void)
{
    if (idx > N8 - 4) {
		gen_rand_all();
		idx = 0;
	}
    uint32 r = *(uint32*)(((uint8*)&sfmt[0]) + idx);
	idx += 4;
    return r;
}

uint64 SFMT::getUInt64(void)
{
    if (idx >= N8 - 8) {
		gen_rand_all();
		idx = 0;
    }
    uint64 r = *(uint64*)(((uint8*)&sfmt[0]) + idx);
    idx += 8;
    return r;
}

void SFMT::fill(uint8 *array, size_t size)
{
	ATLASSERT(size < N8);
	if (idx >= N8 - size) {
		gen_rand_all();
		idx = 0;
	}
	memcpy(array, ((uint8*)&sfmt[0]) + idx, size);
	idx += size;
}

void SFMT::init(uint32 seed)
{
    int i;

	uint32 *psfmt32 = &sfmt[0].u[0];
    psfmt32[idxof(0)] = seed;
    for (i = 1; i < N32; i++) {
	psfmt32[idxof(i)] = 1812433253UL * (psfmt32[idxof(i - 1)] 
					    ^ (psfmt32[idxof(i - 1)] >> 30))
	    + i;
    }
    idx = N8;
	period_certification();
}

void SFMT::init(uint32 *init_key, int key_length)
{
	int i, j, count;
	uint32 r;
	size_t lag;
	size_t mid;
	const size_t size = N * 4;

	if (size >= 623) {
		lag = 11;
	} else if (size >= 68) {
		lag = 7;
	} else if (size >= 39) {
		lag = 5;
	} else {
		lag = 3;
	}
	mid = (size - lag) / 2;

	memset(sfmt, 0x8b, sizeof(sfmt));
	if (key_length + 1 > N32) {
		count = key_length + 1;
	} else {
		count = N32;
	}
	uint32 *psfmt32 = &sfmt[0].u[0];
	r = func1(psfmt32[idxof(0)] ^ psfmt32[idxof(mid)] 
	^ psfmt32[idxof(N32 - 1)]);
	psfmt32[idxof(mid)] += r;
	r += key_length;
	psfmt32[idxof(mid + lag)] += r;
	psfmt32[idxof(0)] = r;

	count--;
	for (i = 1, j = 0; (j < count) && (j < key_length); j++) {
		r = func1(psfmt32[idxof(i)] ^ psfmt32[idxof((i + mid) % N32)] 
		^ psfmt32[idxof((i + N32 - 1) % N32)]);
		psfmt32[idxof((i + mid) % N32)] += r;
		r += init_key[j] + i;
		psfmt32[idxof((i + mid + lag) % N32)] += r;
		psfmt32[idxof(i)] = r;
		i = (i + 1) % N32;
	}
	for (; j < count; j++) {
		r = func1(psfmt32[idxof(i)] ^ psfmt32[idxof((i + mid) % N32)] 
		^ psfmt32[idxof((i + N32 - 1) % N32)]);
		psfmt32[idxof((i + mid) % N32)] += r;
		r += i;
		psfmt32[idxof((i + mid + lag) % N32)] += r;
		psfmt32[idxof(i)] = r;
		i = (i + 1) % N32;
	}
	for (j = 0; j < N32; j++) {
		r = func2(psfmt32[idxof(i)] + psfmt32[idxof((i + mid) % N32)] 
		+ psfmt32[idxof((i + N32 - 1) % N32)]);
		psfmt32[idxof((i + mid) % N32)] ^= r;
		r -= i;
		psfmt32[idxof((i + mid + lag) % N32)] ^= r;
		psfmt32[idxof(i)] = r;
		i = (i + 1) % N32;
	}

	idx = N8;
	period_certification();
}
