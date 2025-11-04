// pow.cpp: test suite runner for power functions for ereal adaptive-precision
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

		// Verify pow function - special cases
		template<typename Real>
		int VerifyPowSpecialCases(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: x^0 = 1 for any x
			Real x(5.0), y(0.0), expected(1.0);
			Real result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(5, 0) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: x^1 = x
			y = 1.0; expected = x;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(5, 1) != 5\n";
				++nrOfFailedTestCases;
			}

			// Test: 1^y = 1
			x = 1.0; y = 42.0; expected = 1.0;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(1, 42) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: 0^y = 0 for y > 0
			x = 0.0; y = 2.0; expected = 0.0;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(0, 2) != 0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify pow function - integer powers
		template<typename Real>
		int VerifyPowIntegerPowers(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

			double error_mag;

			// Test: 2^3 = 8
			Real x(2.0), y(3.0), expected(8.0);
			Real result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(2, 3) != 8\n";
				++nrOfFailedTestCases;
			}

			// Test: 10^2 = 100
			x = 10.0; y = 2.0; expected = 100.0;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(10, 2) != 100\n";
				++nrOfFailedTestCases;
			}

			// Test: 3^4 = 81
			x = 3.0; y = 4.0; expected = 81.0;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(3, 4) != 81\n";
				++nrOfFailedTestCases;
			}

			// Test: 2^(-1) = 0.5
			x = 2.0; y = -1.0; expected = 0.5;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(2, -1) != 0.5\n";
				++nrOfFailedTestCases;
			}

			// Test: 10^(-2) = 0.01
			x = 10.0; y = -2.0; expected = 0.01;
			result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(10, -2) != 0.01\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify pow function - fractional powers
		template<typename Real>
		int VerifyPowFractionalPowers(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: 4^0.5 = 2 (square root)
			Real x(4.0), y(0.5), expected(2.0);
			Real result = pow(x, y);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(4, 0.5) != 2\n";
				++nrOfFailedTestCases;
			}

			// Test: 8^(1/3) ≈ 2 (cube root)
			x = 8.0; y = Real(1.0) / Real(3.0);
			result = pow(x, y);
			error_mag = std::abs(double(result) - 2.0);
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: pow(8, 1/3) != 2\n";
				++nrOfFailedTestCases;
			}

			// Test: 2^0.5 = sqrt(2)
			x = 2.0; y = 0.5;
			result = pow(x, y);
			double sqrt_2 = std::sqrt(2.0);
			error_mag = std::abs(double(result) - sqrt_2);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: pow(2, 0.5) != sqrt(2)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify pow function - general powers
		template<typename Real>
		int VerifyPowGeneralPowers(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: 2^π
			Real x(2.0), y(3.141592653589793);
			Real result = pow(x, y);
			double expected = std::pow(2.0, 3.141592653589793);
			error_mag = std::abs(double(result) - expected);
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: pow(2, π) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: e^2 = exp(2)
			x = Real(std::exp(1.0)); y = 2.0;
			result = pow(x, y);
			expected = std::exp(2.0);
			error_mag = std::abs(double(result) - expected);
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: pow(e, 2) != exp(2)\n";
				++nrOfFailedTestCases;
			}

			// Test: 10^1.5
			x = 10.0; y = 1.5;
			result = pow(x, y);
			expected = std::pow(10.0, 1.5);
			error_mag = std::abs(double(result) - expected);
			if (error_mag >= 1e-13) {  // relaxed for compound operations
				if (reportTestCases) std::cerr << "FAIL: pow(10, 1.5) precision\n";
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

	std::string test_suite  = "ereal mathlib power function validation";
	std::string test_tag    = "power";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of power functions:\n";
	std::cout << "pow(2, 3) = " << double(pow(ereal<>(2.0), ereal<>(3.0))) << " (expected: 8)\n";
	std::cout << "pow(4, 0.5) = " << double(pow(ereal<>(4.0), ereal<>(0.5))) << " (expected: 2)\n";
	std::cout << "pow(e, 2) = " << double(pow(ereal<>(std::exp(1.0)), ereal<>(2.0))) << " (expected: " << std::exp(2.0) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 4b function: pow
	test_tag = "pow special cases";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<>>(reportTestCases), "pow(ereal) special", test_tag);

	test_tag = "pow integer powers";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<>>(reportTestCases), "pow(ereal) integer", test_tag);

	test_tag = "pow fractional powers";
	nrOfFailedTestCases += ReportTestResult(VerifyPowFractionalPowers<ereal<>>(reportTestCases), "pow(ereal) fractional", test_tag);

	test_tag = "pow general powers";
	nrOfFailedTestCases += ReportTestResult(VerifyPowGeneralPowers<ereal<>>(reportTestCases), "pow(ereal) general", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "pow special cases high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<8>>(reportTestCases), "pow(ereal<8>) special", test_tag);

	test_tag = "pow integer powers high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<8>>(reportTestCases), "pow(ereal<8>) integer", test_tag);

	test_tag = "pow fractional powers high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowFractionalPowers<ereal<8>>(reportTestCases), "pow(ereal<8>) fractional", test_tag);

	test_tag = "pow general powers high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowGeneralPowers<ereal<8>>(reportTestCases), "pow(ereal<8>) general", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "pow special cases very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<16>>(reportTestCases), "pow(ereal<16>) special", test_tag);

	test_tag = "pow integer powers very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<16>>(reportTestCases), "pow(ereal<16>) integer", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 2048 bits (≈617 decimal digits)
	test_tag = "pow special cases extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<32>>(reportTestCases), "pow(ereal<32>) special", test_tag);

	test_tag = "pow integer powers extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<32>>(reportTestCases), "pow(ereal<32>) integer", test_tag);
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
