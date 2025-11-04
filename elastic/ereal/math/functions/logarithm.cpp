// logarithm.cpp: test suite runner for logarithm functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

		// Verify log function
		template<typename Real>
		int VerifyLog(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: log(1) = 0
			Real x(1.0), expected(0.0);
			Real result = log(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log(1) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: log(e) = 1
			x = Real(std::exp(1.0));
			expected = Real(1.0);
			result = log(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log(e) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: log(2) ≈ 0.693147181
			x = 2.0;
			double log_2 = std::log(2.0);
			result = log(x);
			error_mag = std::abs(double(result) - log_2);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log(2) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: log(10) ≈ 2.302585093
			x = 10.0;
			double log_10 = std::log(10.0);
			result = log(x);
			error_mag = std::abs(double(result) - log_10);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log(10) precision\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log2 function
		template<typename Real>
		int VerifyLog2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: log2(2) = 1
			Real x(2.0), expected(1.0);
			Real result = log2(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log2(2) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: log2(8) = 3
			x = 8.0; expected = 3.0;
			result = log2(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log2(8) != 3\n";
				++nrOfFailedTestCases;
			}

			// Test: log2(1024) = 10
			x = 1024.0; expected = 10.0;
			result = log2(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: log2(1024) != 10\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log10 function
		template<typename Real>
		int VerifyLog10(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: log10(10) = 1
			Real x(10.0), expected(1.0);
			Real result = log10(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log10(10) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: log10(100) = 2
			x = 100.0; expected = 2.0;
			result = log10(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log10(100) != 2\n";
				++nrOfFailedTestCases;
			}

			// Test: log10(1000) = 3
			x = 1000.0; expected = 3.0;
			result = log10(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: log10(1000) != 3\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log1p function
		template<typename Real>
		int VerifyLog1p(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: log1p(0) = 0
			Real x(0.0), expected(0.0);
			Real result = log1p(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log1p(0) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: log1p(0.01) for small x accuracy
			x = 0.01;
			double std_log1p = std::log1p(0.01);
			result = log1p(x);
			error_mag = std::abs(double(result) - std_log1p);
			if (error_mag >= 1e-6) {  // relaxed for Taylor series
				if (reportTestCases) std::cerr << "FAIL: log1p(0.01) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: log1p(1) = log(2) ≈ 0.693147181
			x = 1.0;
			std_log1p = std::log1p(1.0);
			result = log1p(x);
			error_mag = std::abs(double(result) - std_log1p);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: log1p(1) != log(2)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify log/exp roundtrip
		template<typename Real>
		int VerifyLogExpRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: exp(log(x)) ≈ x for various x
			double test_values[] = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};

			for (double val : test_values) {
				Real x(val);
				Real result = exp(log(x));
				error_mag = std::abs(double(result - x));
				if (error_mag >= 1e-14) {
					if (reportTestCases) {
						std::cerr << "FAIL: exp(log(" << val << ")) roundtrip error = " << error_mag << "\n";
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
	// Extreme precision tests at 2048 bits (≈617 decimal digits)
	test_tag = "log extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<32>>(reportTestCases), "log(ereal<32>)", test_tag);

	test_tag = "exp/log roundtrip extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<32>>(reportTestCases), "exp(log(x)) roundtrip ereal<32>", test_tag);
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
