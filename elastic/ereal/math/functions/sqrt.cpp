// sqrt.cpp: test suite runner for sqrt/cbrt functions for ereal adaptive-precision
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

		// Verify sqrt function
		template<typename Real>
		int VerifySqrt(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: sqrt(4.0) ≈ 2.0 (exact value)
			Real x(4.0), expected(2.0);
			Real result = sqrt(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: sqrt(4.0) != 2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: sqrt(9.0) ≈ 3.0 (exact value)
			x = 9.0; expected = 3.0;
			result = sqrt(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: sqrt(9.0) != 3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: (sqrt(2))^2 ≈ 2.0 (precision validation)
			x = 2.0;
			result = sqrt(x);
			Real squared = result * result;
			Real error = squared - x;
			error_mag = std::abs(double(error));
			if (error_mag >= 1e-15) {
				if (reportTestCases) {
					std::cerr << "FAIL: sqrt(2) precision: (sqrt(2))^2 - 2 = " << error_mag << "\n";
				}
				++nrOfFailedTestCases;
			}

			// Test: (sqrt(3))^2 ≈ 3.0 (precision validation)
			x = 3.0;
			result = sqrt(x);
			squared = result * result;
			error = squared - x;
			error_mag = std::abs(double(error));
			if (error_mag >= 1e-15) {
				if (reportTestCases) {
					std::cerr << "FAIL: sqrt(3) precision: (sqrt(3))^2 - 3 = " << error_mag << "\n";
				}
				++nrOfFailedTestCases;
			}

			// Test: sqrt(0.0) == 0.0
			Real zero(0.0);
			result = sqrt(zero);
			if (result != zero) {
				if (reportTestCases) std::cerr << "FAIL: sqrt(0.0) != 0.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cbrt function
		template<typename Real>
		int VerifyCbrt(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: cbrt(8.0) ≈ 2.0 (exact value)
			Real x(8.0), expected(2.0);
			Real result = cbrt(x);
			double error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cbrt(8.0) != 2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: cbrt(27.0) ≈ 3.0 (exact value)
			x = 27.0; expected = 3.0;
			result = cbrt(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cbrt(27.0) != 3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: cbrt(-8.0) ≈ -2.0 (negative value, sign preservation)
			x = -8.0; expected = -2.0;
			result = cbrt(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cbrt(-8.0) != -2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: cbrt(-27.0) ≈ -3.0 (negative value, sign preservation)
			x = -27.0; expected = -3.0;
			result = cbrt(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cbrt(-27.0) != -3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: (cbrt(2))^3 ≈ 2.0 (precision validation)
			x = 2.0;
			result = cbrt(x);
			Real cubed = result * result * result;
			Real error = cubed - x;
			error_mag = std::abs(double(error));
			if (error_mag >= 1e-15) {
				if (reportTestCases) {
					std::cerr << "FAIL: cbrt(2) precision: (cbrt(2))^3 - 2 = " << error_mag << "\n";
				}
				++nrOfFailedTestCases;
			}

			// Test: cbrt(0.0) == 0.0
			Real zero(0.0);
			result = cbrt(zero);
			if (result != zero) {
				if (reportTestCases) std::cerr << "FAIL: cbrt(0.0) != 0.0\n";
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
