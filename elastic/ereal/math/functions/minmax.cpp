// minmax.cpp: test suite runner for minmax functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <limits>
#include <random>

namespace {
	using namespace sw::universal;

		// Verify min function
		template<typename Real>
		int VerifyMin(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: min(3.0, 4.0) == 3.0
			Real x(3.0), y(4.0);
			Real result = min(x, y);
			if (result != x) {
				if (reportTestCases) std::cout << "    FAIL min(3.0, 4.0) != 3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: min(5.0, 5.0) == 5.0 (equal values)
			x = 5.0; y = 5.0;
			result = min(x, y);
			if (result != x) {
				if (reportTestCases) std::cout << "    FAIL min(5.0, 5.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: min(-3.0, -1.0) == -3.0 (negative)
			x = -3.0; y = -1.0;
			result = min(x, y);
			if (result != x) {
				if (reportTestCases) std::cout << "    FAIL min(-3.0, -1.0) != -3.0\n";
				++nrOfFailedTestCases;
			}

			// Test: min(0.0, 1.0) == 0.0
			Real zero(0.0), pos(1.0);
			result = min(zero, pos);
			if (result != zero) {
				if (reportTestCases) std::cout << "    FAIL min(0.0, 1.0) != 0.0\n";
				++nrOfFailedTestCases;
			}

			// Test: min(-1.0, 0.0) == -1.0
			Real neg(-1.0);
			result = min(neg, zero);
			if (result != neg) {
				if (reportTestCases) std::cout << "    FAIL min(-1.0, 0.0) != -1.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify max function
		template<typename Real>
		int VerifyMax(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: max(3.0, 4.0) == 4.0
			Real x(3.0), y(4.0);
			Real result = max(x, y);
			if (result != y) {
				if (reportTestCases) std::cout << "    FAIL max(3.0, 4.0) != 4.0\n";
				++nrOfFailedTestCases;
			}

			// Test: max(5.0, 5.0) == 5.0 (equal values)
			x = 5.0; y = 5.0;
			result = max(x, y);
			if (result != x) {
				if (reportTestCases) std::cout << "    FAIL max(5.0, 5.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: max(-3.0, -1.0) == -1.0 (negative)
			x = -3.0; y = -1.0;
			result = max(x, y);
			if (result != y) {
				if (reportTestCases) std::cout << "    FAIL max(-3.0, -1.0) != -1.0\n";
				++nrOfFailedTestCases;
			}

			// Test: max(0.0, 1.0) == 1.0
			Real zero(0.0), pos(1.0);
			result = max(zero, pos);
			if (result != pos) {
				if (reportTestCases) std::cout << "    FAIL max(0.0, 1.0) != 1.0\n";
				++nrOfFailedTestCases;
			}

			// Test: max(-1.0, 0.0) == 0.0
			Real neg(-1.0);
			result = max(neg, zero);
			if (result != zero) {
				if (reportTestCases) std::cout << "    FAIL max(-1.0, 0.0) != 0.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer: ordering, the min+max == a+b conservation, commutativity,
		// and the min(a,b) == -max(-a,-b) reflection, over random pairs.
		template<typename Real>
		int VerifyMinMaxFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-1.0e6, 1.0e6);
			for (unsigned i = 0; i < nrIterations; ++i) {
				Real a(dist(rng)), b(dist(rng));
				Real mn = min(a, b), mx = max(a, b);
				// ordering: mn <= a,b and mx >= a,b
				if (mn > a || mn > b || mx < a || mx < b) {
					if (reportTestCases) std::cout << "    FAIL ordering\n";
					++nrOfFailedTestCases;
				}
				// conservation: {mn,mx} is a permutation of {a,b}, so mn+mx == a+b exactly
				if (mn + mx != a + b) {
					if (reportTestCases) std::cout << "    FAIL min+max != a+b\n";
					++nrOfFailedTestCases;
				}
				// commutativity
				if (min(a, b) != min(b, a) || max(a, b) != max(b, a)) {
					if (reportTestCases) std::cout << "    FAIL commutativity\n";
					++nrOfFailedTestCases;
				}
				// reflection: min(a,b) == -max(-a,-b)
				if (min(a, b) != -max(-a, -b)) {
					if (reportTestCases) std::cout << "    FAIL min/max reflection\n";
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib min/max function validation";
	std::string test_tag    = "minmax";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of min/max functions:\n";
	std::cout << "min(3.0, 4.0) = " << double(min(ereal<>(3.0), ereal<>(4.0))) << " (expected: 3.0)\n";
	std::cout << "max(3.0, 4.0) = " << double(max(ereal<>(3.0), ereal<>(4.0))) << " (expected: 4.0)\n";
	std::cout << "min(-3.0, -1.0) = " << double(min(ereal<>(-3.0), ereal<>(-1.0))) << " (expected: -3.0)\n";
	std::cout << "max(-3.0, -1.0) = " << double(max(ereal<>(-3.0), ereal<>(-1.0))) << " (expected: -1.0)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	test_tag = "min";
	nrOfFailedTestCases += ReportTestResult(VerifyMin<ereal<>>(reportTestCases), "min(ereal)", test_tag);

	test_tag = "max";
	nrOfFailedTestCases += ReportTestResult(VerifyMax<ereal<>>(reportTestCases), "max(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	test_tag = "minmax fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxFuzz<ereal<>>(reportTestCases, 1000), "minmax(ereal) fuzz", test_tag);
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
