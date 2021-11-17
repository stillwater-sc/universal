//  subtraction.cpp : test suite runner for subracting abitrary precision integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <typeinfo>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/numeric_limits.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

template<typename Scalar>
void GenerateSubTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
	z = x - y;
	std::cout << typeid(Scalar).name() << ": " << x << " - " << y << " = " << z << std::endl;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string tag = "integer subtraction";

#if MANUAL_TESTING

	integer<12> a, b, c;
	a = 1234;
	b = 1235;
	GenerateSubTest(a, b, c);

	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer subtraction verfication\n";

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, uint8_t >(bReportIndividualTestCases), "integer< 5, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, uint8_t >(bReportIndividualTestCases), "integer< 7, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 9, uint8_t >(bReportIndividualTestCases), "integer< 9, uint8_t >", "subtraction");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, uint8_t >(bReportIndividualTestCases), "integer< 7, uint8_t >", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, uint16_t>(bReportIndividualTestCases), "integer< 7, uint16_t>", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<10, uint8_t >(bReportIndividualTestCases), "integer<10, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<10, uint16_t>(bReportIndividualTestCases), "integer<10, uint16_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<11, uint8_t >(bReportIndividualTestCases), "integer<11, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<13, uint16_t>(bReportIndividualTestCases), "integer<13, uint16_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_4
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
//	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction<uint8_t >(bReportIndividualTestCases), "integer<16, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction<uint16_t>(bReportIndividualTestCases), "integer<16, uint16_t>", "subtraction");
	// this is a 'standard' comparision against a native int64_t
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<16, uint8_t>(bReportIndividualTestCases), "integer<16, uint8_t>", "subtraction");
#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
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
