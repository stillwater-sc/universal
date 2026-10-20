// signed_zero.cpp: the sign of zero, infinite and finite results of efloat's + - * / against IEEE-754 (#1768)
//
// Every combination of +-0, +-3 and +-inf through + - * /, the compound forms, an integer operand
// on either side, unary minus and x - x, in all four rounding modes. The reference is the same
// expression in double, evaluated in the same rounding mode: the operands are small integers,
// zeros and infinities, so every double result is exact, and only the sign of an exact zero sum
// depends on the mode (IEEE-754 6.3: +0, or -0 rounding toward -inf). Each efloat result must
// have the double result's value and sign, or be NaN where it is NaN.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfenv>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using Real = efloat<4>;

struct Mode { RoundingMode efloat_mode; int fenv_mode; const char* name; };
const Mode modes[] = {
	{ RoundingMode::RoundToNearest,      FE_TONEAREST,  "nearest" },
	{ RoundingMode::RoundToZero,         FE_TOWARDZERO, "toward zero" },
	{ RoundingMode::RoundTowardPositive, FE_UPWARD,     "toward +inf" },
	{ RoundingMode::RoundTowardNegative, FE_DOWNWARD,   "toward -inf" },
};

// does the efloat result match the double one: both NaN, or the same value and sign
bool matches(const Real& got, double expected) {
	if (std::isnan(expected)) return got.isnan();
	if (got.isnan()) return false;
	if (std::isinf(expected)) return got.isinf() && ((got.sign() == -1) == std::signbit(expected));
	if (expected == 0.0) return got.iszero() && ((got.sign() == -1) == std::signbit(expected));
	return !got.iszero() && !got.isinf() && double(got) == expected;
}

std::string show(double v) {
	if (std::isnan(v)) return "nan";
	if (std::isinf(v)) return std::signbit(v) ? "-inf" : "+inf";
	if (v == 0.0) return std::signbit(v) ? "-0" : "+0";
	return std::to_string(static_cast<int>(v));
}

std::string show(const Real& v) {
	if (v.isnan()) return "nan";
	if (v.isinf()) return (v.sign() == -1) ? "-inf" : "+inf";
	if (v.iszero()) return (v.sign() == -1) ? "-0" : "+0";
	return std::to_string(static_cast<int>(double(v)));
}

int check(bool reportTestCases, const char* what, double a, double b, const Real& got, double expected, const Mode& m) {
	if (matches(got, expected)) return 0;
	if (reportTestCases) {
		std::cerr << "FAIL: (" << show(a) << ") " << what << " (" << show(b) << ") rounded " << m.name
		          << ": " << show(got) << ", expected " << show(expected) << '\n';
	}
	return 1;
}

