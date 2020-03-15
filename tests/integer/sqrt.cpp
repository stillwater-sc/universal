//  sqrt.cpp : square root tests for abitrary precision integers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <cmath>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/math_functions.hpp>
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

template<size_t nbits, typename BlockType>
int VerifyIntegerSqrt(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (1 << (nbits-1));
	using Integer = sw::unum::integer<nbits, BlockType>;

	int nrOfTestFailures = 0;
	Integer a, result;
	size_t ref;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a = i;
		result = sqrt(a);
		ref = size_t(std::sqrt(double(i)));
//		std::cout << "sqrt of " << a << " " << result << " vs " << ref << " vs " << Integer(ref) << std::endl;
		if (result != ref) {
			++nrOfTestFailures;
			if (bReportIndividualTestCases) ReportUnaryArithmeticError("FAIL", "sqrt", a, Integer(ref), result);
		}
		if (nrOfTestFailures > 24) return nrOfTestFailures;
	}
	return nrOfTestFailures;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = "square root integer tests failed";

#if MANUAL_TESTING

	constexpr size_t nbits = 1024;
	using BlockType = uint32_t;
	using Integer = integer<nbits, BlockType>;
	Integer a;

	a = 1024 * 1024;
	cout << "sqrt of " << a << " = " << sqrt(a) << endl;
	a *= a;
	cout << "sqrt of " << a << " = " << sqrt(a) << endl;
	a *= a;
	cout << "sqrt of " << a << " = " << sqrt(a) << endl;
	a *= a;
	cout << "sqrt of " << a << " = " << sqrt(a) << endl;
	a *= a;
	cout << "sqrt of " << a << " = " << sqrt(a) << endl;
	a *= a;
	cout << "sqrt of " << a << " = " << sqrt(a) << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8,uint8_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10,uint8_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12,uint16_t>", "sqrt");

	nrOfFailedTestCases = 0; // nullify in manual testing

#else // !MANUAL_TESTING

	std::cout << "square root integer function verfication" << std::endl;

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8,uint8_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10,uint8_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12,uint16_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<14, uint8_t>(tag, bReportIndividualTestCases), "integer<14,uint8_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<16, uint16_t>(tag, bReportIndividualTestCases), "integer<16,uint16_t>", "sqrt");
	// you can use uint64_t as BlockType for types <= 64bits
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<16, uint64_t>(tag, bReportIndividualTestCases), "integer<16,uint64_t>", "sqrt");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<20, uint8_t>(tag, bReportIndividualTestCases), "integer<20,uint8_t>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerSqrt<20, uint16_t>(tag, bReportIndividualTestCases), "integer<20,uint16_t>", "sqrt");

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
