#pragma once
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
	typedef unsigned char       posit8_t;	// posit<8,0>
	typedef unsigned short      posit16_t;	// posit<16,1>
	typedef unsigned long       posit32_t;	// posit<32,2>
	typedef unsigned long long  posit64_t;	// posit<64,3>
	typedef struct posit128_t {
		unsigned char x[16];
	}							posit128_t;	// posit<128,4>
	typedef struct posit256_t {
		unsigned char x[32];
	}							posit256_t;	// posit<256,5>

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
	static const posit8_t  NAR8  = 0x80;
	static const posit16_t NAR16 = 0x8000;
	static const posit32_t NAR32 = 0x80000000;
	static const posit64_t NAR64 = 0x8000000000000000;
	static const posit128_t NAR128 = {{   // we are storing this in little endian
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	}};
	static const posit256_t NAR256 = { {   // we are storing this in little endian
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
		} };


	static const posit8_t   ZERO8 = 0;
	static const posit16_t  ZERO16 = 0;
	static const posit32_t  ZERO32 = 0;
	static const posit64_t  ZERO64 = 0;
	static const posit128_t ZERO128 = {{ 0 }};
	static const posit256_t ZERO256 = {{ 0 }};

	///////////////////////////////////////////////////////////////
	/////////        output

// posit format size helpers
#define POSIT_FORMAT8_SIZE 8
#define POSIT_FORMAT16_SIZE 11
#define POSIT_FORMAT32_SIZE 15
#define POSIT_FORMAT64_SIZE 23
#define POSIT_FORMAT128_SIZE 40
#define POSIT_FORMAT256_SIZE 72
	/// report posit format for posit8_t. str must be at least 8 characters in size:    8.0x40p + /0 is 8 chars
	void posit_format8(posit8_t a, char* str);
	/// report posit format for posit16_t. str must be at least 11 characters in size:  16.1x4000p + /0 is 11 chars
	void posit_format16(posit16_t a, char* str);
	/// report posit format for posit32_t. str must be at least 15 characters in size:  32.2x40000000p + /0 is 15 chars
	void posit_format32(posit32_t a, char* str);
	/// report posit format for posit64_t. str must be at least 23 characters in size:  64.3x1234567812345678p + /0 is 23 chars
	void posit_format64(posit64_t a, char* str);
	/// report posit format for posit128_t. str must be at least 40 characters in size:  128.4x12345678123456781234567812345678p + /0 is 40 chars
	void posit_format128(posit128_t a, char* str);
	/// report posit format for posit256_t. str must be at least 72 characters in size:  256.5x1234567812345678123456781234567812345678123456781234567812345678p + /0 is 72 chars
	void posit_format256(posit256_t a, char* str);

	// casts to double
	double      posit_value8(posit8_t a);
	double      posit_value16(posit16_t a);
	double      posit_value32(posit32_t a);
	long double posit_value64(posit64_t a);
	long double posit_value128(posit128_t a);
	long double posit_value256(posit256_t a);

	// Raw bit assignments
	// small posits don't need help as you can simply assign to them directly
	// helper for the bigger posit to make it easier to create them
	posit128_t posit_assign128(unsigned long long lower, unsigned long long upper);
	posit256_t posit_assign256(unsigned long long lower0, unsigned long long lower1, unsigned long long lower2, unsigned long long lower3);

	// Integer assignments
	posit8_t   posit_assign8i(int  a);
	posit16_t  posit_assign16i(int a);
	posit32_t  posit_assign32i(long a);
	posit64_t  posit_assign64i(long long a);
	posit128_t posit_assign128i(long long a);
	posit256_t posit_assign256i(long long a);

	// IEEE floating point assignments
	posit8_t   posit_assign8f(float  a);
	posit16_t  posit_assign16f(float a);
	posit32_t  posit_assign32f(double a);
	posit64_t  posit_assign64f(long double a);
	posit128_t posit_assign128f(long double a);
	posit256_t posit_assign256f(long double a);

	// Addition
	posit8_t   posit_add8  (posit8_t  a, posit8_t  b);
	posit16_t  posit_add16 (posit16_t a, posit16_t b);
	posit32_t  posit_add32 (posit32_t a, posit32_t b);
	posit64_t  posit_add64 (posit64_t a, posit64_t b);
	posit128_t posit_add128(posit128_t a, posit128_t b);
	posit256_t posit_add256(posit256_t a, posit256_t b);
	// Subtraction
	posit8_t   posit_sub8  (posit8_t  a, posit8_t  b);
	posit16_t  posit_sub16 (posit16_t a, posit16_t b);
	posit32_t  posit_sub32 (posit32_t a, posit32_t b);
	posit64_t  posit_sub64 (posit64_t a, posit64_t b);
	posit128_t posit_sub128(posit128_t a, posit128_t b);
	posit256_t posit_sub256(posit256_t a, posit256_t b);
	// Multiplication
	posit8_t   posit_mul8  (posit8_t  a, posit8_t  b);
	posit16_t  posit_mul16 (posit16_t a, posit16_t b);
	posit32_t  posit_mul32 (posit32_t a, posit32_t b);
	posit64_t  posit_mul64 (posit64_t a, posit64_t b);
	posit128_t posit_mul128(posit128_t a, posit128_t b);
	posit256_t posit_mul256(posit256_t a, posit256_t b);
	// Division
	posit8_t   posit_div8  (posit8_t  a, posit8_t  b);
	posit16_t  posit_div16 (posit16_t a, posit16_t b);
	posit32_t  posit_div32 (posit32_t a, posit32_t b);
	posit64_t  posit_div64 (posit64_t a, posit64_t b);
	posit128_t posit_div128(posit128_t a, posit128_t b);
	posit256_t posit_div256(posit256_t a, posit256_t b);
	// Square Root
	posit8_t   posit_sqrt8(posit8_t  a);
	posit16_t  posit_sqrt16(posit16_t a);
	posit32_t  posit_sqrt32(posit32_t a);
	posit64_t  posit_sqrt64(posit64_t a);
	posit128_t posit_sqrt128(posit128_t a);
	posit256_t posit_sqrt256(posit256_t a);
	// Natural Logarithm 
	posit8_t   posit_log8(posit8_t  a);
	posit16_t  posit_log16(posit16_t a);
	posit32_t  posit_log32(posit32_t a);
	posit64_t  posit_log64(posit64_t a);
	posit128_t posit_log128(posit128_t a);
	posit256_t posit_log256(posit256_t a);
	// Exponent 
	posit8_t   posit_exp8(posit8_t  a);
	posit16_t  posit_exp16(posit16_t a);
	posit32_t  posit_exp32(posit32_t a);
	posit64_t  posit_exp64(posit64_t a);
	posit128_t posit_exp128(posit128_t a);
	posit256_t posit_exp256(posit256_t a);

	// logic operators
	// equal: true if a == b, false otherwise
	bool       posit_equal8(posit8_t a, posit8_t b);
	bool       posit_equal16(posit16_t a, posit16_t b);
	bool       posit_equal32(posit32_t a, posit32_t b);
	bool       posit_equal64(posit64_t a, posit64_t b);
	bool       posit_equal128(posit128_t a, posit128_t b);
	bool       posit_equal256(posit256_t a, posit256_t b);
	// compare: -1 if a < b, 0 if a == b, +1 if a > b
	int        posit_cmp8(posit8_t a, posit8_t b);
	int        posit_cmp16(posit16_t a, posit16_t b);
	int        posit_cmp32(posit32_t a, posit32_t b);
	int        posit_cmp64(posit64_t a, posit64_t b);
	int        posit_cmp128(posit128_t a, posit128_t b);
	int        posit_cmp256(posit256_t a, posit256_t b);

#ifdef __cplusplus
}
#endif
