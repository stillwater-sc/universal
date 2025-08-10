#pragma once
// posit_c_api.h: generic C API defining the posit api
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// set up the correct C11 infrastructure
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// posit C types
#include <universal/number/posit/positctypes.h>

#ifdef __cplusplus
// export a C interface if used by C++ source code
extern "C" {
#endif

enum {
	// assume UTF-16 UNICODE strings: 2 bytes per character
	/// report posit format for posit4_t. str must be at least 7 characters in size:    4.0x4p + /0 is 7 chars
	posit4_str_SIZE = 7,
	#define posit4_str_SIZE 2*posit4_str_SIZE

	/// report posit format for posit8_t. str must be at least 8 characters in size:    8.0x40p + /0 is 8 chars
	posit8_str_SIZE = 8,
	#define posit8_str_SIZE 2*posit8_str_SIZE

	/// report posit format for posit16_t. str must be at least 11 characters in size:  16.1x4000p + /0 is 11 chars
	posit16_str_SIZE = 11,
	#define posit16_str_SIZE 2*posit16_str_SIZE

	/// report posit format for posit32_t. str must be at least 15 characters in size:  32.2x40000000p + /0 is 15 chars
	posit32_str_SIZE = 16,
	#define posit32_str_SIZE 2*posit32_str_SIZE

	/// report posit format for posit64_t. str must be at least 23 characters in size:  64.3x1234567812345678p + /0 is 23 chars
	posit64_str_SIZE = 23,
	#define posit64_str_SIZE 2*posit64_str_SIZE

	/// report posit format for posit128_t. str must be at least 40 characters in size:  128.4x12345678123456781234567812345678p + /0 is 40 chars
	posit128_str_SIZE = 40,
	#define posit128_str_SIZE 2*posit128_str_SIZE

	/// report posit format for posit256_t. str must be at least 72 characters in size:  128.4x1234567812345678123456781234567812345678123456781234567812345678p + /0 is 72 chars
	posit256_str_SIZE = 72,
	#define posit256_str_SIZE 2*posit256_str_SIZE
};

// reinterpret bits from an unsigned integer type to a posit type
static inline posit4_t   posit4_reinterpret(uint8_t n)   { posit4_t  x; x.v = n; return x; }
static inline posit8_t   posit8_reinterpret(uint8_t n)   { posit8_t  x; x.v = n; return x; }
static inline posit16_t  posit16_reinterpret(uint16_t n) { posit16_t x; x.v = n; return x; }
static inline posit32_t  posit32_reinterpret(uint32_t n) { posit32_t x; x.v = n; return x; }
static inline posit64_t  posit64_reinterpret(uint64_t n) { posit64_t x; x.v = n; return x; }

#if defined(__cplusplus) || defined(_MSC_VER)
static inline posit128_t posit128_reinterpret(uint64_t* n) {
    posit128_t out;
    out.longs[0] = n[0];
    out.longs[1] = n[1];
    return out;
}
static inline posit256_t posit256_reinterpret(uint64_t* n) {
    posit256_t out;
    out.longs[0] = n[0];
    out.longs[1] = n[1];
    out.longs[2] = n[2];
    out.longs[3] = n[3];
    return out;
}
#else
// static array parameters are illegal in C++ but they provide valuable verification in C
static inline posit128_t posit128_reinterpret(uint64_t n[static 2]) {
    return (posit128_t){ .longs = { n[0], n[1] } };
}
static inline posit256_t posit256_reinterpret(uint64_t n[static 4]) {
    return (posit256_t){ .longs = { n[0], n[1], n[2], n[3] } };
}
#endif

// And reinterpret the bits from a posit to an unsigned integer type (where possible)
static inline uint8_t   posit4_bits(posit4_t p)   { return p.v; }
static inline uint8_t   posit8_bits(posit8_t p)   { return p.v; }
static inline uint16_t  posit16_bits(posit16_t p) { return p.v; }
static inline uint32_t  posit32_bits(posit32_t p) { return p.v; }
static inline uint64_t  posit64_bits(posit64_t p) { return p.v; }

#define POSIT_NBITS 4
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 8
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 16
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 32
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 64
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 128
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 256
#include "posit_c_macros.h"
#undef POSIT_NBITS

#if __STDC_VERSION__ >= 201112L && !defined(POSIT_NO_GENERICS)

#define POSIT_FROM(nbits, x) (_Generic((x), \
	long double: 		POSIT_GLUE4(posit,nbits,_from,ld), \
	double: 			POSIT_GLUE4(posit,nbits,_from,d), \
	float: 				POSIT_GLUE4(posit,nbits,_from,f), \
	long long: 			POSIT_GLUE4(posit,nbits,_from,sll), \
	long: 				POSIT_GLUE4(posit,nbits,_from,sl), \
	int: 				POSIT_GLUE4(posit,nbits,_from,si), \
	unsigned long long: POSIT_GLUE4(posit,nbits,_from,ull), \
	unsigned long: 		POSIT_GLUE4(posit,nbits,_from,ul), \
	unsigned int: 		POSIT_GLUE4(posit,nbits,_from,ui), \
	posit4_t: 			POSIT_GLUE4(posit,nbits,_from,p4), \
	posit8_t: 			POSIT_GLUE4(posit,nbits,_from,p8), \
	posit16_t: 			POSIT_GLUE4(posit,nbits,_from,p16), \
	posit32_t: 			POSIT_GLUE4(posit,nbits,_from,p32), \
	posit64_t: 			POSIT_GLUE4(posit,nbits,_from,p64), \
	posit128_t: 		POSIT_GLUE4(posit,nbits,_from,p128), \
	posit256_t: 		POSIT_GLUE4(posit,nbits,_from,p256) \
)(x))
#define posit4(x)       POSIT_FROM(4, (x))
#define posit8(x)       POSIT_FROM(8, (x))
#define posit16(x)      POSIT_FROM(16, (x))
#define posit32(x)      POSIT_FROM(32, (x))
#define posit64(x)      POSIT_FROM(64, (x))
#define posit128(x)     POSIT_FROM(128, (x))
#define posit256(x)     POSIT_FROM(256, (x))