// the binary operators, their compound forms, and an integer operand on either side, with every
// combination of the operands below
int VerifyBinaryOperators(bool reportTestCases) {
	const double inf = std::numeric_limits<double>::infinity();
	const double values[] = { 0.0, -0.0, 3.0, -3.0, inf, -inf };
	int fails = 0;
	const RoundingMode saved = efloat_rounding_mode;
	for (const Mode& m : modes) {
		for (double av : values) {
			for (double bv : values) {
				// evaluated at run time in the mode set: the volatile stores keep the operations
				// between the two fesetround calls
				volatile double va = av, vb = bv, vs, vd, vp, vq;
				std::fesetround(m.fenv_mode);
				vs = va + vb;
				vd = va - vb;
				vp = va * vb;
				vq = va / vb;
				std::fesetround(FE_TONEAREST);
				const double sum = vs, difference = vd, product = vp, quotient = vq;

				efloat_rounding_mode = m.efloat_mode;
				const Real a(av), b(bv);
				Real s(a), d(a), p(a), q(a);
				s += b;
				d -= b;
				p *= b;
				q /= b;
				const Real sum_e = a + b, difference_e = a - b, product_e = a * b, quotient_e = a / b;
				efloat_rounding_mode = saved;

				fails += check(reportTestCases, "+", av, bv, sum_e, sum, m);
				fails += check(reportTestCases, "-", av, bv, difference_e, difference, m);
				fails += check(reportTestCases, "*", av, bv, product_e, product, m);
				fails += check(reportTestCases, "/", av, bv, quotient_e, quotient, m);
				fails += check(reportTestCases, "+=", av, bv, s, sum, m);
				fails += check(reportTestCases, "-=", av, bv, d, difference, m);
				fails += check(reportTestCases, "*=", av, bv, p, product, m);
				fails += check(reportTestCases, "/=", av, bv, q, quotient, m);

				// a finite b as an integer operand, on either side
				if (std::isfinite(bv) && !(bv == 0.0 && std::signbit(bv))) {   // an int has no -0
					const int k = static_cast<int>(bv);
					volatile double vks, vkd, vkp, vkq;
					std::fesetround(m.fenv_mode);
					vks = vb + va;
					vkd = vb - va;
					vkp = vb * va;
					vkq = vb / va;
					std::fesetround(FE_TONEAREST);
					const double ks = vks, kd = vkd, kp = vkp, kq = vkq;
					efloat_rounding_mode = m.efloat_mode;
					Real si(a), di(a), pi(a), qi(a);
					si += k;
					di -= k;
					pi *= k;
					qi /= k;
					const Real a_plus_k = a + k, a_minus_k = a - k, a_times_k = a * k, a_over_k = a / k;
					const Real k_plus_a = k + a, k_minus_a = k - a, k_times_a = k * a, k_over_a = k / a;
					efloat_rounding_mode = saved;
					fails += check(reportTestCases, "+ int", av, bv, a_plus_k, sum, m);
					fails += check(reportTestCases, "- int", av, bv, a_minus_k, difference, m);
					fails += check(reportTestCases, "* int", av, bv, a_times_k, product, m);
					fails += check(reportTestCases, "/ int", av, bv, a_over_k, quotient, m);
					fails += check(reportTestCases, "+= int", av, bv, si, sum, m);
					fails += check(reportTestCases, "-= int", av, bv, di, difference, m);
					fails += check(reportTestCases, "*= int", av, bv, pi, product, m);
					fails += check(reportTestCases, "/= int", av, bv, qi, quotient, m);
					fails += check(reportTestCases, "int +", bv, av, k_plus_a, ks, m);
					fails += check(reportTestCases, "int -", bv, av, k_minus_a, kd, m);
					fails += check(reportTestCases, "int *", bv, av, k_times_a, kp, m);
					fails += check(reportTestCases, "int /", bv, av, k_over_a, kq, m);
				}
			}
		}
	}
	efloat_rounding_mode = saved;
	return fails;
}

// unary minus negates zeros and infinities too, and x - x and x + (-x) are +0, or -0 rounding
// toward -inf, for finite x of any precision
int VerifyNegationAndCancellation(bool reportTestCases) {
	const double inf = std::numeric_limits<double>::infinity();
	int fails = 0;
	const RoundingMode saved = efloat_rounding_mode;
	for (double v : { 0.0, -0.0, 3.0, -3.0, inf, -inf }) {
		const Real x(v);
		volatile double vv = v;
		const double negated = -vv;
		fails += check(reportTestCases, "unary -", 0.0, v, -x, negated, modes[0]);
	}
	for (const Mode& m : modes) {
		for (double v : { 3.0, -3.0, 0.1, -1e300 }) {
			volatile double vv = v, vc, vn;
			std::fesetround(m.fenv_mode);
			vc = vv - vv;
			vn = vv + (-vv);
			std::fesetround(FE_TONEAREST);
			const double cancel = vc, cancel_neg = vn;
			efloat_rounding_mode = m.efloat_mode;
			Real x(1.0);
			x.set_precision(256);
			x = x / Real(7.0) * Real(v);   // a full 256-bit significand
			const Real got = x - x, got_neg = x + (-x);
			Real compound(x);
			compound -= x;
			efloat_rounding_mode = saved;
			fails += check(reportTestCases, "- itself", v, v, got, cancel, m);
			fails += check(reportTestCases, "+ its negation", v, -v, got_neg, cancel_neg, m);
			fails += check(reportTestCases, "-= itself", v, v, compound, cancel, m);
		}
	}
	efloat_rounding_mode = saved;
	return fails;
}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite          = "efloat signed zero";
	std::string test_tag            = "signed zero";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperators(reportTestCases), "efloat<4>", "+ - * / against double");
	nrOfFailedTestCases += ReportTestResult(VerifyNegationAndCancellation(reportTestCases), "efloat<4>", "negation and cancellation");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperators(reportTestCases), "efloat<4>", "+ - * / against double");
	nrOfFailedTestCases += ReportTestResult(VerifyNegationAndCancellation(reportTestCases), "efloat<4>", "negation and cancellation");
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
