// exponent.cpp: test suite runner for exponential functions for ereal adaptive-precision
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

		// Verify exp function
		template<typename Real>
		int VerifyExp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: exp(0) = 1
			Real x(0.0), expected(1.0);
			Real result = exp(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp(0) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: exp(1) = e ≈ 2.718281828
			x = 1.0;
			double exp_1 = std::exp(1.0);
			expected = Real(exp_1);
			result = exp(x);
			error_mag = std::abs(double(result) - exp_1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp(1) != e\n";
				++nrOfFailedTestCases;
			}

			// Test: exp(2) = e² ≈ 7.389056099
			x = 2.0;
			double exp_2 = std::exp(2.0);
			result = exp(x);
			error_mag = std::abs(double(result) - exp_2);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp(2) != e²\n";
				++nrOfFailedTestCases;
			}

			// Test: exp(-1) = 1/e ≈ 0.367879441
			x = -1.0;
			double exp_neg1 = std::exp(-1.0);
			result = exp(x);
			error_mag = std::abs(double(result) - exp_neg1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp(-1) != 1/e\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify exp2 function
		template<typename Real>
		int VerifyExp2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: exp2(3) = 8
			Real x(3.0), expected(8.0);
			Real result = exp2(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp2(3) != 8\n";
				++nrOfFailedTestCases;
			}

			// Test: exp2(10) = 1024
			x = 10.0; expected = 1024.0;
			result = exp2(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 3e-13) {  // slightly relaxed for larger values
				if (reportTestCases) std::cerr << "FAIL: exp2(10) != 1024\n";
				++nrOfFailedTestCases;
			}

			// Test: exp2(-1) = 0.5
			x = -1.0; expected = 0.5;
			result = exp2(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp2(-1) != 0.5\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify exp10 function
		template<typename Real>
		int VerifyExp10(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: exp10(2) = 100
			Real x(2.0), expected(100.0);
			Real result = exp10(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-13) {  // relaxed for exp10
				if (reportTestCases) std::cerr << "FAIL: exp10(2) != 100\n";
				++nrOfFailedTestCases;
			}

			// Test: exp10(3) = 1000
			x = 3.0; expected = 1000.0;
			result = exp10(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-12) {  // relaxed for exp10
				if (reportTestCases) std::cerr << "FAIL: exp10(3) != 1000\n";
				++nrOfFailedTestCases;
			}

			// Test: exp10(-1) = 0.1
			x = -1.0; expected = 0.1;
			result = exp10(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: exp10(-1) != 0.1\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify expm1 function
		template<typename Real>
		int VerifyExpm1(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: expm1(0) = 0
			Real x(0.0), expected(0.0);
			Real result = expm1(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: expm1(0) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: expm1(0.01) for small x accuracy
			x = 0.01;
			double std_expm1 = std::expm1(0.01);
			result = expm1(x);
			error_mag = std::abs(double(result) - std_expm1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: expm1(0.01) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: expm1(1) ≈ e - 1 ≈ 1.718281828
			x = 1.0;
			std_expm1 = std::expm1(1.0);
			result = expm1(x);
			error_mag = std::abs(double(result) - std_expm1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: expm1(1) != e-1\n";
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
	// Future: Extended precision tests (100-200 bits)
#endif

#if REGRESSION_LEVEL_3
	// Future: High precision tests (200-500 bits)
#endif

#if REGRESSION_LEVEL_4
	// Future: Extreme precision tests (500-1000 bits)
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
