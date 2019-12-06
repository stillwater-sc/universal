//  posit_integer.cpp : test suite for conversions between integers and posits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer>
// configure the posit arithmetic class
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
// conversion layer
#include <universal/conversion/integer_to_posit.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
// test helpers
#include "../test_helpers.hpp"


#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "Conversion between integer and posit failed";

#if MANUAL_TESTING

	using int128_t = sw::unum::integer<128>;
	using int256_t = sw::unum::integer<256>;
	using posit8_t = sw::unum::posit<8,0>;
	using posit16_t = sw::unum::posit<16,1>;
	using posit32_t = sw::unum::posit<32,2>;
	using posit64_t = sw::unum::posit<64,3>;
	using posit128_t = sw::unum::posit<128,4>;
	using posit256_t = sw::unum::posit<256,5>;


	int128_t int128;
	int256_t int256;
	posit8_t p8;
	posit16_t p16;
	posit32_t p32;
	posit64_t p64;
	posit128_t p128;
	posit256_t p256;

	// decimal
	int128.assign("1234567890");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;

	// octal
	//	int128.assign("01234567");
	//cout << to_binary(int128) << " " << int128 << endl;

	// hex
	int128.assign("0x5555'5555");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;
	int128.assign("0x5555'5555'5555'5555");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;
	int128.assign("0x5555'5555'5555'5555'5555'5555'5555'5555");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;
	int128.assign("0x8000'0000'0000'0000'0000'0000'0000'0000");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;
	int128.assign("0xAAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;
	int128.assign("0xffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff");
	cout << to_binary(int128) << " " << int128 << " " << hexfloat << scale(int128) << defaultfloat << endl;

	convert_i2p(int128, p8);
	cout << color_print(p8) << endl;


	int256.assign("0xAAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA");

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;


#if STRESS_TESTING

#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
