// efloat_math_regressions.cpp: custom mathematical function validation for efloat.
//
// Categories tested:
//   - Tier 1: Bootstrap Square Root (sqrt)
//   - Tier 1: Natural Logarithm (log)
//   - Tier 1: Exponential (exp)
//   - Logarithmic Suite: log2, log10, log1p (Issue #1108)
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

	// Custom assertion helper to verify values match up to a small tolerance
	template<unsigned nlimbs>
	bool IsClose(const sw::universal::efloat<nlimbs>& a, double target_val, double tolerance = 1e-15) {
		double diff = std::abs(double(a) - target_val);
		return diff <= tolerance;
	}

	int VerifyTier1MathFunctions(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Square Root (sqrt) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Square Root (sqrt)...\n";
		{
			clear_efloat_exceptions();

			// 1a. Identity values
			efloat<4> zero(0.0);
			if (sqrt(zero) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(0.0) is not 0.0\n";
				++failures;
			}
			efloat<4> one(1.0);
			if (sqrt(one) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(1.0) is not 1.0\n";
				++failures;
			}
			efloat<4> four(4.0);
			if (sqrt(four) != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(4.0) is not 2.0\n";
				++failures;
			}

			// 1b. Inexact value: sqrt(2.0)
			efloat<8> two(2.0);
			double expected_sqrt2 = std::sqrt(2.0);
			if (!IsClose(sqrt(two), expected_sqrt2)) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(2.0) is inaccurate. Result: " << double(sqrt(two)) << "\n";
				++failures;
			}

			// 1c. Exceptional value: sqrt(-1.0) -> NaN + InvalidOperation
			efloat<4> neg_one(-1.0);
			efloat<4> res = sqrt(neg_one);
			if (!res.isnan()) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(-1.0) did not return NaN\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(-1.0) did not set InvalidOperation\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. Exponential (exp) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Exponential (exp)...\n";
		{
			clear_efloat_exceptions();

			// 2a. exp(0.0) == 1.0
			efloat<4> zero(0.0);
			if (exp(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp(0.0) is not 1.0\n";
				++failures;
			}

			// 2b. exp(1.0) == e
			efloat<8> one(1.0);
			double expected_e = std::exp(1.0);
			if (!IsClose(exp(one), expected_e)) {
				if (reportTestCases) std::cout << "    FAIL: exp(1.0) is inaccurate. Result: " << double(exp(one)) << "\n";
				++failures;
			}

			// 2c. exp(ln2) == 2.0
			efloat<8> ln2;
			parse("0.693147180559945309417232121", ln2);
			if (!IsClose(exp(ln2), 2.0)) {
				if (reportTestCases) std::cout << "    FAIL: exp(ln2) is not 2.0. Result: " << double(exp(ln2)) << "\n";
				++failures;
			}

			// 2d. Exceptional / special values (CodeRabbit feedback)
			efloat<4> pos_inf;
			pos_inf.setinf(false);
			efloat<4> pos_inf_res = exp(pos_inf);
			if (!pos_inf_res.isinf() || pos_inf_res.sign() != 1) {
				if (reportTestCases) std::cout << "    FAIL: exp(+inf) did not return +Inf\n";
				++failures;
			}

			efloat<4> neg_inf;
			neg_inf.setinf(true);
			if (exp(neg_inf) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: exp(-inf) is not 0.0\n";
				++failures;
			}

			efloat<4> nan;
			nan.setnan();
			if (!exp(nan).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: exp(nan) did not return NaN\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Natural Logarithm (log) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Logarithm (log)...\n";
		{
			clear_efloat_exceptions();

			// 3a. log(1.0) == 0.0
			efloat<4> one(1.0);
			if (log(one) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: log(1.0) is not 0.0\n";
				++failures;
			}

			// 3b. log(2.0) == ln2
			efloat<8> two(2.0);
			double expected_ln2 = std::log(2.0);
			if (!IsClose(log(two), expected_ln2)) {
				if (reportTestCases) std::cout << "    FAIL: log(2.0) is inaccurate. Result: " << double(log(two)) << "\n";
				++failures;
			}

			// 3c. Exceptional value: log(0.0) -> -infinity + DivisionByZero
			efloat<4> zero(0.0);
			efloat<4> res = log(zero);
			if (!res.isinf() || res.sign() != -1) {
				if (reportTestCases) std::cout << "    FAIL: log(0.0) did not return -Inf\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::DivisionByZero)) {
				if (reportTestCases) std::cout << "    FAIL: log(0.0) did not set DivisionByZero\n";
				++failures;
			}

			// 3d. Exceptional value: log(-1.0) -> NaN + InvalidOperation
			efloat<4> neg_one(-1.0);
			efloat<4> res_nan = log(neg_one);
			if (!res_nan.isnan()) {
				if (reportTestCases) std::cout << "    FAIL: log(-1.0) did not return NaN\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: log(-1.0) did not set InvalidOperation\n";
				++failures;
			}

			// 3e. Exceptional value: log(-inf) -> NaN + InvalidOperation (signbit check)
			clear_efloat_exceptions();
			efloat<4> neg_inf;
			neg_inf.setinf(true);
			if (!log(neg_inf).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: log(-inf) did not return NaN\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: log(-inf) did not set InvalidOperation\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Logarithmic Suite Validation (log2, log10, log1p)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Logarithmic Suite (log2, log10, log1p)...\n";
		{
			clear_efloat_exceptions();

			// 4a. log2(4.0) == 2.0
			efloat<8> four(4.0);
			if (!IsClose(log2(four), 2.0)) {
				if (reportTestCases) std::cout << "    FAIL: log2(4.0) is not 2.0\n";
				++failures;
			}

			// 4b. log10(100.0) == 2.0
			efloat<8> hundred(100.0);
			if (!IsClose(log10(hundred), 2.0)) {
				if (reportTestCases) std::cout << "    FAIL: log10(100.0) is not 2.0\n";
				++failures;
			}

			// 4c. log1p(1e-12) avoids catastrophic cancellation as x -> 0
			efloat<8> small;
			parse("0.000000000001", small); // 10^-12
			double expected_log1p = std::log1p(1e-12);
			if (!IsClose(log1p(small), expected_log1p, 1e-25)) {
				if (reportTestCases) std::cout << "    FAIL: log1p(10^-12) is inaccurate. Result: " << double(log1p(small)) << "\n";
				++failures;
			}

			// 4d. log1p(-2.0) -> NaN + InvalidOperation
			efloat<4> neg_two(-2.0);
			if (!log1p(neg_two).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: log1p(-2.0) did not return NaN\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: log1p(-2.0) did not set InvalidOperation\n";
				++failures;
			}

			// 4e. log1p(-1.0) -> -Inf + DivisionByZero
			efloat<4> neg_one(-1.0);
			efloat<4> res_log1p = log1p(neg_one);
			if (!res_log1p.isinf() || res_log1p.sign() != -1) {
				if (reportTestCases) std::cout << "    FAIL: log1p(-1.0) did not return -Inf\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::DivisionByZero)) {
				if (reportTestCases) std::cout << "    FAIL: log1p(-1.0) did not set DivisionByZero\n";
				++failures;
			}

			// 4f. log1p(-inf) -> NaN + InvalidOperation (extended signbit domain check)
			clear_efloat_exceptions();
			efloat<4> neg_inf;
			neg_inf.setinf(true);
			if (!log1p(neg_inf).isnan()) {
				if (reportTestCases) std::cout << "    FAIL: log1p(-inf) did not return NaN\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: log1p(-inf) did not set InvalidOperation\n";
				++failures;
			}
		}

		clear_efloat_exceptions();
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

	std::string test_suite          = "efloat mathematical functions library";
	std::string test_tag            = "math";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyTier1MathFunctions(reportTestCases), "efloat", "math manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTier1MathFunctions(reportTestCases), "efloat", "math foundational");
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
