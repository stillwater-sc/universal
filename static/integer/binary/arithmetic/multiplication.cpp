// multiplication.cpp : test suite runner for multiplication operator on fixed-size abitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

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

	std::string test_suite  = "Integer Arithmetic Multiplication verfication";
	std::string test_tag    = "integer<> multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	integer<16, uint8_t> a, b, c;
	a = 1;
	b = -1;
	c = a * b;
	std::cout << to_binary(c, true) << " = " << c << '\n';

	a = 0x0202;
	b = 0x0101;
	std::cout << to_binary(a, true) << " = " << a << '\n';
	std::cout << to_binary(b, true) << " = " << b << '\n';
	c = a * b;
	std::cout << to_binary(c, true) << " = " << c << '\n';
	GenerateMulTest(a, b, c);

	ExamplePattern();

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint8_t >(reportTestCases), "integer<12, uint8_t >", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, uint8_t >(reportTestCases), "integer< 4, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, uint8_t >(reportTestCases), "integer< 8, uint8_t >", "multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, uint8_t >(reportTestCases), "integer< 4, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, uint8_t >(reportTestCases), "integer< 6, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, uint8_t >(reportTestCases), "integer< 8, uint8_t >", "multiplication");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 7, uint8_t >(reportTestCases), "integer< 7, uint8_t >", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 7, uint16_t>(reportTestCases), "integer< 7, uint16_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 9, uint8_t >(reportTestCases), "integer< 9, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 9, uint16_t>(reportTestCases), "integer< 9, uint16_t>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint8_t >(reportTestCases), "integer<12, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint16_t>(reportTestCases), "integer<12, uint16_t>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	// VerifyShortMultiplication compares an integer<16> to native short type to make certain it has all the same behavior
//	nrOfFailedTestCases += ReportTestResult(VerifyShortMultiplication<uint8_t >(reportTestCases), "integer<16, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyShortMultiplication<uint16_t>(reportTestCases), "integer<16, uint16_t>", "multiplication");
	// this is a 'standard' comparision against a native int64_t which will not have overflow conditions
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<16, uint8_t >(reportTestCases), "integer<16, uint8_t >", "multiplication");
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
