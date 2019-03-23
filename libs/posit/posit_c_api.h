#pragma once
// posit_api.h: generic C and C++ header defining the posit api
#ifdef __cplusplus
// export a C interface if used by C++ source code
extern "C" {
#endif

	//////////////////////////////////////////////////////////////////////
	// Standard posit configuration per the POSIT standard
	typedef unsigned char       posit8_t;
	typedef unsigned short      posit16_t;
	typedef unsigned long       posit32_t;
	typedef unsigned long long  posit64_t;
	typedef struct posit128_t {
		unsigned char data[16];
	} posit128_t;
	typedef struct posit256_t {
		unsigned char data[32];
	} posit256_t;

	// special posits
	typedef unsigned char       posit4_t;
	// for DSP applications and ADC/DAC pipelines
	typedef unsigned char       posit10_t;
	typedef unsigned char       posit12_t;
	typedef unsigned char       posit14_t;
	// for Linear Algebra and general CAD/CAE/CAM/HPC applications
	//////////////////////////////////////////////////////////////////////
	// posits between posit<32,2> and posit<64,3> staying with ES = 2
	typedef unsigned char       posit40_t;  // posit<40,2>
	typedef unsigned char       posit48_t;	// posit<48,2>
	typedef unsigned char       posit56_t;	// posit<56,2>
	//////////////////////////////////////////////////////////////////////
	// posits between posit<64,3> and posit<128,4> staying with ES = 3
	typedef unsigned char       posit80_t;  // posit<80,3>
	typedef unsigned char       posit96_t;  // posit<96,3>
	typedef unsigned char       posit112_t; // posit<112,3>

	//////////////////////////////////////////////////////////////////////
	// C API function definitions
	posit8_t  posit_add8 (posit8_t  a, posit8_t  b);
	/*
	posit16_t posit_add16(posit16_t a, posit16_t b);
	posit32_t posit_add32(posit32_t a, posit32_t b);
	posit64_t posit_add64(posit64_t a, posit64_t b);
	*/

#ifdef __cplusplus
}
#endif
