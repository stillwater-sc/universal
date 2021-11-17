//  remainder.cpp : test suite runner for remainder operation on abitrary precision integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
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

#include <typeinfo>
template<typename Scalar>
void GenerateDivTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
	z = x / y;
	std::cout << typeid(Scalar).name() << ": " << x << " / " << y << " = " << z << std::endl;
}

// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateDivTest<short>(2, 16, s);
	sw::universal::integer<16> z = 0;
	GenerateDivTest<sw::universal::integer<16> >(2, 16, z);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING

	integer<12> a, b, c;
	a = 10000;
	b = 100;
	GenerateDivTest(a, b, c);

	ReportTestResult(VerifyRemainder<4>("manual test", true), "integer<4>", "remainder");
	ReportTestResult(VerifyRemainder<11>("manual test", true), "integer<11>", "remainder");

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 4, uint8_t >(bReportIndividualTestCases), "integer< 4, uint8_t >", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 6, uint8_t >(bReportIndividualTestCases), "integer< 6, uint8_t >", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 8, uint8_t >(bReportIndividualTestCases), "integer< 8, uint8_t >", "remainder");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<10, uint8_t >(bReportIndividualTestCases), "integer<10, uint8_t >", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<10, uint16_t>(bReportIndividualTestCases), "integer<10, uint16_t>", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint8_t >(bReportIndividualTestCases), "integer<12, uint8_t >", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint16_t>(bReportIndividualTestCases), "integer<12, uint16_t>", "remainder");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<14, uint8_t >(bReportIndividualTestCases), "integer<14, uint8_t >", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<14, uint8_t >(bReportIndividualTestCases), "integer<14, uint16_t>", "remainder");
#endif

#if REGRESSION_LEVEL_4
	// VerifyShortRemainder compares an integer<16> to native short type to make certain it has all the same behavior
//	nrOfFailedTestCases += ReportTestResult(VerifyShortRemainder<uint8_t>(bReportIndividualTestCases), "integer<16, uint8_t>", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyShortRemainder<uint16_t>(bReportIndividualTestCases), "integer<16, uint16_t>", "remainder");
	// this is a 'standard' comparision against a native int64_t
//	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<16, uint8_t>(bReportIndividualTestCases), "integer<16, uint8_t>", "remainder");
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
