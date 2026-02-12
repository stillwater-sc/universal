// posit.cpp: components of a posit: cli to show the sign/scale/regime/exponent/fraction components of standard posit configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>

const char* msg = "\
posit< 8,0>  = 01101001 : 3.125\n\
posit< 8, 1> = 01011001 : 3.125\n\
posit< 8, 2> = 01001101 : 3.25\n\
posit< 8, 3> = 01000110 : 3\n\
posit<16, 1> = 0101100100100010 : 3.1416\n\
posit<16, 2> = 0100110010010001 : 3.1416\n\
posit<16, 3> = 0100011001001000 : 3.1406\n\
posit<24, 1> = 010110010010000111111011 : 3.141592\n\
posit<24, 2> = 010011001001000011111110 : 3.141594\n\
posit<24, 3> = 010001100100100001111111 : 3.141594\n\
posit<32, 1> = 01011001001000011111101101010100 : 3.14159265\n\
posit<32, 2> = 01001100100100001111110110101010 : 3.14159265\n\
posit<32, 3> = 01000110010010000111111011010101 : 3.14159265\n\
posit<48, 1> = 010110010010000111111011010101000100010000101101 : 3.1415926535898\n\
posit<48, 2> = 010011001001000011111101101010100010001000010111 : 3.1415926535899\n\
posit<48, 3> = 010001100100100001111110110101010001000100001011 : 3.1415926535897\n\
posit<64, 1> = 0101100100100001111110110101010001000100001011010001100000000000 : 3.14159265358979312\n\
posit<64, 2> = 0100110010010000111111011010101000100010000101101000110000000000 : 3.14159265358979312\n\
posit<64, 3> = 0100011001001000011111101101010100010001000010110100011000000000 : 3.14159265358979312\n\
posit<64, 4> = 0100001100100100001111110110101010001000100001011010001100000000 : 3.14159265358979312\n";

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "posit : posit components\n";
		std::cerr << "Show the sign/scale/regime/exponent/fraction components of a posit.\n";
		std::cerr << "Usage: posit value\n\n";
		std::cerr << "Example: posit 3.1415926535897932384626433832795028841971\n";
		std::cerr <<  msg << '\n';
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	long double ld = strtold(argv[1], NULL);
	posit<8, 0>  p8_0(ld);
	posit<8, 1>  p8_1(ld);
	posit<8, 2>  p8_2(ld);
	posit<8, 3>  p8_3(ld);
	posit<16, 1> p16_1(ld);
	posit<16, 2> p16_2(ld);
	posit<16, 3> p16_3(ld);
	posit<24, 1> p24_1(ld);
	posit<24, 2> p24_2(ld);
	posit<24, 3> p24_3(ld);
	posit<32, 1> p32_1(ld);
	posit<32, 2> p32_2(ld);
	posit<32, 3> p32_3(ld);
	posit<48, 1> p48_1(ld);
	posit<48, 2> p48_2(ld);
	posit<48, 3> p48_3(ld);
	posit<64, 1> p64_1(ld);
	posit<64, 2> p64_2(ld);
	posit<64, 3> p64_3(ld);
	posit<64, 4> p64_4(ld);
	posit<80, 1> p80_1(ld);
	posit<80, 2> p80_2(ld);
	posit<80, 3> p80_3(ld);
	posit<80, 4> p80_4(ld);

//	typedef std::numeric_limits<long double > Real;
//	int precision = Real::max_digits10;
	constexpr size_t BIT_COLUMN_WIDTH = 10;
	std::cout << " posit< 8,0>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_0) << " : " << p8_0 << '\n';
	std::cout << " posit< 8,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_1) << " : " << p8_1 << '\n';
	std::cout << " posit< 8,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_2) << " : " << p8_2 << '\n';
	std::cout << " posit< 8,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_3) << " : " << p8_3 << '\n';

	std::cout << std::setprecision(5);
	std::cout << " posit<16,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p16_1) << " : " << p16_1 << '\n';
	std::cout << " posit<16,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p16_2) << " : " << p16_2 << '\n';
	std::cout << " posit<16,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p16_3) << " : " << p16_3 << '\n';
	
	std::cout << std::setprecision(7);
	std::cout << " posit<24,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p24_1) << " : " << p24_1 << '\n';
	std::cout << " posit<24,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p24_2) << " : " << p24_2 << '\n';
	std::cout << " posit<24,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p24_3) << " : " << p24_3 << '\n';

	std::cout << std::setprecision(9);
	std::cout << " posit<32,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p32_1) << " : " << p32_1 << '\n';
	std::cout << " posit<32,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p32_2) << " : " << p32_1 << '\n';
	std::cout << " posit<32,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p32_3) << " : " << p32_3 << '\n';
	
	std::cout << std::setprecision(14);
	std::cout << " posit<48,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p48_1) << " : " << p48_1 << '\n';
	std::cout << " posit<48,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p48_2) << " : " << p48_2 << '\n';
	std::cout << " posit<48,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p48_3) << " : " << p48_3 << '\n';
	
	std::cout << std::setprecision(18);
	std::cout << " posit<64,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_1) << " : " << p64_1 << '\n';
	std::cout << " posit<64,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_2) << " : " << p64_2 << '\n';
	std::cout << " posit<64,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_3) << " : " << p64_3 << '\n';
	std::cout << " posit<64,4>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_4) << " : " << p64_4 << '\n';

	std::cout << std::setprecision(18);
	std::cout << " posit<80,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p80_1) << " : " << p80_1 << '\n';
	std::cout << " posit<80,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p80_2) << " : " << p80_2 << '\n';
	std::cout << " posit<80,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p80_3) << " : " << p80_3 << '\n';
	std::cout << " posit<80,4>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p80_4) << " : " << p80_4 << '\n';

#ifdef WIDE_CONSOLE
	posit<128, 2> p128_2(d);
	posit<128, 3> p128_3(d);
	posit<128, 4> p128_4(d);
	posit<128, 5> p128_5(d);
	posit<256, 2> p256_2(d);
	posit<256, 3> p256_3(d);
	posit<256, 4> p256_4(d);
	posit<256, 5> p256_5(d);
	std::cout << "posit<128,2> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p128_2) << " : " << p128_2 << '\n';
	std::cout << "posit<128,3> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p128_3) << " : " << p128_3 << '\n';
	std::cout << "posit<128,4> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p128_4) << " : " << p128_4 << '\n';
	std::cout << "posit<128,5> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p128_5) << " : " << p128_5 << '\n';

	std::cout << "posit<256,2> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p256_2) << " : " << p256_2 << '\n';
	std::cout << "posit<256,3> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p256_3) << " : " << p256_3 << '\n';
	std::cout << "posit<256,4> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p256_4) << " : " << p256_4 << '\n';
	std::cout << "posit<256,5> = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p256_5) << " : " << p256_5 << '\n';
#endif

	std::cout << "\n\nDifferent printing formats\n";
	std::cout << "floating-point : " << std::setprecision(std::numeric_limits< posit<64,2> >::max_digits10) << p64_2 << '\n';
	std::cout << "triple form    : " << to_triple(p64_2) << '\n';
	std::cout << "binary form    : " << to_binary(p64_2, true) << '\n';
	std::cout << "pretty print   : " << pretty_print(p64_2) << '\n';
	std::cout << "color coded    : " << color_print(p64_2) << '\n';
	std::cout << "hex print      : " << hex_print(p64_2) << '\n';

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
