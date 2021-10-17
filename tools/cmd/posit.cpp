// posit.cpp: components of a posit: cli to show the sign/scale/regime/exponent/fraction components of standard posit configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>

const char* msg = "posit< 8, 0> = s1 r1111111 e f qNW v-64\n\
posit< 8, 1> = s1 r1111111 e f qNW v-4096\n\
posit< 8, 2> = s1 r1111111 e f qNW v-16777216\n\
posit< 8, 3> = s1 r1111111 e f qNW v-281474976710656\n\
posit<16, 1> = s1 r111111111111111 e f qNW v-268435456\n\
posit<16, 2> = s1 r111111111111111 e f qNW v-72057594037927936\n\
posit<16, 3> = s1 r111111110 e000 f100 qNW v-1.080863910568919e+17\n\
posit<32, 1> = s1 r111111111111111111111111111110 e1 f qNW v-1.4411518807585587e+17\n\
posit<32, 2> = s1 r1111111111111110 e00 f1000111100100 qNW v-1.1234370007964058e+17\n\
posit<32, 3> = s1 r111111110 e000 f1000111100100001110 qNW v-1.1234562422498918e+17\n\
posit<48, 1> = s1 r111111111111111111111111111110 e0 f1000111100100010 qNW v-1.1234589910289613e+17\n\
posit<48, 2> = s1 r1111111111111110 e00 f10001111001000011100110010111 qNW v-1.1234567885160448e+17\n\
posit<48, 3> = s1 r111111110 e000 f10001111001000011100110010111010111 qNW v-1.1234567889983898e+17\n\
posit<64, 1> = s1 r111111111111111111111111111110 e0 f10001111001000011100110010111011 qNW v-1.1234567890193613e+17\n\
posit<64, 2> = s1 r1111111111111110 e00 f100011110010000111001100101110101110001001111 qNW v-1.1234567890000077e+17\n\
posit<64, 3> = s1 r111111110 e000 f100011110010000111001100101110101110001001110101000 qNW v-1.123456789e+17\n\
posit<64, 4> = s1 r11110 e1000 f100011110010000111001100101110101110001001110101000000 qNW v-1.123456789e+17\n";

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "posit : posit components\n";
		std::cerr << "Show the sign/scale/regime/exponent/fraction components of a posit.\n";
		std::cerr << "Usage: posit value\n";
		std::cerr << "Example: posit -1.123456789e17\n";
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


	typedef std::numeric_limits<long double > Real;
	int precision = Real::max_digits10;
	constexpr size_t BIT_COLUMN_WIDTH = 10;
	std::cout << "posit< 8,0>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_0) << " : " << p8_0 << '\n';
	std::cout << "posit< 8,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_1) << " : " << p8_1 << '\n';
	std::cout << "posit< 8,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_2) << " : " << p8_2 << '\n';
	std::cout << "posit< 8,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p8_3) << " : " << p8_3 << '\n';

	std::cout << std::setprecision(5);
	std::cout << "posit<16,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p16_1) << " : " << p16_1 << '\n';
	std::cout << "posit<16,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p16_2) << " : " << p16_2 << '\n';
	std::cout << "posit<16,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p16_3) << " : " << p16_3 << '\n';
	
	std::cout << std::setprecision(7);
	std::cout << "posit<24,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p24_1) << " : " << p24_1 << '\n';
	std::cout << "posit<24,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p24_2) << " : " << p24_2 << '\n';
	std::cout << "posit<24,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p24_3) << " : " << p24_3 << '\n';

	std::cout << std::setprecision(9);
	std::cout << "posit<32,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p32_1) << " : " << p32_1 << '\n';
	std::cout << "posit<32,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p32_2) << " : " << p32_1 << '\n';
	std::cout << "posit<32,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p32_3) << " : " << p32_3 << '\n';
	
	std::cout << std::setprecision(14);
	std::cout << "posit<48,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p48_1) << " : " << p48_1 << '\n';
	std::cout << "posit<48,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p48_2) << " : " << p48_2 << '\n';
	std::cout << "posit<48,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p48_3) << " : " << p48_3 << '\n';
	
	std::cout << std::setprecision(18);
	std::cout << "posit<64,1>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_1) << " : " << p64_1 << '\n';
	std::cout << "posit<64,2>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_2) << " : " << p64_2 << '\n';
	std::cout << "posit<64,3>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_3) << " : " << p64_3 << '\n';
	std::cout << "posit<64,4>  = " << std::setw(BIT_COLUMN_WIDTH) << color_print(p64_4) << " : " << p64_4 << '\n';

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
