#pragma once
// positctypes.h: generic C header defining the posit types

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
// export a C interface if used by C++ source code
extern "C" {
#endif

	//////////////////////////////////////////////////////////////////////
	/// Standard posit configuration per the POSIT standard
	typedef union posit8_u   {
		uint8_t x[1];
		uint8_t v;
	}							posit8_t;	// posit<8,0>
	typedef union posit16_u  {
		uint8_t x[2];
		uint16_t v;
	}							posit16_t;	// posit<16,1>
	typedef union posit32_u {
		uint8_t x[4];
		uint32_t v;
	}							posit32_t;	// posit<32,2>
	typedef union posit64_u {
		uint8_t x[8];
		uint64_t v;
	}							posit64_t;	// posit<64,3>
	typedef union posit128_u {
		uint8_t x[16];
		uint64_t longs[2];
	} 							posit128_t;	// posit<128,4>
	typedef union posit256_u {
		uint8_t x[32];
		uint64_t longs[4];
	}							posit256_t;	// posit<256,5>

	// these storage formats are the same, but interpretation of the bits is specialized
	typedef posit8_t posit8_0_t; 
	typedef posit8_t posit8_1_t; 
	typedef posit8_t posit8_2_t; 
	typedef posit8_t posit8_3_t; 
	typedef posit16_t posit16_0_t; 
	typedef posit16_t posit16_1_t; 
	typedef posit16_t posit16_2_t; 
	typedef posit16_t posit16_3_t; 
	typedef posit32_t posit32_0_t; 
	typedef posit32_t posit32_1_t; 
	typedef posit32_t posit32_2_t; 
	typedef posit32_t posit32_3_t; 
	typedef posit64_t posit64_0_t; 
	typedef posit64_t posit64_1_t; 
	typedef posit64_t posit64_2_t; 
	typedef posit64_t posit64_3_t; 

	///////////////////////////////////////////////////////////////////////
	///   associated quire configurations
	typedef union quire8_u {
		uint8_t x[8];
		uint64_t v;
	}							quire8_t;	// quire<8,0,39>
	typedef union quire16_u {
		uint8_t x[16];
		uint64_t v[2];
	}							quire16_t;	// quire<16,1,15>
	typedef union quire32_u {
		uint8_t x[64];
		uint64_t v[8];
	}							quire32_t;	// quire<32,2,31>
	typedef union quire64_u {
		uint8_t x[256];
		uint64_t v[32];
	}							quire64_t;	// quire<64,3,63>
	typedef union quire128_u {
		uint8_t x[1024];
		uint64_t v[128];
	}							quire128_t;	// quire<128,4,127>
	typedef union quire256_u {
		uint8_t x[4096];
		uint64_t v[512];
	}							quire256_t; // quire<256,5,255>

	///	quire<  8, 0, 7>      32 bits		<--- likely not enough capacity bits
	///	quire< 16, 1, 15>    128 bits
	///	quire< 32, 2, 31>    512 bits
	///	quire< 64, 3, 63>   2048 bits
	///	quire<128, 4, 127>  8192 bits		<--- likely too many capacity bits
	///	quire<256, 5, 7>   32520 bits		<--- 4065 bytes: smallest size aligned to 4byte boundary
	///	quire<256, 5, 255> 32768 bits       <--- 4096 bytes

	//////////////////////////////////////////////////////////////////////
	/// special posits

	//////////////////////////////////////////////////////////////////////
	// for Deep Learning/AI algorithms
	typedef union posit4_u   {
		uint8_t x[1];
		uint8_t v;
	}							posit4_t;	// posit<4,0>
#ifdef DEEP_LEARNING
	typedef union posit5_u   {
		uint8_t x[1];
		uint8_t v;
	}							posit5_t;	// posit<5,0>
	typedef union posit6_u   {
		uint8_t x[1];
		uint8_t v;
	}							posit6_t;	// posit<6,0>
	typedef union posit7_u   {
		uint8_t x[1];
		uint8_t v;
	}							posit7_t;	// posit<7,0>
#endif // DEEP_LEARNING

#ifdef DSP_PIPELINES
	//////////////////////////////////////////////////////////////////////
	// for DSP applications and ADC/DAC pipelines
	typedef union posit9_u  {
		uint8_t x[2];
		uint16_t v;
	}							posit9_t;	// posit<9,0>
	typedef union posit10_u  {
		uint8_t x[2];
		uint16_t v;
	}							posit10_t;	// posit<10,0>
	typedef union posit12_u  {
		uint8_t x[2];
		uint16_t v;
	}							posit12_t;	// posit<12,0>
	typedef union posit14_u  {
		uint8_t x[2];
		uint16_t v;
	}							posit14_t;	// posit<14,0>
#endif // DSP_PIPELINES

#ifdef EXTENDED_STANDARD
	//////////////////////////////////////////////////////////////////////
	// for Linear Algebra and general CAD/CAE/CAM/HPC applications

	//////////////////////////////////////////////////////////////////////
	// posits between posit<16,1> and posit<32,2> staying with ES = 1
	typedef union posit20_u {
		uint8_t x[4];
		uint32_t v;
	}							posit20_t;	// posit<20,1>
	typedef union posit24_u {
		uint8_t x[4];
		uint32_t v;
	}							posit24_t;	// posit<24,1>

	// posits between posit<32,2> and posit<64,3> staying with ES = 2
	// notice we keep the cast to a uint64_t
	typedef union posit40_u {
		uint8_t x[8];
		uint64_t v;
	}							posit40_t;	// posit<40,2>
	typedef union posit48_u {
		uint8_t x[8];
		uint64_t v;
	}							posit48_t;	// posit<48,2>
	typedef union posit56_u {
		uint8_t x[8];
		uint64_t v;
	}							posit56_t;	// posit<56,2>

	//////////////////////////////////////////////////////////////////////
	// posits between posit<64,3> and posit<128,4> staying with ES = 3
	typedef union posit80_u {
		uint8_t x[10];
		//uint64_t v[2];// if we cast it to exactly 10 bytes, this cast would not work
	}							posit80_t;	// posit<80,3>
	typedef union posit96_u {
		uint8_t x[12];
		//uint64_t v[2];
	}							posit96_t;	// posit<96,3>
	typedef union posit96_u {
		uint8_t x[14];
		//uint64_t v[2];
	}							posit112_t;	// posit<112,3>
#endif // EXTENDED_STANDARD

	//////////////////////////////////////////////////////////////////////
	// C API function definitions

	//////////////////////////////////////////////////////////////////////
	// Important posit constants // we a storing this in little endian
	static const posit4_t  NAR4  = {{
		0x08
	}};
	static const posit8_t  NAR8  = {{
		0x80
	}};
	static const posit16_t NAR16 = {{
		0x00, 0x80
	}};
	static const posit32_t NAR32 = {{
		0x00, 0x00, 0x00, 0x80
	}};
	static const posit64_t NAR64 = {{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
	}};
	static const posit128_t NAR128 = {{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	}};
	static const posit256_t NAR256 = {{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	}};

	static const posit4_t   ZERO4   = {{ 0 }};
	static const posit8_t   ZERO8   = {{ 0 }};
	static const posit16_t  ZERO16  = {{ 0 }};
	static const posit32_t  ZERO32  = {{ 0 }};
	static const posit64_t  ZERO64  = {{ 0 }};
	static const posit128_t ZERO128 = {{ 0 }};
	static const posit256_t ZERO256 = {{ 0 }};


#ifdef __cplusplus
}
#endif
