// exponent.cpp: regression tests for efloat mathematical exponential functions.
//
// Categories tested:
//   - exp, exp2, exp10, expm1
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

	int VerifyEfloatExponent(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Natural Exponential (exp) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Natural Exponential (exp)...\n";
		{
			clear_efloat_exceptions();

			// exp(0.0) == 1.0
			efloat<4> zero(0.0);
			if (exp(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp(0.0) is not 1.0\n";
				++failures;
			}

			// exp(1.0) == e
			efloat<8> one(1.0);
			double expected_e = std::exp(1.0);
			if (!IsClose(exp(one), expected_e)) {
				if (reportTestCases) std::cout << "    FAIL: exp(1.0) is inaccurate. Result: " << double(exp(one)) << "\n";
				++failures;
			}

			// Special values
			efloat<4> pos_inf; pos_inf.setinf(false);
			if (!exp(pos_inf).isinf() || exp(pos_inf).sign() != 1) {
				if (reportTestCases) std::cout << "    FAIL: exp(+inf) did not return +Inf\n";
				++failures;
			}

			efloat<4> neg_inf; neg_inf.setinf(true);
			if (exp(neg_inf) != 0.0 || exp(neg_inf).sign() != 1) {
				if (reportTestCases) std::cout << "    FAIL: exp(-inf) did not return +0.0\n";
				++failures;
			}

			// exp(nan) is nan
			efloat<4> nan; nan.setnan();
			if (!exp(nan).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: exp(nan) did not return NaN\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. Base-2 Exponential (exp2) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Base-2 Exponential (exp2)...\n";
		{
			efloat<4> zero(0.0);
			if (exp2(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp2(0.0) is not 1.0. Result: " << double(exp2(zero)) << "\n";
				++failures;
			}

			efloat<4> two(2.0);
			if (exp2(two) != 4.0) {
				if (reportTestCases) std::cout << "    FAIL: exp2(2.0) is not 4.0. Result: " << double(exp2(two)) << "\n";
				++failures;
			}

			efloat<4> neg_inf; neg_inf.setinf(true);
			if (exp2(neg_inf) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: exp2(-inf) is not 0.0\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Base-10 Exponential (exp10) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Base-10 Exponential (exp10)...\n";
		{
			efloat<4> zero(0.0);
			if (exp10(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp10(0.0) is not 1.0\n";
				++failures;
			}

			efloat<4> two(2.0);
			if (!IsClose(exp10(two), 100.0)) {
				if (reportTestCases) std::cout << "    FAIL: exp10(2.0) is not 100.0. Result: " << double(exp10(two)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. expm1 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying expm1...\n";
		{
			efloat<4> zero(0.0);
			if (expm1(zero) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: expm1(0.0) is not 0.0\n";
				++failures;
			}

			// Taylor series small-input protection
			efloat<4> small(1e-12);
			efloat<4> res_expm1 = expm1(small);
			double d_res = double(res_expm1);
			if (std::abs(d_res - 1e-12) > 1e-20) {
				if (reportTestCases) std::cout << "    FAIL: expm1(1e-12) was not accurate. Result: " << d_res << "\n";
				++failures;
			}

			efloat<4> neg_inf; neg_inf.setinf(true);
			if (expm1(neg_inf) != -1.0) {
				if (reportTestCases) std::cout << "    FAIL: expm1(-inf) is not -1.0. Result: " << double(expm1(neg_inf)) << "\n";
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

	std::string test_suite          = "efloat mathematical exponent library";
	std::string test_tag            = "exponent";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatExponent(reportTestCases), "efloat", "exponent manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatExponent(reportTestCases), "efloat", "exponent foundational");
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
