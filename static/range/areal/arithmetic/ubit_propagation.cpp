// ubit_propagation.cpp: test suite for ubit propagation in areal arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/areal_test_suite.hpp>

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal ubit propagation verification";
	std::string test_tag    = "ubit propagation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// manual test - enable reportTestCases to see failures
	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<5, 2, uint8_t> >(reportTestCases), "areal<5,2>", "ubit add");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<5, 2, uint8_t> >(reportTestCases), "areal<5,2>", "ubit mul");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Test ubit propagation for addition
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<4, 1, uint8_t> >(reportTestCases), "areal< 4,1>", "ubit add");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<5, 1, uint8_t> >(reportTestCases), "areal< 5,1>", "ubit add");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<5, 2, uint8_t> >(reportTestCases), "areal< 5,2>", "ubit add");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<6, 2, uint8_t> >(reportTestCases), "areal< 6,2>", "ubit add");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<8, 2, uint8_t> >(reportTestCases), "areal< 8,2>", "ubit add");

	// Test ubit propagation for subtraction
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<4, 1, uint8_t> >(reportTestCases), "areal< 4,1>", "ubit sub");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<5, 1, uint8_t> >(reportTestCases), "areal< 5,1>", "ubit sub");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<5, 2, uint8_t> >(reportTestCases), "areal< 5,2>", "ubit sub");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<6, 2, uint8_t> >(reportTestCases), "areal< 6,2>", "ubit sub");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<8, 2, uint8_t> >(reportTestCases), "areal< 8,2>", "ubit sub");

	// Test ubit propagation for multiplication
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<4, 1, uint8_t> >(reportTestCases), "areal< 4,1>", "ubit mul");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<5, 1, uint8_t> >(reportTestCases), "areal< 5,1>", "ubit mul");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<5, 2, uint8_t> >(reportTestCases), "areal< 5,2>", "ubit mul");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<6, 2, uint8_t> >(reportTestCases), "areal< 6,2>", "ubit mul");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<8, 2, uint8_t> >(reportTestCases), "areal< 8,2>", "ubit mul");

	// Test ubit propagation for division
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<4, 1, uint8_t> >(reportTestCases), "areal< 4,1>", "ubit div");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<5, 1, uint8_t> >(reportTestCases), "areal< 5,1>", "ubit div");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<5, 2, uint8_t> >(reportTestCases), "areal< 5,2>", "ubit div");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<6, 2, uint8_t> >(reportTestCases), "areal< 6,2>", "ubit div");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<8, 2, uint8_t> >(reportTestCases), "areal< 8,2>", "ubit div");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<9, 2, uint8_t> >(reportTestCases), "areal< 9,2>", "ubit add");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationAdd< areal<10, 3, uint8_t> >(reportTestCases), "areal<10,3>", "ubit add");

	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<9, 2, uint8_t> >(reportTestCases), "areal< 9,2>", "ubit sub");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationSub< areal<10, 3, uint8_t> >(reportTestCases), "areal<10,3>", "ubit sub");

	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<9, 2, uint8_t> >(reportTestCases), "areal< 9,2>", "ubit mul");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationMul< areal<10, 3, uint8_t> >(reportTestCases), "areal<10,3>", "ubit mul");

	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<9, 2, uint8_t> >(reportTestCases), "areal< 9,2>", "ubit div");
	nrOfFailedTestCases += ReportTestResult(VerifyUbitPropagationDiv< areal<10, 3, uint8_t> >(reportTestCases), "areal<10,3>", "ubit div");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
