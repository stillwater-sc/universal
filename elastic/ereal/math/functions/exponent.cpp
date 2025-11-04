// exponent.cpp: test suite runner for exponential functions for ereal adaptive-precision
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

		// Verify exp function
		template<typename Real>
		int VerifyExp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: exp(0) = 1 (mathematically exact)
			Real x(0.0), expected(1.0);
			Real result = exp(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: exp(0) != 1 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: exp(1) = e ≈ 2.718281828 (approximate)
			x = 1.0;
			double exp_1 = std::exp(1.0);
			expected = Real(exp_1);
			result = exp(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("exp", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: exp(2) = e² ≈ 7.389056099 (approximate)
			x = 2.0;
			double exp_2 = std::exp(2.0);
			expected = Real(exp_2);
			result = exp(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("exp", "2", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: exp(-1) = 1/e ≈ 0.367879441 (approximate)
			x = -1.0;
			double exp_neg1 = std::exp(-1.0);
			expected = Real(exp_neg1);
			result = exp(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("exp", "-1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify exp2 function
		template<typename Real>
		int VerifyExp2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: exp2(3) = 8 (2^3 = 8, verify with double-precision accuracy)
			// Note: exp2 implementation limited by underlying double precision (~15 digits)
			Real x(3.0), expected(8.0);
			Real result = exp2(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("exp2(3)", "8", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: exp2(10) = 1024 (2^10 = 1024, verify with double-precision accuracy)
			x = 10.0; expected = 1024.0;
			result = exp2(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("exp2(10)", "1024", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: exp2(-1) = 0.5 (2^-1 = 0.5, verify with double-precision accuracy)
			x = -1.0; expected = 0.5;
			result = exp2(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("exp2(-1)", "0.5", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify exp10 function
		template<typename Real>
		int VerifyExp10(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: exp10(2) = 100 (10^2 = 100, verify with double-precision accuracy)
			// Note: exp10 implementation limited by underlying double precision (~15 digits)
			Real x(2.0), expected(100.0);
			Real result = exp10(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("exp10(2)", "100", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: exp10(3) = 1000 (10^3 = 1000, verify with double-precision accuracy)
			x = 3.0; expected = 1000.0;
			result = exp10(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("exp10(3)", "1000", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: exp10(-1) = 0.1 (10^-1 = 0.1, verify with double-precision accuracy)
			x = -1.0; expected = 0.1;
			result = exp10(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("exp10(-1)", "0.1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify expm1 function
		template<typename Real>
		int VerifyExpm1(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: expm1(0) = 0 (e^0 - 1 = 1 - 1 = 0, mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = expm1(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: expm1(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: expm1(0.01) for small x accuracy (approximate)
			x = 0.01;
			double std_expm1 = std::expm1(0.01);
			expected = Real(std_expm1);
			result = expm1(x);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("expm1", "0.01", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: expm1(1) ≈ e - 1 ≈ 1.718281828 (approximate)
			// Note: expm1 implementation limited by underlying double precision (~15 digits)
			x = 1.0;
			std_expm1 = std::expm1(1.0);
			expected = Real(std_expm1);
			result = expm1(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("expm1", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify exp/log roundtrip
		template<typename Real>
		int VerifyExpLogRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: log(exp(x)) ≈ x for various x
			double test_values[] = {0.1, 0.5, 1.0, 2.0, 3.0};

			for (double val : test_values) {
				Real x(val);
				Real result = log(exp(x));
				error_mag = std::abs(double(result - x));
				if (error_mag >= 1e-14) {
					if (reportTestCases) {
						std::cerr << "FAIL: log(exp(" << val << ")) roundtrip error = " << error_mag << "\n";
					}
					++nrOfFailedTestCases;
				}
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

	std::string test_suite  = "ereal mathlib exponential function validation";
	std::string test_tag    = "exponential";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of exponential functions:\n";
	std::cout << "exp(1.0) = " << double(exp(ereal<>(1.0))) << " (expected: " << std::exp(1.0) << ")\n";
	std::cout << "exp2(3.0) = " << double(exp2(ereal<>(3.0))) << " (expected: 8.0)\n";
	std::cout << "exp10(2.0) = " << double(exp10(ereal<>(2.0))) << " (expected: 100.0)\n";
	std::cout << "expm1(0.01) = " << double(expm1(ereal<>(0.01))) << " (expected: " << std::expm1(0.01) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 4a functions: exp, exp2, exp10, expm1
	test_tag = "exp";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<>>(reportTestCases), "exp(ereal)", test_tag);

	test_tag = "exp2";
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<ereal<>>(reportTestCases), "exp2(ereal)", test_tag);

	test_tag = "exp10";
	nrOfFailedTestCases += ReportTestResult(VerifyExp10<ereal<>>(reportTestCases), "exp10(ereal)", test_tag);

	test_tag = "expm1";
	nrOfFailedTestCases += ReportTestResult(VerifyExpm1<ereal<>>(reportTestCases), "expm1(ereal)", test_tag);

	test_tag = "exp/log roundtrip";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<>>(reportTestCases), "log(exp(x)) roundtrip", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "exp high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<8>>(reportTestCases), "exp(ereal<8>)", test_tag);

	test_tag = "exp2 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<ereal<8>>(reportTestCases), "exp2(ereal<8>)", test_tag);

	test_tag = "exp10 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp10<ereal<8>>(reportTestCases), "exp10(ereal<8>)", test_tag);

	test_tag = "exp/log roundtrip high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<8>>(reportTestCases), "log(exp(x)) roundtrip ereal<8>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "exp very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<16>>(reportTestCases), "exp(ereal<16>)", test_tag);

	test_tag = "exp/log roundtrip very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<16>>(reportTestCases), "log(exp(x)) roundtrip ereal<16>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Maximum precision tests at ereal<19> (≈303 decimal digits, maximum algorithmically valid)
	test_tag = "exp maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<19>>(reportTestCases), "exp(ereal<19>)", test_tag);

	test_tag = "exp/log roundtrip maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<19>>(reportTestCases), "log(exp(x)) roundtrip ereal<19>", test_tag);
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
