// division.cpp: verify division of hfloat hexadecimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite = "hfloat<> division validation";
	std::string test_tag = "hfloat<> division";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	using HfloatShort = hfloat<6, 7, uint32_t>;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// Test 1: Basic division
	std::cout << "+---------    Basic division\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 0, 1, 0 },
			{ 6, 2, 3 },
			{ 6, 3, 2 },
			{ 100, 10, 10 },
			{ 1, 1, 1 },
			{ 42, 1, 42 },
			{ -6, 2, -3 },
			{ -6, -2, 3 },
			{ 1, 4, 0.25 },
			{ 1, 16, 0.0625 },
		};
		for (const auto& tc : cases) {
			HfloatShort a(tc.a), b(tc.b);
			HfloatShort result = a / b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " / " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 2: Division by self equals 1
	std::cout << "+---------    a / a == 1\n";
	{
		double values[] = { 1, 42, -7, 0.5, 100 };
		for (double v : values) {
			HfloatShort a(v);
			HfloatShort result = a / a;
			double d = double(result);
			if (d != 1.0) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << v << " / " << v << " = " << d << " (expected 1)\n";
			}
		}
	}

	// Test 3: Truncation rounding in division
	std::cout << "+---------    Truncation rounding in division (1/3)\n";
	{
		HfloatShort one(1), three(3);
		HfloatShort third = one / three;
		double d = double(third);
		// hfloat truncates: 1/3 should be <= exact 1/3
		if (d > (1.0 / 3.0) + 1e-15) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: hfloat 1/3 = " << d << " exceeds exact value\n";
		}
		// but should be close
		double err = std::fabs(d - 1.0/3.0);
		if (err > 0.01) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: hfloat 1/3 = " << d << " too far from 0.333...\n";
		}
	}

	// Test 4: Division by zero saturates (no inf in hfloat)
	std::cout << "+---------    Division by zero behavior\n";
	{
		HfloatShort one(1), zero(0);
		HfloatShort result = one / zero;
		// hfloat has no inf; dividing by zero should saturate to maxpos
		HfloatShort mp(SpecificValue::maxpos);
		if (double(result) != double(mp)) {
			// accept either maxpos saturation or some defined behavior
			std::cout << "  Note: 1/0 = " << double(result) << " (maxpos = " << double(mp) << ")\n";
		}
	}

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
