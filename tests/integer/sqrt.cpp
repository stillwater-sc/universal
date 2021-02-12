//  sqrt.cpp : test runner for square root on abitrary precision integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <cmath>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/math_functions.hpp>
#include <universal/number/integer/numeric_limits.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
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
int VerifyIntegerFloorSqrt(const std::string& tag, bool bReportIndividualTestCases) {
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
			if (bReportIndividualTestCases) ReportUnaryArithmeticError("FAIL", "sqrt", a, result, Integer(ref));
		}
		if (nrOfTestFailures > 24) return nrOfTestFailures;
	}
	return nrOfTestFailures;
}

template<size_t nbits, typename BlockType>
int VerifyIntegerCeilSqrt(const std::string& tag, bool bReportIndividualTestCases) {
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

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = "square root integer tests failed";

#if MANUAL_TESTING

	cout << floor(sqrt(5.0)) << " - " << ceil(sqrt(5.0)) << endl << endl;
	cout << floor_sqrt(integer<8, uint8_t>(5)) << endl;
	cout << endl;
	cout << ceil_sqrt(integer<8, uint8_t>(5)) << endl;

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

	// quick big test
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8,uint8_t>", "ceil_sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10,uint8_t>", "ceil_sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12,uint16_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12,uint16_t>", "ceil_sqrt");

	nrOfFailedTestCases = 0; // nullify in manual testing

#else // !MANUAL_TESTING

	std::cout << "square root integer function verfication" << std::endl;

	cout << "floor(sqrt(x)) tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12,uint16_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<14, uint8_t>(tag, bReportIndividualTestCases), "integer<14,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<16, uint16_t>(tag, bReportIndividualTestCases), "integer<16,uint16_t>", "floor_sqrt");
	// you can use uint64_t as BlockType for types <= 64bits
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<16, uint64_t>(tag, bReportIndividualTestCases), "integer<16,uint64_t>", "floor_sqrt");

	cout << "ceil(sqrt(x)) tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<8, uint8_t>(tag, bReportIndividualTestCases), "integer<8,uint8_t>", "ceil_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<10, uint8_t>(tag, bReportIndividualTestCases), "integer<10,uint8_t>", "ceil_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<12, uint16_t>(tag, bReportIndividualTestCases), "integer<12,uint16_t>", "ceil_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<14, uint8_t>(tag, bReportIndividualTestCases), "integer<14,uint8_t>", "ceil_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<16, uint16_t>(tag, bReportIndividualTestCases), "integer<16,uint16_t>", "ceil_sqrt");
	// you can use uint64_t as BlockType for types <= 64bits
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCeilSqrt<16, uint64_t>(tag, bReportIndividualTestCases), "integer<16,uint64_t>", "ceil_sqrt");


#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<20, uint8_t>(tag, bReportIndividualTestCases), "integer<20,uint8_t>", "floor_sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerFloorSqrt<20, uint16_t>(tag, bReportIndividualTestCases), "integer<20,uint16_t>", "floor_sqrt");

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
