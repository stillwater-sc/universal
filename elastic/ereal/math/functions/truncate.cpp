// truncate.cpp: test suite runner for truncate functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

		// Verify floor function
		template<typename Real>
		int VerifyFloor(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: floor(2.7) == 2.0
			Real x(2.7), expected(2.0);
			Real result = floor(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL floor(2.7) != 2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: floor(-2.3) == -3.0
			x = -2.3; expected = -3.0;
			result = floor(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL floor(-2.3) != -3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: floor(5.0) == 5.0 (integer)
			x = 5.0; expected = 5.0;
			result = floor(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL floor(5.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: floor(0.0) == 0.0
			Real zero(0.0);
			result = floor(zero);
			if (result != zero) {
				if (reportTestCases) std::cout << "    FAIL floor(0.0) != 0.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify ceil function
		template<typename Real>
		int VerifyCeil(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: ceil(2.3) == 3.0
			Real x(2.3), expected(3.0);
			Real result = ceil(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL ceil(2.3) != 3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: ceil(-2.7) == -2.0
			x = -2.7; expected = -2.0;
			result = ceil(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL ceil(-2.7) != -2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: ceil(5.0) == 5.0 (integer)
			x = 5.0; expected = 5.0;
			result = ceil(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL ceil(5.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: ceil(0.0) == 0.0
			Real zero(0.0);
			result = ceil(zero);
			if (result != zero) {
				if (reportTestCases) std::cout << "    FAIL ceil(0.0) != 0.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify trunc function
		template<typename Real>
		int VerifyTrunc(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: trunc(2.7) == 2.0
			Real x(2.7), expected(2.0);
			Real result = trunc(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL trunc(2.7) != 2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: trunc(-2.7) == -2.0
			x = -2.7; expected = -2.0;
			result = trunc(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL trunc(-2.7) != -2.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify round function
		template<typename Real>
		int VerifyRound(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: round(2.3) == 2.0
			Real x(2.3), expected(2.0);
			Real result = round(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL round(2.3) != 2.0\n";
				++nrOfFailedTestCases;
			}

			// Test: round(2.5) == 3.0
			x = 2.5; expected = 3.0;
			result = round(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL round(2.5) != 3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: round(2.7) == 3.0
			x = 2.7; expected = 3.0;
			result = round(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL round(2.7) != 3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: round(-2.5) == -3.0
			x = -2.5; expected = -3.0;
			result = round(x);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL round(-2.5) != -3.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer: bracketing, reflection, toward-zero, and agreement
		// with the std:: rounding of the projected double, over random values.
		template<typename Real>
		int VerifyTruncateFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-1.0e6, 1.0e6);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double d = dist(rng);
				Real x(d);
				Real fl = floor(x), ce = ceil(x), tr = trunc(x);
				// bracketing: floor(x) <= x <= ceil(x)
				if (fl > x || ce < x) {
					if (reportTestCases) std::cout << "    FAIL bracket at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// reflection: floor(-x) == -ceil(x)
				if (floor(-x) != -ceil(x)) {
					if (reportTestCases) std::cout << "    FAIL reflection at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// trunc rounds toward zero: it equals floor for x>=0, ceil for x<0
				Real expectedTrunc = (d >= 0.0) ? fl : ce;
				if (tr != expectedTrunc) {
					if (reportTestCases) std::cout << "    FAIL trunc toward zero at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// projection agrees with std:: for integer-valued results
				if (double(fl) != std::floor(d) || double(ce) != std::ceil(d) || double(tr) != std::trunc(d)) {
					if (reportTestCases) std::cout << "    FAIL std agreement at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
			}
			return nrOfFailedTestCases;
		}

}  // anonymous namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib truncate function validation";
	std::string test_tag    = "truncate";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of truncation functions:\n";
	std::cout << "floor(2.7) = " << double(floor(ereal<>(2.7))) << " (expected: 2.0)\n";
	std::cout << "ceil(2.3) = " << double(ceil(ereal<>(2.3))) << " (expected: 3.0)\n";
	std::cout << "trunc(2.7) = " << double(trunc(ereal<>(2.7))) << " (expected: 2.0)\n";
	std::cout << "round(2.5) = " << double(round(ereal<>(2.5))) << " (expected: 3.0)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 1 functions: floor, ceil
	test_tag = "floor";
	nrOfFailedTestCases += ReportTestResult(VerifyFloor<ereal<>>(reportTestCases), "floor(ereal)", test_tag);

	test_tag = "ceil";
	nrOfFailedTestCases += ReportTestResult(VerifyCeil<ereal<>>(reportTestCases), "ceil(ereal)", test_tag);

	// Phase 2 functions: trunc, round
	test_tag = "trunc";
	nrOfFailedTestCases += ReportTestResult(VerifyTrunc<ereal<>>(reportTestCases), "trunc(ereal)", test_tag);

	test_tag = "round";
	nrOfFailedTestCases += ReportTestResult(VerifyRound<ereal<>>(reportTestCases), "round(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	test_tag = "truncate fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyTruncateFuzz<ereal<>>(reportTestCases, 50), "truncate(ereal) fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// Future: Multi-component precision validation
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
