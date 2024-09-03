// propenv.cpp: cli to show the type properties of the compiler environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/arithmetic_traits.hpp>
// configure the posit environment
#define POSIT_FAST_SPECIALIZATION 1
#include <universal/number/posit/posit.hpp>

//#define POSIT_DECODED_CLASS
#ifdef POSIT_DECODED_CLASS
#include "posit_decoded.hpp"

// the decoded posits structure is caching decoded regime, exponent, and fraction ballooning the size 
// of the class and making it unusable for real computational work.
void WhyWeRemovedDecodedPosits() {
	using namespace sw::universal;

	// report on the size of different posit components and implementations
	posit<4, 0> p4_0;
	posit_decoded<4, 0> pd4_0;
	posit<8, 0> p8_0;
	posit_decoded<8, 0> pd8_0;
	posit<16, 1> p16_1;
	posit_decoded<16, 1> pd16_1;
	posit<32, 2> p32_2;
	posit_decoded<32, 2> pd32_2;
	posit<64, 3> p64_3;
	posit_decoded<64, 3> pd64_3;
	posit<128, 4> p128_4;
	posit_decoded<128, 4> pd128_4;
	std::cout << std::left << std::setw(20) << "configuration" << std::setw(10) << "bytes" << '\n';
	std::cout << std::left << std::setw(20) << "posit<4,0>" << std::setw(10) << sizeof(p4_0) << '\n';
	std::cout << std::left << std::setw(20) << "decoded<4,0>" << std::setw(10) << sizeof(pd4_0) << '\n';
	std::cout << std::left << std::setw(20) << "posit<8,0>" << std::setw(10) << sizeof(p8_0) << '\n';
	std::cout << std::left << std::setw(20) << "decoded<8,0>" << std::setw(10) << sizeof(pd8_0) << '\n';
	std::cout << std::left << std::setw(20) << "posit<16,1>" << std::setw(10) << sizeof(p16_1) << '\n';
	std::cout << std::left << std::setw(20) << "decoded<16,1>" << std::setw(10) << sizeof(pd16_1) << '\n';
	std::cout << std::left << std::setw(20) << "posit<32,2>" << std::setw(10) << sizeof(p32_2) << '\n';
	std::cout << std::left << std::setw(20) << "decoded<32,2>" << std::setw(10) << sizeof(pd32_2) << '\n';
	std::cout << std::left << std::setw(20) << "posit<64,3>" << std::setw(10) << sizeof(p64_3) << '\n';
	std::cout << std::left << std::setw(20) << "decoded<64,3>" << std::setw(10) << sizeof(pd64_3) << '\n';
	std::cout << std::left << std::setw(20) << "posit<128,4>" << std::setw(10) << sizeof(p128_4) << '\n';
	std::cout << std::left << std::setw(20) << "decoded<128,4>" << std::setw(10) << sizeof(pd128_4) << '\n';
}
#endif


