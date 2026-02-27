// subtraction.cpp: verify subtraction of hfloat hexadecimal floating-point
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

	std::string test_suite = "hfloat<> subtraction validation";
	std::string test_tag = "hfloat<> subtraction";
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

	// Test 1: Basic subtraction
	std::cout << "+---------    Basic subtraction\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 0, 0, 0 },
			{ 1, 0, 1 },
			{ 0, 1, -1 },
			{ 5, 3, 2 },
			{ 3, 5, -2 },
			{ 100, 1, 99 },
			{ -5, -3, -2 },
			{ -3, -5, 2 },
			{ 42, 42, 0 },
		};
		for (const auto& tc : cases) {
			HfloatShort a(tc.a), b(tc.b);
			HfloatShort result = a - b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " - " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 2: Anti-commutativity: a - b == -(b - a)
	std::cout << "+---------    Anti-commutativity: a - b == -(b - a)\n";
	{
		double values[] = { 1, 42, -7, 0.5, 100 };
		for (double va : values) {
			for (double vb : values) {
				HfloatShort a(va), b(vb);
				HfloatShort ab = a - b;
				HfloatShort ba = -(b - a);
				if (double(ab) != double(ba)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << va << " - " << vb
					                               << " = " << double(ab) << " but -(b-a) = " << double(ba) << '\n';
				}
			}
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
