// minmax.cpp: regression tests for efloat mathematical minimum, maximum, and difference functions.
//
// Categories tested:
//   - min, max, fmin, fmax, fdim
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/efloat/math/minmax.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	int VerifyEfloatMinmax(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. min & max Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying min and max...\n";
		{
			efloat<4> x(5.5);
			efloat<4> y(2.0);
			if (min(x, y) != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: min(5.5, 2.0) is not 2.0\n";
				++failures;
			}
			if (max(x, y) != 5.5) {
				if (reportTestCases) std::cout << "    FAIL: max(5.5, 2.0) is not 5.5\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. fmin & fmax NaN Suppression & Signed Zero Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying fmin and fmax...\n";
		{
			efloat<4> x(5.5);
			efloat<4> nan; nan.setnan();

			// fmin(5.5, NaN) == 5.5
			if (fmin(x, nan) != 5.5) {
				if (reportTestCases) std::cout << "    FAIL: fmin(5.5, NaN) is not 5.5\n";
				++failures;
			}
			if (fmin(nan, x) != 5.5) {
				if (reportTestCases) std::cout << "    FAIL: fmin(NaN, 5.5) is not 5.5\n";
				++failures;
			}

			// fmax(5.5, NaN) == 5.5
			if (fmax(x, nan) != 5.5) {
				if (reportTestCases) std::cout << "    FAIL: fmax(5.5, NaN) is not 5.5\n";
				++failures;
			}
			if (fmax(nan, x) != 5.5) {
				if (reportTestCases) std::cout << "    FAIL: fmax(NaN, 5.5) is not 5.5\n";
				++failures;
			}

			// fmin(NaN, NaN) is NaN
			if (!fmin(nan, nan).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: fmin(NaN, NaN) did not return NaN\n";
				++failures;
			}

			// fmax(NaN, NaN) is NaN
			if (!fmax(nan, nan).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: fmax(NaN, NaN) did not return NaN\n";
				++failures;
			}

			// IEEE-754 signed-zero tie-breaking (CodeRabbit feedback)
			efloat<4> pos_zero(0.0);
			efloat<4> neg_zero(-0.0);

			// fmin(-0.0, +0.0) -> -0.0
			efloat<4> res_fmin1 = fmin(neg_zero, pos_zero);
			if (res_fmin1 != 0.0 || res_fmin1.sign() != -1) {
				if (reportTestCases) std::cout << "    FAIL: fmin(-0.0, +0.0) is not -0.0\n";
				++failures;
			}
			efloat<4> res_fmin2 = fmin(pos_zero, neg_zero);
			if (res_fmin2 != 0.0 || res_fmin2.sign() != -1) {
				if (reportTestCases) std::cout << "    FAIL: fmin(+0.0, -0.0) is not -0.0\n";
				++failures;
			}

			// fmax(-0.0, +0.0) -> +0.0
			efloat<4> res_fmax1 = fmax(neg_zero, pos_zero);
			if (res_fmax1 != 0.0 || res_fmax1.sign() != 1) {
				if (reportTestCases) std::cout << "    FAIL: fmax(-0.0, +0.0) is not +0.0\n";
				++failures;
			}
			efloat<4> res_fmax2 = fmax(pos_zero, neg_zero);
			if (res_fmax2 != 0.0 || res_fmax2.sign() != 1) {
				if (reportTestCases) std::cout << "    FAIL: fmax(+0.0, -0.0) is not +0.0\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. fdim Validation (Positive Difference)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying fdim (Positive Difference)...\n";
		{
			efloat<4> x(5.5);
			efloat<4> y(2.0);
			efloat<4> nan; nan.setnan();

			// fdim(5.5, 2.0) == 3.5
			if (fdim(x, y) != 3.5) {
				if (reportTestCases) std::cout << "    FAIL: fdim(5.5, 2.0) is not 3.5. Result: " << double(fdim(x, y)) << "\n";
				++failures;
			}

			// fdim(2.0, 5.5) == 0.0
			if (fdim(y, x) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: fdim(2.0, 5.5) is not 0.0. Result: " << double(fdim(y, x)) << "\n";
				++failures;
			}

			// fdim NaN propagation (CodeRabbit feedback)
			if (!fdim(x, nan).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: fdim(5.5, NaN) did not return NaN\n";
				++failures;
			}
			if (!fdim(nan, y).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: fdim(NaN, 2.0) did not return NaN\n";
				++failures;
			}
		}

		return failures;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat mathematical minmax library";
	std::string test_tag            = "minmax";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatMinmax(reportTestCases), "efloat", "minmax manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatMinmax(reportTestCases), "efloat", "minmax foundational");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
