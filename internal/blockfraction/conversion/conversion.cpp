//  conversion.cpp : test suite runner for blockfraction construction and conversion from float/double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_suite.hpp>

/*
A blockfraction is an unsigned binary encoding with a radix point 
that is aligned with the hidden bit of the fraction encoding in a 
floating-point representation. 

The main goal of the blockfraction abstraction is to support arbitrary 
floating-point number systems with a high-quality decimal string conversion.

For arbitrary and adaptive size number systems, blockfraction is not the
right abstraction. High-performance arbitrary precision systems use a
dynamic data structure and a custom memory manager to avoid copies.

*/

/// <summary>
/// decimal conversion algorithm that demonstrates how rounding
/// error causes incorrect binary to decimal conversion
/// </summary>
/// <param name="v"></param>
void DecimalConversionConceptAlgorithm(double v) {
	constexpr unsigned n = 53; // 53 bits in double mantissa
	constexpr unsigned width = 15;
	char digits[n];
	unsigned k{ 0 }; // nr of digits in final conversion

	sw::universal::ReportValue(v, "input value", width);

	// compute exponent of decimal representation
	int decimalExponent = static_cast<int>(floor(log10(v)));

	// normalize input value so first decimal is in the 10^0 spot
	v /= pow(10, decimalExponent); // this division adds a lot of garbage in the tail

	// while there are bits to interpret
	sw::universal::ReportValue(v, "scaled value", width);
	while (v > 0 && k < n) {
		double digit = floor(v);
		digits[k++] = '0' + static_cast<char>(digit);
		v -= digit;
		v *= 10.0; // scale to get the next digit in 10^0 slot
		sw::universal::ReportValue(v, "iteration value", width);
	}
	digits[k] = 0;
	std::cout << digits << '\n';
}

template<typename BlockFraction>
void Dragon1(const BlockFraction& v) {
	using namespace sw::universal;

	constexpr BlockFraction m(0x1, 9), zero(0, 9), one(0x200, 9), half(0x100, 9);
	BlockFraction B(0x1400, 9);
	BlockFraction R, M, RB(0, 9), oneMinusM(0, 9);
	constexpr unsigned n = 9;
	char digits[n];

	// NOTE: the blockfraction needs its radixpoint set to yield the correct interpretation
	// for arithmetic operators. With the default, the fraction has no integer part

	ReportValue(v, "value to convert");
	ReportValue(m, "starting M");
	ReportValue(half, "half");
	ReportValue(B, "base 10");
	ReportValue(RB, "RB");

	unsigned k{ 0 }, U{ 0 };
	R = v;
	M = m;
	oneMinusM.sub(one, M);
	ReportValue(R, "R");
	ReportValue(M, "M");
	ReportValue(oneMinusM, "oneMinusM");
	do {
		++k;
		std::cout << "iteration " << k << '\n';
		RB.scaleByBase(R, B);
		RB.setradix(9);
		ReportValue(RB, "RB");
		U = RB.integer();
		R = RB.fraction();
		M.scaleByBase(M, B);
		oneMinusM.sub(one, M);
		std::cout << std::setw(20) << "U" << " : " << U << '\n';
		ReportValue(R, "R");
		ReportValue(M, "M");
		ReportValue(oneMinusM, "oneMinusM");
		digits[n - k] = static_cast<char>(U);
//		} while (R >= M && R <= oneMinusM);
	} while (R != zero);
	std::cout << "nr of digits is " << k << '\n';
	std::cout << "digits       : 0.";
	for (unsigned i = 0; i < k; ++i) {
		std::cout << static_cast<unsigned>(digits[n - i - 1]);
	}
	std::cout << '\n';
	std::cout << "source value : " << v << '\n';
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
	using namespace sw::universal;

	std::string test_suite  = "blockfraction conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// we have deprecated the blockfraction copy constructor to catch any
	// unsuspecting conversion copies in blockfraction use-cases

	{
		std::cout << "conceptual conversion algorithm that shows sensitivity to rounding error\n";
		DecimalConversionConceptAlgorithm(123.456);
		std::cout << "+---------------------------------\n";
	}

#ifdef SHOW_CONVERSION
	{
		// blockfraction doesn't have conversion operators so that it 
		// isn't going to be used as an arithmetic type. Downside is
		// that we need to populate its bits manually if we want to
		// inject it with native floating-point values.
		float v{ 1.5f };
		bool s{ false };
		uint64_t rawExp{ 0 };
		uint64_t rawFraction{ 0 };
		extractFields(v, s, rawExp, rawFraction);
		std::cout << "floating-point " << to_binary(v) << " : " << v << '\n';
		std::cout << "exponent bits  " << to_binary(rawExp, 8) << '\n';
		std::cout << "fraction bits  " << to_binary(rawFraction, 24) << '\n';
		blockfraction<26, uint32_t> sp;
		sp.setradix(23);
		sp.setbits(rawFraction);
		std::cout << "fraction bits  " << to_binary(sp, true) << " : " << sp << '\n';
	}
#endif

	{
		std::cout << "Dragon1 algorithm\n";
		using BlockFraction = blockfraction<13, uint32_t>;
		// to process 8 fraction bits
		// we need 4 integer bits to represent B = 10
		// we need 9 fraction bits to represent b^-n/2
		//          0b0000.0000'0000'0
		// B = 10 = 0b1010.0000'0000'0 = 0b1'010.0'0000'0000 = 0x1400
		BlockFraction v(0x10, 9); // v(0x190, 9); // v(0x1FE, 9);
		Dragon1(v);
		std::cout << "+---------------------------------\n";
	}



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
#endif  // MANUAL_TESTING
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