// Report on size and numeric limits details of different data types
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// long double attributes
	constexpr int f_prec = std::numeric_limits<float>::max_digits10;
	//constexpr int d_prec = std::numeric_limits<double>::max_digits10;
	constexpr int q_prec = std::numeric_limits<long double>::max_digits10;

	constexpr int uint8_bits  = std::numeric_limits<unsigned char>::digits;
	constexpr int uint16_bits = std::numeric_limits<unsigned short>::digits;
	constexpr int uint32_bits = std::numeric_limits<unsigned int>::digits;
	constexpr int uint64_bits = std::numeric_limits<unsigned long long>::digits;
	constexpr int int8_bits   = std::numeric_limits<signed char>::digits;
	constexpr int int16_bits  = std::numeric_limits<short>::digits;
	constexpr int int32_bits  = std::numeric_limits<int>::digits;
	constexpr int int64_bits  = std::numeric_limits<long long>::digits;
	constexpr int f_fbits     = std::numeric_limits<float>::digits;
	constexpr int d_fbits     = std::numeric_limits<double>::digits;
	constexpr int q_fbits     = std::numeric_limits<long double>::digits;

	std::cout << "Bit sizes for native types" << '\n';
	std::cout << "unsigned char        " << std::setw(4) << uint8_bits  << " bits" << '\n';
	std::cout << "unsigned short       " << std::setw(4) << uint16_bits << " bits" << '\n';
	std::cout << "unsigned int         " << std::setw(4) << uint32_bits << " bits" << '\n';
	std::cout << "unsigned long long   " << std::setw(4) << uint64_bits << " bits" << '\n';
	std::cout << "  signed char        " << std::setw(4) << int8_bits   << " bits" << '\n';
	std::cout << "  signed short       " << std::setw(4) << int16_bits  << " bits" << '\n';
	std::cout << "  signed int         " << std::setw(4) << int32_bits  << " bits" << '\n';
	std::cout << "  signed long long   " << std::setw(4) << int64_bits  << " bits" << '\n';
	std::cout << "         float       " << std::setw(4) << f_fbits     << " bits" << '\n';
	std::cout << "         double      " << std::setw(4) << d_fbits     << " bits" << '\n';
	std::cout << "         long double " << std::setw(4) << q_fbits     << " bits" << '\n';
	std::cout << '\n';


	std::cout << "Min-Max range for floats and posit<32,2> comparison\n";
	std::cout << minmax_range<float>() << '\n';
	std::cout << minmax_range<posit<32, 2>>() << '\n';
	std::cout << '\n';

	// report on the size of different posit components and implementations
	posit<4, 0> p4_0;
	posit<8, 0> p8_0;
	posit<16, 1> p16_1;
	posit<20, 1> p20_1;
	posit<24, 1> p24_1;
	posit<28, 1> p28_1;
	posit<32, 2> p32_2;
	posit<40, 2> p40_2;
	posit<48, 2> p48_2;
	posit<56, 2> p56_2;
	posit<64, 3> p64_3;
	posit<80, 3> p80_3;
	posit<96, 3> p96_3;
	posit<112, 3> p112_3;
	posit<128, 4> p128_4;
	posit<256, 5> p256_5;
	constexpr int columnWidth = 21;
	std::cout << "Bit sizes for standard posit configurations\n";
	std::cout << std::left << std::setw(columnWidth) << "posit<8,0>" << std::setw(4) << std::right << sizeof(p8_0) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<16,1>" << std::setw(4) << std::right << sizeof(p16_1) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<32,2>" << std::setw(4) << std::right << sizeof(p32_2) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<64,3>" << std::setw(4) << std::right << sizeof(p64_3) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<128,4>" << std::setw(4) << std::right << sizeof(p128_4) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<256,5>" << std::setw(4) << std::right << sizeof(p256_5) * 8 << " bits" << '\n';
	std::cout << '\n';

	std::cout << '\n';
	std::cout << "Bit sizes for extended posit configurations\n";
	std::cout << std::left << std::setw(columnWidth) << "posit<4,0>" << std::setw(4) << std::right << sizeof(p4_0) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<8,0>" << std::setw(4) << std::right << sizeof(p8_0) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<16,1>" << std::setw(4) << std::right << sizeof(p16_1) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<20,1>" << std::setw(4) << std::right << sizeof(p20_1) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<24,1>" << std::setw(4) << std::right << sizeof(p24_1) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<28,1>" << std::setw(4) << std::right << sizeof(p28_1) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<32,2>" << std::setw(4) << std::right << sizeof(p32_2) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<40,2>" << std::setw(4) << std::right << sizeof(p40_2) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<48,2>" << std::setw(4) << std::right << sizeof(p48_2) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<56,2>" << std::setw(4) << std::right << sizeof(p56_2) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<64,3>" << std::setw(4) << std::right << sizeof(p64_3) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<80,3>" << std::setw(4) << std::right << sizeof(p80_3) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<96,3>" << std::setw(4) << std::right << sizeof(p96_3) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<112,3>" << std::setw(4) << std::right << sizeof(p112_3) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<128,4>" << std::setw(4) << std::right << sizeof(p128_4) * 8 << " bits" << '\n';
	std::cout << std::left << std::setw(columnWidth) << "posit<256,5>" << std::setw(4) << std::right << sizeof(p256_5) * 8 << " bits" << '\n';
	std::cout << '\n';

	std::cout << "Long double properties" << '\n';
	union {
		long double da;
		char        bytes[16];
	} ld;
	ld.da = 1.234567890123456789;

	bool        sign  	    = false;
	int         scale 	    = 0;
	long double fr    	    = 0;
	if constexpr (std::numeric_limits<long double>::digits <= 64) {
		std::uint64_t fraction = 0;
		sw::universal::extract_fp_components(ld.da, sign, scale, fr, fraction);

		std::cout << "value    " << std::setprecision(q_prec) << ld.da << std::setprecision(f_prec) << '\n';
		std::cout << "hex      ";
		std::cout << std::hex << std::setfill('0');
		for (int i = 15; i >= 0; i--) {
			std::cout << std::setw(2) << (int)(uint8_t)ld.bytes[i] << " ";
		}
		std::cout << std::dec << std::setfill(' ') << '\n';
		std::cout << "sign     " << (sign ? "-" : "+") << '\n';
		std::cout << "scale    " << scale << '\n';
		std::cout << "fraction " << fraction << '\n';
	} else {
		sw::universal::internal::uint128 fraction{0};
		sw::universal::extract_fp_components(ld.da, sign, scale, fr, fraction);

		std::cout << "value         " << std::setprecision(q_prec) << ld.da << std::setprecision(f_prec) << '\n';
		std::cout << "hex           ";
		std::cout << std::hex << std::setfill('0');
		for (int i = 15; i >= 0; i--) {
			std::cout << std::setw(2) << (int)(uint8_t)ld.bytes[i] << " ";
		}
		std::cout << std::dec << std::setfill(' ') << '\n';
		std::cout << "sign          " << (sign ? "-" : "+") << '\n';
		std::cout << "scale         " << scale << '\n';
		std::cout << "fraction upper" << fraction.upper << '\n';
		std::cout << "fraction lower" << fraction.lower << '\n';
	}

	std::cout << std::endl;

	// WhyWeRemovedDecodedPosits();

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
