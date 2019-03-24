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

	//////////////////////////////////////////////////////////////////////
	// Important posit constants
	static const posit8_t  NAR8  = 0x80;
	static const posit16_t NAR16 = 0x8000;
	static const posit32_t NAR32 = 0x8000'0000;
	static const posit64_t NAR64 = 0x8000'0000'0000'0000;

	///////////////////////////////////////////////////////////////
/////////        output

// report posit format for posit8_t. str must be at least 8 characters in size
	void posit_format8(posit8_t a, char* str);
	// report posit format for posit16_t. str must be at least 10 characters in size
	void posit_format16(posit16_t a, char* str);
	// report posit format for posit32_t. str must be at least 14 characters in size
	void posit_format32(posit32_t a, char* str);
	// report posit format for posit64_t. str must be at least 22 characters in size
	void posit_format64(posit64_t a, char* str);

	// casts to double
	double posit_value8(posit8_t a);
	double posit_value16(posit16_t a);
	double posit_value32(posit32_t a);
	double posit_value64(posit64_t a);

	// Raw bit assignments
	posit8_t  posit_bit_assign8(unsigned char  a);
	posit16_t posit_bit_assign16(unsigned short a);
	posit32_t posit_bit_assign32(unsigned long a);
	posit64_t posit_bit_assign64(unsigned long long a);

	// Integer assignments
	posit8_t  posit_integer_assign8(int  a);
	posit16_t posit_integer_assign16(int a);
	posit32_t posit_integer_assign32(long a);
	posit64_t posit_integer_assign64(long long a);

	// IEEE floating point assignments
	posit8_t  posit_float_assign8(float  a);
	posit16_t posit_float_assign16(float a);
	posit32_t posit_float_assign32(double a);
	posit64_t posit_float_assign64(long double a);

	// Addition
	posit8_t  posit_add8 (posit8_t  a, posit8_t  b);
	posit16_t posit_add16(posit16_t a, posit16_t b);
	posit32_t posit_add32(posit32_t a, posit32_t b);
	posit64_t posit_add64(posit64_t a, posit64_t b);
	// Subtraction
	posit8_t  posit_sub8(posit8_t  a, posit8_t  b);
	posit16_t posit_sub16(posit16_t a, posit16_t b);
	posit32_t posit_sub32(posit32_t a, posit32_t b);
	posit64_t posit_sub64(posit64_t a, posit64_t b);
	// Multiplication
	posit8_t  posit_mul8(posit8_t  a, posit8_t  b);
	posit16_t posit_mul16(posit16_t a, posit16_t b);
	posit32_t posit_mul32(posit32_t a, posit32_t b);
	posit64_t posit_mul64(posit64_t a, posit64_t b);
	// Division
	posit8_t  posit_div8(posit8_t  a, posit8_t  b);
	posit16_t posit_div16(posit16_t a, posit16_t b);
	posit32_t posit_div32(posit32_t a, posit32_t b);
	posit64_t posit_div64(posit64_t a, posit64_t b);
	// Square Root
	posit8_t  posit_sqrt8(posit8_t  a);
	posit16_t posit_sqrt16(posit16_t a);
	posit32_t posit_sqrt32(posit32_t a);
	posit64_t posit_sqrt64(posit64_t a);


#ifdef __cplusplus
}
#endif
