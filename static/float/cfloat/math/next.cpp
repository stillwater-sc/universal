// next.cpp: test suite runner for ULP functions nextafter, nextforward
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_suite_mathlib.hpp>

namespace sw { namespace universal {


} } // namespace sw::universal

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

	std::string test_suite  = "cfloat<> nextafter/toward validation";
	std::string test_tag    = "nextafter/toward";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 16, 5, std::uint16_t, true, true, false> >(reportTestCases), "cfloat< 16, 5>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat<  8, 2, std::uint8_t , true, true, false> >(reportTestCases), "cfloat<  8, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 16, 5, std::uint16_t, true, true, false> >(reportTestCases), "cfloat< 16, 5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 32, 8, std::uint32_t, true, true, false> >(reportTestCases), "cfloat< 32, 8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 64,11, std::uint64_t, true, true, false> >(reportTestCases), "cfloat< 64,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 80,11, std::uint32_t, true, true, false> >(reportTestCases), "cfloat< 80,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat<128,15, std::uint32_t, true, true, false> >(reportTestCases), "cfloat<128,15>", test_tag);
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
