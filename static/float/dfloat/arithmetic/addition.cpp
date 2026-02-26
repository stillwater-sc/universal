// addition.cpp: verify addition of dfloat decimal floating-point
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

	std::string test_suite = "dfloat<> addition validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Basic addition
	std::cout << "+---------    Basic addition\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 0, 0, 0 },
			{ 1, 0, 1 },
			{ 0, 1, 1 },
			{ 1, 1, 2 },
			{ 1, 2, 3 },
			{ 10, 20, 30 },
			{ 100, 3, 103 },
			{ 3, 100, 103 },
			{ 999, 1, 1000 },
			{ -1, 1, 0 },
			{ -5, 3, -2 },
			{ 42, -42, 0 },
		};
		for (const auto& tc : cases) {
			Decimal32 a(tc.a), b(tc.b);
			Decimal32 result = a + b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " + " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 2: Fractional addition (powers of 2, exactly representable in both binary and decimal)
	std::cout << "+---------    Fractional addition\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 0.5, 0.5, 1.0 },
			{ 0.25, 0.75, 1.0 },
			{ 0.125, 0.125, 0.25 },
			{ 0.5, 0.25, 0.75 },
		};
		for (const auto& tc : cases) {
			Decimal32 a(tc.a), b(tc.b);
			Decimal32 c = a + b;
			double d = double(c);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " + " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 3: Addition with different exponents
	std::cout << "+---------    Addition with different scales\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 1000000, 1, 1000001 },
			{ 1, 1000000, 1000001 },
			{ 0.001, 1000, 1000.001 },
		};
		for (const auto& tc : cases) {
			Decimal32 a(tc.a), b(tc.b);
			Decimal32 result = a + b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " + " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 4: Addition with negatives
	std::cout << "+---------    Addition with negatives\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ -1, -1, -2 },
			{ -10, -20, -30 },
			{ -100, 50, -50 },
			{ 100, -50, 50 },
		};
		for (const auto& tc : cases) {
			Decimal32 a(tc.a), b(tc.b);
			Decimal32 result = a + b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " + " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 5: Commutativity
	std::cout << "+---------    Commutativity: a + b == b + a\n";
	{
		double values[] = { 1, 42, -7, 0.5, 100, -100 };
		for (double va : values) {
			for (double vb : values) {
				Decimal32 a(va), b(vb);
				Decimal32 ab = a + b;
				Decimal32 ba = b + a;
				if (double(ab) != double(ba)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << va << " + " << vb
					                               << " = " << double(ab) << " but "
					                               << vb << " + " << va << " = " << double(ba) << '\n';
				}
			}
		}
	}

	// Test 6: Inf/NaN addition
	std::cout << "+---------    Inf and NaN addition\n";
	{
		Decimal32 inf(SpecificValue::infpos), ninf(SpecificValue::infneg), nan(SpecificValue::qnan);
		Decimal32 one(1);

		Decimal32 r1 = inf + one;
		if (!r1.isinf()) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: inf + 1 should be inf\n"; }

		Decimal32 r2 = inf + ninf;
		if (!r2.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: inf + (-inf) should be NaN\n"; }

		Decimal32 r3 = nan + one;
		if (!r3.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: NaN + 1 should be NaN\n"; }
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
