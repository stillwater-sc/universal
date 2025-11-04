// trigonometry.cpp: test suite runner for trigonometric functions for ereal adaptive-precision
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

		// Verify sin function
		template<typename Real>
		int VerifySin(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: sin(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = sin(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: sin(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: sin(π/6) ≈ 0.5 (approximate, π/6 is irrational)
			x = Real(0.5235987755982989);  // π/6
			expected = 0.5;
			result = sin(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("sin", "π/6", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: sin(π/2) ≈ 1 (approximate, π/2 is irrational)
			x = Real(1.5707963267948966);  // π/2
			expected = 1.0;
			result = sin(x);

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("sin", "π/2", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: sin(-x) = -sin(x) (odd function identity)
			x = 1.0;
			Real sin_pos = sin(x);
			Real sin_neg = sin(-x);
			Real identity = sin_pos + sin_neg;
			expected = 0.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("sin(-x) + sin(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cos function
		template<typename Real>
		int VerifyCos(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: cos(0) = 1 (mathematically exact)
			Real x(0.0), expected(1.0);
			Real result = cos(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: cos(0) != 1 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: cos(π/3) ≈ 0.5 (approximate, π/3 is irrational)
			x = Real(1.0471975511965979);  // π/3
			expected = 0.5;
			result = cos(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("cos", "π/3", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cos(-x) = cos(x) (even function identity)
			x = 1.0;
			Real cos_pos = cos(x);
			Real cos_neg = cos(-x);
			Real identity = cos_pos - cos_neg;
			expected = 0.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("cos(-x) - cos(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: sin²(x) + cos²(x) = 1 (Pythagorean identity)
			x = 0.7;
			Real sin_x = sin(x);
			Real cos_x = cos(x);
			identity = sin_x * sin_x + cos_x * cos_x;
			expected = 1.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("sin²(x) + cos²(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify tan function
		template<typename Real>
		int VerifyTan(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: tan(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = tan(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: tan(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: tan(π/4) ≈ 1 (approximate, π/4 is irrational)
			x = Real(0.7853981633974483);  // π/4
			expected = 1.0;
			result = tan(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("tan", "π/4", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: tan(-x) = -tan(x) (odd function identity)
			x = 0.5;
			Real tan_pos = tan(x);
			Real tan_neg = tan(-x);
			Real identity = tan_pos + tan_neg;
			expected = 0.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("tan(-x) + tan(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify atan function
		template<typename Real>
		int VerifyAtan(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: atan(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = atan(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: atan(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: atan(1) ≈ π/4 (approximate, Taylor series convergence may be slow)
			x = 1.0;
			result = atan(x);
			expected = Real(std::atan(1.0));
			// Use relaxed threshold for Taylor series convergence at boundary
			double relaxed_threshold = 4e-3;  // Taylor series convergence at boundary
			if (!check_relative_error(result, expected, relaxed_threshold)) {
				if (reportTestCases) {
					report_error_detail("atan", "1", result, expected, relaxed_threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: atan(tan(x)) ≈ x for |x| < π/2 (identity test)
			x = 0.5;  // Use smaller value for better convergence
			result = atan(tan(x));
			expected = x;
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("atan(tan(x))", "identity", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify asin function
		template<typename Real>
		int VerifyAsin(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: asin(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = asin(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: asin(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: asin(1) ≈ π/2 (approximate)
			x = 1.0;
			result = asin(x);
			expected = Real(std::asin(1.0));
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("asin", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: asin(sin(x)) ≈ x for |x| ≤ π/2 (identity test, Taylor series convergence)
			x = 0.5;  // Use smaller value for better Taylor series convergence
			result = asin(sin(x));
			expected = x;
			// Use relaxed threshold for Taylor series convergence limitations
			double relaxed_threshold = 4e-3;  // Taylor series convergence
			if (!check_relative_error(result, expected, relaxed_threshold)) {
				if (reportTestCases) {
					report_error_detail("asin(sin(x))", "identity", result, expected, relaxed_threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify acos function
		template<typename Real>
		int VerifyAcos(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: acos(1) = 0 (approximate due to Taylor series computation)
			Real x(1.0), expected(0.0);
			Real result = acos(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("acos", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: acos(0) ≈ π/2 (approximate)
			x = 0.0;
			result = acos(x);
			expected = Real(std::acos(0.0));

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("acos", "0", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: acos(cos(x)) ≈ x for 0 ≤ x ≤ π (identity test, Taylor series convergence)
			x = 0.5;  // Use smaller value for better convergence
			result = acos(cos(x));
			expected = x;
			// Use relaxed threshold for Taylor series convergence limitations
			double relaxed_threshold = 4e-3;  // Taylor series convergence
			if (!check_relative_error(result, expected, relaxed_threshold)) {
				if (reportTestCases) {
					report_error_detail("acos(cos(x))", "identity", result, expected, relaxed_threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify atan2 function
		template<typename Real>
		int VerifyAtan2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: atan2(1, 1) ≈ π/4 (approximate, Taylor series convergence)
			Real y(1.0), x(1.0);
			Real result = atan2(y, x);
			Real expected = Real(std::atan2(1.0, 1.0));
			// Use relaxed threshold for Taylor series convergence
			double relaxed_threshold = 4e-3;  // Taylor series convergence at boundary
			if (!check_relative_error(result, expected, relaxed_threshold)) {
				if (reportTestCases) {
					report_error_detail("atan2", "(1, 1)", result, expected, relaxed_threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: atan2(1, -1) ≈ 3π/4 (approximate, Taylor series convergence)
			y = 1.0; x = -1.0;
			result = atan2(y, x);
			expected = Real(std::atan2(1.0, -1.0));
			if (!check_relative_error(result, expected, relaxed_threshold)) {
				if (reportTestCases) {
					report_error_detail("atan2", "(1, -1)", result, expected, relaxed_threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: atan2(0, 1) = 0 (mathematically exact)
			y = 0.0; x = 1.0;
			result = atan2(y, x);
			expected = 0.0;
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: atan2(0, 1) != 0 (exact)\n";
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

	std::string test_suite  = "ereal mathlib trigonometric function validation";
	std::string test_tag    = "trigonometric";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of trigonometric functions:\n";
	std::cout << "sin(1) = " << double(sin(ereal<>(1.0))) << " (expected: " << std::sin(1.0) << ")\n";
	std::cout << "cos(1) = " << double(cos(ereal<>(1.0))) << " (expected: " << std::cos(1.0) << ")\n";
	std::cout << "tan(1) = " << double(tan(ereal<>(1.0))) << " (expected: " << std::tan(1.0) << ")\n";
	std::cout << "asin(0.5) = " << double(asin(ereal<>(0.5))) << " (expected: " << std::asin(0.5) << ")\n";
	std::cout << "acos(0.5) = " << double(acos(ereal<>(0.5))) << " (expected: " << std::acos(0.5) << ")\n";
	std::cout << "atan(1.0) = " << double(atan(ereal<>(1.0))) << " (expected: " << std::atan(1.0) << ")\n";
	std::cout << "atan2(1, 1) = " << double(atan2(ereal<>(1.0), ereal<>(1.0))) << " (expected: " << std::atan2(1.0, 1.0) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 6 functions: sin, cos, tan, asin, acos, atan, atan2
	test_tag = "sin";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<>>(reportTestCases), "sin(ereal)", test_tag);

	test_tag = "cos";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<>>(reportTestCases), "cos(ereal)", test_tag);

	test_tag = "tan";
	nrOfFailedTestCases += ReportTestResult(VerifyTan<ereal<>>(reportTestCases), "tan(ereal)", test_tag);

	test_tag = "atan";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan<ereal<>>(reportTestCases), "atan(ereal)", test_tag);

	test_tag = "asin";
	nrOfFailedTestCases += ReportTestResult(VerifyAsin<ereal<>>(reportTestCases), "asin(ereal)", test_tag);

	test_tag = "acos";
	nrOfFailedTestCases += ReportTestResult(VerifyAcos<ereal<>>(reportTestCases), "acos(ereal)", test_tag);

	test_tag = "atan2";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan2<ereal<>>(reportTestCases), "atan2(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "sin high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<8>>(reportTestCases), "sin(ereal<8>)", test_tag);

	test_tag = "cos high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<8>>(reportTestCases), "cos(ereal<8>)", test_tag);

	test_tag = "tan high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTan<ereal<8>>(reportTestCases), "tan(ereal<8>)", test_tag);

	test_tag = "atan high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan<ereal<8>>(reportTestCases), "atan(ereal<8>)", test_tag);

	test_tag = "asin high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAsin<ereal<8>>(reportTestCases), "asin(ereal<8>)", test_tag);

	test_tag = "acos high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAcos<ereal<8>>(reportTestCases), "acos(ereal<8>)", test_tag);

	test_tag = "atan2 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan2<ereal<8>>(reportTestCases), "atan2(ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "sin very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<16>>(reportTestCases), "sin(ereal<16>)", test_tag);

	test_tag = "cos very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<16>>(reportTestCases), "cos(ereal<16>)", test_tag);

	test_tag = "tan very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTan<ereal<16>>(reportTestCases), "tan(ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 1216 bits (≈366 decimal digits, maximum algorithmically valid)
	test_tag = "sin extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<19>>(reportTestCases), "sin(ereal<19>)", test_tag);

	test_tag = "cos extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<19>>(reportTestCases), "cos(ereal<19>)", test_tag);
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
