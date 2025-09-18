//  conversion.cpp : test suite runner for blocksignificant construction and conversion from float/double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <universal/verification/test_suite.hpp>

/*
A blocksignificant is a 1's or 2's complement binary encoding with a radix point 
that is aligned with the hidden bit of the fraction encoding in a 
floating-point representation. 
  - multiplication uses a 1's complement encoding.
  - addition and subtraction use a 2's complement encoding.
  - division uses a 2's complement encoding.
  - square root uses a 1's complement encoding.


The main goal of the blocksignificant abstraction is to support arbitrary floating-point 
number systems with a high-quality, high-performance arithmetic engine.

The expensive part in these abstractions is the need to receive, expand, and align
bit strings, so special attention must be given to fast implementations.
Implementations that use copies leads to cleaner code, and is ok for small representations. 
However, for larger representations these copies become prohibitive, 
and implementations that do not copy the fraction bits are superior.
The current blocksignificant implementation avoids copies but the block storage
is assumed to be allocated on the stack. This implies that blocksignificant
is useful for representing fixed-size number systems with good performance
for sizes up to several thousands of bits. 

For arbitrary and adaptive size number systems, blocksignificant is not the
right abstraction. High-performance arbitrary precision systems use a
dynamic data structure and a custom memory manager to avoid copies.

*/

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

	std::string test_suite  = "blocksignificant conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// we have deprecated the blocksignificant copy constructor to catch any
	// unsuspecting conversion copies in blocksignificant use-cases
	{
		// scenario that happens in unrounded add/sub
		//  0b0'10.00'0000 : 2
		//  0b0'11.00'0000 : 3
		//	0b0'11.10'0000 : 3.5
		//	0b0'11.11'0000 : 3.75
		//	0b0'11.11'1000 : 3.875
		//	0b0'11.11'1100 : 3.9375
		//	0b0'11.11'1110 : 3.96875
		//	0b0'11.11'1111 : 3.98438
		// for add and sub the significant uses a 2's complement format 00h.ffff
		constexpr size_t fbits   = 8;
		constexpr size_t fhbits  = fbits + 1;
		//constexpr size_t abits   = fhbits + 3;
		//constexpr size_t sumbits = abits + 1;
		size_t msbMask = (1 << (fbits-1));
		size_t frac = msbMask;
		blocksignificant<fhbits, uint8_t> a;
		a.setradix(fhbits - 3);
		for (size_t i = 0; i < fbits; ++i) {
			a.setbits(frac);
			std::cout << to_binary(a, true) << " : " << a << '\n';
			msbMask >>= 1;
			frac |= msbMask;
		}
		// negative values
		//	0b1'00.00'0000 : -0
		//	0b1'10.00'0000 : -2
		//	0b1'11.00'0000 : -1
		//	0b1'11.10'0000 : -0.5
		//	0b1'11.11'0000 : -0.25
		//	0b1'11.11'1000 : -0.125
		//	0b1'11.11'1100 : -0.0625
		//	0b1'11.11'1110 : -0.03125
		msbMask = (1 << fbits);
		frac = msbMask;
		for (size_t i = 0; i < fbits; ++i) {
			a.setbits(frac);
			std::cout << to_binary(a, true) << " : " << double(a) << '\n';
			msbMask >>= 1;
			frac |= msbMask;
		}
	}

	{
		//	0b1111111.1 : 127.5
		//	0b111111.11 : 63.75
		// 	0b11111.111 : 31.875
		//	0b1111.1111 : 15.9375
		//	0b111.11111 : 7.96875
		//	0b11.111111 : 3.98438
		//	0b1.1111111 : 1.99219
		constexpr size_t nbits = 8;
		blocksignificant<nbits, uint8_t> a(0xff, 1);
		for (int radix = 1; radix < static_cast<int>(nbits); ++radix) {
			a.setradix(radix);
			std::cout << to_binary(a) << " : " << a << '\n';
		}
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
