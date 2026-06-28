// pow.cpp: regression tests for efloat mathematical power and exponential functions.
//
// Categories tested:
//   - integer_power, pow, exp2, exp10, expm1
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

	int VerifyEfloatPower(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. integer_power & pow(x, int_y) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying integer_power...\n";
		{
			efloat<4> base(2.0);
			if (pow(base, 3) != 8.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(2.0, 3) is not 8.0. Result: " << double(pow(base, 3)) << "\n";
				++failures;
			}
			if (pow(base, -2) != 0.25) {
				if (reportTestCases) std::cout << "    FAIL: pow(2.0, -2) is not 0.25. Result: " << double(pow(base, -2)) << "\n";
				++failures;
			}
			if (pow(base, 0) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(2.0, 0) is not 1.0\n";
				++failures;
			}
			// Verify INT_MIN safe negation (CodeRabbit feedback)
			efloat<4> neg_one(-1.0);
			int min_int = std::numeric_limits<int>::min();
			efloat<4> res_min_int = pow(neg_one, min_int);
			if (res_min_int != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(-1.0, INT_MIN) is not 1.0. Result: " << double(res_min_int) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. pow(x, y) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying pow(x, y)...\n";
		{
			// x^y positive base fractional exponent: pow(4.0, 0.5) == 2.0
			efloat<4> base(4.0);
			efloat<4> exp_val(0.5);
			double d_pow = double(pow(base, exp_val));
			if (std::abs(d_pow - 2.0) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: pow(4.0, 0.5) is not 2.0. Result: " << d_pow << "\n";
				++failures;
			}

			// Negative base integer exponent: pow(-2.0, 3.0) == -8.0
			efloat<4> neg_base(-2.0);
			efloat<4> odd_exp(3.0);
			double d_neg_pow = double(pow(neg_base, odd_exp));
			if (std::abs(d_neg_pow - (-8.0)) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: pow(-2.0, 3.0) is not -8.0. Result: " << d_neg_pow << "\n";
				++failures;
			}

			// Negative base fractional exponent (should yield NaN)
			efloat<4> frac_exp(1.5);
			efloat<4> res_frac = pow(neg_base, frac_exp);
			if (!res_frac.isnan()) {
				if (reportTestCases) std::cout << "    FAIL: pow(-2.0, 1.5) did not return NaN\n";
				++failures;
			}

			// Zero base negative exponent (Divide by Zero Exception, yields Inf)
			clear_efloat_exceptions();
			efloat<4> zero(0.0);
			efloat<4> neg_exp(-2.0);
			efloat<4> res_zero = pow(zero, neg_exp);
			if (!res_zero.isinf()) {
				if (reportTestCases) std::cout << "    FAIL: pow(0.0, -2.0) did not return Inf\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::DivisionByZero)) {
				if (reportTestCases) std::cout << "    FAIL: pow(0.0, -2.0) did not raise DivisionByZero exception\n";
				++failures;
			}

			// IEEE-754 Boundary cases (CodeRabbit feedback)
			efloat<4> nan; nan.setnan();
			efloat<4> one(1.0);
			if (pow(nan, zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(NaN, 0.0) is not 1.0\n";
				++failures;
			}
			if (pow(one, nan) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(1.0, NaN) is not 1.0\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. exp2 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying exp2...\n";
		{
			efloat<4> exponent(2.0);
			double d_exp2 = double(exp2(exponent));
			if (std::abs(d_exp2 - 4.0) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: exp2(2.0) is not 4.0. Result: " << d_exp2 << "\n";
				++failures;
			}
			// Zero input check
			efloat<4> zero(0.0);
			if (exp2(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp2(0.0) is not 1.0. Result: " << double(exp2(zero)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. exp10 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying exp10...\n";
		{
			efloat<4> exponent(2.0);
			double d_exp10 = double(exp10(exponent));
			if (std::abs(d_exp10 - 100.0) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: exp10(2.0) is not 100.0. Result: " << d_exp10 << "\n";
				++failures;
			}
			// Zero input check
			efloat<4> zero(0.0);
			if (exp10(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp10(0.0) is not 1.0. Result: " << double(exp10(zero)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 5. expm1 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying expm1...\n";
		{
			efloat<4> zero(0.0);
			if (expm1(zero) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: expm1(0.0) is not 0.0\n";
				++failures;
			}

			// Small input Taylor series check (cancellation prevention)
			// expm1(1e-12) ~= 1.0000000000005e-12
			efloat<4> small(1e-12);
			efloat<4> res_expm1 = expm1(small);
			double d_res = double(res_expm1);
			if (std::abs(d_res - 1e-12) > 1e-20) {
				if (reportTestCases) std::cout << "    FAIL: expm1(1e-12) was not accurate. Result: " << d_res << "\n";
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

	std::string test_suite          = "efloat mathematical power library";
	std::string test_tag            = "pow";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPower(reportTestCases), "efloat", "pow manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPower(reportTestCases), "efloat", "pow foundational");
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
