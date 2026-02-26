// addition.cpp: test suite runner for complex addition on classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_ADD
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = true;
	constexpr bool hasMaxExpValues = false;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "classic cfloat complex addition validation";
	std::string test_tag           = "cfloat_fff addition";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// shorthand alias types
	using c16  = cfloat< 16,  5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c32  = cfloat< 32,  8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c48  = cfloat< 48,  8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c64  = cfloat< 64, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c80  = cfloat< 80, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c96  = cfloat< 96, 15, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c128 = cfloat<128, 15, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;

	// driving the intensity of the randomized arithmetic tests
	size_t nrRandoms = 0;

#if MANUAL_TESTING


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures

#else

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if	REGRESSION_LEVEL_3

#endif

#if	REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
