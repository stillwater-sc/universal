// numerics.cpp: test suite runner for numeric support functions for ereal adaptive-precision
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

		// Verify copysign function
		template<typename Real>
		int VerifyCopysign(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: copysign(5.0, -3.0) == -5.0
			Real x(5.0), y(-3.0), expected(-5.0);
			Real result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: copysign(5.0, -3.0) != -5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: copysign(-5.0, 3.0) == 5.0
			x = -5.0; y = 3.0; expected = 5.0;
			result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: copysign(-5.0, 3.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: copysign(5.0, 3.0) == 5.0 (both positive)
			x = 5.0; y = 3.0; expected = 5.0;
			result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: copysign(5.0, 3.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: copysign(-5.0, -3.0) == -5.0 (both negative)
			x = -5.0; y = -3.0; expected = -5.0;
			result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: copysign(-5.0, -3.0) != -5.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify ldexp function
		template<typename Real>
		int VerifyLdexp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: ldexp(1.0, 3) == 8.0 (1.0 * 2^3)
			Real x(1.0), expected(8.0);
			Real result = ldexp(x, 3);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: ldexp(1.0, 3) != 8.0\n";
				++nrOfFailedTestCases;
			}

			// Test: ldexp(1.0, -2) == 0.25 (1.0 * 2^-2)
			expected = 0.25;
			result = ldexp(x, -2);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: ldexp(1.0, -2) != 0.25\n";
				++nrOfFailedTestCases;
			}

			// Test: ldexp(1.0, 0) == 1.0
			expected = 1.0;
			result = ldexp(x, 0);
			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: ldexp(1.0, 0) != 1.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify frexp function
		template<typename Real>
		int VerifyFrexp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: frexp(8.0) == (0.5, 4)  because 8.0 = 0.5 * 2^4
			Real x(8.0), expected_mantissa(0.5);
			int exp, expected_exp = 4;
			Real mantissa = frexp(x, &exp);

			if (mantissa != expected_mantissa) {
				if (reportTestCases) std::cerr << "FAIL: frexp(8.0) mantissa != 0.5\n";
				++nrOfFailedTestCases;
			}
			if (exp != expected_exp) {
				if (reportTestCases) std::cerr << "FAIL: frexp(8.0) exponent != 4\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify frexp/ldexp roundtrip
		template<typename Real>
		int VerifyFrexpLdexpRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: ldexp(frexp(x, &e), e) == x
			Real x(6.0);
			int exp;
			Real mantissa = frexp(x, &exp);
			Real reconstructed = ldexp(mantissa, exp);

			if (reconstructed != x) {
				if (reportTestCases) std::cerr << "FAIL: ldexp(frexp(6.0)) != 6.0\n";
				++nrOfFailedTestCases;
			}

			// Test with different value
			x = 100.0;
			mantissa = frexp(x, &exp);
			reconstructed = ldexp(mantissa, exp);

			if (reconstructed != x) {
				if (reportTestCases) std::cerr << "FAIL: ldexp(frexp(100.0)) != 100.0\n";
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

	std::string test_suite  = "ereal mathlib numeric support function validation";
	std::string test_tag    = "numerics";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of numeric functions:\n";
	std::cout << "copysign(5.0, -3.0) = " << double(copysign(ereal<>(5.0), ereal<>(-3.0))) << " (expected: -5.0)\n";
	std::cout << "ldexp(1.0, 3) = " << double(ldexp(ereal<>(1.0), 3)) << " (expected: 8.0)\n";

	int exp;
	ereal<> mantissa = frexp(ereal<>(8.0), &exp);
	std::cout << "frexp(8.0) = (" << double(mantissa) << ", " << exp << ") (expected: (0.5, 4))\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 1 function: copysign
	test_tag = "copysign";
	nrOfFailedTestCases += ReportTestResult(VerifyCopysign<ereal<>>(reportTestCases), "copysign(ereal)", test_tag);

	// Phase 2 functions: ldexp, frexp
	test_tag = "ldexp";
	nrOfFailedTestCases += ReportTestResult(VerifyLdexp<ereal<>>(reportTestCases), "ldexp(ereal)", test_tag);

	test_tag = "frexp";
	nrOfFailedTestCases += ReportTestResult(VerifyFrexp<ereal<>>(reportTestCases), "frexp(ereal)", test_tag);

	test_tag = "frexp/ldexp roundtrip";
	nrOfFailedTestCases += ReportTestResult(VerifyFrexpLdexpRoundtrip<ereal<>>(reportTestCases), "frexp/ldexp roundtrip", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Future: Extended tests with edge cases
#endif

#if REGRESSION_LEVEL_3
	// Future: Precision validation
#endif

#if REGRESSION_LEVEL_4
	// Future: Stress tests
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
