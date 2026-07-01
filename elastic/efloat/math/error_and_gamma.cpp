// error_and_gamma.cpp: regression tests for efloat mathematical error and gamma functions.
//
// Categories tested:
//   - erf, erfc, tgamma, lgamma
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

	// Helper to check if two values are close
	template<unsigned nlimbs>
	bool IsClose(const sw::universal::efloat<nlimbs>& a, double target_val, double tolerance = 1e-12) {
		double diff = std::abs(double(a) - target_val);
		return diff <= tolerance;
	}

	int VerifyEfloatErrorAndGamma(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Error Function (erf) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Error Function (erf)...\n";
		{
			efloat<4> zero(0.0);
			if (erf(zero) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: erf(0.0) is not 0.0\n";
				++failures;
			}

			efloat<4> one(1.0);
			double expected_erf1 = std::erf(1.0);
			if (!IsClose(erf(one), expected_erf1)) {
				if (reportTestCases) std::cout << "    FAIL: erf(1.0) is inaccurate. Result: " << double(erf(one)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. Complementary Error Function (erfc) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Complementary Error Function (erfc)...\n";
		{
			efloat<4> zero(0.0);
			if (erfc(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: erfc(0.0) is not 1.0\n";
				++failures;
			}

			efloat<4> one(1.0);
			double expected_erfc1 = std::erfc(1.0);
			if (!IsClose(erfc(one), expected_erfc1)) {
				if (reportTestCases) std::cout << "    FAIL: erfc(1.0) is inaccurate. Result: " << double(erfc(one)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Gamma Function (tgamma) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Gamma Function (tgamma)...\n";
		{
			efloat<4> four(4.0);
			if (!IsClose(tgamma(four), 6.0)) {
				if (reportTestCases) std::cout << "    FAIL: tgamma(4.0) is not 6.0. Result: " << double(tgamma(four)) << "\n";
				++failures;
			}

			efloat<4> half(0.5);
			double expected_gamma_half = std::tgamma(0.5); // should be sqrt(pi) approx. 1.77245...
			if (!IsClose(tgamma(half), expected_gamma_half)) {
				if (reportTestCases) std::cout << "    FAIL: tgamma(0.5) is inaccurate\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Log-gamma Function (lgamma) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Log-gamma Function (lgamma)...\n";
		{
			efloat<4> four(4.0);
			double expected_lgamma4 = std::lgamma(4.0); // should be ln(6) approx. 1.79175...
			if (!IsClose(lgamma(four), expected_lgamma4)) {
				if (reportTestCases) std::cout << "    FAIL: lgamma(4.0) is inaccurate. Result: " << double(lgamma(four)) << "\n";
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

	std::string test_suite          = "efloat mathematical error and gamma library";
	std::string test_tag            = "error_and_gamma";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatErrorAndGamma(reportTestCases), "efloat", "error_and_gamma manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatErrorAndGamma(reportTestCases), "efloat", "error_and_gamma foundational");
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
