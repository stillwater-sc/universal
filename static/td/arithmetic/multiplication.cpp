// multiplication.cpp: test suite runner for multiplication of triple-double (td) floating-point values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/td/td.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include "td_corner_case_tests.hpp"

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

	std::string test_suite         = "triple-double multiplication validation";
	std::string test_tag           = "triple-double multiplication";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	td a, b, c;

	a = 2.0;
	b = 3.0;
	c = a * b;
	std::cout << "2.0 * 3.0 = " << c << '\n';

	a = std::numeric_limits<double>::epsilon();
	b = std::numeric_limits<double>::epsilon();
	c = a * b;
	std::cout << "eps * eps = " << c << '\n';

	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// Corner Case 1: Zero absorption (0 × a = 0, a × 0 = 0)
	{
		td zero(0.0, 0.0, 0.0);
		td a = td_corner_cases::create_well_separated(1.0);

		auto result = td_corner_cases::verify_zero(zero * a, "0 × a = 0");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_zero(a * zero, "a × 0 = 0");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_zero(zero * zero, "0 × 0 = 0");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 2: Identity (1 × a ≈ a, a × 1 ≈ a)
	{
		td one(1.0, 0.0, 0.0);
		td a = td_corner_cases::create_well_separated(2.5);

		// Note: multiply_cascades applies renormalization, so exact component preservation
		// is not guaranteed. We verify the result equals the input within tolerance.
		td result_1a = one * a;
		td result_a1 = a * one;

		// Verify the high component is preserved
		if (std::abs(result_1a[0] - a[0]) > a[0] * td_corner_cases::TD_EPS * 10.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "1 × a: high component not preserved\n";
		}

		if (std::abs(result_a1[0] - a[0]) > a[0] * td_corner_cases::TD_EPS * 10.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "a × 1: high component not preserved\n";
		}

		// Verify normalization
		auto result = td_corner_cases::verify_normalized(result_1a, "1 × a normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_normalized(result_a1, "a × 1 normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 3: Commutativity (a × b = b × a)
	{
		td a = td_corner_cases::create_well_separated(1.5);
		td b = td_corner_cases::create_well_separated(2.5);

		auto result = td_corner_cases::verify_commutativity(a, b, "commutativity: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Test with different magnitudes
		td c = td_corner_cases::create_large_magnitude_separation();
		td d = td_corner_cases::create_small_magnitude_separation();
		result = td_corner_cases::verify_commutativity(c, d, "commutativity: extreme magnitudes");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 4: Powers of 2 (should be exact but renormalization may change component structure)
	{
		td a = td_corner_cases::create_well_separated(1.0);

		// Note: While powers of 2 are mathematically exact, multiply_cascades applies
		// renormalization which may change component structure. We verify the high component
		// scales exactly and the result is properly normalized.

		td result_2 = a * 2.0;
		if (std::abs(result_2[0] - 2.0 * a[0]) > 0.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "multiply by 2: high component not exact\n";
		}

		td result_4 = a * 4.0;
		if (std::abs(result_4[0] - 4.0 * a[0]) > 0.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "multiply by 4: high component not exact\n";
		}

		td result_half = a * 0.5;
		if (std::abs(result_half[0] - 0.5 * a[0]) > 0.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "multiply by 0.5: high component not exact\n";
		}

		td result_quarter = a * 0.25;
		if (std::abs(result_quarter[0] - 0.25 * a[0]) > 0.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "multiply by 0.25: high component not exact\n";
		}
	}

	// Corner Case 5: Sign patterns
	{
		td pos(1.5, 1e-17, 1e-34);
		td neg(-1.5, -1e-17, -1e-34);

		// (+) × (+) = (+)
		td result_pp = pos * pos;
		if (result_pp[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(+) × (+) produced negative result\n";
		}

		// (+) × (-) = (-)
		td result_pn = pos * neg;
		if (result_pn[0] >= 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(+) × (-) produced non-negative result\n";
		}

		// (-) × (+) = (-)
		td result_np = neg * pos;
		if (result_np[0] >= 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(-) × (+) produced non-negative result\n";
		}

		// (-) × (-) = (+)
		td result_nn = neg * neg;
		if (result_nn[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(-) × (-) produced negative result\n";
		}
	}

	// Corner Case 6: Near-1 values (precision accumulation)
	{
		td near_one_a = td_corner_cases::create_near_one(1.0);
		td near_one_b = td_corner_cases::create_near_one(2.0);
		td product = near_one_a * near_one_b;

		auto result = td_corner_cases::verify_normalized(product, "near-1 multiplication normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Product should be close to 1
		if (std::abs(product[0] - 1.0) > 1e-10) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "near-1 product not close to 1.0\n";
		}
	}

	// Corner Case 7: Well-separated components
	{
		td a = td_corner_cases::create_well_separated(1.5);
		td b = td_corner_cases::create_well_separated(2.5);
		td product = a * b;

		auto result = td_corner_cases::verify_normalized(product, "well-separated multiplication normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_self_consistency_mul(a, b, "well-separated self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 8: Component interaction (all 9 products contribute)
	{
		td a(1.0, 0.1, 0.01);
		td b(2.0, 0.2, 0.02);
		td product = a * b;

		auto result = td_corner_cases::verify_normalized(product, "component interaction normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Note: These are denormalized inputs (overlapping components)
		// Skip self-consistency for this pathological case
	}

	// Corner Case 9: Associativity test
	{
		td a = td_corner_cases::create_well_separated(1.5);
		td b = td_corner_cases::create_well_separated(2.0);
		td c = td_corner_cases::create_well_separated(3.0);

		auto result = td_corner_cases::verify_associativity_mul(a, b, c, "associativity: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 10: Distributivity test
	{
		td a = td_corner_cases::create_well_separated(2.0);
		td b = td_corner_cases::create_well_separated(1.0);
		td c = td_corner_cases::create_well_separated(0.5);

		auto result = td_corner_cases::verify_distributivity(a, b, c, "distributivity: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 11: Large magnitude values
	{
		td a = td_corner_cases::create_large_magnitude_separation();
		td b(1.0e50, 1.0e33, 1.0e16);  // Moderate size to avoid overflow
		td product = a * b;

		auto result = td_corner_cases::verify_normalized(product, "large magnitude multiplication normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_self_consistency_mul(a, b, "large magnitude self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 12: Small magnitude values
	{
		td a = td_corner_cases::create_small_magnitude_separation();
		td b(1.0e-50, 1.0e-67, 1.0e-84);  // Moderate size to avoid underflow
		td product = a * b;

		auto result = td_corner_cases::verify_normalized(product, "small magnitude multiplication normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_self_consistency_mul(a, b, "small magnitude self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 13: Mixed signs in components
	{
		td a = td_corner_cases::create_mixed_signs_internal();
		td b(2.0, 1e-17, 1e-34);
		td product = a * b;

		auto result = td_corner_cases::verify_normalized(product, "mixed signs multiplication normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_corner_cases::verify_self_consistency_mul(a, b, "mixed signs self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 14: Squaring (a × a)
	{
		td a = td_corner_cases::create_square_test_value();
		td square = a * a;

		auto result = td_corner_cases::verify_normalized(square, "squaring normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Square should be positive
		if (square[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "square produced negative result\n";
		}

		result = td_corner_cases::verify_commutativity(a, a, "squaring commutativity");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
