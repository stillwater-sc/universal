#pragma once

#include <stdint.h>

// posit_api.h: generic C and C++ header defining the posit api

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef __cplusplus
// export a C interface if used by C++ source code
extern "C" {
#endif

	//////////////////////////////////////////////////////////////////////
	/// Standard posit configuration per the POSIT standard
	typedef union posit8_u   { 
		uint8_t x[1];
		uint8_t v;
	}											posit8_t;	// posit<8,0>
	typedef union posit16_u  { 
		uint8_t x[2];
		uint16_t v;
	}											posit16_t;	// posit<16,1>
	typedef struct posit32_s { 
		uint8_t x[4];
		uint32_t v;
	}											posit32_t;	// posit<32,2>
	typedef struct posit64_s { 
		uint8_t x[8];
		uint64_t v;
	}											posit64_t;	// posit<64,3>
	typedef union posit128_u {
		uint8_t x[16];
		uint64_t longs[2];
	} 											posit128_t; // posit<128,4>
	typedef struct posit256_u {
		uint8_t x[32];
		uint64_t longs[4];
	}											posit256_t;	// posit<256,5>

	///////////////////////////////////////////////////////////////////////
	///   associated quire configurations
	typedef unsigned long long	quire8_t;   // quire<8,0,39>
	typedef struct quire16_t {
		unsigned char x[16];
	}							quire16_t;	// quire<16,1,15>
	typedef struct quire32_t {
		unsigned char x[64];
	}							quire32_t;	// quire<32,2,31>
	typedef struct quire64_t {
		unsigned char x[256];
	}							quire64_t;	// quire<64,3,63>
	typedef struct quire128_t {
		unsigned char x[1024];
	}							quire128_t;	// quire<128,4,127>
	typedef struct quire256_t {
		unsigned char x[4096];
	}							quire256_t; // quire<256,5,255>

	/// quire<  8, 0, 7>      32 bits		<--- likely not enough capacity bits
	///	quire< 16, 1, 15>    128 bits
	///	quire< 32, 2, 31>    512 bits
	///	quire< 64, 3, 63>   2048 bits
	///	quire<128, 4, 127>  8192 bits		<--- likely too many capacity bits
	///	quire<256, 5, 7>   32520 bits		<--- 4065 bytes: smallest size aligned to 4byte boundary
	/// quire<256, 5, 255> 32768 bits       <--- 4096 bytes

	//////////////////////////////////////////////////////////////////////
	/// special posits
#ifdef DEEP_LEARNING
	//////////////////////////////////////////////////////////////////////
	// for Deep Learning/AI algorithms
	typedef unsigned char       posit4_t;   // posit<4,0>
	typedef unsigned char       posit5_t;   // posit<5,0>
	typedef unsigned char       posit6_t;   // posit<6,0>
	typedef unsigned char       posit7_t;   // posit<7,0>
#endif // DEEP_LEARNING

#ifdef DSP_PIPELINES
	//////////////////////////////////////////////////////////////////////
	// for DSP applications and ADC/DAC pipelines
	typedef unsigned char       posit10_t;   // posit<10,0>
	typedef unsigned char       posit12_t;   // posit<12,0>
	typedef unsigned char       posit14_t;   // posit<14,0>
#endif // DSP_PIPELINES

#ifdef EXTENDED_STANDARD
	//////////////////////////////////////////////////////////////////////
	// for Linear Algebra and general CAD/CAE/CAM/HPC applications

	//////////////////////////////////////////////////////////////////////
	// posits between posit<16,1> and posit<32,2> staying with ES = 1
	typedef unsigned char       posit20_t;  // posit<20,1>
	typedef unsigned char       posit28_t;	// posit<28,1>

	// posits between posit<32,2> and posit<64,3> staying with ES = 2
	typedef unsigned char       posit40_t;  // posit<40,2>
	typedef unsigned char       posit48_t;	// posit<48,2>
	typedef unsigned char       posit56_t;	// posit<56,2>

	//////////////////////////////////////////////////////////////////////
	// posits between posit<64,3> and posit<128,4> staying with ES = 3
	typedef unsigned char       posit80_t;  // posit<80,3>
	typedef unsigned char       posit96_t;  // posit<96,3>
	typedef unsigned char       posit112_t; // posit<112,3>
#endif // EXTENDED_STANDARD

	//////////////////////////////////////////////////////////////////////
	// C API function definitions

	//////////////////////////////////////////////////////////////////////
	// Important posit constants
	static const posit8_t  NAR8  = { 0x80 };
	static const posit16_t NAR16 = { 0x00, 0x80 };
	static const posit32_t NAR32 = { 0x00, 0x00, 0x00, 0x80 };
	static const posit64_t NAR64 = { 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 
	};
	static const posit128_t NAR128 = {{   // we a storing this in little endian
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	}};
	static const posit256_t NAR256 = {{   // we are storing this in little endian
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	}};


	static const posit8_t   ZERO8 = { 0 };
	static const posit16_t  ZERO16 = { 0 };
	static const posit32_t  ZERO32 = { 0 };
	static const posit64_t  ZERO64 = { 0 };
	static const posit128_t ZERO128 = {{ 0 }};
	static const posit256_t ZERO256 = {{ 0 }};

enum {
	/// report posit format for posit8_t. str must be at least 8 characters in size:    8.0x40p + /0 is 8 chars
	posit8_str_SIZE = 8,
	#define posit8_str_SIZE posit8_str_SIZE

	/// report posit format for posit16_t. str must be at least 11 characters in size:  16.1x4000p + /0 is 11 chars
	posit16_str_SIZE = 11,
	#define posit16_str_SIZE posit16_str_SIZE

	/// report posit format for posit32_t. str must be at least 15 characters in size:  32.2x40000000p + /0 is 15 chars
	posit32_str_SIZE = 16,
	#define posit32_str_SIZE posit32_str_SIZE

	/// report posit format for posit64_t. str must be at least 23 characters in size:  64.3x1234567812345678p + /0 is 23 chars
	posit64_str_SIZE = 23,
	#define posit64_str_SIZE posit64_str_SIZE

	/// report posit format for posit128_t. str must be at least 40 characters in size:  128.4x12345678123456781234567812345678p + /0 is 40 chars
	posit128_str_SIZE = 40,
	#define posit128_str_SIZE posit128_str_SIZE

	/// report posit format for posit128_t. str must be at least 40 characters in size:  128.4x1234567812345678123456781234567812345678123456781234567812345678p + /0 is 72 chars
	posit256_str_SIZE = 72
	#define posit256_str_SIZE posit256_str_SIZE
};

// reinterpret bits from an insigned integer type to a posit
static inline posit8_t   posit8_reinterpret(uint8_t n)   { posit8_t  x; x.v = n; return x; }
static inline posit16_t  posit16_reinterpret(uint16_t n) { posit16_t x; x.v = n; return x; }
static inline posit32_t  posit32_reinterpret(uint32_t n) { posit32_t x; x.v = n; return x; }
static inline posit64_t  posit64_reinterpret(uint64_t n) { posit64_t x; x.v = n; return x; }
#ifdef __cplusplus
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
static inline posit128_t posit128_reinterpret(uint64_t n[2]) {
    return (posit128_t){ .longs = { n[0], n[1] } };
}
static inline posit256_t posit256_reinterpret(uint64_t n[4]) {
    return (posit256_t){ .longs = { n[0], n[1], n[2], n[3] } };
}
#endif

// And reinterpret the bits from a posit to an unsigned integer type (where possible)
static inline uint8_t   posit8_bits(posit8_t p)   { return p.v; }
static inline uint16_t  posit16_bits(posit16_t p) { return p.v; }
static inline uint32_t  posit32_bits(posit32_t p) { return p.v; }
static inline uint64_t  posit64_bits(posit64_t p) { return p.v; }


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
	posit8_t: 			POSIT_GLUE4(posit,nbits,_from,p8), \
	posit16_t: 			POSIT_GLUE4(posit,nbits,_from,p16), \
	posit32_t: 			POSIT_GLUE4(posit,nbits,_from,p32), \
	posit64_t: 			POSIT_GLUE4(posit,nbits,_from,p64), \
	posit128_t: 		POSIT_GLUE4(posit,nbits,_from,p128) \
)(x))
#define posit8(x)       POSIT_FROM(8, (x))
#define posit16(x)      POSIT_FROM(16, (x))
#define posit32(x)      POSIT_FROM(32, (x))
#define posit64(x)      POSIT_FROM(64, (x))
#define posit128(x)     POSIT_FROM(128, (x))

