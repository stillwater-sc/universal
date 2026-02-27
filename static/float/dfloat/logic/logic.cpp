// logic.cpp: verify comparison operators of dfloat decimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "dfloat<> comparison operator validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Equality
	std::cout << "+---------    Equality tests\n";
	{
		Decimal32 a(42), b(42), c(43);
		if (!(a == b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 == 42\n";
		}
		if (a == c) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 != 43\n";
		}
	}

	// Test 2: Inequality
	std::cout << "+---------    Inequality tests\n";
	{
		Decimal32 a(42), b(43);
		if (!(a != b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 != 43\n";
		}
		Decimal32 c(42);
		if (a != c) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 == 42 via !=\n";
		}
	}

	// Test 3: Less than
	std::cout << "+---------    Less than tests\n";
	{
		Decimal32 a(10), b(20), c(-5);
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 < 20\n";
		}
		if (b < a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 20 not < 10\n";
		}
		if (!(c < a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -5 < 10\n";
		}
	}

	// Test 4: Greater than
	std::cout << "+---------    Greater than tests\n";
	{
		Decimal32 a(100), b(50);
		if (!(a > b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 100 > 50\n";
		}
		if (b > a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 50 not > 100\n";
		}
	}

	// Test 5: Less than or equal
	std::cout << "+---------    Less than or equal tests\n";
	{
		Decimal32 a(42), b(42), c(43);
		if (!(a <= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 <= 42\n";
		}
		if (!(a <= c)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 <= 43\n";
		}
		if (c <= a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 43 not <= 42\n";
		}
	}

	// Test 6: Greater than or equal
	std::cout << "+---------    Greater than or equal tests\n";
	{
		Decimal32 a(42), b(42), c(41);
		if (!(a >= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 >= 42\n";
		}
		if (!(a >= c)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 >= 41\n";
		}
		if (c >= a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 41 not >= 42\n";
		}
	}

	// Test 7: Zero comparisons
	std::cout << "+---------    Zero comparison tests\n";
	{
		Decimal32 zero(0), pos(1), neg(-1);
		if (!(zero == Decimal32(0))) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: zero == zero\n";
		}
		if (!(zero < pos)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 0 < 1\n";
		}
		if (!(neg < zero)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -1 < 0\n";
		}
	}

	// Test 8: Negative number comparisons
	std::cout << "+---------    Negative number comparison tests\n";
	{
		Decimal32 a(-10), b(-5);
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -10 < -5\n";
		}
		if (b < a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -5 not < -10\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
