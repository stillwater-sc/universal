// conversion.cpp : test suite runner for conversion operators to posit2 numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit2/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>

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

	std::string test_suite  = "posit2 conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<3, 0>, posit<4, 0>, float>(reportTestCases), "posit<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<4, 1>, posit<5, 1>, float>(reportTestCases), "posit<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<5, 2>, posit<6, 2>, float>(reportTestCases), "posit<5,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<3, 0>>(reportTestCases), "posit<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<4, 0>>(reportTestCases), "posit<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<5, 0>>(reportTestCases), "posit<5,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<6, 0>>(reportTestCases), "posit<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<7, 0>>(reportTestCases), "posit<7,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<8, 0>>(reportTestCases), "posit<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<9, 0>>(reportTestCases), "posit<9,0>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 3, 0>, posit< 4, 0>, float>(reportTestCases), "posit<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 4, 0>, posit< 5, 0>, float>(reportTestCases), "posit<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 5, 0>, posit< 6, 0>, float>(reportTestCases), "posit<5,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 6, 0>, posit< 7, 0>, float>(reportTestCases), "posit<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 7, 0>, posit< 8, 0>, float>(reportTestCases), "posit<7,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 8, 0>, posit< 9, 0>, float>(reportTestCases), "posit<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 9, 0>, posit<10, 0>, float>(reportTestCases), "posit<9,0>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 4, 1>, posit< 5, 1>, float>(reportTestCases), "posit<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 5, 1>, posit< 6, 1>, float>(reportTestCases), "posit<5,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 6, 1>, posit< 7, 1>, float>(reportTestCases), "posit<6,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 7, 1>, posit< 8, 1>, float>(reportTestCases), "posit<7,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 8, 1>, posit< 9, 1>, float>(reportTestCases), "posit<8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 9, 1>, posit<10, 1>, float>(reportTestCases), "posit<9,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 5, 2>, posit< 6, 2>, float>(reportTestCases), "posit<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 6, 2>, posit< 7, 2>, float>(reportTestCases), "posit<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 7, 2>, posit< 8, 2>, float>(reportTestCases), "posit<7,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 8, 2>, posit< 9, 2>, float>(reportTestCases), "posit<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 9, 2>, posit<10, 2>, float>(reportTestCases), "posit<9,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 6, 3>, posit< 7, 3>, double>(reportTestCases), "posit<6,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 7, 3>, posit< 8, 3>, double>(reportTestCases), "posit<7,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 8, 3>, posit< 9, 3>, double>(reportTestCases), "posit<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 9, 3>, posit<10, 3>, double>(reportTestCases), "posit<9,3>", test_tag);
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<10, 0>, posit<11, 0>, double>(reportTestCases), "posit<10,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<10, 1>, posit<11, 1>, double>(reportTestCases), "posit<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<10, 2>, posit<11, 2>, double>(reportTestCases), "posit<10,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<10, 3>, posit<11, 3>, double>(reportTestCases), "posit<10,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<12, 0>, posit<13, 0>, double>(reportTestCases), "posit<12,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<12, 1>, posit<13, 1>, double>(reportTestCases), "posit<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<12, 2>, posit<13, 2>, double>(reportTestCases), "posit<12,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<12, 3>, posit<13, 3>, double>(reportTestCases), "posit<12,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<14, 0>, posit<15, 0>, double>(reportTestCases), "posit<14,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<14, 1>, posit<15, 1>, double>(reportTestCases), "posit<14,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<14, 2>, posit<15, 2>, double>(reportTestCases), "posit<14,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<14, 3>, posit<15, 3>, double>(reportTestCases), "posit<14,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<16, 0>, posit<17, 0>, double>(reportTestCases), "posit<16,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<16, 1>, posit<17, 1>, double>(reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<16, 2>, posit<17, 2>, double>(reportTestCases), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<16, 3>, posit<17, 3>, double>(reportTestCases), "posit<16,3>", test_tag);
#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
