// efloat_math_regressions.cpp: custom mathematical function validation for efloat.
//
// Categories tested:
//   - Tier 1: Bootstrap Square Root (sqrt)
//   - Tier 1: Natural Logarithm (log)
//   - Tier 1: Exponential (exp)
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

	std::string test_suite          = "efloat Tier-1 mathematical functions";
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
