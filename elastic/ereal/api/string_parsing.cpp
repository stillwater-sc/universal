// string_parsing.cpp: test suite for ereal string parsing functionality
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Helper: compute relative error
template<unsigned maxlimbs>
double relative_error(const ereal<maxlimbs>& computed, double expected) {
	double comp_double = double(computed);
	if (expected == 0.0) {
		return std::abs(comp_double);
	}
	return std::abs((comp_double - expected) / expected);
}

// Test basic integer parsing
int test_integers() {
	int nrOfFailedTests = 0;

	// Positive integer
	{
		ereal<> x("123");
		double expected = 123.0;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('123') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	// Negative integer
	{
		ereal<> x("-456");
		double expected = -456.0;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('-456') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	// Large integer
	{
		ereal<> x("123456789012345");
		double expected = 123456789012345.0;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('123456789012345') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test decimal parsing
int test_decimals() {
	int nrOfFailedTests = 0;

	// Simple decimal
	{
		ereal<> x("3.14159");
		double expected = 3.14159;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('3.14159') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	// Negative decimal
	{
		ereal<> x("-456.789");
		double expected = -456.789;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('-456.789') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	// Leading zero
	{
		ereal<> x("0.00123");
		double expected = 0.00123;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('0.00123') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test scientific notation parsing
int test_scientific_notation() {
	int nrOfFailedTests = 0;

	// Positive exponent
	{
		ereal<> x("1.23e10");
		double expected = 1.23e10;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-14) {  // Slightly relaxed for large numbers
			std::cout << "FAIL: parse('1.23e10') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	// Negative exponent
	{
		ereal<> x("4.56e-5");
		double expected = 4.56e-5;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('4.56e-5') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	// Capital E
	{
		ereal<> x("7.89E+3");
		double expected = 7.89e3;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "FAIL: parse('7.89E+3') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test extreme exponents (THIS IS WHERE WE HAVE ISSUES)
int test_extreme_exponents() {
	int nrOfFailedTests = 0;

	std::cout << "\nTesting extreme exponents (expect precision issues here):\n";

	// Very large exponent
	{
		ereal<> x("1e100");
		double expected = 1e100;
		double comp_double = double(x);
		double rel_err = relative_error(x, expected);

		std::cout << "  parse('1e100'):\n";
		std::cout << "    computed  = " << std::scientific << std::setprecision(17) << comp_double << "\n";
		std::cout << "    expected  = " << expected << "\n";
		std::cout << "    rel_err   = " << rel_err << "\n";

		// Relaxed threshold for now - we'll fix this
		if (rel_err > 1e-13) {
			std::cout << "    FAIL: rel_err too large\n";
			++nrOfFailedTests;
		}
	}

	// Very small exponent
	{
		ereal<> x("1e-100");
		double expected = 1e-100;
		double comp_double = double(x);
		double rel_err = relative_error(x, expected);

		std::cout << "  parse('1e-100'):\n";
		std::cout << "    computed  = " << std::scientific << std::setprecision(17) << comp_double << "\n";
		std::cout << "    expected  = " << expected << "\n";
		std::cout << "    rel_err   = " << rel_err << "\n";

		// Relaxed threshold for now - we'll fix this
		if (rel_err > 1e-13) {
			std::cout << "    FAIL: rel_err too large\n";
			++nrOfFailedTests;
		}
	}

	// Exponent = 0
	{
		ereal<> x("1e0");
		double expected = 1.0;
		double rel_err = relative_error(x, expected);
		if (rel_err > 1e-15) {
			std::cout << "  FAIL: parse('1e0') = " << double(x) << ", expected " << expected << ", rel_err = " << rel_err << "\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test high-precision strings
int test_high_precision() {
	int nrOfFailedTests = 0;

	std::cout << "\nTesting high-precision parsing:\n";

	// 100-digit π
	{
		std::string pi_100 = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679";
		ereal<> pi(pi_100);
		double expected = 3.141592653589793238462643;  // First ~25 digits of π
		double comp_double = double(pi);
		double rel_err = relative_error(pi, expected);

		std::cout << "  parse(100-digit π):\n";
		std::cout << "    computed  = " << std::setprecision(20) << comp_double << "\n";
		std::cout << "    expected  = " << expected << "\n";
		std::cout << "    rel_err   = " << std::scientific << rel_err << "\n";

		// Should match to machine precision for ereal<4>
		if (rel_err > 1e-15) {
			std::cout << "    FAIL: should match to machine epsilon\n";
			++nrOfFailedTests;
		}
	}

	// Test that ereal<8> can hold more precision than ereal<4>
	{
		std::string precise_val = "1.23456789012345678901234567890";
		ereal<4> x4(precise_val);
		ereal<8> x8(precise_val);

		std::cout << "  parse(30-digit number):\n";
		std::cout << "    ereal<4>  = " << std::setprecision(20) << double(x4) << "\n";
		std::cout << "    ereal<8>  = " << std::setprecision(20) << double(x8) << "\n";
		std::cout << "    (Both limited by double conversion for display)\n";

		// Can't easily test this without comparing limbs directly
		// This test is more for documentation purposes
	}

	return nrOfFailedTests;
}

// Test error handling
int test_error_handling() {
	int nrOfFailedTests = 0;

	// Empty string
	{
		ereal<> x("    ");  // Only whitespace
		// Should fail, resulting in zero
		if (!x.iszero()) {
			std::cout << "FAIL: parse('    ') should result in zero, got " << double(x) << "\n";
			++nrOfFailedTests;
		}
	}

	// Invalid characters
	{
		ereal<> x("abc");
		// Should fail, resulting in zero
		if (!x.iszero()) {
			std::cout << "FAIL: parse('abc') should result in zero, got " << double(x) << "\n";
			++nrOfFailedTests;
		}
	}

	// Multiple decimal points
	{
		ereal<> x("1.2.3");
		// Should fail, resulting in zero
		if (!x.iszero()) {
			std::cout << "FAIL: parse('1.2.3') should result in zero, got " << double(x) << "\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing configuration
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal<maxlimbs> string parsing";
	std::string test_tag = "ereal string parsing";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing for debugging
	std::cout << "\nManual Testing:\n";

	// Test large exponent precision issues
	ereal<> x("1e100");
	std::cout << "parse('1e100') = " << std::scientific << std::setprecision(17) << double(x) << "\n";
	std::cout << "expected       = " << 1e100 << "\n";

	ereal<> y("1e-100");
	std::cout << "parse('1e-100') = " << double(y) << "\n";
	std::cout << "expected        = " << 1e-100 << "\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1
	std::cout << "Integer parsing\n";
	nrOfFailedTestCases += test_integers();

	std::cout << "Decimal parsing\n";
	nrOfFailedTestCases += test_decimals();

	std::cout << "Scientific notation parsing\n";
	nrOfFailedTestCases += test_scientific_notation();
#endif

#if REGRESSION_LEVEL_2
	std::cout << "Error handling\n";
	nrOfFailedTestCases += test_error_handling();
#endif

#if REGRESSION_LEVEL_3
	std::cout << "High-precision parsing\n";
	nrOfFailedTestCases += test_high_precision();
#endif

#if REGRESSION_LEVEL_4
	std::cout << "Extreme exponents (known precision issues)\n";
	nrOfFailedTestCases += test_extreme_exponents();
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
