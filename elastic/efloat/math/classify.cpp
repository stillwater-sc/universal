// classify.cpp: regression tests for efloat mathematical classification functions.
//
// Categories tested:
//   - fpclassify, isfinite, isinf, isnan, isnormal, isdenorm, signbit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	int VerifyEfloatClassification(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. fpclassify Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying fpclassify...\n";
		{
			efloat<4> zero(0.0);
			if (fpclassify(zero) != FP_ZERO) {
				if (reportTestCases) std::cout << "    FAIL: fpclassify(0.0) is not FP_ZERO\n";
				++failures;
			}
			efloat<4> one(1.0);
			if (fpclassify(one) != FP_NORMAL) {
				if (reportTestCases) std::cout << "    FAIL: fpclassify(1.0) is not FP_NORMAL\n";
				++failures;
			}
			efloat<4> inf;
			inf.setinf(false);
			if (fpclassify(inf) != FP_INFINITE) {
				if (reportTestCases) std::cout << "    FAIL: fpclassify(+inf) is not FP_INFINITE\n";
				++failures;
			}
			efloat<4> nan;
			nan.setnan();
			if (fpclassify(nan) != FP_NAN) {
				if (reportTestCases) std::cout << "    FAIL: fpclassify(nan) is not FP_NAN\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. isfinite Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying isfinite...\n";
		{
			efloat<4> zero(0.0);
			efloat<4> one(1.0);
			efloat<4> inf; inf.setinf(false);
			efloat<4> nan; nan.setnan();

			if (!isfinite(zero) || !isfinite(one)) {
				if (reportTestCases) std::cout << "    FAIL: finite values classified as non-finite\n";
				++failures;
			}
			if (isfinite(inf) || isfinite(nan)) {
				if (reportTestCases) std::cout << "    FAIL: infinite/nan values classified as finite\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. isinf & isnan Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying isinf and isnan...\n";
		{
			efloat<4> one(1.0);
			efloat<4> inf; inf.setinf(false);
			efloat<4> nan; nan.setnan();

			if (!isinf(inf) || isinf(one) || isinf(nan)) {
				if (reportTestCases) std::cout << "    FAIL: isinf check failed\n";
				++failures;
			}
			if (!isnan(nan) || isnan(one) || isnan(inf)) {
				if (reportTestCases) std::cout << "    FAIL: isnan check failed\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. isnormal & isdenorm Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying isnormal and isdenorm...\n";
		{
			efloat<4> zero(0.0);
			efloat<4> one(1.0);
			efloat<4> inf; inf.setinf(false);
			efloat<4> nan; nan.setnan();

			if (!isnormal(one) || isnormal(zero) || isnormal(inf) || isnormal(nan)) {
				if (reportTestCases) std::cout << "    FAIL: isnormal check failed\n";
				++failures;
			}
			if (isdenorm(one) || isdenorm(zero)) {
				if (reportTestCases) std::cout << "    FAIL: isdenorm check failed (should always be false)\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 5. signbit Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying signbit...\n";
		{
			efloat<4> pos(1.0);
			efloat<4> neg(-1.0);
			efloat<4> neg_zero(-0.0);
			efloat<4> neg_inf; neg_inf.setinf(true);

			if (signbit(pos)) {
				if (reportTestCases) std::cout << "    FAIL: signbit(pos) is true\n";
				++failures;
			}
			if (!signbit(neg)) {
				if (reportTestCases) std::cout << "    FAIL: signbit(neg) is false\n";
				++failures;
			}
			if (!signbit(neg_zero)) {
				if (reportTestCases) std::cout << "    FAIL: signbit(-0.0) is false\n";
				++failures;
			}
			if (!signbit(neg_inf)) {
				if (reportTestCases) std::cout << "    FAIL: signbit(-inf) is false\n";
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

	std::string test_suite          = "efloat mathematical classification library";
	std::string test_tag            = "classify";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatClassification(reportTestCases), "efloat", "classify manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatClassification(reportTestCases), "efloat", "classify foundational");
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
