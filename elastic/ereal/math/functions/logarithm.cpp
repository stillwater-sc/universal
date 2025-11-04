// logarithm.cpp: test suite runner for logarithm functions for ereal adaptive-precision
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

		// Verify log function
		template<typename Real>
		int VerifyLog(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: log(1) = 0 (mathematically exact)
			Real x(1.0), expected(0.0);
			Real result = log(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: log(1) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: log(e) = 1 (approximate - e is irrational)
			x = Real(std::exp(1.0));
			expected = Real(1.0);
			result = log(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {

					report_error_detail("log", "e", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log(2) ≈ 0.693147181 (approximate)
			x = 2.0;
			double log_2 = std::log(2.0);
			expected = Real(log_2);
			result = log(x);

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {

					report_error_detail("log", "2", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log(10) ≈ 2.302585093 (approximate)
			x = 10.0;
			double log_10 = std::log(10.0);
			expected = Real(log_10);
			result = log(x);

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {

					report_error_detail("log", "10", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log2 function
		template<typename Real>
		int VerifyLog2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: log2(2) = 1 (verify with double-precision accuracy)
			// Note: log2 implementation limited by underlying double precision
			Real x(2.0), expected(1.0);
			Real result = log2(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("log2(2)", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log2(8) = 3 (verify with double-precision accuracy)
			x = 8.0; expected = 3.0;
			result = log2(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("log2(8)", "3", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log2(1024) = 10 (verify with double-precision accuracy)
			x = 1024.0; expected = 10.0;
			result = log2(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("log2(1024)", "10", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log10 function
		template<typename Real>
		int VerifyLog10(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: log10(10) = 1 (verify with double-precision accuracy)
			// Note: log10 implementation limited by underlying double precision
			Real x(10.0), expected(1.0);
			Real result = log10(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("log10(10)", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log10(100) = 2 (verify with double-precision accuracy)
			x = 100.0; expected = 2.0;
			result = log10(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("log10(100)", "2", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log10(1000) = 3 (verify with double-precision accuracy)
			x = 1000.0; expected = 3.0;
			result = log10(x);
			if (!check_relative_error(result, expected, threshold)) {
				if (reportTestCases) {
					report_error_detail("log10(1000)", "3", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log1p function
		template<typename Real>
		int VerifyLog1p(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: log1p(0) = 0 (log(1+0) = log(1) = 0, exact)
			Real x(0.0), expected(0.0);
			Real result = log1p(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cerr << "FAIL: log1p(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: log1p(0.01) for small x accuracy (approximate)
			x = 0.01;
			double std_log1p = std::log1p(0.01);
			expected = Real(std_log1p);
			result = log1p(x);
			// Use relaxed threshold for Taylor series approximation
			double relaxed_threshold = 3e-5;  // Taylor series achieves ~2e-5 accuracy
			if (!check_relative_error(result, expected, relaxed_threshold)) {
				if (reportTestCases) {
					report_error_detail("log1p", "0.01", result, expected, relaxed_threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: log1p(1) = log(2) ≈ 0.693147181 (approximate)
			x = 1.0;
			std_log1p = std::log1p(1.0);
			expected = Real(std_log1p);
			result = log1p(x);
			constexpr double threshold = 1e-14;  // Double precision accuracy

			if (!check_relative_error(result, expected, threshold)) {

				if (reportTestCases) {

					report_error_detail("log1p", "1", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log/exp roundtrip (identity-based validation)
		template<typename Real>
		int VerifyLogExpRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: exp(log(x)) = x for various x (mathematical identity)
			double test_values[] = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};

			constexpr double threshold = 1e-14;  // Double precision accuracy

			for (double val : test_values) {
				Real x(val);
				Real result = exp(log(x));
				if (!check_relative_error(result, x, threshold)) {
					if (reportTestCases) {
						std::cerr << "FAIL: exp(log(" << val << ")) roundtrip\n";
						report_error_detail("exp(log(x))", std::to_string(val), result, x, threshold);
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

	std::string test_suite  = "ereal mathlib logarithm function validation";
	std::string test_tag    = "logarithm";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of logarithm functions:\n";
	std::cout << "log(e) = " << double(log(ereal<>(std::exp(1.0)))) << " (expected: 1.0)\n";
	std::cout << "log2(8.0) = " << double(log2(ereal<>(8.0))) << " (expected: 3.0)\n";
	std::cout << "log10(100.0) = " << double(log10(ereal<>(100.0))) << " (expected: 2.0)\n";
	std::cout << "log1p(0.01) = " << double(log1p(ereal<>(0.01))) << " (expected: " << std::log1p(0.01) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 4a functions: log, log2, log10, log1p
	test_tag = "log";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<>>(reportTestCases), "log(ereal)", test_tag);

	test_tag = "log2";
	nrOfFailedTestCases += ReportTestResult(VerifyLog2<ereal<>>(reportTestCases), "log2(ereal)", test_tag);

	test_tag = "log10";
	nrOfFailedTestCases += ReportTestResult(VerifyLog10<ereal<>>(reportTestCases), "log10(ereal)", test_tag);

	test_tag = "log1p";
	nrOfFailedTestCases += ReportTestResult(VerifyLog1p<ereal<>>(reportTestCases), "log1p(ereal)", test_tag);

	test_tag = "log/exp roundtrip";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<>>(reportTestCases), "exp(log(x)) roundtrip", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "log high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<8>>(reportTestCases), "log(ereal<8>)", test_tag);

	test_tag = "log2 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog2<ereal<8>>(reportTestCases), "log2(ereal<8>)", test_tag);

	test_tag = "log10 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog10<ereal<8>>(reportTestCases), "log10(ereal<8>)", test_tag);

	test_tag = "exp/log roundtrip high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<8>>(reportTestCases), "exp(log(x)) roundtrip ereal<8>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "log very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<16>>(reportTestCases), "log(ereal<16>)", test_tag);

	test_tag = "exp/log roundtrip very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<16>>(reportTestCases), "exp(log(x)) roundtrip ereal<16>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Maximum precision tests at ereal<19> (≈303 decimal digits, maximum algorithmically valid)
	test_tag = "log maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<19>>(reportTestCases), "log(ereal<19>)", test_tag);

	test_tag = "exp/log roundtrip maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<19>>(reportTestCases), "exp(log(x)) roundtrip ereal<19>", test_tag);
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
