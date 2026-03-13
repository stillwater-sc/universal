//  quire_accumulations.cpp : computational path experiments with quires for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/fdp.hpp>  // blocktriple-based quire_mul for lns
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include "../quire_accumulation_tests.hpp"

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "lns<> quire accumulation";
	std::string test_tag            = "lns<> quire";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#	if MANUAL_TESTING

	// Power-of-two sweep: exercises every quire bit position across full dynamic range
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<lns<8, 4>>();
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<lns<8, 2>>();

	// Maxpos/minpos direct cancellation: extreme products at both ends of the quire
	nrOfFailedTestCases += TestQuireMaxposCancellation<lns<8, 4>>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<lns<8, 2>>();

	// Repeated minpos^2 round-trip: exact summation of many identical small products
	nrOfFailedTestCases += TestQuireAccumulationRepeated<lns<8, 4>>();
	nrOfFailedTestCases += TestQuireAccumulationRepeated<lns<8, 2>>();

	// Bit walk: single bit from minpos^2 to capacity, then negate back to zero
	nrOfFailedTestCases += TestQuireBitWalk<lns<8, 4>>();
	nrOfFailedTestCases += TestQuireBitWalk<lns<8, 2>>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1

#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
