// sqrt.cpp: test suite runner for sqrt/cbrt functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib_adaptive.hpp>

namespace sw {
	namespace universal {

		// Verify sqrt function
		template<typename Real>
		int VerifySqrt(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: sqrt(4.0) = 2.0 (perfect square, verify with high precision)
			// Note: For adaptive precision types, even perfect squares may have correction terms
			Real x(4.0), expected(2.0);
			Real result = sqrt(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("sqrt(4.0)", "2.0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: sqrt(9.0) = 3.0 (perfect square, verify with high precision)
			x = 9.0; expected = 3.0;
			result = sqrt(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("sqrt(9.0)", "3.0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: (sqrt(2))² = 2.0 (identity verification)
			x = 2.0;
			result = sqrt(x);
			Real squared = result * result;
			expected = x;
			if (!check_relative_error(squared, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("sqrt(2)²", "identity", squared, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: (sqrt(3))² = 3.0 (identity verification)
			x = 3.0;
			result = sqrt(x);
			squared = result * result;
			expected = x;
			if (!check_relative_error(squared, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("sqrt(3)²", "identity", squared, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: sqrt(0.0) = 0.0 (mathematically exact)
			Real zero(0.0);
			result = sqrt(zero);
			expected = 0.0;
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: sqrt(0.0) != 0.0 (exact)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cbrt function
		template<typename Real>
		int VerifyCbrt(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: cbrt(8.0) = 2.0 (perfect cube, verify with high precision)
			// Note: For adaptive precision types, even perfect cubes may have correction terms
			Real x(8.0), expected(2.0);
			Real result = cbrt(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("cbrt(8.0)", "2.0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cbrt(27.0) = 3.0 (perfect cube, verify with high precision)
			x = 27.0; expected = 3.0;
			result = cbrt(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("cbrt(27.0)", "3.0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cbrt(-8.0) = -2.0 (perfect cube, negative value, verify with high precision)
			x = -8.0; expected = -2.0;
			result = cbrt(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("cbrt(-8.0)", "-2.0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cbrt(-27.0) = -3.0 (perfect cube, negative value, verify with high precision)
			x = -27.0; expected = -3.0;
			result = cbrt(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("cbrt(-27.0)", "-3.0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: (cbrt(2))³ = 2.0 (identity verification)
			x = 2.0;
			result = cbrt(x);
			Real cubed = result * result * result;
			expected = x;
			if (!check_relative_error(cubed, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("cbrt(2)³", "identity", cubed, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cbrt(0.0) = 0.0 (mathematically exact)
			Real zero(0.0);
			result = cbrt(zero);
			expected = 0.0;
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: cbrt(0.0) != 0.0 (exact)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

	}
}

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

	std::string test_suite  = "ereal mathlib sqrt/cbrt function validation";
	std::string test_tag    = "sqrt/cbrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of sqrt/cbrt functions:\n";
	std::cout << "sqrt(4.0) = " << double(sqrt(ereal<>(4.0))) << " (expected: 2.0)\n";
	std::cout << "cbrt(8.0) = " << double(cbrt(ereal<>(8.0))) << " (expected: 2.0)\n";
	std::cout << "cbrt(-8.0) = " << double(cbrt(ereal<>(-8.0))) << " (expected: -2.0)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 3 functions: sqrt, cbrt
	test_tag = "sqrt";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<>>(reportTestCases), "sqrt(ereal)", test_tag);

	test_tag = "cbrt";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<>>(reportTestCases), "cbrt(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "sqrt high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<8>>(reportTestCases), "sqrt(ereal<8>)", test_tag);

	test_tag = "cbrt high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<8>>(reportTestCases), "cbrt(ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "sqrt very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<16>>(reportTestCases), "sqrt(ereal<16>)", test_tag);

	test_tag = "cbrt very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<16>>(reportTestCases), "cbrt(ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 1216 bits (≈303 decimal digits, maximum algorithmically valid)
	test_tag = "sqrt extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<19>>(reportTestCases), "sqrt(ereal<19>)", test_tag);

	test_tag = "cbrt extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<19>>(reportTestCases), "cbrt(ereal<19>)", test_tag);
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
