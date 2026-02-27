// division.cpp: verify division of dfloat decimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
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

	std::string test_suite = "dfloat<> division validation";
	std::string test_tag = "dfloat<> division";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;

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
			{ 10, -5, -2 },
			{ 1, 4, 0.25 },
			{ 1, 8, 0.125 },
		};
		for (const auto& tc : cases) {
			Decimal32 a(tc.a), b(tc.b);
			Decimal32 result = a / b;
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
		double values[] = { 1, 42, -7, 0.5, 100, -0.125 };
		for (double v : values) {
			Decimal32 a(v);
			Decimal32 result = a / a;
			double d = double(result);
			if (d != 1.0) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << v << " / " << v << " = " << d << " (expected 1)\n";
			}
		}
	}

	// Test 3: Division by zero
	std::cout << "+---------    Division by zero\n";
	{
		Decimal32 one(1), zero(0);
		Decimal32 r = one / zero;
		if (!r.isinf()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1 / 0 should be inf\n";
		}

		Decimal32 z = zero / zero;
		if (!z.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 0 / 0 should be NaN\n";
		}
	}

	// Test 4: Inf/NaN division
	std::cout << "+---------    Inf and NaN division\n";
	{
		Decimal32 inf(SpecificValue::infpos), one(1), nan(SpecificValue::qnan);

		Decimal32 r1 = inf / inf;
		if (!r1.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: inf / inf should be NaN\n"; }

		Decimal32 r2 = inf / one;
		if (!r2.isinf()) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: inf / 1 should be inf\n"; }

		Decimal32 r3 = nan / one;
		if (!r3.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: NaN / 1 should be NaN\n"; }
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
