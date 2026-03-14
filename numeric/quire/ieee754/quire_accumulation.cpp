//  quire_accumulations.cpp : computational path experiments with quires for native IEEE-754 types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/native/ieee754.hpp>
#include <universal/native/fdp.hpp>  // blocktriple-based quire_mul for float/double
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include "../quire_accumulation_tests.hpp"

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite          = "IEEE-754 quire accumulation";
	std::string test_tag            = "IEEE-754 quire";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#	if MANUAL_TESTING

	// float tests
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<float>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<float>();
	nrOfFailedTestCases += TestQuireAccumulationRepeated<float>();
	nrOfFailedTestCases += TestQuireBitWalk<float>();

	// double tests (4226-bit quire)
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<double>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<double>();
	nrOfFailedTestCases += TestQuireAccumulationRepeated<double>();
	nrOfFailedTestCases += TestQuireBitWalk<double>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1

	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<float>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<float>();
	nrOfFailedTestCases += TestQuireAccumulationRepeated<float>();
	nrOfFailedTestCases += TestQuireBitWalk<float>();

#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<double>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<double>();
	nrOfFailedTestCases += TestQuireAccumulationRepeated<double>();
	nrOfFailedTestCases += TestQuireBitWalk<double>();

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
