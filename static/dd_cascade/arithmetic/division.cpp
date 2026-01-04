// division.cpp: test suite runner for division of double-double cascade (dd_cascade) floating-point values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include "corner_cases.hpp"

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

	std::string test_suite         = "double-double cascade division validation";
	std::string test_tag           = "double-double cascade division";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	dd_cascade a, b, c;

	a = 1.0;
	b = 3.0;
	c = a / b;
	std::cout << "1.0 / 3.0 = " << c << '\n';

	a = 1.0;
	b = std::numeric_limits<double>::epsilon();
	c = a / b;
	std::cout << "1.0 / eps = " << c << '\n';

	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// Corner Case 1: Division by zero handling
	{
		dd_cascade zero(0.0, 0.0);
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(1.0);

		// 0 / 0 should be NaN
		dd_cascade result_00 = zero / zero;
		if (!result_00.isnan()) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "0/0 did not produce NaN\n";
		}

		// a / 0 should be ±Inf
		dd_cascade result_a0 = a / zero;
		if (!result_a0.isinf()) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "a/0 did not produce Inf\n";
		}
	}

	// Corner Case 2: Division identity (a / a = 1)
	{
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(2.5);
		auto result = dd_cascade_corner_cases::verify_division_identity(a, "a/a=1: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		dd_cascade b = dd_cascade_corner_cases::create_large_magnitude_separation();
		result = dd_cascade_corner_cases::verify_division_identity(b, "a/a=1: large magnitude");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		dd_cascade c = dd_cascade_corner_cases::create_small_magnitude_separation();
		result = dd_cascade_corner_cases::verify_division_identity(c, "a/a=1: small magnitude");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 3: Division by 1 (a / 1 = a)
	{
		dd_cascade one(1.0, 0.0);
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(2.5);
		dd_cascade quotient = a / one;

		// High component should be preserved
		if (std::abs(quotient[0] - a[0]) > a[0] * dd_cascade_corner_cases::DD_EPS * 10.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "a / 1: high component not preserved\n";
		}

		auto result = dd_cascade_corner_cases::verify_normalized(quotient, "a / 1 normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 4: Double reciprocal (1 / (1 / a) = a)
	{
		dd_cascade a = dd_cascade_corner_cases::create_for_reciprocal_test(2.0);
		auto result = dd_cascade_corner_cases::verify_double_reciprocal(a, "double reciprocal: scale 2.0");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		dd_cascade b = dd_cascade_corner_cases::create_for_reciprocal_test(0.5);
		result = dd_cascade_corner_cases::verify_double_reciprocal(b, "double reciprocal: scale 0.5");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 5: Powers of 2 (should be exact-ish)
	{
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(8.0);

		dd_cascade result_2 = a / 2.0;
		if (std::abs(result_2[0] - 4.0) > dd_cascade_corner_cases::DD_EPS * 100.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "division by 2: high component incorrect\n";
		}

		dd_cascade result_4 = a / 4.0;
		if (std::abs(result_4[0] - 2.0) > dd_cascade_corner_cases::DD_EPS * 100.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "division by 4: high component incorrect\n";
		}

		dd_cascade result_half = a / 0.5;
		if (std::abs(result_half[0] - 16.0) > dd_cascade_corner_cases::DD_EPS * 100.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "division by 0.5: high component incorrect\n";
		}
	}

	// Corner Case 6: Sign patterns
	{
		dd_cascade pos(1.5, 1e-17);
		dd_cascade neg(-1.5, -1e-17);

		// (+) / (+) = (+)
		dd_cascade result_pp = pos / pos;
		if (result_pp[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(+) / (+) produced negative result\n";
		}

		// (+) / (-) = (-)
		dd_cascade result_pn = pos / neg;
		if (result_pn[0] >= 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(+) / (-) produced non-negative result\n";
		}

		// (-) / (+) = (-)
		dd_cascade result_np = neg / pos;
		if (result_np[0] >= 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(-) / (+) produced non-negative result\n";
		}

		// (-) / (-) = (+)
		dd_cascade result_nn = neg / neg;
		if (result_nn[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(-) / (-) produced negative result\n";
		}
	}

	// Corner Case 7: Non-commutativity (a / b ≠ b / a)
	{
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(2.0);
		dd_cascade b = dd_cascade_corner_cases::create_well_separated(3.0);

		auto result = dd_cascade_corner_cases::verify_non_commutativity(a, b, "non-commutativity: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 8: Self-consistency (a / b) × b ≈ a
	{
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(1.5);
		dd_cascade b = dd_cascade_corner_cases::create_well_separated(2.5);

		auto result = dd_cascade_corner_cases::verify_self_consistency_div(a, b, "well-separated self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 9: Well-known divisions (1/3, 1/7, 1/9)
	{
		dd_cascade one(1.0, 0.0);
		dd_cascade three(3.0, 0.0);
		//dd_cascade one_third = one / three;

		// Verify self-consistency
		auto result = dd_cascade_corner_cases::verify_self_consistency_div(one, three, "1/3 self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		dd_cascade seven(7.0, 0.0);
		result = dd_cascade_corner_cases::verify_self_consistency_div(one, seven, "1/7 self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		dd_cascade nine(9.0, 0.0);
		result = dd_cascade_corner_cases::verify_self_consistency_div(one, nine, "1/9 self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 10: Large / small (convergence test)
	{
		dd_cascade large = dd_cascade_corner_cases::create_large_magnitude_separation();
		dd_cascade small(1.0e-50, 1.0e-67);

		dd_cascade quotient = large / small;
		auto result = dd_cascade_corner_cases::verify_normalized(quotient, "large/small normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Self-consistency (may be loose due to extreme magnitudes)
		result = dd_cascade_corner_cases::verify_self_consistency_div(large, small, "large/small self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 11: Small / large (convergence test)
	{
		dd_cascade small = dd_cascade_corner_cases::create_small_magnitude_separation();
		dd_cascade large(1.0e50, 1.0e33);

		dd_cascade quotient = small / large;
		auto result = dd_cascade_corner_cases::verify_normalized(quotient, "small/large normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_div(small, large, "small/large self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 12: Component-rich division
	{
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(5.0);
		dd_cascade b = dd_cascade_corner_cases::create_well_separated(3.0);

		dd_cascade quotient = a / b;
		auto result = dd_cascade_corner_cases::verify_normalized(quotient, "component-rich division normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_div(a, b, "component-rich self-consistency");
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
