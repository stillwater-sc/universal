// division_throw.cpp: tests for elreal_divide_by_zero exception (Phase C)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Companion to arithmetic/division.cpp. The default-mode tests there exercise
// the IEEE-754 fall-through path (div-by-zero -> +/-inf or NaN). This file
// flips ELREAL_THROW_ARITHMETIC_EXCEPTION on and asserts the catchable
// exception path.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase C divide-by-zero exception";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// 1 / 0 must throw
	{
		elreal a(1.0), zero;
		bool threw = false;
		try {
			elreal r = a / zero;
			(void)r;
		}
		catch (const elreal_divide_by_zero&) {
			threw = true;
		}
		if (!threw) {
			std::cerr << "FAIL: 1 / 0 did not throw elreal_divide_by_zero "
				<< "with ELREAL_THROW_ARITHMETIC_EXCEPTION set\n";
			++nrOfFailedTestCases;
		}
	}

	// 0 / 0 must also throw (the leading-double divide would yield NaN,
	// but the exception path takes precedence when the macro is set).
	{
		elreal zero1, zero2;
		bool threw = false;
		try {
			elreal r = zero1 / zero2;
			(void)r;
		}
		catch (const elreal_divide_by_zero&) {
			threw = true;
		}
		if (!threw) {
			std::cerr << "FAIL: 0 / 0 did not throw elreal_divide_by_zero\n";
			++nrOfFailedTestCases;
		}
	}

	// /= also throws
	{
		elreal x(1.0), zero;
		bool threw = false;
		try {
			x /= zero;
		}
		catch (const elreal_divide_by_zero&) {
			threw = true;
		}
		if (!threw) {
			std::cerr << "FAIL: x /= 0 did not throw elreal_divide_by_zero\n";
			++nrOfFailedTestCases;
		}
	}

	// Non-zero division still works in throw mode
	{
		elreal a(6.0), b(2.0);
		bool threw = false;
		double result = 0.0;
		try {
			elreal r = a / b;
			result = double(r);
		}
		catch (...) {
			threw = true;
		}
		if (threw) {
			std::cerr << "FAIL: 6 / 2 threw in throw mode (should only "
				<< "throw on divisor == 0)\n";
			++nrOfFailedTestCases;
		}
		if (result != 3.0) {
			std::cerr << "FAIL: 6 / 2 != 3 in throw mode\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
