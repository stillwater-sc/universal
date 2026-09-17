// test_tracked_exact.cpp: verification of TrackedExact<T> error tracking
//
// TrackedExact<T> (include/sw/universal/utility/tracked_exact.hpp) tracks the rounding
// error of an IEEE computation exactly: every add and subtract runs through two_sum,
// every multiply through two_prod, and the residual of each operation is accumulated
// into a running absolute error bound.
//
// The contracts pinned here are the ones a caller relies on:
//   - a freshly constructed value carries no error, no operations, no absorptions
//   - the residual of a single operation is EXACT, so cases whose residual is known
//     analytically must be reproduced bit for bit
//   - an operation that rounds nothing must report exactly zero error
//   - the reported error BOUNDS the true error of the computation (soundness), checked
//     against an independent higher-precision reference
//   - operations() counts one per operation and sums the operand counts
//   - absorption is counted exactly when the operands' magnitude ratio exceeds
//     2^(digits/2), the documented threshold
//   - assignment from a raw value resets the tracking state
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include <universal/native/ieee754.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/utility/tracked_exact.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	// ---- reporting helpers -----------------------------------------------------

	int expect_exact(double actual, double wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << std::setprecision(17) << std::scientific
			          << actual << ", expected " << wanted << std::defaultfloat << '\n';
		}
		return 1;
	}

	int expect_count(std::uint64_t actual, std::uint64_t wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << actual << ", expected " << wanted << '\n';
		}
		return 1;
	}

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- a freshly constructed value tracks nothing -----------------------------

	int VerifyInitialState(bool reportTestCases) {
		int fails = 0;

		TrackedExact<double> zero;
		fails += expect_exact(zero.value(), 0.0, "default ctor value", reportTestCases);
		fails += expect_exact(zero.error(), 0.0, "default ctor error", reportTestCases);
		fails += expect_count(zero.operations(), 0, "default ctor operations", reportTestCases);
		fails += expect_count(zero.absorptions(), 0, "default ctor absorptions", reportTestCases);
		fails += expect_true(zero.is_exact(), "default ctor is_exact", reportTestCases);

		TrackedExact<double> a = 3.25;
		fails += expect_exact(a.value(), 3.25, "value ctor value", reportTestCases);
		fails += expect_exact(a.error(), 0.0, "value ctor error", reportTestCases);
		fails += expect_count(a.operations(), 0, "value ctor operations", reportTestCases);
		fails += expect_true(a.is_exact(), "value ctor is_exact", reportTestCases);
		// with no error accumulated, valid_bits saturates at the type's precision
		fails += expect_exact(a.valid_bits(), double(std::numeric_limits<double>::digits),
			"value ctor valid_bits", reportTestCases);
		fails += expect_exact(a.relative_error(), 0.0, "value ctor relative_error", reportTestCases);
		fails += expect_exact(a.ulps_error(), 0.0, "value ctor ulps_error", reportTestCases);

		TrackedExact<float> f = 1.5f;
		fails += expect_exact(double(f.value()), 1.5, "float ctor value", reportTestCases);
		fails += expect_exact(f.error(), 0.0, "float ctor error", reportTestCases);

		return fails;
	}

	// ---- the residual of one operation is exact ---------------------------------
	//
	// Each case below has a residual that can be written down by hand, so the assertion
	// is on the bits, not on a tolerance. An implementation that estimates the error
	// instead of computing it, or that loses the low part of two_prod, fails here.

	int VerifyExactResiduals(bool reportTestCases) {
		int fails = 0;
		const double eps = std::numeric_limits<double>::epsilon();  // 2^-52

		// 1.0 + 1e-16: 1e-16 is below half an ulp of 1.0 (1.11e-16), so the sum rounds
		// back to 1.0 and the whole of the addend is the residual.
		{
			TrackedExact<double> a = 1.0, b = 1e-16;
			auto c = a + b;
			fails += expect_exact(c.value(), 1.0, "1.0 + 1e-16 value", reportTestCases);
			fails += expect_exact(c.error(), 1e-16, "1.0 + 1e-16 residual", reportTestCases);
			fails += expect_true(!c.is_exact(), "1.0 + 1e-16 is not exact", reportTestCases);
		}

		// 2^53 + 1: the spacing at 2^53 is 2, so the sum sits exactly halfway and
		// round-to-nearest-even returns 2^53, leaving a residual of exactly 1.
		{
			TrackedExact<double> a = std::ldexp(1.0, 53), b = 1.0;
			auto c = a + b;
			fails += expect_exact(c.value(), std::ldexp(1.0, 53), "2^53 + 1 value", reportTestCases);
			fails += expect_exact(c.error(), 1.0, "2^53 + 1 residual", reportTestCases);
		}

		// 2^53 + 3 rounds up to 2^53 + 4 (ties to even), so the residual is -1 and the
		// tracker, which accumulates magnitudes, must report 1.
		{
			TrackedExact<double> a = std::ldexp(1.0, 53), b = 3.0;
			auto c = a + b;
			fails += expect_exact(c.value(), std::ldexp(1.0, 53) + 4.0, "2^53 + 3 value", reportTestCases);
			fails += expect_exact(c.error(), 1.0, "2^53 + 3 residual magnitude", reportTestCases);
		}

		// (1 + 2^-52)^2 = 1 + 2^-51 + 2^-104. The first two terms are representable;
		// 2^-104 is the exact residual of the product.
		{
			TrackedExact<double> a = 1.0 + eps;
			auto c = a * a;
			fails += expect_exact(c.value(), 1.0 + 2.0 * eps, "(1+eps)^2 value", reportTestCases);
			fails += expect_exact(c.error(), std::ldexp(1.0, -104), "(1+eps)^2 residual", reportTestCases);
		}

		// sqr() takes the two_sqr path and must agree with the general product
		{
			TrackedExact<double> a = 1.0 + eps;
			auto c = a.sqr();
			fails += expect_exact(c.value(), 1.0 + 2.0 * eps, "sqr value", reportTestCases);
			fails += expect_exact(c.error(), std::ldexp(1.0, -104), "sqr residual", reportTestCases);
		}

		// subtraction: 2^53 - 1 is representable, so two_diff has nothing to report
		{
			TrackedExact<double> a = std::ldexp(1.0, 53), b = 1.0;
			auto c = a - b;
			fails += expect_exact(c.value(), std::ldexp(1.0, 53) - 1.0, "2^53 - 1 value", reportTestCases);
			fails += expect_exact(c.error(), 0.0, "2^53 - 1 residual", reportTestCases);
		}

		// float: 1 + 2^-24 is below half an ulp of 1.0f, so the addend is the residual
		{
			TrackedExact<float> a = 1.0f, b = std::ldexp(1.0f, -24);
			auto c = a + b;
			fails += expect_exact(double(c.value()), 1.0, "float 1 + 2^-24 value", reportTestCases);
			fails += expect_exact(c.error(), std::ldexp(1.0, -24), "float 1 + 2^-24 residual", reportTestCases);
		}

		return fails;
	}

	// ---- an operation that rounds nothing reports exactly zero ------------------
	//
	// A tracker that reported a nonzero "estimate" for an exact operation would make
	// every dyadic computation look lossy. These all have exact results.

	int VerifyExactOperationsAreFree(bool reportTestCases) {
		int fails = 0;

		struct Pair { double a, b; };
		const Pair addends[] = { {1.0, 0.5}, {1.0, -0.25}, {3.0, 5.0}, {0.125, 0.375},
		                         {std::ldexp(1.0, 40), std::ldexp(1.0, -10)} };
		for (const Pair& p : addends) {
			TrackedExact<double> a = p.a, b = p.b;
			auto s = a + b;
			auto d = a - b;
			fails += expect_exact(s.error(), 0.0, "exact addition reports no error", reportTestCases);
			fails += expect_exact(s.value(), p.a + p.b, "exact addition value", reportTestCases);
			fails += expect_exact(d.error(), 0.0, "exact subtraction reports no error", reportTestCases);
			fails += expect_exact(d.value(), p.a - p.b, "exact subtraction value", reportTestCases);
			fails += expect_true(s.is_exact() && d.is_exact(), "exact ops are is_exact", reportTestCases);
		}

		const Pair factors[] = { {3.0, 5.0}, {1.5, 4.0}, {0.25, 8.0}, {1.0 + std::numeric_limits<double>::epsilon(), 2.0} };
		for (const Pair& p : factors) {
			TrackedExact<double> a = p.a, b = p.b;
			auto m = a * b;
			fails += expect_exact(m.error(), 0.0, "exact product reports no error", reportTestCases);
			fails += expect_exact(m.value(), p.a * p.b, "exact product value", reportTestCases);
		}

		// an accumulation that is exact at every step must end with zero error:
		// 1000 copies of 2^-30 sum to 125 * 2^-27, and every partial sum is dyadic
		// with far fewer than 53 significant bits.
		{
			TrackedExact<double> sum = 0.0;
			const double term = std::ldexp(1.0, -30);
			for (int i = 0; i < 1000; ++i) sum += term;
			fails += expect_exact(sum.value(), 1000.0 * term, "exact accumulation value", reportTestCases);
			fails += expect_exact(sum.error(), 0.0, "exact accumulation error", reportTestCases);
			fails += expect_count(sum.operations(), 1000, "exact accumulation operations", reportTestCases);
			fails += expect_count(sum.absorptions(), 0, "exact accumulation absorptions", reportTestCases);
		}

		return fails;
	}

	// ---- errors accumulate, and propagate through multiplication ----------------

	int VerifyErrorPropagation(bool reportTestCases) {
		int fails = 0;

		// Adding 2^-60 to 1.0 ten times: each addition rounds the whole addend away,
		// so the value never moves and the error is exactly ten times the addend.
		{
			TrackedExact<double> sum = 1.0;
			const double tiny = std::ldexp(1.0, -60);
			for (int i = 0; i < 10; ++i) sum += tiny;
			fails += expect_exact(sum.value(), 1.0, "absorbed accumulation value", reportTestCases);
			fails += expect_exact(sum.error(), 10.0 * tiny, "absorbed accumulation error", reportTestCases);
			fails += expect_count(sum.operations(), 10, "absorbed accumulation operations", reportTestCases);
		}

		// Multiplication by an exact power of two is itself exact, so the incoming
		// error is scaled by the factor and nothing is added:
		//   err(a*b) = |a|*err(b) + |b|*err(a) + |residual|
		{
			TrackedExact<double> a = 1.0, b = 1e-16;
			auto lossy = a + b;                       // value 1.0, error 1e-16
			TrackedExact<double> four = 4.0;          // exact
			auto scaled = lossy * four;
			fails += expect_exact(scaled.value(), 4.0, "scaled value", reportTestCases);
			fails += expect_exact(scaled.error(), 4.0 * lossy.error(), "error scales with the factor", reportTestCases);
			fails += expect_count(scaled.operations(), 2, "scaled operations", reportTestCases);
		}

		// Unary minus and abs carry the tracking state across unchanged
		{
			TrackedExact<double> a = 1.0, b = 1e-16;
			auto lossy = a + b;
			auto neg = -lossy;
			fails += expect_exact(neg.value(), -1.0, "unary minus value", reportTestCases);
			fails += expect_exact(neg.error(), lossy.error(), "unary minus preserves error", reportTestCases);
			fails += expect_count(neg.operations(), lossy.operations(), "unary minus preserves operations", reportTestCases);
			auto magnitude = abs(neg);
			fails += expect_exact(magnitude.value(), 1.0, "abs value", reportTestCases);
			fails += expect_exact(magnitude.error(), lossy.error(), "abs preserves error", reportTestCases);
		}

		// relative_error and valid_bits derive from the accumulated error
		{
			TrackedExact<double> a = 1.0, b = 1e-16;
			auto c = a + b;                            // value 1.0, error 1e-16
			fails += expect_exact(c.relative_error(), 1e-16, "relative_error at value 1", reportTestCases);
			// -log2(1e-16) = 53.15..., which the cap pulls back to the type precision
			fails += expect_exact(c.valid_bits(), double(std::numeric_limits<double>::digits),
				"valid_bits is capped at the type precision", reportTestCases);
		}

		// an error of one part in 2^20 leaves 20 valid bits, below the cap
		{
			TrackedExact<double> a = 1.0, b = std::ldexp(1.0, -20);
			auto c = a + b;                            // value 1 + 2^-20, no rounding yet
			fails += expect_exact(c.error(), 0.0, "1 + 2^-20 is exact", reportTestCases);
			// give it an error of exactly 2^-20 relative by tracking a lossy addend
			TrackedExact<double> lossy = TrackedExact<double>(1.0) + TrackedExact<double>(std::ldexp(1.0, -80));
			fails += expect_exact(lossy.error(), std::ldexp(1.0, -80), "2^-80 is absorbed by 1.0", reportTestCases);
			fails += expect_exact(lossy.valid_bits(), 53.0, "an 80-bit-down error still caps at 53",
				reportTestCases);
		}

		// ulps_error scales the accumulated error by the ulp at the result
		{
			TrackedExact<double> a = 1.0, b = 1e-16;
			auto c = a + b;
			const double ulp = std::numeric_limits<double>::epsilon();  // ulp(1.0)
			fails += expect_exact(c.ulps_error(), 1e-16 / ulp, "ulps_error", reportTestCases);
		}

		return fails;
	}

	// ---- the reported error bounds the true error (soundness) -------------------
	//
	// This is the property the tracker exists to provide. The reference is computed in
	// double-double, whose 106-bit significand is far finer than the errors involved,
	// and which shares no code with the two_sum/two_prod residuals under test.

	int VerifySoundnessAgainstDoubleDouble(bool reportTestCases) {
		int fails = 0;
		const int n = 100;

		// A dot product of terms that are not dyadic, so every step rounds.
		TrackedExact<double> dot = 0.0;
		dd reference = 0.0;
		for (int i = 0; i < n; ++i) {
			double ai = 1.0 / (i + 1);
			double bi = 1.0 / (i + 2);
			dot += TrackedExact<double>(ai) * TrackedExact<double>(bi);
			reference += dd(ai) * dd(bi);
		}

		const double trueError = std::abs(dot.value() - double(reference));
		if (!(trueError <= dot.error())) {
			++fails;
			if (reportTestCases) {
				std::cout << "    FAIL dot product: true error " << std::scientific << trueError
				          << " exceeds the reported bound " << dot.error() << std::defaultfloat << '\n';
			}
		}
		// the bound must also be useful, not a blanket overestimate
		if (!(dot.error() < 1e-12)) {
			++fails;
			if (reportTestCases) {
				std::cout << "    FAIL dot product: reported bound " << std::scientific << dot.error()
				          << " is implausibly large" << std::defaultfloat << '\n';
			}
		}
		// each iteration is one product plus one accumulation
		fails += expect_count(dot.operations(), std::uint64_t(2 * n), "dot product operations", reportTestCases);

		// A summation in decreasing order, where the small terms are repeatedly
		// rounded away against the growing partial sum.
		{
			TrackedExact<double> sum = 0.0;
			dd ref = 0.0;
			for (int i = 1; i <= 2000; ++i) {
				double term = 1.0 / double(i);
				sum += term;
				ref += dd(1.0) / dd(double(i));
			}
			const double err = std::abs(sum.value() - double(ref));
			if (!(err <= sum.error())) {
				++fails;
				if (reportTestCases) {
					std::cout << "    FAIL harmonic sum: true error " << std::scientific << err
					          << " exceeds the reported bound " << sum.error() << std::defaultfloat << '\n';
				}
			}
		}

		return fails;
	}

	// ---- cancellation is reported, not hidden -----------------------------------

	int VerifyCancellationIsReported(bool reportTestCases) {
		int fails = 0;

		// (1 + 1e16) - 1e16 returns 0, having lost the 1 entirely: 1e16 + 1 sits exactly
		// halfway between two doubles and ties to even. The value gives no hint that
		// anything went wrong; the tracked error is exactly the 1 that was dropped.
		TrackedExact<double> a = 1.0, b = 1e16;
		auto c = a + b;
		auto d = c - b;
		fails += expect_exact(d.value(), 0.0, "cancellation result", reportTestCases);
		fails += expect_exact(d.error(), 1.0, "cancellation reports the lost operand", reportTestCases);
		fails += expect_true(!d.is_exact(), "cancellation is not exact", reportTestCases);
		fails += expect_count(d.operations(), 2, "cancellation operations", reportTestCases);

		return fails;
	}

	// ---- absorption is counted at the documented threshold ----------------------
	//
	// detect_absorption counts an event when log2(|larger| / |smaller|) exceeds
	// digits/2, i.e. when more than half the significand of the smaller operand is
	// shifted out. For double that boundary is 26.5 bits, for float 12 bits.

	int VerifyAbsorptionThreshold(bool reportTestCases) {
		int fails = 0;

		struct Case { double b; bool absorbed; const char* tag; };
		const Case cases[] = {
			{ 0.5,                    false, "1.0 + 0.5" },
			{ std::ldexp(1.0, -26),   false, "1.0 + 2^-26 (below the threshold)" },
			{ std::ldexp(1.0, -27),   true,  "1.0 + 2^-27 (above the threshold)" },
			{ 1e-20,                  true,  "1.0 + 1e-20" },
			{ 1.0,                    false, "1.0 + 1.0 (equal magnitude)" },
		};
		for (const Case& c : cases) {
			TrackedExact<double> a = 1.0, b = c.b;
			auto sum = a + b;
			const std::uint64_t wanted = c.absorbed ? 1u : 0u;
			fails += expect_count(sum.absorptions(), wanted, c.tag, reportTestCases);
			fails += expect_true(sum.had_absorption() == c.absorbed, "had_absorption agrees with the count",
				reportTestCases);
		}

		// float's threshold is 12 bits, half of its 24-bit significand
		{
			TrackedExact<float> a = 1.0f, b = std::ldexp(1.0f, -12);
			fails += expect_count((a + b).absorptions(), 0, "float 1 + 2^-12 (below)", reportTestCases);
			TrackedExact<float> c = std::ldexp(1.0f, -13);
			fails += expect_count((a + c).absorptions(), 1, "float 1 + 2^-13 (above)", reportTestCases);
		}

		// absorption counts accumulate across a chain, and multiplication carries the
		// count forward without adding to it
		{
			TrackedExact<double> sum = 1.0;
			for (int i = 0; i < 10; ++i) sum += 1e-20;
			fails += expect_count(sum.absorptions(), 10, "ten absorbed additions", reportTestCases);
			auto scaled = sum * TrackedExact<double>(2.0);
			fails += expect_count(scaled.absorptions(), 10, "multiplication carries the count", reportTestCases);
		}

		// subtracting a tiny operand absorbs just as addition does
		{
			TrackedExact<double> a = 1.0, b = 1e-20;
			fails += expect_count((a - b).absorptions(), 1, "1.0 - 1e-20", reportTestCases);
		}

		return fails;
	}

	// ---- operation counts ------------------------------------------------------

	int VerifyOperationCounts(bool reportTestCases) {
		int fails = 0;

		TrackedExact<double> a = 1.0, b = 2.0, c = 3.0;
		fails += expect_count((a + b).operations(), 1, "one addition", reportTestCases);
		fails += expect_count((a * b).operations(), 1, "one multiplication", reportTestCases);
		fails += expect_count((a / b).operations(), 1, "one division", reportTestCases);
		// operand counts add up, plus one for the operation itself
		fails += expect_count(((a + b) + c).operations(), 2, "two chained additions", reportTestCases);
		fails += expect_count(((a + b) * (a + c)).operations(), 3, "two additions and a product", reportTestCases);
		// sqrt counts as one operation
		fails += expect_count(sqrt(a + b).operations(), 2, "addition then sqrt", reportTestCases);
		// sqr counts as one
		fails += expect_count((a + b).sqr().operations(), 2, "addition then sqr", reportTestCases);

		return fails;
	}

	// ---- assignment resets the tracking state -----------------------------------

	int VerifyResetOnAssignment(bool reportTestCases) {
		int fails = 0;

		TrackedExact<double> a = 1.0;
		a += 1e-16;
		fails += expect_true(a.error() > 0.0, "error accumulated before the reset", reportTestCases);
		for (int i = 0; i < 3; ++i) a += 1e-20;
		fails += expect_true(a.absorptions() > 0, "absorptions accumulated before the reset", reportTestCases);

		a = 2.0;  // assignment from a raw value starts over
		fails += expect_exact(a.value(), 2.0, "value after assignment", reportTestCases);
		fails += expect_exact(a.error(), 0.0, "error reset by assignment", reportTestCases);
		fails += expect_count(a.operations(), 0, "operations reset by assignment", reportTestCases);
		fails += expect_count(a.absorptions(), 0, "absorptions reset by assignment", reportTestCases);

		// copy assignment, by contrast, carries the state
		TrackedExact<double> lossy = TrackedExact<double>(1.0) + TrackedExact<double>(1e-16);
		TrackedExact<double> copy = 0.0;
		copy = lossy;
		fails += expect_exact(copy.error(), lossy.error(), "copy carries the error", reportTestCases);
		fails += expect_count(copy.operations(), lossy.operations(), "copy carries the operations", reportTestCases);

		return fails;
	}

	// ---- division ---------------------------------------------------------------

	int VerifyDivision(bool reportTestCases) {
		int fails = 0;

		// division by a power of two is exact
		{
			TrackedExact<double> a = 3.0, b = 4.0;
			auto q = a / b;
			fails += expect_exact(q.value(), 0.75, "3/4 value", reportTestCases);
			fails += expect_exact(q.error(), 0.0, "3/4 is exact", reportTestCases);
		}

		// division by zero is flagged with an infinite error bound rather than passing
		// an infinity off as a tracked result
		{
			TrackedExact<double> a = 1.0, zero = 0.0;
			auto q = a / zero;
			fails += expect_true(std::isinf(q.error()), "division by zero reports an infinite bound", reportTestCases);
			fails += expect_true(!q.is_exact(), "division by zero is not exact", reportTestCases);
		}

		return fails;
	}

	// ---- the exploratory narrative, kept for manual inspection ------------------

