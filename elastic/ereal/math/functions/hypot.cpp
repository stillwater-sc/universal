// hypot.cpp: test suite runner for hypot function for ereal adaptive-precision
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

		// Verify hypot 2D function
		template<typename Real>
		int VerifyHypot2D(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: hypot(3, 4) = 5 (Pythagorean triple, verify with high precision)
			// Note: For adaptive precision types, even exact Pythagorean triples may have correction terms
			Real x(3.0), y(4.0), expected(5.0);
			Real result = hypot(x, y);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(3, 4)", "5", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: hypot(5, 12) = 13 (Pythagorean triple, verify with high precision)
			x = 5.0; y = 12.0; expected = 13.0;
			result = hypot(x, y);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(5, 12)", "13", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: hypot(8, 15) = 17 (Pythagorean triple, verify with high precision)
			x = 8.0; y = 15.0; expected = 17.0;
			result = hypot(x, y);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(8, 15)", "17", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: hypot(1, 1)^2 = 1^2 + 1^2 (identity verification)
			x = 1.0; y = 1.0;
			result = hypot(x, y);
			Real result_squared = result * result;
			Real expected_sum = x*x + y*y;
			Real identity = result_squared;
			expected = expected_sum;
			if (!check_relative_error(identity, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(1,1)²", "identity", identity, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: hypot(0, 0) = 0 (mathematically exact)
			result = hypot(Real(0.0), Real(0.0));
			if (!result.iszero()) {
				if (reportTestCases) std::cerr << "FAIL: hypot(0, 0) != 0 (not zero)\n";
				++nrOfFailedTestCases;
			}

			// Test: hypot(3, 0) = 3 (verify with high precision)
			x = 3.0; expected = 3.0;
			result = hypot(x, Real(0.0));
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(3, 0)", "3", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify hypot 3D function
		template<typename Real>
		int VerifyHypot3D(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: hypot(0, 0, 0) = 0 (mathematically exact)
			Real result = hypot(Real(0.0), Real(0.0), Real(0.0));
			if (!result.iszero()) {
				if (reportTestCases) std::cerr << "FAIL: hypot(0, 0, 0) != 0 (not zero)\n";
				++nrOfFailedTestCases;
			}

			// Test: hypot(2, 3, 6) = 7 (Pythagorean quadruple, verify with high precision)
			// Note: For adaptive precision types, even exact Pythagorean quadruples may have correction terms
			Real x(2.0), y(3.0), z(6.0);
			Real expected = 7.0;
			result = hypot(x, y, z);
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(2, 3, 6)", "7", result, expected, threshold);
				}
				++nrOfFailedTestCases;
			}

			// Test: hypot(1, 1, 1) = sqrt(3) (unit cube diagonal, approximate as sqrt(3) is irrational)
			Real one(1.0);
			result = hypot(one, one, one);
			expected = sqrt(Real(3.0));
			if (!check_relative_error(result, expected)) {
				if (reportTestCases) {
					double threshold = get_adaptive_threshold<Real>();
					report_error_detail("hypot(1, 1, 1)", "sqrt(3)", result, expected, threshold);
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

	std::string test_suite  = "ereal mathlib hypot function validation";
	std::string test_tag    = "hypot";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of hypot functions:\n";
	std::cout << "hypot(3, 4) = " << double(hypot(ereal<>(3.0), ereal<>(4.0))) << " (expected: 5.0)\n";
	std::cout << "hypot(2, 3, 6) = " << double(hypot(ereal<>(2.0), ereal<>(3.0), ereal<>(6.0))) << " (expected: 7.0)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 3 functions: hypot 2D and 3D
	test_tag = "hypot 2D";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<>>(reportTestCases), "hypot(ereal, ereal)", test_tag);

	test_tag = "hypot 3D";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<>>(reportTestCases), "hypot(ereal, ereal, ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (≈154 decimal digits)
	test_tag = "hypot 2D high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<8>>(reportTestCases), "hypot(ereal<8>, ereal<8>)", test_tag);

	test_tag = "hypot 3D high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<8>>(reportTestCases), "hypot(ereal<8>, ereal<8>, ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (≈308 decimal digits)
	test_tag = "hypot 2D very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<16>>(reportTestCases), "hypot(ereal<16>, ereal<16>)", test_tag);

	test_tag = "hypot 3D very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<16>>(reportTestCases), "hypot(ereal<16>, ereal<16>, ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 1216 bits (≈303 decimal digits, maximum algorithmically valid)
	test_tag = "hypot 2D extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<19>>(reportTestCases), "hypot(ereal<19>, ereal<19>)", test_tag);

	test_tag = "hypot 3D extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<19>>(reportTestCases), "hypot(ereal<19>, ereal<19>, ereal<19>)", test_tag);
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