#define POSIT_OP2(nbits, op, y) _Generic((y), \
	long double: 		POSIT_GLUE5(posit,nbits,_,op,ld), \
	double: 			POSIT_GLUE5(posit,nbits,_,op,d), \
	float: 				POSIT_GLUE5(posit,nbits,_,op,f), \
	long long: 			POSIT_GLUE5(posit,nbits,_,op,sll), \
	long: 				POSIT_GLUE5(posit,nbits,_,op,sl), \
	int: 				POSIT_GLUE5(posit,nbits,_,op,si), \
	unsigned long long:	POSIT_GLUE5(posit,nbits,_,op,ull), \
	unsigned long: 		POSIT_GLUE5(posit,nbits,_,op,ul), \
	unsigned int: 		POSIT_GLUE5(posit,nbits,_,op,ui), \
	posit4_t: 			POSIT_GLUE5(posit,nbits,_,op,p4), \
	posit8_t: 			POSIT_GLUE5(posit,nbits,_,op,p8), \
	posit16_t: 			POSIT_GLUE5(posit,nbits,_,op,p16), \
	posit32_t: 			POSIT_GLUE5(posit,nbits,_,op,p32), \
	posit64_t: 			POSIT_GLUE5(posit,nbits,_,op,p64), \
	posit128_t: 		POSIT_GLUE5(posit,nbits,_,op,p128), \
	posit256_t:			POSIT_GLUE5(posit,nbits,_,op,p256) \
)
#define POSIT_OP2X(nbits, op, x) _Generic((x), \
	long double: 		POSIT_GLUE5(posit,nbits,_,ld,op), \
	double: 			POSIT_GLUE5(posit,nbits,_,d,op), \
	float: 				POSIT_GLUE5(posit,nbits,_,f,op), \
	long long: 			POSIT_GLUE5(posit,nbits,_,op,sll), \
	long: 				POSIT_GLUE5(posit,nbits,_,sl,op), \
	int: 				POSIT_GLUE5(posit,nbits,_,si,op), \
	unsigned long long:	POSIT_GLUE5(posit,nbits,_,ull,op), \
	unsigned long: 		POSIT_GLUE5(posit,nbits,_,ul,op), \
	unsigned int: 		POSIT_GLUE5(posit,nbits,_,ui,op), \
	posit4_t: 			POSIT_GLUE5(posit,nbits,_,p4,op), \
	posit8_t: 			POSIT_GLUE5(posit,nbits,_,p8,op), \
	posit16_t: 			POSIT_GLUE5(posit,nbits,_,p16,op), \
	posit32_t: 			POSIT_GLUE5(posit,nbits,_,p32,op), \
	posit64_t: 			POSIT_GLUE5(posit,nbits,_,p64,op), \
	posit128_t: 		POSIT_GLUE5(posit,nbits,_,p128,op), \
	posit256_t:			POSIT_GLUE5(posit,nbits,_,p256,op) \
)

