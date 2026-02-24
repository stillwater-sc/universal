// logic.cpp: comparison operator tests for blockdigit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/internal/blockdigit/blockdigit.hpp>
#include <universal/verification/test_suite.hpp>

template<unsigned ndigits, unsigned radix>
int VerifyComparison(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	using BD = blockdigit<ndigits, radix>;

	// equality
	{
		BD a(42), b(42);
		if (!(a == b)) {
			if (reportTestCases) std::cerr << "FAIL: 42 == 42\n";
			++nrOfFailedTestCases;
		}
		if (a != b) {
			if (reportTestCases) std::cerr << "FAIL: 42 != 42\n";
			++nrOfFailedTestCases;
		}
	}
	// inequality
	{
		BD a(42), b(43);
		if (a == b) {
			if (reportTestCases) std::cerr << "FAIL: 42 != 43\n";
			++nrOfFailedTestCases;
		}
		if (!(a != b)) {
			if (reportTestCases) std::cerr << "FAIL: !(42 != 43)\n";
			++nrOfFailedTestCases;
		}
	}
	// less-than
	{
		BD a(5), b(10);
		if (!(a < b)) {
			if (reportTestCases) std::cerr << "FAIL: 5 < 10\n";
			++nrOfFailedTestCases;
		}
		if (b < a) {
			if (reportTestCases) std::cerr << "FAIL: 10 < 5\n";
			++nrOfFailedTestCases;
		}
	}
	// greater-than
	{
		BD a(10), b(5);
		if (!(a > b)) {
			if (reportTestCases) std::cerr << "FAIL: 10 > 5\n";
			++nrOfFailedTestCases;
		}
	}
	// less-or-equal
	{
		BD a(5), b(5), c(10);
		if (!(a <= b)) {
			if (reportTestCases) std::cerr << "FAIL: 5 <= 5\n";
			++nrOfFailedTestCases;
		}
		if (!(a <= c)) {
			if (reportTestCases) std::cerr << "FAIL: 5 <= 10\n";
			++nrOfFailedTestCases;
		}
	}
	// greater-or-equal
	{
		BD a(10), b(10), c(5);
		if (!(a >= b)) {
			if (reportTestCases) std::cerr << "FAIL: 10 >= 10\n";
			++nrOfFailedTestCases;
		}
		if (!(a >= c)) {
			if (reportTestCases) std::cerr << "FAIL: 10 >= 5\n";
			++nrOfFailedTestCases;
		}
	}
	// negative comparisons
	{
		BD a(-5), b(3);
		if (!(a < b)) {
			if (reportTestCases) std::cerr << "FAIL: -5 < 3\n";
			++nrOfFailedTestCases;
		}
		if (a > b) {
			if (reportTestCases) std::cerr << "FAIL: -5 > 3 should be false\n";
			++nrOfFailedTestCases;
		}
	}
	// both negative
	{
		BD a(-10), b(-5);
		if (!(a < b)) {
			if (reportTestCases) std::cerr << "FAIL: -10 < -5\n";
			++nrOfFailedTestCases;
		}
	}
	// zero signs
	{
		BD a(0), b(0);
		a.setneg(); // -0
		if (!(a == b)) {
			if (reportTestCases) std::cerr << "FAIL: -0 == +0\n";
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdigit logic";
	std::string test_tag    = "blockdigit/logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	blockdigit<8, 10> a(123), b(456);
	bool c;
	c = a < b;
	std::cout << a << " < " << b << " = " << c << '\n';
	c = a > b;
	std::cout << a << " > " << b << " = " << c << '\n';
	c = a == b;
	std::cout << a << " == " << b << " = " << c << '\n';
	c = a != b;
	std::cout << a << " != " << b << " = " << c << '\n';
	c = a <= b;
	std::cout << a << "<= " << b << " = " << c << '\n';
	c = a >= b;
	std::cout << a << ">= " << b << " = " << c << '\n';

#else

#if REGRESSION_LEVEL_1
	std::cout << "+---------    Comparison: octal\n";
	nrOfFailedTestCases += VerifyComparison<8, 8>(reportTestCases);
	std::cout << "+---------    Comparison: decimal\n";
	nrOfFailedTestCases += VerifyComparison<8, 10>(reportTestCases);
	std::cout << "+---------    Comparison: hexadecimal\n";
	nrOfFailedTestCases += VerifyComparison<8, 16>(reportTestCases);

#endif

#if REGRESSION_LEVEL_2
#endif


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
