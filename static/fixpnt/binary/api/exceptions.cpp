//  exceptions.cpp : test suite for arithmetic exceptions of fixpnt numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the number system
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixpnt arithmetic exceptions ";
	std::string test_tag    = "arithmetic exception";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Number = sw::universal::fixpnt<8, 4, Modulo, uint16_t>;

	nrOfFailedTestCases += TestDivisionByZero<Number>(reportTestCases);
	nrOfFailedTestCases += TestNegativeSqrtArgument<Number>(reportTestCases);

#ifdef IMPLEMENTED
	// special value-add cases
	constexpr Number maxpos(SpecificValue::maxpos);
	constexpr Number minpos(SpecificValue::minpos);
	constexpr Number maxneg(SpecificValue::maxneg);
	nrOfFailedTestCases += TestOverflowOnAddition(reportTestCases, maxpos, maxpos);
	nrOfFailedTestCases += TestOverflowOnSubtraction(reportTestCases, maxneg, maxpos);
	nrOfFailedTestCases += TestOverflowOnMultiplication(reportTestCases, maxneg, maxpos);
	nrOfFailedTestCases += TestOverflowOnDivision(reportTestCases, maxneg, minpos);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	using Number = sw::universal::fixpnt<8, 4, Modulo, uint16_t>;

	nrOfFailedTestCases += ReportTestResult(TestDivisionByZero<Number>(reportTestCases), test_tag, "division by zero");

	nrOfFailedTestCases += ReportTestResult(TestNegativeSqrtArgument<Number>(reportTestCases), test_tag, "negative sqrt argument");
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
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
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
