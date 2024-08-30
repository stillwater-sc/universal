// rounding.cpp : rounding and assignment test suite for fixed-sized, arbitrary precision integers to real number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the posit arithmetic
#include <universal/number/posit/posit.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

////////////////// rounding rules

/*
Rounding rules:
  ULP = Unit in the Last Place
  G   = guard bit
  R   = round bit
  S   = sticky bit
 ...ULP|GRS...
  GRS | Action
  0xx | round-down
  100 | tie: round-up to even when ULP = 1, else round down
  101 | round-up
  110 | round-up
  111 | round-up

  sticky = OR(remaining bits)
 */

// generate a posit conversion test case
// process to convert an integer to a posit is to
// transform the integer into a 1.####eExp format
// find msb -> scale is msb
// shift all the msb-1 bits into a fraction, making the msb the hidden bit
// round the bits we have with respect to the scale of the number
template<unsigned nbits, unsigned es, unsigned ibits>
void GeneratePositConversionTestCase(sw::universal::posit<nbits, es>& p, const sw::universal::integer<ibits>& w) {
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



template<unsigned nbits>
void VerifyScale() {
	assert(nbits > 1); // we are representing numbers not booleans
	int cntr = 0;
	sw::universal::integer<nbits> i = 1;
	while (cntr < nbits) {
		std::cout << std::setw(20) << sw::universal::to_binary(i) << std::setw(20) << i << " scale is " << sw::universal::scale(i) << std::endl;
		i *= 2;
		++cntr;
	}
	// enumerate the negative integers
	// i is zero at this point
	i.set(nbits - 1, true); i >>= 1; i.set(nbits-1, true);
	cntr = 1;
	while (cntr < nbits) {
		std::cout << std::setw(20) << sw::universal::to_binary(i) << std::setw(20) << i << " scale is " << sw::universal::scale(i) << std::endl;
		i >>= 1; i.set(nbits-1, true);
		++cntr;
	}
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

	std::string test_suite  = "Integer Rounding";
	std::string test_tag    = "rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	//using int10_t = integer<10>;
	//using int11_t = integer<11>;
	//using int12_t = integer<12>;
	//using int13_t = integer<13>;
	using int14_t = integer<14>;
	using int15_t = integer<15>;
	using int16_t = integer<16>;
	//using int17_t = integer<17>;
	//using int18_t = integer<18>;

	int14_t i14 = 0x1fff;
	int15_t i15 = 0x3fff;
	int16_t i16 = 0x7fff;

	cout << to_binary(i14) << " " << i14 << endl;
	cout << to_binary(i15) << " " << i15 << endl;
	cout << to_binary(i16) << " " << i16 << endl;

	using posit8_t = posit<8, 0>;
	posit8_t p8;
	GeneratePositConversionTestCase(p8, i14);
	GeneratePositConversionTestCase(p8, i15);
	GeneratePositConversionTestCase(p8, i16);

	using posit16_t = posit<16, 1>;
	posit16_t p16;
	GeneratePositConversionTestCase(p16, i14);
	GeneratePositConversionTestCase(p16, i15);
	GeneratePositConversionTestCase(p16, i16);

	// create the 5 rounding configurations for a 14bit integer
	//using int32_t = integer<32>;
	//int32_t tie = 0x00001fff;

	// if we take the posit around 1.0 then we know exactly that the scale is 0
	// and the rounding-down and rounding-up cases are then easily constructed.
	// say we have a posit<16,1>, it has 1 sign bit, 2 regime bits, 1 exponent
	// bit, and 12 mantissa bits

	// using posit32_t = posit<32, 2>;

	//VerifyScale<16>();
	//VerifyScale<24>();
	//VerifyScale<32>();

	//cout << "minimum for integer<16> " << min_int<16>() << endl;
	//cout << "maximum for integer<16> " << max_int<16>() << endl;


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
#if REGRESSION_LEVEL_1

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
