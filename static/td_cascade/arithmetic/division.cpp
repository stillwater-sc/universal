// division.cpp: test suite runner for division of triple-double cascade (td_cascade) floating-point values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include "td_cascade_corner_case_tests.hpp"

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

	std::string test_suite         = "triple-double cascade division validation";
	std::string test_tag           = "triple-double cascade division";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	td_cascade a, b, c;

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
		td_cascade zero(0.0, 0.0, 0.0);
		td_cascade a = td_cascade_corner_cases::create_well_separated(1.0);

		// 0 / 0 should be NaN
		td_cascade result_00 = zero / zero;
		if (!result_00.isnan()) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "0/0 did not produce NaN\n";
		}

		// a / 0 should be ±Inf
		td_cascade result_a0 = a / zero;
		if (!result_a0.isinf()) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "a/0 did not produce Inf\n";
		}
	}

	// Corner Case 2: Division identity (a / a = 1)
	{
		td_cascade a = td_cascade_corner_cases::create_well_separated(2.5);
		auto result = td_cascade_corner_cases::verify_division_identity(a, "a/a=1: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		td_cascade b = td_cascade_corner_cases::create_large_magnitude_separation();
		result = td_cascade_corner_cases::verify_division_identity(b, "a/a=1: large magnitude");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		td_cascade c = td_cascade_corner_cases::create_small_magnitude_separation();
		result = td_cascade_corner_cases::verify_division_identity(c, "a/a=1: small magnitude");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 3: Division by 1 (a / 1 = a)
	{
		td_cascade one(1.0, 0.0, 0.0);
		td_cascade a = td_cascade_corner_cases::create_well_separated(2.5);
		td_cascade quotient = a / one;

		// High component should be preserved
		if (std::abs(quotient[0] - a[0]) > a[0] * td_cascade_corner_cases::TD_EPS * 10.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "a / 1: high component not preserved\n";
		}

		auto result = td_cascade_corner_cases::verify_normalized(quotient, "a / 1 normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 4: Double reciprocal (1 / (1 / a) = a)
	{
		td_cascade a = td_cascade_corner_cases::create_for_reciprocal_test(2.0);
		auto result = td_cascade_corner_cases::verify_double_reciprocal(a, "double reciprocal: scale 2.0");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		td_cascade b = td_cascade_corner_cases::create_for_reciprocal_test(0.5);
		result = td_cascade_corner_cases::verify_double_reciprocal(b, "double reciprocal: scale 0.5");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 5: Powers of 2 (should be exact-ish)
	{
		td_cascade a = td_cascade_corner_cases::create_well_separated(8.0);

		td_cascade result_2 = a / 2.0;
		if (std::abs(result_2[0] - 4.0) > td_cascade_corner_cases::TD_EPS * 100.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "division by 2: high component incorrect\n";
		}

		td_cascade result_4 = a / 4.0;
		if (std::abs(result_4[0] - 2.0) > td_cascade_corner_cases::TD_EPS * 100.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "division by 4: high component incorrect\n";
		}

		td_cascade result_half = a / 0.5;
		if (std::abs(result_half[0] - 16.0) > td_cascade_corner_cases::TD_EPS * 100.0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "division by 0.5: high component incorrect\n";
		}
	}

	// Corner Case 6: Sign patterns
	{
		td_cascade pos(1.5, 1e-17, 1e-34);
		td_cascade neg(-1.5, -1e-17, -1e-34);

		// (+) / (+) = (+)
		td_cascade result_pp = pos / pos;
		if (result_pp[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(+) / (+) produced negative result\n";
		}

		// (+) / (-) = (-)
		td_cascade result_pn = pos / neg;
		if (result_pn[0] >= 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(+) / (-) produced non-negative result\n";
		}

		// (-) / (+) = (-)
		td_cascade result_np = neg / pos;
		if (result_np[0] >= 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(-) / (+) produced non-negative result\n";
		}

		// (-) / (-) = (+)
		td_cascade result_nn = neg / neg;
		if (result_nn[0] < 0) {
			nrOfFailedTestCases++;
			if (reportTestCases) std::cerr << "(-) / (-) produced negative result\n";
		}
	}

	// Corner Case 7: Non-commutativity (a / b ≠ b / a)
	{
		td_cascade a = td_cascade_corner_cases::create_well_separated(2.0);
		td_cascade b = td_cascade_corner_cases::create_well_separated(3.0);

		auto result = td_cascade_corner_cases::verify_non_commutativity(a, b, "non-commutativity: well-separated");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 8: Self-consistency (a / b) × b ≈ a
	{
		td_cascade a = td_cascade_corner_cases::create_well_separated(1.5);
		td_cascade b = td_cascade_corner_cases::create_well_separated(2.5);

		auto result = td_cascade_corner_cases::verify_self_consistency_div(a, b, "well-separated self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 9: Well-known divisions (1/3, 1/7, 1/9)
	{
		td_cascade one(1.0, 0.0, 0.0);
		td_cascade three(3.0, 0.0, 0.0);
		//td_cascade one_third = one / three;

		// Verify self-consistency
		auto result = td_cascade_corner_cases::verify_self_consistency_div(one, three, "1/3 self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		td_cascade seven(7.0, 0.0, 0.0);
		result = td_cascade_corner_cases::verify_self_consistency_div(one, seven, "1/7 self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		td_cascade nine(9.0, 0.0, 0.0);
		result = td_cascade_corner_cases::verify_self_consistency_div(one, nine, "1/9 self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 10: Large / small (convergence test)
	{
		td_cascade large = td_cascade_corner_cases::create_large_magnitude_separation();
		td_cascade small(1.0e-50, 1.0e-67, 1.0e-84);

		td_cascade quotient = large / small;
		auto result = td_cascade_corner_cases::verify_normalized(quotient, "large/small normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Self-consistency (may be loose due to extreme magnitudes)
		result = td_cascade_corner_cases::verify_self_consistency_div(large, small, "large/small self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 11: Small / large (convergence test)
	{
		td_cascade small = td_cascade_corner_cases::create_small_magnitude_separation();
		td_cascade large(1.0e50, 1.0e33, 1.0e16);

		td_cascade quotient = small / large;
		auto result = td_cascade_corner_cases::verify_normalized(quotient, "small/large normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_cascade_corner_cases::verify_self_consistency_div(small, large, "small/large self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 12: Component-rich division
	{
		td_cascade a = td_cascade_corner_cases::create_well_separated(5.0);
		td_cascade b = td_cascade_corner_cases::create_well_separated(3.0);

		td_cascade quotient = a / b;
		auto result = td_cascade_corner_cases::verify_normalized(quotient, "component-rich division normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = td_cascade_corner_cases::verify_self_consistency_div(a, b, "component-rich self-consistency");
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
