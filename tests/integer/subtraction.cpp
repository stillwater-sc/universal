//  subtraction.cpp : arithmetic test suite for subracting abitrary precision integers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <typeinfo>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/integer_test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

template<typename Scalar>
void GenerateSubTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x - y;
	std::cout << typeid(Scalar).name() << ": " << x << " - " << y << " = " << z << std::endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "integer subtraction";

#if MANUAL_TESTING

	integer<12> a, b, c;
	a = 1234;
	b = 1235;
	GenerateSubTest(a, b, c);

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// allocation is the only functionality of integer<N> at this time

	// tests
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint8_t>(tag, bReportIndividualTestCases), "integer<4, uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<6, uint8_t>(tag, bReportIndividualTestCases), "integer<6, uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8, uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10, uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint8_t>(tag, bReportIndividualTestCases), "integer<12, uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12, uint16_t>", "subtraction");

#if STRESS_TESTING

	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction<uint8_t>(tag, bReportIndividualTestCases), "integer<16, uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction<uint16_t>(tag, bReportIndividualTestCases), "integer<16, uint16_t>", "subtraction");
	// this is a 'standard' comparision against a native int64_t
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<16, uint8_t>(tag, bReportIndividualTestCases), "integer<16, uint8_t>", "subtraction");

#endif // STRESS_TESTING
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