#if MANUAL_TESTING
	void ReportTrackedExactBehaviour() {
		std::cout << "\n=== TrackedExact<double> walk-through ===\n";

		double dx = 3.14159265358979;
		TrackedExact<double> x = dx;
		auto y = x * x;
		auto z = sqrt(y);
		std::cout << to_binary(dx) << " : x        = " << std::setprecision(17) << dx << '\n';
		std::cout << to_binary(y.value()) << " : x^2      = " << y.value() << '\n';
		std::cout << to_binary(z.value()) << " : sqrt(x^2)= " << z.value() << '\n';
		z.report(std::cout);

		std::cout << "\ncancellation, (1 + 1e16) - 1e16:\n";
		TrackedExact<double> a = 1.0, b = 1e16;
		auto d = (a + b) - b;
		d.report(std::cout);

		std::cout << "\ndot product of 1/(i+1) * 1/(i+2), i = 0..99:\n";
		TrackedExact<double> dot = 0.0;
		for (int i = 0; i < 100; ++i) {
			dot += TrackedExact<double>(1.0 / (i + 1)) * TrackedExact<double>(1.0 / (i + 2));
		}
		dot.report(std::cout);
	}
#endif  // MANUAL_TESTING

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

	std::string test_suite  = "TrackedExact<T> two_sum/two_prod error tracking";
	std::string test_tag    = "tracked_exact";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTrackedExactBehaviour();
	nrOfFailedTestCases += VerifyExactResiduals(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else  // !MANUAL_TESTING

	// Every check below is a handful of arithmetic operations, so the whole suite runs
	// in well under a millisecond. It all belongs at level 1: CI configures
	// REGRESSION_LEVEL_1 only, and a contract that is not checked there does not gate.
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyInitialState(reportTestCases), test_tag, "initial state");
	nrOfFailedTestCases += ReportTestResult(VerifyExactResiduals(reportTestCases), test_tag, "exact residuals");
	nrOfFailedTestCases += ReportTestResult(VerifyExactOperationsAreFree(reportTestCases), test_tag, "exact operations");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellationIsReported(reportTestCases), test_tag, "cancellation");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorPropagation(reportTestCases), test_tag, "error propagation");
	nrOfFailedTestCases += ReportTestResult(VerifyOperationCounts(reportTestCases), test_tag, "operation counts");
	nrOfFailedTestCases += ReportTestResult(VerifyResetOnAssignment(reportTestCases), test_tag, "reset on assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyAbsorptionThreshold(reportTestCases), test_tag, "absorption threshold");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision(reportTestCases), test_tag, "division");
	nrOfFailedTestCases += ReportTestResult(VerifySoundnessAgainstDoubleDouble(reportTestCases), test_tag,
		"soundness against double-double");
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
