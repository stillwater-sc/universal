// logic.cpp: verify comparison operators of hfloat hexadecimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "hfloat<> comparison operator validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using HfloatShort = hfloat<6, 7, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Equality
	std::cout << "+---------    Equality tests\n";
	{
		HfloatShort a(42), b(42), c(43);
		if (!(a == b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 == 42\n"; }
		if (a == c)    { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 != 43\n"; }
	}

	// Test 2: Inequality
	std::cout << "+---------    Inequality tests\n";
	{
		HfloatShort a(42), b(43);
		if (!(a != b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 != 43\n"; }
	}

	// Test 3: Less than
	std::cout << "+---------    Less than tests\n";
	{
		HfloatShort a(10), b(20), c(-5);
		if (!(a < b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 10 < 20\n"; }
		if (b < a)    { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 20 not < 10\n"; }
		if (!(c < a)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: -5 < 10\n"; }
	}

	// Test 4: Greater than
	std::cout << "+---------    Greater than tests\n";
	{
		HfloatShort a(100), b(50);
		if (!(a > b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 100 > 50\n"; }
		if (b > a)    { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 50 not > 100\n"; }
	}

	// Test 5: Less than or equal
	std::cout << "+---------    Less than or equal tests\n";
	{
		HfloatShort a(42), b(42), c(43);
		if (!(a <= b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 <= 42\n"; }
		if (!(a <= c)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 <= 43\n"; }
		if (c <= a)    { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 43 not <= 42\n"; }
	}

	// Test 6: Greater than or equal
	std::cout << "+---------    Greater than or equal tests\n";
	{
		HfloatShort a(42), b(42), c(41);
		if (!(a >= b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 >= 42\n"; }
		if (!(a >= c)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 42 >= 41\n"; }
		if (c >= a)    { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 41 not >= 42\n"; }
	}

	// Test 7: Zero comparisons
	std::cout << "+---------    Zero comparison tests\n";
	{
		HfloatShort zero(0), pos(1), neg(-1);
		if (!(zero < pos)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: 0 < 1\n"; }
		if (!(neg < zero)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: -1 < 0\n"; }
	}

	// Test 8: Negative number ordering
	std::cout << "+---------    Negative number ordering\n";
	{
		HfloatShort a(-10), b(-5);
		if (!(a < b)) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: -10 < -5\n"; }
		if (b < a)    { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: -5 not < -10\n"; }
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
