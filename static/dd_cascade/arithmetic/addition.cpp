// addition.cpp: test suite runner for addition of double-double cascade (dd_cascade) floating-point values
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
#include "dd_cascade_corner_case_tests.hpp"

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

	std::string test_suite         = "double-double cascade addition validation";
	std::string test_tag           = "double-double cascade addition";
	bool reportTestCases           = true;  // Enable to see test details
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	dd_cascade a, b, c;

	a = 1.0;
	b = std::numeric_limits<double>::epsilon();
	c = a + b;
	std::cout << "1.0 + eps = " << c << '\n';

	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// Corner Case 1: Zero operations
	{
		dd_cascade zero(0.0, 0.0);
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(1.0);

		auto result = dd_cascade_corner_cases::verify_components(zero + a, a[0], a[1], 0.0, "0 + a = a");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_components(a + zero, a[0], a[1], 0.0, "a + 0 = a");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_zero(zero + zero, "0 + 0 = 0");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 2: Well-separated components (typical normalized case)
	{
		dd_cascade a = dd_cascade_corner_cases::create_well_separated(1.0);
		dd_cascade b = dd_cascade_corner_cases::create_well_separated(2.0);
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "well-separated addition normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Self-consistency check
		result = dd_cascade_corner_cases::verify_self_consistency_add(a, b, "well-separated self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 3: ULP boundary - adding half a ULP to 1.0
	{
		dd_cascade one(1.0, 0.0);
		double eps = std::numeric_limits<double>::epsilon();
		dd_cascade half_ulp(eps / 2.0, 0.0);
		dd_cascade sum = one + half_ulp;

		// The half_ulp should be captured in the lower components
		// sum should be > 1.0 but the high component might still be 1.0 if captured in mid/lo
		auto result = dd_cascade_corner_cases::verify_normalized(sum, "ULP boundary normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Verify self-consistency
		result = dd_cascade_corner_cases::verify_self_consistency_add(one, half_ulp, "ULP boundary self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 4: Overlapping components (triggers renormalization)
	{
		dd_cascade a = dd_cascade_corner_cases::create_overlapping_components(1.0);
		dd_cascade b = dd_cascade_corner_cases::create_overlapping_components(0.5);
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "overlapping components normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Note: overlapping components are denormalized inputs, so self-consistency
		// has larger errors due to renormalization happening during arithmetic
		// Skip self-consistency for this intentionally pathological case
	}

	// Corner Case 5: Mixed signs in internal components
	{
		dd_cascade a = dd_cascade_corner_cases::create_mixed_signs_internal();
		dd_cascade b(1.0, 1e-17);
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "mixed signs normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_add(a, b, "mixed signs self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 6: Values requiring lower components
	{
		dd_cascade a = dd_cascade_corner_cases::create_requires_lower_components();
		dd_cascade b = dd_cascade_corner_cases::create_requires_lower_components();
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "requires lower components normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		// Note: these are also denormalized inputs (overlapping components)
		// Skip self-consistency test for this pathological case
	}

	// Corner Case 7: Large magnitude values
	{
		dd_cascade a = dd_cascade_corner_cases::create_large_magnitude_separation();
		dd_cascade b = dd_cascade_corner_cases::create_large_magnitude_separation();
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "large magnitude normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_add(a, b, "large magnitude self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 8: Small magnitude values
	{
		dd_cascade a = dd_cascade_corner_cases::create_small_magnitude_separation();
		dd_cascade b = dd_cascade_corner_cases::create_small_magnitude_separation();
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "small magnitude normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_add(a, b, "small magnitude self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 9: Opposite signs (partial cancellation in addition context)
	{
		dd_cascade a(1.0, 1e-17);
		dd_cascade b(-0.5, -5e-18);
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "opposite signs normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_add(a, b, "opposite signs self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 10: Component carry propagation
	{
		// Create a scenario where lower components add up to affect higher ones
		dd_cascade a(1.0, 5e-17);
		dd_cascade b(0.0, 5e-17);
		dd_cascade sum = a + b;

		auto result = dd_cascade_corner_cases::verify_normalized(sum, "carry propagation normalization");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;

		result = dd_cascade_corner_cases::verify_self_consistency_add(a, b, "carry propagation self-consistency");
		nrOfFailedTestCases += (result.passed ? 0 : 1);
		if (!result.passed && reportTestCases) std::cerr << result.message;
	}

	// Corner Case 11: Identity test (a+b)-a=b (exposes naive compression bug)
	// This test specifically targets the precision loss from naive addition in compression
	{
		dd_cascade a(1.5, 1.5e-17);
		dd_cascade b(0.5, 5e-18);
		dd_cascade sum = a + b;
		dd_cascade recovered_b = sum - a;

		// recovered_b should equal b within reasonable tolerance
		double tolerance = std::abs(b[0]) * dd_cascade_corner_cases::DD_EPS * 10.0;
		auto result = dd_cascade_corner_cases::verify_components(
			recovered_b, b[0], b[1], tolerance, "identity (a+b)-a=b (naive compression test)"
		);
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
