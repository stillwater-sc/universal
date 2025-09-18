// rounding.cpp: functional tests for blocksignificant rounding
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificant/blocksignificant.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/blocksignificant_test_suite.hpp>

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
	
	std::string test_suite  = "blocksignificant rounding validation";
	std::string test_tag    = "rounding";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

//	constexpr BitEncoding twos = BitEncoding::Twos;

#if MANUAL_TESTING
	// Map out the full rounding truth table
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
	{
		blocksignificant<10, uint32_t> a;
		// test rounding of 0b00'0lgr'ssss
		//                        |          position of the lsb
		// lsb is 6
		/*
		*         lgr'ssss
			0b00'0000'0000 round down
			0b00'0000'0001 round down
			0b00'0001'0000 round down
			0b00'0001'0001 round down
			0b00'0010'0000 round down   <-- rounding to even on tie
			0b00'0010'0001 round up
			0b00'0011'0000 round up
			0b00'0011'0001 round up
			0b00'0100'0000 round down
			0b00'0100'0001 round down
			0b00'0101'0000 round down
			0b00'0101'0001 round down
			0b00'0110'0000 round up     <-- rounding to even on tie
			0b00'0110'0001 round up
			0b00'0111'0000 round up
			0b00'0111'0001 round up
		*/
		for (size_t i = 0; i < 8; ++i) {
			size_t bits = (i << 4);
			a.setbits(bits);
			std::cout << to_binary(a, true) << " round " << (a.roundingDirection(6) ? "up" : "down") << '\n';
			bits |= 0x1;
			a.setbits(bits);
			std::cout << to_binary(a, true) << " round " << (a.roundingDirection(6) ? "up" : "down") << '\n';
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