#define POSIT_OP2(nbits, op, y) _Generic((y), \
	long double: 		POSIT_GLUE5(posit,nbits,_,op,ld), \
	double: 			POSIT_GLUE5(posit,nbits,_,op,d), \
	float: 				POSIT_GLUE5(posit,nbits,_,op,f), \
	long long: 			POSIT_GLUE5(posit,nbits,_,op,sll), \
	long: 				POSIT_GLUE5(posit,nbits,_,op,sl), \
	int: 				POSIT_GLUE5(posit,nbits,_,op,si), \
	unsigned long long: POSIT_GLUE5(posit,nbits,_,op,ull), \
	unsigned long: 		POSIT_GLUE5(posit,nbits,_,op,ul), \
	unsigned int: 		POSIT_GLUE5(posit,nbits,_,op,ui), \
	posit8_t: 			POSIT_GLUE5(posit,nbits,_,op,p8), \
	posit16_t: 			POSIT_GLUE5(posit,nbits,_,op,p16), \
	posit32_t: 			POSIT_GLUE5(posit,nbits,_,op,p32), \
	posit64_t: 			POSIT_GLUE5(posit,nbits,_,op,p64), \
	posit128_t: 		POSIT_GLUE5(posit,nbits,_,op,p128) \
)
#define POSIT_OP2X(nbits, op, x) _Generic((x), \
	long double: 		POSIT_GLUE5(posit,nbits,_,ld,op), \
	double: 			POSIT_GLUE5(posit,nbits,_,d,op), \
	float: 				POSIT_GLUE5(posit,nbits,_,f,op), \
	long long: 			POSIT_GLUE5(posit,nbits,_,op,sll), \
	long: 				POSIT_GLUE5(posit,nbits,_,sl,op), \
	int: 				POSIT_GLUE5(posit,nbits,_,si,op), \
	unsigned long long: POSIT_GLUE5(posit,nbits,_,ull,op), \
	unsigned long: 		POSIT_GLUE5(posit,nbits,_,ul,op), \
	unsigned int: 		POSIT_GLUE5(posit,nbits,_,ui,op), \
	posit8_t: 			POSIT_GLUE5(posit,nbits,_,p8,op), \
	posit16_t: 			POSIT_GLUE5(posit,nbits,_,p16,op), \
	posit32_t: 			POSIT_GLUE5(posit,nbits,_,p32,op), \
	posit64_t: 			POSIT_GLUE5(posit,nbits,_,p64,op), \
	posit128_t: 		POSIT_GLUE5(posit,nbits,_,p128,op) \
)

