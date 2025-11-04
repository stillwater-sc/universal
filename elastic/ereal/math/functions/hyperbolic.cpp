// hyperbolic.cpp: test suite runner for hyperbolic functions for ereal adaptive-precision
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

		// Verify sinh function
		template<typename Real>
		int VerifySinh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: sinh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = sinh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: sinh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: sinh(1) ≈ 1.175201194 (approximate)
			x = 1.0;
			expected = Real(std::sinh(1.0));
			result = sinh(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("sinh", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: sinh(-x) = -sinh(x) (odd function identity)
			x = 2.0;
			Real sinh_pos = sinh(x);
			Real sinh_neg = sinh(-x);
			Real identity = sinh_pos + sinh_neg;
			expected = 0.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("sinh(-x) + sinh(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cosh function
		template<typename Real>
		int VerifyCosh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: cosh(0) = 1 (mathematically exact)
			Real x(0.0), expected(1.0);
			Real result = cosh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: cosh(0) != 1 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: cosh(1) ≈ 1.543080635 (approximate)
			x = 1.0;
			expected = Real(std::cosh(1.0));
			result = cosh(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("cosh", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cosh(-x) = cosh(x) (even function identity)
			x = 2.0;
			Real cosh_pos = cosh(x);
			Real cosh_neg = cosh(-x);
			Real identity = cosh_pos - cosh_neg;
			expected = 0.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("cosh(-x) - cosh(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: cosh²(x) - sinh²(x) = 1 (fundamental hyperbolic identity)
			x = 1.5;
			Real cosh_x = cosh(x);
			Real sinh_x = sinh(x);
			identity = cosh_x * cosh_x - sinh_x * sinh_x;
			expected = 1.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("cosh²(x) - sinh²(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify tanh function
		template<typename Real>
		int VerifyTanh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: tanh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = tanh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: tanh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: tanh(1) ≈ 0.761594156 (approximate)
			x = 1.0;
			expected = Real(std::tanh(1.0));
			result = tanh(x);
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("tanh", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: tanh(-x) = -tanh(x) (odd function identity)
			x = 2.0;
			Real tanh_pos = tanh(x);
			Real tanh_neg = tanh(-x);
			Real identity = tanh_pos + tanh_neg;
			expected = 0.0;

			if (!check_relative_error(identity, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("tanh(-x) + tanh(x)", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: |tanh(x)| < 1 for all x (mathematical bound)
			x = 10.0;
			result = tanh(x);
			if (std::abs(double(result)) >= 1.0) {
				if (reportTestCases) std::cerr << "FAIL: |tanh(x)| >= 1 (bound violation)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify asinh function
		template<typename Real>
		int VerifyAsinh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: asinh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = asinh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: asinh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: asinh(sinh(x)) ≈ x (identity test, roundtrip)
			x = 1.5;
			result = asinh(sinh(x));
			expected = x;
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("asinh(sinh(x))", "identity", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: asinh(2) ≈ 1.443635475 (approximate)
			x = 2.0;
			result = asinh(x);
			expected = Real(std::asinh(2.0));

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("asinh", "2", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify acosh function
		template<typename Real>
		int VerifyAcosh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: acosh(1) = 0 (mathematically exact)
			Real x(1.0), expected(0.0);
			Real result = acosh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: acosh(1) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: acosh(cosh(x)) ≈ x for x > 0 (identity test, roundtrip)
			x = 1.5;
			result = acosh(cosh(x));
			expected = x;
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("acosh(cosh(x))", "identity", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: acosh(2) ≈ 1.316957897 (approximate)
			x = 2.0;
			result = acosh(x);
			expected = Real(std::acosh(2.0));

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("acosh", "2", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify atanh function
		template<typename Real>
		int VerifyAtanh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: atanh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = atanh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: atanh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: atanh(tanh(x)) ≈ x (identity test, roundtrip)
			x = 0.5;
			result = atanh(tanh(x));
			expected = x;
			double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("atanh(tanh(x))", "identity", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: atanh(0.5) ≈ 0.549306144 (approximate)
			x = 0.5;
			result = atanh(x);
			expected = Real(std::atanh(0.5));

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {
					report_error_detail("atanh", "0.5", result, expected, threshold);
				}
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

	std::string test_suite  = "ereal mathlib hyperbolic function validation";
	std::string test_tag    = "hyperbolic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of hyperbolic functions:\n";
	std::cout << "sinh(1) = " << double(sinh(ereal<>(1.0))) << " (expected: " << std::sinh(1.0) << ")\n";
	std::cout << "cosh(1) = " << double(cosh(ereal<>(1.0))) << " (expected: " << std::cosh(1.0) << ")\n";
	std::cout << "tanh(1) = " << double(tanh(ereal<>(1.0))) << " (expected: " << std::tanh(1.0) << ")\n";
	std::cout << "asinh(2) = " << double(asinh(ereal<>(2.0))) << " (expected: " << std::asinh(2.0) << ")\n";
	std::cout << "acosh(2) = " << double(acosh(ereal<>(2.0))) << " (expected: " << std::acosh(2.0) << ")\n";
	std::cout << "atanh(0.5) = " << double(atanh(ereal<>(0.5))) << " (expected: " << std::atanh(0.5) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 5 functions: sinh, cosh, tanh, asinh, acosh, atanh
	test_tag = "sinh";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<>>(reportTestCases), "sinh(ereal)", test_tag);

	test_tag = "cosh";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<>>(reportTestCases), "cosh(ereal)", test_tag);

	test_tag = "tanh";
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<ereal<>>(reportTestCases), "tanh(ereal)", test_tag);

	test_tag = "asinh";
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh<ereal<>>(reportTestCases), "asinh(ereal)", test_tag);

	test_tag = "acosh";
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh<ereal<>>(reportTestCases), "acosh(ereal)", test_tag);

	test_tag = "atanh";
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh<ereal<>>(reportTestCases), "atanh(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "sinh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<8>>(reportTestCases), "sinh(ereal<8>)", test_tag);

	test_tag = "cosh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<8>>(reportTestCases), "cosh(ereal<8>)", test_tag);

	test_tag = "tanh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<ereal<8>>(reportTestCases), "tanh(ereal<8>)", test_tag);

	test_tag = "asinh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh<ereal<8>>(reportTestCases), "asinh(ereal<8>)", test_tag);

	test_tag = "acosh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh<ereal<8>>(reportTestCases), "acosh(ereal<8>)", test_tag);

	test_tag = "atanh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh<ereal<8>>(reportTestCases), "atanh(ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "sinh very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<16>>(reportTestCases), "sinh(ereal<16>)", test_tag);

	test_tag = "cosh very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<16>>(reportTestCases), "cosh(ereal<16>)", test_tag);

	test_tag = "tanh very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<ereal<16>>(reportTestCases), "tanh(ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 1216 bits (≈303 decimal digits, maximum algorithmically valid)
	test_tag = "sinh extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<19>>(reportTestCases), "sinh(ereal<19>)", test_tag);

	test_tag = "cosh extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<19>>(reportTestCases), "cosh(ereal<19>)", test_tag);
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
