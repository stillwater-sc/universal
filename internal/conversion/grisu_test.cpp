// grisu_test.cpp: test suite for Grisu3 decimal conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

// Configure the value<> template environment
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0

// Include Grisu3 implementation
#include <universal/number/support/grisu.hpp>
#include <universal/internal/value/value.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Test Grisu3 with native IEEE-754 doubles
void test_grisu3_native_doubles() {
	using namespace sw::universal::grisu;

	std::cout << "Testing Grisu3 with native IEEE-754 doubles...\n";

	struct TestCase {
		double value;
		const char* description;
	};

	TestCase tests[] = {
		{ 1.0, "1.0" },
		{ 0.125, "0.125 (1/8)" },
		{ 3.14159, "3.14159 (approx pi)" },
		{ -3.14159, "-3.14159" },
		{ 1.0e20, "1.0e20 (large)" },
		{ 1.0e-20, "1.0e-20 (small)" },
		{ 123.456, "123.456" },
		{ 2.0, "2.0" },
		{ 0.1, "0.1" },
		{ 0.5, "0.5" },
		{ 1.0/3.0, "1/3" },
		{ 2.718281828459045, "e" }
	};

	for (const auto& test : tests) {
		// Use MathGeoLib's Grisu3 directly
		char buffer[32];
		int length = 0;
		int d_exp = 0;

		double v = test.value;
		bool negative = v < 0;
		if (negative) v = -v;

		bool success = grisu3_mathgeolib(v, buffer, &length, &d_exp);
		std::string result;
		if (success) {
			result = format_grisu3_output(negative, buffer, length, d_exp);
		} else {
			result = "FAILED";
		}

		std::cout << "  " << std::setw(25) << std::left << test.description
		          << " => " << result << "\n";
	}

	std::cout << "\n";
}

// Test value<> conversion (when implemented)
void test_value_to_grisu() {
	// TODO: Implement value<> to IEEE-754 conversion
	std::cout << "Testing value<> to Grisu3 conversion...\n";
	std::cout << "  (Not yet implemented - requires value<> to IEEE-754 conversion)\n";
	std::cout << "\n";
}

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 1
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "Grisu3 Converter Test Suite";
	std::string test_tag = "grisu3";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	test_grisu3_native_doubles();
	test_value_to_grisu();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures in manual testing

#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// Add regression tests here

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
