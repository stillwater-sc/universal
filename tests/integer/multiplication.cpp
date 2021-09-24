//  multiplication.cpp : test suite runner for multiplication of abitrary precision integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
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

#include <typeinfo>
template<typename Scalar>
void GenerateMulTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
	z = x * y;
	std::cout << typeid(Scalar).name() << ": " << x << " * " << y << " = " << z << std::endl;
}

// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateMulTest<short>(2, 16, s);
	sw::universal::integer<16> z = 0;
	GenerateMulTest<sw::universal::integer<16> >(2, 16, z);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

	integer<12, uint8_t> a, b, c;
	a = 0x800; 
	std::endl; cout << to_binary(a) << " = " << a << '\n';
	b = 0x800;
	GenerateMulTest(a, b, c);

	ExamplePattern();

	std::endl; cout << "done" << std::endl; endl;

	ReportTestResult(VerifyMultiplication<4, uint8_t>(true), "integer<4, uint8_t>", "multiplication");


#else
	std::cout << "Integer Multiplication Arithmetic verfication\n";

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, uint8_t >(bReportIndividualTestCases), "integer< 4, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, uint8_t >(bReportIndividualTestCases), "integer< 6, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, uint8_t >(bReportIndividualTestCases), "integer< 8, uint8_t >", "multiplication");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, uint8_t >(bReportIndividualTestCases), "integer<10, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, uint16_t>(bReportIndividualTestCases), "integer<10, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint8_t >(bReportIndividualTestCases), "integer<12, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint16_t>(bReportIndividualTestCases), "integer<12, uint16_t>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<14, uint8_t >(bReportIndividualTestCases), "integer<14, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<14, uint16_t>(bReportIndividualTestCases), "integer<14, uint16_t>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	// VerifyShortMultiplication compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortMultiplication<uint8_t >(bReportIndividualTestCases), "integer<16, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyShortMultiplication<uint16_t>(bReportIndividualTestCases), "integer<16, uint16_t>", "multiplication");
	// this is a 'standard' comparision against a native int64_t which will not have overflow conditions
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<16, uint8_t >(bReportIndividualTestCases), "integer<16, uint8_t >", "multiplication");
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
