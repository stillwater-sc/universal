// to_posit.cpp : test suite for conversions to posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable conversion between posits and integers
#include <universal/adapters/adapt_integer_and_posit.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
// configure the posit arithmetic class
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

// is representable
#include <math/functions/isrepresentable.hpp>
#include <universal/verification/test_suite.hpp>

// generate a posit conversion test case
// process to convert an integer to a posit is to
// transform the integer into a 1.####eExp format
// find msb -> scale is msb
// shift all the msb-1 bits into a fraction, making the msb the hidden bit
// round the bits we have with respect to the scale of the number
template<unsigned nbits, unsigned es, unsigned ibits, typename BlockType, sw::universal::IntegerNumberType NumberType>
void GeneratePositConversionTestCase(sw::universal::posit<nbits, es>& p, const sw::universal::integer<ibits, BlockType, NumberType>& w) {
	using namespace std;
	using namespace sw::universal;

	internal::value<ibits> v;

	bool sign = w < 0;
	bool isZero = w == 0;
	bool isInf = false;
	bool isNan = false;
	long _scale = scale(w);
	int msb = findMsb(w);
	internal::bitblock<ibits> fraction_without_hidden_bit;
	int fbit = ibits - 1;
	for (int i = msb - 1; i >= 0; --i) {
		fraction_without_hidden_bit.set(fbit, w.at(i));
		--fbit;
	}
	v.set(sign, _scale, fraction_without_hidden_bit, isZero, isInf, isNan);
	cout << "integer is " << w << endl;
	cout << "value is   " << v << endl;
	p = v;
	cout << "posit is   " << color_print(p) << " " << p << " " << hex_format(p) << endl;
}

template<unsigned ibits, unsigned pbits, unsigned pes>
int VerifyInteger2PositConversion(bool reportTestCases) {
	using namespace std;
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	posit<pbits, pes> p;
	integer<ibits> i;
	constexpr unsigned NR_INTEGERS = (1 << ibits);
	//for (integer<ibits> i = min_int<ibits>(); i <= max_int<ibits>(); ++i) {  // this doesn't work for signed integers
	for (unsigned pattern = 0; pattern < NR_INTEGERS; ++pattern) {
		i.setbits(pattern);
		p = i; 
		// p = i requires ADAPTER_POSIT_AND_INTEGER to be set which is accomplished by
		// #include <universal/adapters/adapt_integer_and_posit.hpp>
		// we need to enhance this with an integer type concept
		long diff = long(p) - long(i);
		cout << setw(ibits) << i << " " << to_binary(i) << " -> " << color_print(p) << setw(ibits) << p << " diff is " << diff << std::endl;
		if (diff != 0) ++nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string test_suite  = "Integer to posit conversion verfication";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using int128_t = sw::universal::integer<128, uint32_t, IntegerNumberType::IntegerNumber>;
	using int256_t = sw::universal::integer<256, uint32_t, IntegerNumberType::IntegerNumber>;

	int128_t int128;
	int256_t int256;

/*
	using posit8_t = sw::universal::posit<8,0>;
	using posit16_t = sw::universal::posit<16,1>;
	using posit32_t = sw::universal::posit<32,2>;
	using posit64_t = sw::universal::posit<64,3>;
	using posit128_t = sw::universal::posit<128,4>;
	using posit256_t = sw::universal::posit<256,5>;
	posit8_t p8;
	posit16_t p16;
	posit32_t p32;
	posit64_t p64;
	posit128_t p128;
	posit256_t p256;
*/

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

	{
		int128.assign("0x5555'5555'5555'5555'5555'5555'5555'5555");
		posit<32, 5> p;
		GeneratePositConversionTestCase(p, int128);
	}

	int256.assign("0xAAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA'AAAA");

	{
		integer<5> bla = -15;
		posit<12, 1> p;
		convert_i2p(bla, p);
		cout << color_print(p) << " " << p << endl;
	}

	nrOfFailedTestCases += ReportTestResult(VerifyInteger2PositConversion<5, 5, 1>(reportTestCases), "integer<5> -> posit<5,1>", "=");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyInteger2PositConversion<5, 5, 1>(reportTestCases), "integer<5> -> posit<5,1>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyInteger2PositConversion<5, 8, 1>(reportTestCases), "integer<5> -> posit<8,1>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyInteger2PositConversion<5, 12, 1>(reportTestCases), "integer<5> -> posit<12,1>", "=");

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
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
