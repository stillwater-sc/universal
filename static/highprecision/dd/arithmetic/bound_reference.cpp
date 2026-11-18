// bound_reference.cpp: the exact bound reference of dd, qd and the cascades (#2124)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>

/*
   dd, qd and the cascades are not correctly rounded; they are guaranteed to an error bound. Their
   random tests compared against a double reference, which holds 53 of their 106 to 212 bits, and
   their random operands came from setbits(uint64_t): integers, with every lower limb zero. The
   exact bound reference (multidouble_bound_oracle.hpp) draws normalized random values and checks
   each result against the operator's bound, exactly. This test checks

   - the defaults: each of the five types picks the exact bound reference, and double is refused;
   - that the check discriminates: a result moved by twice the bound in its lowest limbs fails,
     one moved by a quarter of the bound passes, for + - * / and sqrt;
   - the special cases: an exact zero, results below min(), division by zero, sqrt of a negative;
   - the random values: every limb below half an ulp of the one above it, and cancellation;
   - every type and operator through the verifiers, with their default reference.
*/

namespace sw { namespace universal {

static_assert(std::is_same_v<default_reference_t<dd>, exact_bound_reference>);
static_assert(std::is_same_v<default_reference_t<dd_cascade>, exact_bound_reference>);
static_assert(std::is_same_v<default_reference_t<td_cascade>, exact_bound_reference>);
static_assert(std::is_same_v<default_reference_t<qd>, exact_bound_reference>);
static_assert(std::is_same_v<default_reference_t<qd_cascade>, exact_bound_reference>);
static_assert(!valid_reference_v<dd, double>);
static_assert(!valid_reference_v<qd, double>);
static_assert(!valid_reference_v<dd, exact_reference>);
static_assert(valid_reference_v<dd, exact_bound_reference>);
static_assert(!has_exact_bound<double>);

namespace bound_reference {

using multidouble_oracle::fromLimbs;

// v with t * |v[0]| added to its lowest nonzero limb's position: an error of about t relative
template<typename T>
T perturbed(const T& v, double t) {
	constexpr unsigned L = multidouble_limbs<T>;
	std::array<double, L> x{};
	for (unsigned i = 0; i < L; ++i) x[i] = double(v[i]);
	x[L - 1] += t * std::fabs(x[0]);
	return fromLimbs<T>(x);
}

template<typename T>
T apply(char op, const T& a, const T& b) {
	switch (op) {
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	default:  return a / b;
	}
}

// the check passes the type's own result and a result a quarter of the bound off, and fails a
// result twice the bound off
template<typename T>
int VerifyDiscrimination(int trials, bool reportTestCases) {
	using B = exact_bound<T>;
	std::mt19937_64 eng(0x2124u + multidouble_limbs<T>);
	int fails = 0;
	auto check = [&](const char* what, bound_verdict got, bound_verdict expected, double err) {
		if (got != expected) {
			++fails;
			if (reportTestCases && fails < 8) std::cerr << "FAIL " << what << ": error " << err << " epsilon\n";
		}
	};
	for (int t = 0; t < trials; ++t) {
		T a, b;
		B::random(a, eng);
		B::random(b, eng);
		for (char op : { '+', '-', '*', '/' }) {
			const T r = apply(op, a, b);
			double err{ 0 };
			const bound_verdict own = B::binary(op, a, b, r, err);
			if (own == bound_verdict::unchecked) continue;
			check("the type's own result", own, bound_verdict::pass, err);
			// the result's scale: |r| relative to |a op b| is ~1, so moving r by k * eps * |r| is an error of ~k eps
			const double far = 2.0 * B::bound(op) * B::epsilon(), near = 0.25 * B::bound(op) * B::epsilon();
			check("twice the bound off", B::binary(op, a, b, perturbed(r, far), err), bound_verdict::fail, err);
			check("a quarter of the bound off", B::binary(op, a, b, perturbed(r, near), err), bound_verdict::pass, err);
		}
		T aa = (a < T(0)) ? -a : a;
		const T root = sqrt(aa);
		double err{ 0 };
		check("sqrt", B::sqrt(aa, root, err), bound_verdict::pass, err);
		check("sqrt twice the bound off", B::sqrt(aa, perturbed(root, 2.0 * B::bound('r') * B::epsilon()), err), bound_verdict::fail, err);
		check("sqrt a quarter of the bound off", B::sqrt(aa, perturbed(root, 0.25 * B::bound('r') * B::epsilon()), err), bound_verdict::pass, err);
	}
	return fails;
}

template<typename T>
int VerifySpecialCases(bool reportTestCases) {
	using B = exact_bound<T>;
	int fails = 0;
	auto expect = [&](const char* what, bound_verdict got, bound_verdict expected) {
		if (got != expected) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL " << what << '\n';
		}
	};
	double err{ 0 };
	const T one(1.0), two(2.0), zero(0.0), tiny(0x1p-600);
	expect("1 - 1 = 0 passes", B::binary('-', one, one, one - one, err), bound_verdict::pass);
	expect("1 - 1 = 1 fails", B::binary('-', one, one, one, err), bound_verdict::fail);
	expect("0 / 2 = 0 passes", B::binary('/', zero, two, zero, err), bound_verdict::pass);
	expect("1 / 0 is not checked", B::binary('/', one, zero, one, err), bound_verdict::unchecked);
	expect("2^-600 * 2^-600, below min(), is not checked", B::binary('*', tiny, tiny, zero, err), bound_verdict::unchecked);
	expect("1 * 2^-600, in range, passes", B::binary('*', one, tiny, tiny, err), bound_verdict::pass);
	expect("'^' is not checked", B::binary('^', one, one, one, err), bound_verdict::unchecked);
	expect("sqrt(-1) is not checked", B::sqrt(-one, one, err), bound_verdict::unchecked);
	expect("sqrt(4) = 2 passes", B::sqrt(T(4.0), two, err), bound_verdict::pass);
	expect("sqrt(4) = -2 fails", B::sqrt(T(4.0), -two, err), bound_verdict::fail);
	expect("sqrt(0) = 0 passes", B::sqrt(zero, zero, err), bound_verdict::pass);
	return fails;
}

// every limb of a random value is below half an ulp of the one above it, and a near negation
// cancels all but the last limb's scale
template<typename T>
int VerifyRandomValues(int trials, bool reportTestCases) {
	using B = exact_bound<T>;
	constexpr unsigned L = multidouble_limbs<T>;
	std::mt19937_64 eng(0x2124u);
	int fails = 0, nonzeroLowest = 0;
	for (int t = 0; t < trials; ++t) {
		T a, b;
		B::random(a, eng);
		for (unsigned i = 1; i < L; ++i) {
			const double hi = double(a[i - 1]), lo = double(a[i]);
			if (lo == 0.0) continue;
			if (hi + lo != hi) {   // |lo| below half an ulp of hi: adding it rounds back to hi
				++fails;
				if (reportTestCases && fails < 4) std::cerr << "FAIL limb " << i << " overlaps the one above it\n";
			}
		}
		if (double(a[L - 1]) != 0.0) ++nonzeroLowest;
		B::randomNearNegation(a, b, eng);
		const T s = a + b;
		if (!(std::fabs(double(s[0])) < std::ldexp(std::fabs(double(a[0])), -40))) {
			++fails;
			if (reportTestCases && fails < 4) std::cerr << "FAIL a near negation did not cancel\n";
		}
	}
	if (nonzeroLowest < trials / 2) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL the lowest limb is mostly zero: " << nonzeroLowest << " of " << trials << '\n';
	}
	return fails;
}

template<typename T>
int VerifyThroughVerifiers(size_t n) {
	int fails = 0;
	for (RandomsOp op : { RandomsOp::OPCODE_ADD, RandomsOp::OPCODE_SUB, RandomsOp::OPCODE_MUL, RandomsOp::OPCODE_DIV,
	                      RandomsOp::OPCODE_IPA, RandomsOp::OPCODE_IPD }) {
		fails += VerifyBinaryOperatorThroughRandoms<T>(true, op, n);
	}
	fails += VerifyUnaryOperatorThroughRandoms<T>(true, RandomsOp::OPCODE_SQRT, n);
	return fails;
}

}  // namespace bound_reference
}}  // namespace sw::universal

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
	using namespace sw::universal::bound_reference;

	std::string test_suite  = "multi-component exact bound reference";
	std::string test_tag    = "bound reference";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyDiscrimination<dd>(100, reportTestCases), "dd", "discrimination");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDiscrimination<dd>(2000, reportTestCases), "dd", "discrimination");
	nrOfFailedTestCases += ReportTestResult(VerifyDiscrimination<dd_cascade>(1000, reportTestCases), "dd_cascade", "discrimination");
	nrOfFailedTestCases += ReportTestResult(VerifyDiscrimination<td_cascade>(500, reportTestCases), "td_cascade", "discrimination");
	nrOfFailedTestCases += ReportTestResult(VerifyDiscrimination<qd>(500, reportTestCases), "qd", "discrimination");
	nrOfFailedTestCases += ReportTestResult(VerifyDiscrimination<qd_cascade>(500, reportTestCases), "qd_cascade", "discrimination");

	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<dd>(reportTestCases), "dd", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<qd>(reportTestCases), "qd", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<td_cascade>(reportTestCases), "td_cascade", "special cases");

	nrOfFailedTestCases += ReportTestResult(VerifyRandomValues<dd>(2000, reportTestCases), "dd", "random values");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomValues<qd>(2000, reportTestCases), "qd", "random values");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomValues<td_cascade>(2000, reportTestCases), "td_cascade", "random values");

	nrOfFailedTestCases += ReportTestResult(VerifyThroughVerifiers<dd>(1000), "dd", "verifiers");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughVerifiers<dd_cascade>(1000), "dd_cascade", "verifiers");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughVerifiers<td_cascade>(500), "td_cascade", "verifiers");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughVerifiers<qd>(500), "qd", "verifiers");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughVerifiers<qd_cascade>(500), "qd_cascade", "verifiers");

	// an operator the bound reference does not cover is cancelled, which counts as one failure
	if (VerifyBinaryOperatorThroughRandoms<dd>(false, RandomsOp::OPCODE_POW, 10) != 1) ++nrOfFailedTestCases;
	if (VerifyUnaryOperatorThroughRandoms<dd>(false, RandomsOp::OPCODE_EXP, 10) != 1) ++nrOfFailedTestCases;
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
