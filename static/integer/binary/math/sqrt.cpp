//  sqrt.cpp : test runner for square root on fixed-sized, arbitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <cmath>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/numeric_limits.hpp>
// is representable
#include <universal/math/functions/isrepresentable.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

// straight Babylonian algorithm on floating point type
inline double babylonian(double v) {
	std::cout << v << " input value\n";
	const double eps = 1.0e-7;
	int iter = 1;
	double x_n = 0.5 * v; // initial guess
	std::cout << x_n << " initial guess\n";
	do {
		x_n = (x_n + v / x_n) / 2.0;
		std::cout << x_n << " iteration " << ++iter << std::endl;
	} while (std::abs(x_n * x_n - v) > eps);

	return x_n;
}

template<size_t nbits, typename BlockType>
int VerifyIntegerFloorSqrt(bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (1 << (nbits-1));
	using Integer = sw::universal::integer<nbits, BlockType>;

	int nrOfTestFailures = 0;
	Integer a, result;
	size_t ref; // we use an unsigned type as sqrt can't be negative
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a = i;
		result = floor_sqrt(a);
		ref = size_t(std::floor(std::sqrt(double(i))));
//		std::cout << "sqrt of " << a << " " << result << " vs " << ref << " vs " << Integer(ref) << std::endl;
		if (result != ref) {
			++nrOfTestFailures;
			if (bReportIndividualTestCases) ReportUnaryArithmeticError("FAIL", "floor_sqrt", a, result, Integer(ref));
		}
		if (nrOfTestFailures > 24) return nrOfTestFailures;
	}
	return nrOfTestFailures;
}

template<size_t nbits, typename BlockType>
int VerifyIntegerCeilSqrt(bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (1 << (nbits - 1));
	using Integer = sw::universal::integer<nbits, BlockType>;

	int nrOfTestFailures = 0;
	Integer a, result;
	size_t ref;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a = i;
		result = ceil_sqrt(a);
		ref = size_t(std::ceil(std::sqrt(double(i))));
//		std::cout << "sqrt of " << a << " " << result << " vs " << ref << " vs " << Integer(ref) << std::endl;
		if (result != ref) {
			++nrOfTestFailures;
			if (bReportIndividualTestCases) ReportUnaryArithmeticError("FAIL", "ceil_sqrt", a, result, Integer(ref));
		}
		if (nrOfTestFailures > 24) return nrOfTestFailures;
	}
	return nrOfTestFailures;
}

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

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = "square root integer tests failed";

#if MANUAL_TESTING

	{
		integer<10, uint8_t> a(256);
		std::cout << floor(sqrt(256)) << " - " << ceil(sqrt(256)) << '\n';
		std::cout << floor(sqrt(257)) << " - " << ceil(sqrt(257)) << '\n';
		std::cout << floor_sqrt(a - 1) << '\n';
		std::cout << floor_sqrt(a) << '\n';
		std::cout << ceil_sqrt(a) << '\n';

	}

	{
		// examples of the Babylonian algorithm for approximating sqrt
		babylonian(64.0);
		babylonian(1024.0 * 1024);
		babylonian(1.234567e50*1.234567e50);
	}

	constexpr size_t nbits = 1024;
	using BlockType = uint32_t;
	using Integer = integer<nbits, BlockType>;
	Integer a;

	a = 1024 * 1024;
	std::cout << "sqrt of " << a << " = " << sqrt(a) << '\n';
	a *= a;
	std::cout << "sqrt of " << a << " = " << sqrt(a) << '\n';
	a *= a;
	std::cout << "sqrt of " << a << " = " << sqrt(a) << '\n';
	a *= a;
	std::cout << "sqrt of " << a << " = " << sqrt(a) << '\n';
	a *= a;
	std::cout << "sqrt of " << a << " = " << sqrt(a) << '\n';
	a *= a;
	std::cout << "sqrt of " << a << " = " << sqrt(a) << '\n';

	// quick big test
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<8, uint8_t>(bReportIndividualTestCases), "integer<8,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<8, uint8_t>(bReportIndividualTestCases), "integer<8,uint8_t>", "ceil_sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<10, uint8_t>(bReportIndividualTestCases), "integer<10,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<10, uint8_t>(bReportIndividualTestCases), "integer<10,uint8_t>", "ceil_sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<12, uint16_t>(bReportIndividualTestCases), "integer<12,uint16_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<12, uint16_t>(bReportIndividualTestCases), "integer<12,uint16_t>", "ceil_sqrt");

	nrOfFailedTestCases = 0; // nullify in manual testing

#else // !MANUAL_TESTING

	std::cout << "square root integer function verfication" << std::endl;

	std::cout << "floor(sqrt(x)) tests\n";
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt< 8, uint8_t >(bReportIndividualTestCases), "integer< 8,uint8_t >", "floor_sqrt");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<10, uint8_t >(bReportIndividualTestCases), "integer<10,uint8_t >", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<12, uint16_t>(bReportIndividualTestCases), "integer<12,uint16_t>", "floor_sqrt");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<14, uint8_t >(bReportIndividualTestCases), "integer<14,uint8_t >", "floor_sqrt");

//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<16, uint16_t>(bReportIndividualTestCases), "integer<16,uint16_t>", "floor_sqrt");
	// you can use uint64_t as BlockType for types <= 64bits
//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<16, uint64_t>(bReportIndividualTestCases), "integer<16,uint64_t>", "floor_sqrt");
#endif

#if REGRESSION_LEVEL_4
//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<20, uint8_t >(bReportIndividualTestCases), "integer<20,uint8_t >", "floor_sqrt");
//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<20, uint16_t>(bReportIndividualTestCases), "integer<20,uint16_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<18, uint32_t>(bReportIndividualTestCases), "integer<18,uint32_t>", "floor_sqrt");
#endif

	std::cout << "ceil(sqrt(x)) tests\n";
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt< 8, uint8_t >(bReportIndividualTestCases), "integer< 8,uint8_t >", "ceil_sqrt");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<10, uint8_t >(bReportIndividualTestCases), "integer<10,uint8_t >", "ceil_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<12, uint16_t>(bReportIndividualTestCases), "integer<12,uint16_t>", "ceil_sqrt");
#endif

#if REGRESSION_LEVEL_3
//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<14, uint8_t >(bReportIndividualTestCases), "integer<14,uint8_t >", "ceil_sqrt");

//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<16, uint16_t>(bReportIndividualTestCases), "integer<16,uint16_t>", "ceil_sqrt");
	// you can use uint64_t as BlockType for types <= 64bits
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<16, uint64_t>(bReportIndividualTestCases), "integer<16,uint64_t>", "ceil_sqrt");
#endif

#if REGRESSION_LEVEL_4
//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<20, uint8_t >(bReportIndividualTestCases), "integer<20,uint8_t >", "ceil_sqrt");
//	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<20, uint16_t>(bReportIndividualTestCases), "integer<20,uint16_t>", "ceil_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<18, uint32_t>(bReportIndividualTestCases), "integer<18,uint32_t>", "ceil_sqrt");
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
