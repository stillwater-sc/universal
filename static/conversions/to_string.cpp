// to_string.cpp : test suite for conversions to string
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
// configure the posit arithmetic
#include <universal/number/posit/posit.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>

using namespace sw::universal;

/*
  Given an n-digit radix-2 fraction f, 0 <= f <= 1:
	   f = . f_-1, f_-2, ...f_-n = SUM f_-i * b^-i from i = 1 to n
  Output the digits F_i of an N-digit radix-10 fraction F:
	   F = . F_-1, F_-2, ...F_-N = SUM F_-i * B^-i from i = 1 to N

  Such that:
	1-  | F - f | < b^-n / 2: that is, difference is less then 0.5 ULP in radix-2
	2-  N is the smallest integer >= 1 such that (1) can be true
	3-  | F - f | < B^-N / 2: that is, difference is also less than 0.5 ULP in radix-10
	4-  F digits are generated without needing correction
 */
template<size_t fbits>
std::string fp3(float f) {
	using Fraction = fixpnt<fbits+6, fbits+1, Modulo, uint8_t>;
	std::string F;
	size_t len = fbits / 3 + 2ull;
	F.assign(len, '0');
	Fraction B = 10;
	Fraction R, U;
	Fraction M;
	M.setbit(0, true);  // b^-n/2
	U = f; // marshall the float value into the fraction compute environment
	R = U.fraction();
	sqrt(R);

	std::cout << "R : " << to_binary(R) << " : " << float(R) << '\n';
	std::cout << "M : " << to_binary(M) << " : " << float(M) << '\n';
	int digit{ 0 };
	size_t k{ 1 };
	do {
		U = R * B;  // push the next digit out
		digit = int(U);
		std::cout << "D : " << digit << '\n';
		R = (R * B).fraction();
		M = M * B;
		std::cout << "U : " << to_binary(U) << " : " << float(U) << '\n';
		std::cout << "R : " << to_binary(R) << " : " << float(R) << '\n';
		std::cout << "M : " << to_binary(M) << " : " << float(M) << '\n';
		if (R >= M && R <= (1 - M)) {
			F[k] = '0' + static_cast<char>(digit);
			std::cout << "F : " << F << '\n';
			++k;
		}
		else break;
	} while (R >= M && R <= (1 - M));
	if (R <= 0.5) {
		F[k] = '0' + static_cast<char>(digit);
	}
	if (R >= 0.5) {
		F[k] = '0' + static_cast<char>(digit + 1);
	}
	std::cout << "F : " << F << '\n';
	return F;
}


// float
//0b0.01111001.01000111101011100001010 : 0.02
//0b0.10000100.10010000001010001111011 : 50.02000 : 5.00200e+01 : 50.02
//0b0.10000111.11110100000000001000010 : 500.0020142 : 5.0000201e+02 : 500.002
//0b0.10001011.00111000100000000000000 : 5000.000000000 : 5.000000000e+03 : 5000
//0b0.10001110.10000110101000000000000 : 50000.00000000000 : 5.00000000000e+04 : 50000
//0b0.10010001.11101000010010000000000 : 500000.0000000000000 : 5.0000000000000e+05 : 500000
//0b0.10010101.00110001001011010000000 : 5000000.000000000000000 : 5.000000000000000e+06 : 5000000
//0b0.10011000.01111101011110000100000 : 50000000.00000000000000000 : 5.00000000000000000e+07 : 50000000
//0b0.10011011.11011100110101100101000 : 500000000.0000000000000000000 : 5.0000000000000000000e+08 : 500000000
//0b0.10011111.00101010000001011111001 : 5000000000.000000000000000000000 : 5.000000000000000000000e+09 : 5000000000
//0b0.10100010.01110100100001110110111 : 49999998976.00000000000000000000000 : 4.99999989760000000000000e+10 : 49999998976
// 
// double
//0b0.01111111001.0100011110101110000101000000000000000000000000000000 : 0.02
//0b0.10000000100.1001000000101000111101011100001010000000000000000000 : 50.02000 : 5.00200e+01 : 50.02
//0b0.10000000111.1111010000000000100000110001001001101110011001100110 : 500.0020000 : 5.0000200e+02 : 500.002
//0b0.10000001011.0011100010000000000000001101000110110111000101110001 : 5000.000200000 : 5.000000200e+03 : 5000.0002
//0b0.10000001110.1000011010100000000000000000001010011111000101101011 : 50000.00002000000 : 5.00000000200e+04 : 50000.00002
//0b0.10000010001.1110100001001000000000000000000000001000011000111000 : 500000.0000020000152 : 5.0000000000200e+05 : 500000.000002
//0b0.10000010101.0011000100101101000000000000000000000000000011010111 : 5000000.000000200234354 : 5.000000000000200e+06 : 5000000.0000002
//0b0.10000011000.0111110101111000010000000000000000000000000000000011 : 50000000.00000002235174179 : 5.00000000000000224e+07 : 50000000.000000022
//0b0.10000011011.1101110011010110010100000000000000000000000000000000 : 500000000.0000000000000000000 : 5.0000000000000000000e+08 : 500000000
//0b0.10000011111.0010101000000101111100100000000000000000000000000000 : 5000000000.000000000000000000000 : 5.000000000000000000000e+09 : 5000000000
//0b0.10000100010.0111010010000111011011101000000000000000000000000000 : 50000000000.00000000000000000000000 : 5.00000000000000000000000e+10 : 50000000000

template<typename Real>
void ShowDifferentFloatFormats() {
	// how does fixed represent different floating point formats?

	auto oldPrecision = std::cout.precision();
	Real a, b, c;
	a = 50; b = 1.0f / 50;
	std::cout << to_binary(b) << " : " << b << '\n';
	size_t nrDigits = 4;
	for (int i = 0; i < 10; ++i) {
		c = a + b;
		std::cout << sw::universal::to_binary(c) << " : " << std::setprecision(nrDigits + 1) << std::fixed << c << " : " << std::scientific << c << " : " << std::defaultfloat << c << '\n';
		a *= 10;
		b /= 10;
		nrDigits += 2;
	}
	std::cout << std::setprecision(oldPrecision) << std::endl;
}

template<typename Real>
void ShowFloatingPointFormats(Real v, size_t precision = 0) {
	auto oldPrecision = std::cout.precision();
	if (precision > 0) std::cout << std::setprecision(precision);
	std::cout << "scientific    : " << std::scientific << v << '\n';  // flags: 0b0001'0010'0000'0001
	std::cout << "fixed         : " << std::fixed << v << '\n';       // flags: 0b0010'0010'0000'0001
	std::cout << "hexfloat      : " << std::hexfloat << v << '\n';    // flags: 0b0011'0010'0000'0001
	std::cout << "defaultfloat  : " << std::defaultfloat << v << '\n';// flags: 0b0000'0010'0000'0001
	std::cout << "binary        : " << to_binary(v) << '\n';
	if (precision > 0) std::cout << std::setprecision(oldPrecision);
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

	ShowDifferentFloatFormats<float>();
	ShowDifferentFloatFormats<double>();

	// Dragon algorithm inspired test cases
	ShowFloatingPointFormats(1.3f);        // 1.3, not 1.2999999
	ShowFloatingPointFormats(4.0f / 3.0f); // 1.33333
	ShowFloatingPointFormats(4.0f / 3.0f, 8); // 1.33333
	ShowFloatingPointFormats(4.0f / 3.0f, 15); // 1.33333

	std::string s;
//	s = fp3<4>(0.5f);
//	s = fp3<4>(0.25f);
	s = fp3<4>(0.125f);
//	s = fp3<4>(0.0625f);

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
