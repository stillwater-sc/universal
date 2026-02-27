// multiplication.cpp: verify multiplication of hfloat hexadecimal floating-point
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

	std::string test_suite = "hfloat<> multiplication validation";
	std::string test_tag = "hfloat<> multiplication";
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

	// Test 1: Basic multiplication
	std::cout << "+---------    Basic multiplication\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 0, 5, 0 },
			{ 5, 0, 0 },
			{ 1, 42, 42 },
			{ 42, 1, 42 },
			{ 2, 3, 6 },
			{ 10, 10, 100 },
			{ -1, 5, -5 },
			{ -2, -3, 6 },
			{ 0.5, 2, 1 },
			{ 0.25, 4, 1 },
		};
		for (const auto& tc : cases) {
			HfloatShort a(tc.a), b(tc.b);
			HfloatShort result = a * b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " * " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 2: Commutativity
	std::cout << "+---------    Commutativity: a * b == b * a\n";
	{
		double values[] = { 1, 7, -3, 0.5, 16 };
		for (double va : values) {
			for (double vb : values) {
				HfloatShort a(va), b(vb);
				HfloatShort ab = a * b;
				HfloatShort ba = b * a;
				if (double(ab) != double(ba)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << va << " * " << vb
					                               << " = " << double(ab) << " but "
					                               << vb << " * " << va << " = " << double(ba) << '\n';
				}
			}
		}
	}

	// Test 3: Multiplicative identity
	std::cout << "+---------    Multiplicative identity: a * 1 == a\n";
	{
		double values[] = { 0, 1, -1, 42, -42, 0.5 };
		for (double v : values) {
			HfloatShort a(v), one(1);
			HfloatShort result = a * one;
			if (double(result) != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << v << " * 1 = " << double(result) << '\n';
			}
		}
	}

	// Test 4: Powers of 16 multiplication
	std::cout << "+---------    Powers of 16 multiplication\n";
	{
		HfloatShort a(16.0), b(16.0);
		HfloatShort result = a * b;
		double d = double(result);
		if (d != 256.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 16 * 16 = " << d << " (expected 256)\n";
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
