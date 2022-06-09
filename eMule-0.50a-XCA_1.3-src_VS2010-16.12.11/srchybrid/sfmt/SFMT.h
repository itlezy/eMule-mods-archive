/** 
 * @file SFMT.h 
 *
 * @brief SIMD oriented Fast Mersenne Twister(SFMT) pseudorandom
 * number generator
 *
 * @author Mutsuo Saito (Hiroshima University)
 * @author Makoto Matsumoto (Hiroshima University)
 *
 * Copyright (C) 2006, 2007 Mutsuo Saito, Makoto Matsumoto and Hiroshima
 * University. All rights reserved.
 *
 * The new BSD License is applied to this software.
 * see LICENSE.txt
 */

#pragma once

#define RAND8_MAX _UI8_MAX
#define RAND16_MAX _UI16_MAX
#define RAND32_MAX _UI32_MAX
#define RAND64_MAX _UI64_MAX

union w128_t;

class SFMT
{
public:
	SFMT();
	uint8 getUInt8();
	uint16 getUInt16();
	uint32 getUInt32();
	uint64 getUInt64();
	void fill(uint8 *array, size_t size);
	void init(uint32 seed);
	void init(uint32 *init_key, int key_length);
private:
	void period_certification();
	inline void gen_rand_all();
	/*--------------------------------------
	  internal state, index counter and flag 
	  --------------------------------------*/
	/** the 128-bit internal state array */
	//w128_t sfmt[156];
	uint8 _sfmt[157 * 16];
	w128_t*sfmt;
	/** index counter to the 32-bit internal state array */
	size_t idx;
};

extern ThreadLocal<SFMT> t_rng;