#define POSIT_GENERIC_OP(p,x,op) (_Generic((p), \
	posit4_t:           POSIT_OP2(4, op, (x)), \
	posit8_t:           POSIT_OP2(8, op, (x)), \
	posit16_t:          POSIT_OP2(16, op, (x)), \
	posit32_t:          POSIT_OP2(32, op, (x)), \
	posit64_t:          POSIT_OP2(64, op, (x)), \
	posit128_t:         POSIT_OP2(128, op, (x)), \
	posit256_t:         POSIT_OP2(256, op, (x)), \
	default: _Generic((x), \
		posit4_t:       POSIT_OP2X(4, op, (p)), \
		posit8_t:       POSIT_OP2X(8, op, (p)), \
		posit16_t:      POSIT_OP2X(16, op, (p)), \
		posit32_t:      POSIT_OP2X(32, op, (p)), \
		posit64_t:      POSIT_OP2X(64, op, (p)), \
		posit128_t:     POSIT_OP2X(128, op, (p)), \
		posit256_t:		POSIT_OP2X(256, op, (p)) \
	) \
)((p),(x)))
#define posit_add(p, x) POSIT_GENERIC_OP(p,x,add)
#define posit_sub(p, x) POSIT_GENERIC_OP(p,x,sub)
#define posit_mul(p, x) POSIT_GENERIC_OP(p,x,mul)
#define posit_div(p, x) POSIT_GENERIC_OP(p,x,div)
#define posit_cmp(p, x) POSIT_GENERIC_OP(p,x,cmp)

#define POSIT_GENERIC_1(p, op) _Generic((p), \
	posit4_t: POSIT_GLUE(posit4_,op), \
	posit8_t: POSIT_GLUE(posit8_,op), \
	posit16_t: POSIT_GLUE(posit16_,op), \
	posit32_t: POSIT_GLUE(posit32_,op), \
	posit64_t: POSIT_GLUE(posit64_,op), \
	posit128_t: POSIT_GLUE(posit128_,op), \
	posit256_t: POSIT_GLUE(posit256_,op) \
)
#define posit_str(buf, p)   POSIT_GENERIC_1(p, str)((buf), (p))
#define posit_sqrt(p)       POSIT_GENERIC_1(p, sqrt)(p)
#define posit_log(p)        POSIT_GENERIC_1(p, log)(p)
#define posit_exp(p)        POSIT_GENERIC_1(p, exp)(p)
#define posit_told(p)       POSIT_GENERIC_1(p, told)(p)
#define posit_tod(p)        POSIT_GENERIC_1(p, tod)(p)
#define posit_tof(p)        POSIT_GENERIC_1(p, tof)(p)
#define posit_tosll(p)      POSIT_GENERIC_1(p, tosll)(p)
#define posit_tosl(p)       POSIT_GENERIC_1(p, tosl)(p)
#define posit_tosi(p)       POSIT_GENERIC_1(p, tosi)(p)
#define posit_toull(p)      POSIT_GENERIC_1(p, toull)(p)
#define posit_toul(p)       POSIT_GENERIC_1(p, toul)(p)
#define posit_toui(p)       POSIT_GENERIC_1(p, toui)(p)

#define posit_bits(p) (_Generic((p), \
	posit4_t: posit4_bits, \
	posit8_t: posit8_bits, \
	posit16_t: posit16_bits, \
	posit32_t: posit32_bits, \
	posit64_t: posit64_bits \
)(p))

// we need to leave these defined as we complete because otherwise
// the generics macros will not work correctly.
#define POSIT_GLUE3(a,b,c) POSIT_GLUE(POSIT_GLUE(a,b),c)
#define POSIT_GLUE4(a,b,c,d) POSIT_GLUE(POSIT_GLUE(a,b),POSIT_GLUE(c,d))
#define POSIT_GLUE5(a,b,c,d,e) POSIT_GLUE(POSIT_GLUE4(a,b,c,d),e)
#define POSIT_GLUE(x,y) POSIT_GLUE_(x,y)
#define POSIT_GLUE_(x,y) x ## y

#endif // POSIT_NO_GENERICS

#ifdef __cplusplus
}
#endif
