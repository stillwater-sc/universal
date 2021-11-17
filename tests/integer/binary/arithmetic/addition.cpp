//  addition.cpp : test runner for addition of abitrary precision integers
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
void GenerateAddTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
	z = x + y;
	std::cout << typeid(Scalar).name() << ": " << x << " + " << y << " = " << z << std::endl;
}


// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateAddTest<short>(2, 16, s);
	sw::universal::integer<16> z = 0;
	GenerateAddTest<sw::universal::integer<16> >(2, 16, z);
}

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::universal::reportRepresentability(i, j);
		}
	}
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

std::string convert_to_string(const std::vector<char>& v) {
	std::stringstream ss;
	for (std::vector<char>::const_reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

int main()
try {
	using namespace sw::universal;

	std::cout << "Integer Arithmetic Addition verfication\n";
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
//	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING

	using Integer = integer<16, uint16_t>;
	constexpr Integer a(SpecificValue::maxpos), b(SpecificValue::maxneg);
	Integer c;
	c = a + b;
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 4, uint8_t>(bReportIndividualTestCases), "integer< 4, uint8_t >", "addition");
	std::cout << "done" << std::endl;

#else


#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 4, uint8_t>(bReportIndividualTestCases), "integer< 4, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 6, uint8_t>(bReportIndividualTestCases), "integer< 6, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 8, uint8_t>(bReportIndividualTestCases), "integer< 8, uint8_t >", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 9, uint8_t >(bReportIndividualTestCases), "integer< 9, uint8_t >", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 9, uint16_t>(bReportIndividualTestCases), "integer< 9, uint16_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint8_t >(bReportIndividualTestCases), "integer<11, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint16_t>(bReportIndividualTestCases), "integer<11, uint16_t>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint8_t >(bReportIndividualTestCases), "integer<12, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint16_t>(bReportIndividualTestCases), "integer<12, uint16_t>", "addition");
#endif

#if	REGRESSION_LEVEL_4
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
//	nrOfFailedTestCases += ReportTestResult(VerifyShortAddition<uint8_t >(bReportIndividualTestCases), "integer<16, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyShortAddition<uint16_t>(bReportIndividualTestCases), "integer<16, uint8_t >", "addition");
	// this is a 'standard' comparision against a native int64_t
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<16, uint16_t>(bReportIndividualTestCases), "integer<16, uint16_t>", "remainder");
#endif

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
