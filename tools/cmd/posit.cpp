// posit.cpp: components of a posit: cli to show the sign/scale/regime/exponent/fraction components of standard posit configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit>

typedef std::numeric_limits< double > dbl;
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
	using namespace std;
	using namespace sw::universal;

	if (argc != 2) {
		cerr << "posit : posit components" << endl;
		cerr << "Show the sign/scale/regime/exponent/fraction components of a posit." << endl;
		cerr << "Usage: posit float_value" << endl;
		cerr << "Example: posit -1.123456789e17" << endl;
		cerr <<  msg << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	double d = atof(argv[1]);
	posit<8, 0>  p8_0(d);
	posit<8, 1>  p8_1(d);
	posit<8, 2>  p8_2(d);
	posit<8, 3>  p8_3(d);
	posit<16, 1> p16_1(d);
	posit<16, 2> p16_2(d);
	posit<16, 3> p16_3(d);
	posit<32, 1> p32_1(d);
	posit<32, 2> p32_2(d);
	posit<32, 3> p32_3(d);
	posit<48, 1> p48_1(d);
	posit<48, 2> p48_2(d);
	posit<48, 3> p48_3(d);
	posit<64, 1> p64_1(d);
	posit<64, 2> p64_2(d);
	posit<64, 3> p64_3(d);
	posit<64, 4> p64_4(d);
	posit<128, 2> p128_2(d);
	posit<128, 3> p128_3(d);
	posit<128, 4> p128_4(d);
	posit<128, 5> p128_5(d);
	posit<256, 2> p256_2(d);
	posit<256, 3> p256_3(d);
	posit<256, 4> p256_4(d);
	posit<256, 5> p256_5(d);

	int precision = dbl::max_digits10;
	cout << "posit< 8,0>  = " << pretty_print(p8_0, precision) << endl;
	cout << "posit< 8,1>  = " << pretty_print(p8_1, precision) << endl;
	cout << "posit< 8,2>  = " << pretty_print(p8_2, precision) << endl;
	cout << "posit< 8,3>  = " << pretty_print(p8_3, precision) << endl;
	cout << "posit<16,1>  = " << pretty_print(p16_1, precision) << endl;
	cout << "posit<16,2>  = " << pretty_print(p16_2, precision) << endl;
	cout << "posit<16,3>  = " << pretty_print(p16_3, precision) << endl;
	cout << "posit<32,1>  = " << pretty_print(p32_1, precision) << endl;
	cout << "posit<32,2>  = " << pretty_print(p32_2, precision) << endl;
	cout << "posit<32,3>  = " << pretty_print(p32_3, precision) << endl;
	cout << "posit<48,1>  = " << pretty_print(p48_1, precision) << endl;
	cout << "posit<48,2>  = " << pretty_print(p48_2, precision) << endl;
	cout << "posit<48,3>  = " << pretty_print(p48_3, precision) << endl;
	cout << "posit<64,1>  = " << pretty_print(p64_1, precision) << endl;
	cout << "posit<64,2>  = " << pretty_print(p64_2, precision) << endl;
	cout << "posit<64,3>  = " << pretty_print(p64_3, precision) << endl;
	cout << "posit<64,4>  = " << pretty_print(p64_4, precision) << endl;

	cout << "posit<128,2> = " << pretty_print(p128_2, precision) << endl;
	cout << "posit<128,3> = " << pretty_print(p128_3, precision) << endl;
	cout << "posit<128,4> = " << pretty_print(p128_4, precision) << endl;
	cout << "posit<128,5> = " << pretty_print(p128_5, precision) << endl;

	cout << "posit<256,2> = " << pretty_print(p256_2, precision) << endl;
	cout << "posit<256,3> = " << pretty_print(p256_3, precision) << endl;
	cout << "posit<256,4> = " << pretty_print(p256_4, precision) << endl;
	cout << "posit<256,5> = " << pretty_print(p256_5, precision) << endl;

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