#define POSIT_GENERIC_OP(p,x,op) (_Generic((p), \
	posit8_t:           POSIT_OP2(8, op, (x)), \
	posit16_t:          POSIT_OP2(16, op, (x)), \
	posit32_t:          POSIT_OP2(32, op, (x)), \
	posit64_t:          POSIT_OP2(64, op, (x)), \
	posit128_t:         POSIT_OP2(128, op, (x)), \
	default: _Generic((x), \
		posit8_t:       POSIT_OP2X(8, op, (p)), \
		posit16_t:      POSIT_OP2X(16, op, (p)), \
		posit32_t:      POSIT_OP2X(32, op, (p)), \
		posit64_t:      POSIT_OP2X(64, op, (p)), \
		posit128_t:     POSIT_OP2X(128, op, (p)) \
	) \
)((p),(x)))
#define posit_add(p, x) POSIT_GENERIC_OP(p,x,add)
#define posit_sub(p, x) POSIT_GENERIC_OP(p,x,sub)
#define posit_mul(p, x) POSIT_GENERIC_OP(p,x,mul)
#define posit_div(p, x) POSIT_GENERIC_OP(p,x,div)
#define posit_cmp(p, x) POSIT_GENERIC_OP(p,x,cmp)

#define POSIT_GENETIC1(p, op) _Generic((p), \
	posit8_t: POSIT_GLUE(posit8_,op), \
	posit16_t: POSIT_GLUE(posit16_,op), \
	posit32_t: POSIT_GLUE(posit32_,op), \
	posit64_t: POSIT_GLUE(posit64_,op), \
	posit128_t: POSIT_GLUE(posit128_,op) \
)
#define posit_str(buf, p)   POSIT_GENETIC1(p, str)((buf), (p))
#define posit_sqrt(p)       POSIT_GENETIC1(p, sqrt)(p)
#define posit_told(p)       POSIT_GENETIC1(p, told)(p)
#define posit_tod(p)        POSIT_GENETIC1(p, tod)(p)
#define posit_tof(p)        POSIT_GENETIC1(p, tof)(p)
#define posit_tosll(p)      POSIT_GENETIC1(p, tosll)(p)
#define posit_tosl(p)       POSIT_GENETIC1(p, tosl)(p)
#define posit_tosi(p)       POSIT_GENETIC1(p, tosi)(p)
#define posit_toull(p)      POSIT_GENETIC1(p, toull)(p)
#define posit_toul(p)       POSIT_GENETIC1(p, toul)(p)
#define posit_toui(p)       POSIT_GENETIC1(p, toui)(p)

#define posit_bits(p) (_Generic((p), \
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
