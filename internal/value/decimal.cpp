// decimal.cpp: test suite for decimal conversion of value<> types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// Configure the value<> template environment
// enable/disable value arithmetic exceptions
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0

// Select Dragon algorithm for testing
#define DECIMAL_CONVERTER_USE_DRAGON

// minimum set of include files to reflect source code dependencies
#include <universal/number/support/decimal.hpp>
#include <universal/number/support/dragon.hpp>
#include <universal/number/support/decimal_converter.hpp>
#include <universal/internal/value/value.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Test Dragon algorithm basic functionality
void test_dragon_basic() {
	using namespace sw::universal::dragon;

	std::cout << "Testing Dragon algorithm basic functions...\n";

	// Test power of 2 multiplication
	{
		support::decimal d;
		d.setdigit(1);
		multiply_by_power_of_2(d, 3); // 1 * 2^3 = 8
		std::cout << "1 * 2^3 = " << d << " (expected 8)\n";
	}

	// Test power of 5 multiplication
	{
		support::decimal d;
		d.setdigit(2);
		multiply_by_power_of_5(d, 2); // 2 * 5^2 = 50
		std::cout << "2 * 5^2 = " << d << " (expected 50)\n";
	}

	// Test decimal string formatting
	{
		dragon_context ctx(std::ios_base::scientific, 3);
		std::string result = format_decimal_string(false, "1234", 2, ctx);
		std::cout << "format_decimal_string(1234, exp=2, scientific, prec=3) = " << result << "\n";
	}

	{
		dragon_context ctx(std::ios_base::fixed, 2);
		std::string result = format_decimal_string(false, "1234", 2, ctx);
		std::cout << "format_decimal_string(1234, exp=2, fixed, prec=2) = " << result << "\n";
	}

	std::cout << "\n";
}

// Test value<> to decimal conversion
void test_value_conversion() {
	using namespace sw::universal::internal;

	std::cout << "Testing value<> to decimal conversion...\n";

	// Test simple value
	{
		value<52> v(1.0);
		std::cout << "value<52>(1.0):\n";
		std::cout << "  Default:    " << to_decimal_string(v) << "\n";
		std::cout << "  Scientific: " << to_decimal_string(v, std::ios_base::scientific, 10) << "\n";
		std::cout << "  Fixed:      " << to_decimal_string(v, std::ios_base::fixed, 4) << "\n";
	}

	// Test fractional value
	{
		value<52> v(0.125); // 1/8
		std::cout << "\nvalue<52>(0.125):\n";
		std::cout << "  Default:    " << to_decimal_string(v) << "\n";
		std::cout << "  Scientific: " << to_decimal_string(v, std::ios_base::scientific, 10) << "\n";
		std::cout << "  Fixed:      " << to_decimal_string(v, std::ios_base::fixed, 6) << "\n";
	}

	// Test negative value
	{
		value<52> v(-3.14159);
		std::cout << "\nvalue<52>(-3.14159):\n";
		std::cout << "  Default:    " << to_decimal_string(v) << "\n";
		std::cout << "  Scientific: " << to_decimal_string(v, std::ios_base::scientific, 10) << "\n";
		std::cout << "  Fixed:      " << to_decimal_string(v, std::ios_base::fixed, 8) << "\n";
	}

	// Test large value
	{
		value<52> v(1.0e20);
		std::cout << "\nvalue<52>(1.0e20):\n";
		std::cout << "  Default:    " << to_decimal_string(v) << "\n";
		std::cout << "  Scientific: " << to_decimal_string(v, std::ios_base::scientific, 10) << "\n";
	}

	// Test small value
	{
		value<52> v(1.0e-20);
		std::cout << "\nvalue<52>(1.0e-20):\n";
		std::cout << "  Default:    " << to_decimal_string(v) << "\n";
		std::cout << "  Scientific: " << to_decimal_string(v, std::ios_base::scientific, 10) << "\n";
	}

	// Test special cases
	{
		value<52> zero(0.0);
		value<52> inf;
		inf.setinf();
		value<52> nan;
		nan.setnan();

		std::cout << "\nSpecial values:\n";
		std::cout << "  Zero: " << to_decimal_string(zero) << "\n";
		std::cout << "  +Inf: " << to_decimal_string(inf) << "\n";
		std::cout << "  NaN:  " << to_decimal_string(nan) << "\n";
	}

	std::cout << "\n";
}


// Test ioflags variations
void test_ioflags() {
	using namespace sw::universal::internal;

	std::cout << "Testing ioflags variations...\n";

	value<52> v(123.456);

	std::cout << "value<52>(123.456) with different flags:\n";
	std::cout << "  default:           " << to_decimal_string(v) << "\n";
	std::cout << "  showpos:           " << to_decimal_string(v, std::ios_base::showpos) << "\n";
	std::cout << "  scientific:        " << to_decimal_string(v, std::ios_base::scientific) << "\n";
	std::cout << "  fixed:             " << to_decimal_string(v, std::ios_base::fixed) << "\n";
	std::cout << "  scientific+showpos:" << to_decimal_string(v, std::ios_base::scientific | std::ios_base::showpos) << "\n";
	std::cout << "  scientific, prec=12:" << to_decimal_string(v, std::ios_base::scientific, 12) << "\n";
	std::cout << "  fixed, prec=2:     " << to_decimal_string(v, std::ios_base::fixed, 2) << "\n";
	std::cout << "  fixed, prec=10:    " << to_decimal_string(v, std::ios_base::fixed, 10) << "\n";

	std::cout << "\n";
}

// Test stream insertion operators
void test_stream_insertion() {
	using namespace sw::universal::internal;

	std::cout << "Testing stream insertion operators...\n";

	value<52> v1(42.0);
	value<52> v2(-0.001);

	std::cout << "Default stream insertion:\n";
	std::cout << "  v1 = " << v1 << "\n";
	std::cout << "  v2 = " << v2 << "\n";

	std::cout << "\nWith manipulators:\n";
	std::cout << "  scientific: " << std::scientific << v1 << "\n";
	std::cout << "  fixed:      " << std::fixed << v2 << "\n";
	std::cout << "  precision:  " << std::setprecision(12) << std::scientific << v1 << "\n";
	std::cout << std::defaultfloat << std::setprecision(6); // Reset

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

	std::string test_suite = "Decimal Converter Test Suite";
	std::string test_tag = "decimal_converter";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Report which algorithm is active
#ifdef DECIMAL_CONVERTER_USE_DRAGON
	std::cout << "Using: Dragon Algorithm\n\n";
#else
	std::cout << "Using: Grisu Algorithm (default)\n\n";
#endif

#if MANUAL_TESTING

	test_dragon_basic();
	test_value_conversion();
	test_ioflags();
	test_stream_insertion();

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
