// pown.cpp: ereal integer power
//
// pown(ereal, int) negated a negative exponent in signed arithmetic before recursing, and
// -INT_MIN does not fit an int: undefined behaviour, reachable for any base other than
// zero and one (#1466). The magnitude is now taken in unsigned arithmetic and the
// reciprocal applied once at the end. The sanitizer job in CI is what turns UB here
// into a failure; the value checks pin that the rewrite still computes the power.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <climits>
#include <string>

#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}


	using Real = ereal<>;

	int VerifyPowers(bool reportTestCases) {
		int fails = 0;

		const Real two(2.0), three(3.0);
		// positive powers of two are exact
		for (int n = 0; n <= 60; ++n) {
			if (double(pown(two, n)) != std::ldexp(1.0, n)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL pown(2, " << n << ")\n";
			}
		}
		// negative powers of two are exact too: the reciprocal is applied once, at the end
		for (int n = 1; n <= 60; ++n) {
			if (double(pown(two, -n)) != std::ldexp(1.0, -n)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL pown(2, " << -n << ")\n";
			}
		}
		fails += expect_true(double(pown(three, 4)) == 81.0, "3^4", reportTestCases);
		fails += expect_true(double(pown(three, -2) * Real(9.0)) == 1.0, "3^-2 * 9", reportTestCases);

		// the special cases are untouched
		fails += expect_true(double(pown(three, 0)) == 1.0, "x^0 is 1", reportTestCases);
		fails += expect_true(double(pown(three, 1)) == 3.0, "x^1 is x", reportTestCases);
		fails += expect_true(double(pown(Real(1.0), INT_MIN)) == 1.0, "1^n is 1", reportTestCases);
		fails += expect_true(pown(Real(0.0), 5).iszero(), "0^n is 0", reportTestCases);

		return fails;
	}

	// INT_MIN is the exponent whose negation does not fit. Its result is far outside any
	// representable range, so only the absence of undefined behaviour and a well-defined
	// return are asserted here, not a value.
	int VerifyExtremeExponents(bool reportTestCases) {
		int fails = 0;

		const Real two(2.0), half(0.5);
		const Real r1 = pown(two, INT_MIN);
		const Real r2 = pown(half, INT_MIN);
		const Real r3 = pown(two, INT_MAX);
		(void)r1; (void)r2; (void)r3;
		// reaching here without a sanitizer report is the check; the calls must also
		// terminate, which a signed-overflow in the loop counter would not guarantee
		fails += expect_true(true, "pown(x, INT_MIN) returns", reportTestCases);

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "ereal pown";
	std::string test_tag    = "ereal pown";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPowers(reportTestCases), test_tag, "powers");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremeExponents(reportTestCases), test_tag, "extreme exponents");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
