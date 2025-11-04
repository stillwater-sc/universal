// hyperbolic.cpp: test suite runner for hyperbolic functions for ereal adaptive-precision
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

		// Verify sinh function
		template<typename Real>
		int VerifySinh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: sinh(0) = 0
			Real x(0.0), expected(0.0);
			Real result = sinh(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: sinh(0) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: sinh(1) ≈ 1.175201194
			x = 1.0;
			double sinh_1 = std::sinh(1.0);
			result = sinh(x);
			error_mag = std::abs(double(result) - sinh_1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: sinh(1) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: sinh(-x) = -sinh(x) (odd function)
			x = 2.0;
			Real sinh_pos = sinh(x);
			Real sinh_neg = sinh(-x);
			error_mag = std::abs(double(sinh_pos + sinh_neg));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: sinh(-x) != -sinh(x)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cosh function
		template<typename Real>
		int VerifyCosh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: cosh(0) = 1
			Real x(0.0), expected(1.0);
			Real result = cosh(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cosh(0) != 1\n";
				++nrOfFailedTestCases;
			}

			// Test: cosh(1) ≈ 1.543080635
			x = 1.0;
			double cosh_1 = std::cosh(1.0);
			result = cosh(x);
			error_mag = std::abs(double(result) - cosh_1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cosh(1) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: cosh(-x) = cosh(x) (even function)
			x = 2.0;
			Real cosh_pos = cosh(x);
			Real cosh_neg = cosh(-x);
			error_mag = std::abs(double(cosh_pos - cosh_neg));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: cosh(-x) != cosh(x)\n";
				++nrOfFailedTestCases;
			}

			// Test: cosh²(x) - sinh²(x) = 1 (fundamental identity)
			x = 1.5;
			Real cosh_x = cosh(x);
			Real sinh_x = sinh(x);
			Real identity = cosh_x * cosh_x - sinh_x * sinh_x;
			error_mag = std::abs(double(identity) - 1.0);
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: cosh²(x) - sinh²(x) != 1\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify tanh function
		template<typename Real>
		int VerifyTanh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: tanh(0) = 0
			Real x(0.0), expected(0.0);
			Real result = tanh(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: tanh(0) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: tanh(1) ≈ 0.761594156
			x = 1.0;
			double tanh_1 = std::tanh(1.0);
			result = tanh(x);
			error_mag = std::abs(double(result) - tanh_1);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: tanh(1) precision\n";
				++nrOfFailedTestCases;
			}

			// Test: tanh(-x) = -tanh(x) (odd function)
			x = 2.0;
			Real tanh_pos = tanh(x);
			Real tanh_neg = tanh(-x);
			error_mag = std::abs(double(tanh_pos + tanh_neg));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: tanh(-x) != -tanh(x)\n";
				++nrOfFailedTestCases;
			}

			// Test: |tanh(x)| < 1 for all x
			x = 10.0;
			result = tanh(x);
			if (std::abs(double(result)) >= 1.0) {
				if (reportTestCases) std::cerr << "FAIL: |tanh(x)| >= 1\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify asinh function
		template<typename Real>
		int VerifyAsinh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: asinh(0) = 0
			Real x(0.0), expected(0.0);
			Real result = asinh(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: asinh(0) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: asinh(sinh(x)) ≈ x (roundtrip)
			x = 1.5;
			result = asinh(sinh(x));
			error_mag = std::abs(double(result - x));
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: asinh(sinh(x)) != x\n";
				++nrOfFailedTestCases;
			}

			// Test: asinh(2) comparison with std::asinh
			x = 2.0;
			result = asinh(x);
			double expected_val = std::asinh(2.0);
			error_mag = std::abs(double(result) - expected_val);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: asinh(2) precision\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify acosh function
		template<typename Real>
		int VerifyAcosh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: acosh(1) = 0
			Real x(1.0), expected(0.0);
			Real result = acosh(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: acosh(1) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: acosh(cosh(x)) ≈ x for x > 0 (roundtrip)
			x = 1.5;
			result = acosh(cosh(x));
			error_mag = std::abs(double(result - x));
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: acosh(cosh(x)) != x\n";
				++nrOfFailedTestCases;
			}

			// Test: acosh(2) comparison with std::acosh
			x = 2.0;
			result = acosh(x);
			double expected_val = std::acosh(2.0);
			error_mag = std::abs(double(result) - expected_val);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: acosh(2) precision\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify atanh function
		template<typename Real>
		int VerifyAtanh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			double error_mag;

			// Test: atanh(0) = 0
			Real x(0.0), expected(0.0);
			Real result = atanh(x);
			error_mag = std::abs(double(result - expected));
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: atanh(0) != 0\n";
				++nrOfFailedTestCases;
			}

			// Test: atanh(tanh(x)) ≈ x (roundtrip)
			x = 0.5;
			result = atanh(tanh(x));
			error_mag = std::abs(double(result - x));
			if (error_mag >= 1e-14) {  // slightly relaxed
				if (reportTestCases) std::cerr << "FAIL: atanh(tanh(x)) != x\n";
				++nrOfFailedTestCases;
			}

			// Test: atanh(0.5) comparison with std::atanh
			x = 0.5;
			result = atanh(x);
			double expected_val = std::atanh(0.5);
			error_mag = std::abs(double(result) - expected_val);
			if (error_mag >= 1e-15) {
				if (reportTestCases) std::cerr << "FAIL: atanh(0.5) precision\n";
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
	// Extreme precision tests at 2048 bits (≈617 decimal digits)
	test_tag = "sinh extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<32>>(reportTestCases), "sinh(ereal<32>)", test_tag);

	test_tag = "cosh extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<32>>(reportTestCases), "cosh(ereal<32>)", test_tag);
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
