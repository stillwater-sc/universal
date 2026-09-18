// test_tracked_bounded.cpp: verification of TrackedBounded<T> rigorous error bounds
//
// TrackedBounded<T> (include/sw/universal/utility/tracked_bounded.hpp) keeps a lower and
// an upper bound instead of a point value, and computes each bound under a directed
// rounding mode (FE_DOWNWARD for the lower, FE_UPWARD for the upper) so that the
// enclosure is rigorous: the true real result is guaranteed to lie inside [lo, hi].
//
// The contracts pinned here:
//   - the ordering lo() <= hi() survives every operation, and an interval built from
//     bounded operands encloses the true result
//   - an operation whose result is representable stays a point: width() == 0
//   - width is monotone: an interval never gets narrower through arithmetic, and
//     x - x is NOT zero for an inexact x (the dependency problem)
//   - operations() counts one per operation and sums the operand counts, while unary
//     minus and abs charge nothing
//   - a divisor straddling zero yields the unbounded interval instead of a wrong one
//   - RoundingGuard restores the caller's rounding mode, whatever happened inside
//
// THE ENCLOSURE OF A ROUNDED RESULT IS THE POINT OF THE CLASS, and it is checked here.
// It was not always checkable. While the bounds were computed by switching the FPU
// rounding mode with fesetround, without a "#pragma STDC FENV_ACCESS ON" that GCC and
// Clang honour, an optimizing compiler folded the arithmetic under whatever mode it
// liked: at -O2 Clang returned 1/3 as the single point 0.33333333333333337034, the
// upward-rounded value, which excludes the true 1/3 altogether, and GCC collapsed
// 1.0 + 1e-17 to a point. #1544 replaced that with error-free transformations, which no
// optimizer can undo, so the checks below hold identically on GCC and Clang at -O0 and
// -O2 -- and would have caught the old behaviour on both.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfenv>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include <universal/utility/tracked_bounded.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	using Bounded = TrackedBounded<double>;

	// ---- reporting helpers -----------------------------------------------------

	int expect_exact(double actual, double wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << std::setprecision(17) << std::scientific
			          << actual << ", expected " << wanted << std::defaultfloat << '\n';
		}
		return 1;
	}

	int expect_count(std::size_t actual, std::size_t wanted, const char* what, bool reportTestCases) {
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

	// ---- construction ----------------------------------------------------------

	int VerifyConstruction(bool reportTestCases) {
		int fails = 0;

		Bounded zero;
		fails += expect_exact(zero.lo(), 0.0, "default ctor lo", reportTestCases);
		fails += expect_exact(zero.hi(), 0.0, "default ctor hi", reportTestCases);
		fails += expect_count(zero.operations(), 0, "default ctor operations", reportTestCases);
		fails += expect_true(zero.is_exact(), "default ctor is_exact", reportTestCases);

		// a point value is a degenerate interval
		Bounded a = 2.5;
		fails += expect_exact(a.lo(), 2.5, "point lo", reportTestCases);
		fails += expect_exact(a.hi(), 2.5, "point hi", reportTestCases);
		fails += expect_exact(a.value(), 2.5, "point value", reportTestCases);
		fails += expect_exact(a.width(), 0.0, "point width", reportTestCases);
		fails += expect_exact(a.radius(), 0.0, "point radius", reportTestCases);
		fails += expect_exact(a.error(), 0.0, "point error", reportTestCases);
		fails += expect_exact(a.relative_error(), 0.0, "point relative_error", reportTestCases);
		fails += expect_exact(a.valid_bits(), double(std::numeric_limits<double>::digits),
			"point valid_bits", reportTestCases);
		fails += expect_count(a.operations(), 0, "point operations", reportTestCases);
		fails += expect_true(a.is_exact(), "point is_exact", reportTestCases);

		// the two-bound constructor orders its arguments
		Bounded reversed(3.0, 1.0);
		fails += expect_exact(reversed.lo(), 1.0, "reversed bounds are ordered: lo", reportTestCases);
		fails += expect_exact(reversed.hi(), 3.0, "reversed bounds are ordered: hi", reportTestCases);
		fails += expect_exact(reversed.value(), 2.0, "midpoint of [1,3]", reportTestCases);
		fails += expect_exact(reversed.width(), 2.0, "width of [1,3]", reportTestCases);
		fails += expect_exact(reversed.radius(), 1.0, "radius of [1,3]", reportTestCases);
		fails += expect_exact(reversed.error(), reversed.radius(), "error is the radius", reportTestCases);

		// predicates on a known interval
		Bounded straddling(-1.0, 2.0);
		fails += expect_true(straddling.contains_zero(), "[-1,2] contains zero", reportTestCases);
		fails += expect_true(!straddling.is_positive(), "[-1,2] is not positive", reportTestCases);
		fails += expect_true(!straddling.is_negative(), "[-1,2] is not negative", reportTestCases);
		fails += expect_true(Bounded(1.0, 2.0).is_positive(), "[1,2] is positive", reportTestCases);
		fails += expect_true(Bounded(-2.0, -1.0).is_negative(), "[-2,-1] is negative", reportTestCases);

		return fails;
	}

	// ---- an exactly representable result stays a point --------------------------
	//
	// Directed rounding is a no-op when nothing is rounded, so a computation that IEEE
	// performs exactly must not widen. A tracker that widened unconditionally would
	// make every interval useless after a few operations.

	int VerifyExactOperationsDoNotWiden(bool reportTestCases) {
		int fails = 0;

		struct Case { double a, b, sum, diff, prod; };
		const Case cases[] = {
			{ 2.0,  3.0,  5.0,  -1.0,  6.0 },
			{ 1.5,  0.25, 1.75,  1.25, 0.375 },
			{ 16.0, -4.0, 12.0,  20.0, -64.0 },
		};
		for (const Case& c : cases) {
			Bounded a = c.a, b = c.b;
			auto sum = a + b, diff = a - b, prod = a * b;
			fails += expect_exact(sum.lo(),  c.sum,  "exact sum lo", reportTestCases);
			fails += expect_exact(sum.hi(),  c.sum,  "exact sum hi", reportTestCases);
			fails += expect_exact(diff.lo(), c.diff, "exact difference lo", reportTestCases);
			fails += expect_exact(diff.hi(), c.diff, "exact difference hi", reportTestCases);
			fails += expect_exact(prod.lo(), c.prod, "exact product lo", reportTestCases);
			fails += expect_exact(prod.hi(), c.prod, "exact product hi", reportTestCases);
			fails += expect_true(sum.is_exact() && diff.is_exact() && prod.is_exact(),
				"exact operations stay exact", reportTestCases);
			fails += expect_exact(sum.width(), 0.0, "exact sum has no width", reportTestCases);
		}

		// division that lands on a representable value
		{
			auto q = Bounded(3.0) / Bounded(4.0);
			fails += expect_exact(q.lo(), 0.75, "3/4 lo", reportTestCases);
			fails += expect_exact(q.hi(), 0.75, "3/4 hi", reportTestCases);
			fails += expect_true(q.is_exact(), "3/4 is exact", reportTestCases);
		}

		// sqrt of a perfect square, and an exact power
		{
			auto r = sqrt(Bounded(4.0));
			fails += expect_exact(r.lo(), 2.0, "sqrt(4) lo", reportTestCases);
			fails += expect_exact(r.hi(), 2.0, "sqrt(4) hi", reportTestCases);
			auto p = pow(Bounded(2.0), 10);
			fails += expect_exact(p.lo(), 1024.0, "2^10 lo", reportTestCases);
			fails += expect_exact(p.hi(), 1024.0, "2^10 hi", reportTestCases);
			auto pn = pow(Bounded(2.0), -2);
			fails += expect_exact(pn.lo(), 0.25, "2^-2 lo", reportTestCases);
			fails += expect_exact(pn.hi(), 0.25, "2^-2 hi", reportTestCases);
		}

		return fails;
	}

	// ---- the enclosure is rigorous ----------------------------------------------

	int VerifyEnclosure(bool reportTestCases) {
		int fails = 0;

		// 1/3 is not representable, so the interval must STRADDLE the true value: the
		// lower bound is the round-to-nearest double, which lies below 1/3, and the
		// upper bound is the next double up. One ulp wide, and enclosing.
		Bounded third = Bounded(1.0) / Bounded(3.0);
		fails += expect_exact(third.lo(), 1.0 / 3.0, "the lower bound is the rounded quotient",
			reportTestCases);
		fails += expect_exact(third.hi(), std::nextafter(1.0 / 3.0, 1.0),
			"the upper bound is one ulp above it", reportTestCases);
		fails += expect_true(third.width() > 0.0, "an inexact quotient widens", reportTestCases);
		fails += expect_true(!third.is_exact(), "an inexact quotient is not exact", reportTestCases);
		fails += expect_true(third.lo() <= third.value() && third.value() <= third.hi(),
			"the midpoint lies inside the interval", reportTestCases);

		// the enclosure survives a chain: every step keeps the true value bracketed
		{
			auto chain = third / Bounded(3.0) / Bounded(3.0);
			const double reference = (1.0 / 3.0) / 3.0 / 3.0;
			fails += expect_true(chain.lo() <= reference && reference <= chain.hi(),
				"the chained quotient is enclosed", reportTestCases);
			// each division scales the value down, so compare RELATIVE widths: the
			// uncertainty grows even as the absolute width shrinks with the magnitude
			fails += expect_true(chain.relative_error() > third.relative_error(),
				"the chain keeps widening, relative to its value", reportTestCases);
			fails += expect_count(chain.operations(), 3, "chain operations", reportTestCases);
		}

		// an addition whose result rounds widens by exactly one ulp and brackets the
		// true sum -- the case the fesetround implementation collapsed at -O2
		{
			Bounded a = 1.0, b = 1e-17;
			auto rounded = a + b;
			fails += expect_exact(rounded.lo(), 1.0, "1 + 1e-17 rounds down to 1.0", reportTestCases);
			fails += expect_exact(rounded.hi(), std::nextafter(1.0, 2.0), "and up to the next double",
				reportTestCases);
			fails += expect_true(rounded.width() > 0.0, "an inexact sum widens", reportTestCases);
		}

		// An interval whose width comes from its OPERANDS, not from rounding, does
		// enclose what it should on every compiler: the corner values are computed by
		// separate expressions, so there is nothing for a rounding mode to change.
		{
			Bounded a(1.0, 2.0), b(10.0, 20.0);
			auto sum = a + b;
			fails += expect_true(sum.lo() <= 11.0 && 22.0 <= sum.hi(), "[1,2]+[10,20] encloses [11,22]",
				reportTestCases);
			auto prod = a * b;
			fails += expect_true(prod.lo() <= 10.0 && 40.0 <= prod.hi(), "[1,2]*[10,20] encloses [10,40]",
				reportTestCases);
			auto diff = a - b;
			fails += expect_true(diff.lo() <= -19.0 && -8.0 <= diff.hi(), "[1,2]-[10,20] encloses [-19,-8]",
				reportTestCases);
		}

		// every operation preserves the ordering of the bounds
		{
			Bounded a(0.9, 1.1), b(-2.0, 3.0);
			const Bounded results[] = { a + b, a - b, a * b, b * a, a + a, a * a, abs(b), sqrt(a) };
			for (const Bounded& r : results) {
				fails += expect_true(r.lo() <= r.hi(), "bounds stay ordered", reportTestCases);
			}
		}

		// sqrt of an interval brackets the roots of its endpoints
		{
			auto r = sqrt(Bounded(4.0, 9.0));
			fails += expect_exact(r.lo(), 2.0, "sqrt([4,9]) lo", reportTestCases);
			fails += expect_exact(r.hi(), 3.0, "sqrt([4,9]) hi", reportTestCases);
		}

		// An irrational root must be BRACKETED, and which side the rounded root falls on
		// depends on the argument: for 2 the double root sits above the true one, for 3
		// below. Squaring the bounds settles it without needing a finer reference --
		// lo*lo must not exceed x and hi*hi must not fall short, computed exactly with
		// fma. A missing step on either side is caught here.
		for (double x : { 2.0, 3.0, 5.0, 7.0, 10.0, 0.5 }) {
			auto r = sqrt(Bounded(x));
			const double lowSquaredError  = std::fma(-r.lo(), r.lo(), x);   // x - lo^2
			const double highSquaredError = std::fma(-r.hi(), r.hi(), x);   // x - hi^2
			fails += expect_true(lowSquaredError >= 0.0, "the lower root does not overshoot",
				reportTestCases);
			fails += expect_true(highSquaredError <= 0.0, "the upper root does not undershoot",
				reportTestCases);
			fails += expect_true(r.width() > 0.0, "an irrational root widens", reportTestCases);
		}

		return fails;
	}

	// ---- widths only grow --------------------------------------------------------

	int VerifyWidthMonotonicity(bool reportTestCases) {
		int fails = 0;

		Bounded a(0.9, 1.1), b(1.9, 2.1);

		auto sum = a + b;
		fails += expect_true(sum.width() >= a.width() + b.width(),
			"addition widths add", reportTestCases);
		auto diff = a - b;
		fails += expect_true(diff.width() >= a.width() + b.width(),
			"subtraction widths add", reportTestCases);

		// doubling an interval doubles its width exactly (both bounds double exactly)
		auto doubled = a + a;
		fails += expect_exact(doubled.width(), 2.0 * a.width(), "self-addition doubles the width",
			reportTestCases);

		// the dependency problem: x - x is not zero unless x is a point
		auto selfDiff = a - a;
		fails += expect_true(selfDiff.contains_zero(), "x - x contains zero", reportTestCases);
		fails += expect_true(selfDiff.width() >= 2.0 * a.width(), "x - x is twice as wide", reportTestCases);
		fails += expect_true(!selfDiff.is_exact(), "x - x is not a point for an inexact x", reportTestCases);

		// but for a point value it is exactly zero
		{
			Bounded p = 3.0;
			auto d = p - p;
			fails += expect_exact(d.lo(), 0.0, "point minus itself lo", reportTestCases);
			fails += expect_exact(d.hi(), 0.0, "point minus itself hi", reportTestCases);
		}

		// an interval never narrows through multiplication by a non-degenerate one
		{
			auto prod = a * b;
			fails += expect_true(prod.width() > 0.0, "product of two intervals is not a point",
				reportTestCases);
			fails += expect_true(prod.lo() <= 0.9 * 1.9 && 2.1 * 1.1 <= prod.hi(),
				"the product encloses the corner products", reportTestCases);
		}

		return fails;
	}

	// ---- a divisor that straddles zero yields the unbounded interval --------------

	int VerifyDivisionByZeroInterval(bool reportTestCases) {
		int fails = 0;

		const double inf = std::numeric_limits<double>::infinity();

		struct Case { double lo, hi; const char* tag; };
		const Case divisors[] = {
			{  0.0,  0.0, "divisor [0,0]" },
			{ -1.0,  1.0, "divisor [-1,1]" },
			{  0.0,  2.0, "divisor [0,2]" },
			{ -2.0,  0.0, "divisor [-2,0]" },
		};
		for (const Case& d : divisors) {
			auto q = Bounded(1.0) / Bounded(d.lo, d.hi);
			fails += expect_exact(q.lo(), -inf, d.tag, reportTestCases);
			fails += expect_exact(q.hi(),  inf, d.tag, reportTestCases);
			fails += expect_count(q.operations(), 1, "division still counts as an operation", reportTestCases);
		}

		// a divisor clear of zero divides normally
		{
			auto q = Bounded(1.0) / Bounded(2.0, 4.0);
			fails += expect_true(std::isfinite(q.lo()) && std::isfinite(q.hi()),
				"a divisor clear of zero stays finite", reportTestCases);
			fails += expect_true(q.lo() <= 0.25 && 0.5 <= q.hi(), "1/[2,4] encloses [0.25,0.5]",
				reportTestCases);
		}

		return fails;
	}

	// ---- operation counting ------------------------------------------------------

	int VerifyOperationCounts(bool reportTestCases) {
		int fails = 0;

		Bounded a = 2.0, b = 3.0, c = 5.0;
		fails += expect_count((a + b).operations(), 1, "one addition", reportTestCases);
		fails += expect_count((a - b).operations(), 1, "one subtraction", reportTestCases);
		fails += expect_count((a * b).operations(), 1, "one multiplication", reportTestCases);
		fails += expect_count((a / b).operations(), 1, "one division", reportTestCases);
		fails += expect_count(((a + b) + c).operations(), 2, "two chained additions", reportTestCases);
		fails += expect_count(((a + b) * (a + c)).operations(), 3, "two additions and a product",
			reportTestCases);

		// unary minus and abs charge nothing
		fails += expect_count((-(a + b)).operations(), 1, "unary minus charges nothing", reportTestCases);
		fails += expect_count(abs(a + b).operations(), 1, "abs charges nothing", reportTestCases);
		// sqrt charges one
		fails += expect_count(sqrt(a + b).operations(), 2, "sqrt charges one", reportTestCases);

		// binary exponentiation charges n operations for x^n
		fails += expect_count(pow(a, 2).operations(), 2, "pow(x,2)", reportTestCases);
		fails += expect_count(pow(a, 3).operations(), 3, "pow(x,3)", reportTestCases);
		fails += expect_count(pow(a, 10).operations(), 10, "pow(x,10)", reportTestCases);
		fails += expect_count(pow(a, 1).operations(), 0, "pow(x,1) returns the base", reportTestCases);
		fails += expect_count(pow(a, -2).operations(), 3, "pow(x,-2) adds the reciprocal", reportTestCases);

		return fails;
	}

	// ---- unary minus and abs ------------------------------------------------------

	int VerifyUnaryOperations(bool reportTestCases) {
		int fails = 0;

		Bounded x(-1.0, 2.0);

		auto neg = -x;
		fails += expect_exact(neg.lo(), -2.0, "negation swaps and flips: lo", reportTestCases);
		fails += expect_exact(neg.hi(),  1.0, "negation swaps and flips: hi", reportTestCases);
		fails += expect_exact(neg.width(), x.width(), "negation preserves the width", reportTestCases);

		// abs of an interval straddling zero starts at zero
		auto m = abs(x);
		fails += expect_exact(m.lo(), 0.0, "abs of a straddling interval starts at 0", reportTestCases);
		fails += expect_exact(m.hi(), 2.0, "abs of a straddling interval ends at the magnitude",
			reportTestCases);
		fails += expect_true(m.lo() >= 0.0, "abs is never negative", reportTestCases);

		// abs of an entirely negative interval mirrors it
		{
			auto n = abs(Bounded(-3.0, -1.0));
			fails += expect_exact(n.lo(), 1.0, "abs of a negative interval: lo", reportTestCases);
			fails += expect_exact(n.hi(), 3.0, "abs of a negative interval: hi", reportTestCases);
		}
		// abs of an entirely positive interval is the identity
		{
			auto p = abs(Bounded(1.0, 3.0));
			fails += expect_exact(p.lo(), 1.0, "abs of a positive interval: lo", reportTestCases);
			fails += expect_exact(p.hi(), 3.0, "abs of a positive interval: hi", reportTestCases);
		}

		// sqrt clamps a lower bound that dips below zero rather than returning NaN
		{
			auto r = sqrt(Bounded(-1.0, 4.0));
			fails += expect_exact(r.lo(), 0.0, "sqrt clamps a negative lower bound", reportTestCases);
			fails += expect_exact(r.hi(), 2.0, "sqrt of the upper bound", reportTestCases);
		}

		return fails;
	}

	// ---- interval predicates and set operations -----------------------------------

	int VerifyPredicatesAndSetOperations(bool reportTestCases) {
		int fails = 0;

		Bounded a(1.0, 2.0), b(3.0, 4.0), overlapping(1.5, 3.5);

		fails += expect_true(a.definitely_less(b), "[1,2] is definitely less than [3,4]", reportTestCases);
		fails += expect_true(b.definitely_greater(a), "[3,4] is definitely greater than [1,2]", reportTestCases);
		fails += expect_true(!a.definitely_less(overlapping), "overlapping intervals are not ordered",
			reportTestCases);
		fails += expect_true(a.overlaps(overlapping), "[1,2] overlaps [1.5,3.5]", reportTestCases);
		fails += expect_true(!a.overlaps(b), "[1,2] does not overlap [3,4]", reportTestCases);
		fails += expect_true(Bounded(0.0, 5.0).contains(a), "[0,5] contains [1,2]", reportTestCases);
		fails += expect_true(!a.contains(Bounded(0.0, 5.0)), "[1,2] does not contain [0,5]", reportTestCases);
		// a definite ordering rules out an overlap
		fails += expect_true(!(a.definitely_less(b) && a.overlaps(b)),
			"definitely_less excludes overlaps", reportTestCases);

		// hull covers both operands
		{
			auto h = hull(a, b);
			fails += expect_exact(h.lo(), 1.0, "hull lo", reportTestCases);
			fails += expect_exact(h.hi(), 4.0, "hull hi", reportTestCases);
			fails += expect_true(h.contains(a) && h.contains(b), "the hull contains both", reportTestCases);
		}
		// intersect narrows to the shared part, and refuses disjoint inputs
		{
			auto i = intersect(a, overlapping);
			fails += expect_exact(i.lo(), 1.5, "intersect lo", reportTestCases);
			fails += expect_exact(i.hi(), 2.0, "intersect hi", reportTestCases);
			auto empty = intersect(a, b);
			fails += expect_true(std::isnan(empty.lo()) && std::isnan(empty.hi()),
				"the intersection of disjoint intervals is empty", reportTestCases);
		}

		return fails;
	}

	// ---- the rounding mode is never touched ----------------------------------------
	//
	// The header no longer switches the global FP rounding mode (#1544). This check
	// stays as a guard: if that approach were reintroduced and the mode ever leaked,
	// every later floating-point computation in the process would be quietly wrong --
	// the kind of damage that shows up far from its cause.

	int VerifyRoundingModeIsRestored(bool reportTestCases) {
		int fails = 0;

		const int before = std::fegetround();

		Bounded a(0.9, 1.1), b(1.9, 2.1);
		volatile double sink = 0.0;
		sink += (a + b).value();
		sink += (a - b).value();
		sink += (a * b).value();
		sink += (a / b).value();
		sink += sqrt(a).value();
		sink += pow(a, 5).value();
		sink += (Bounded(1.0) / Bounded(0.0)).lo();
		(void)sink;

		fails += expect_true(std::fegetround() == before, "the rounding mode is unchanged",
			reportTestCases);

		// and a caller's non-default mode survives a chain untouched
		if (std::fesetround(FE_UPWARD) == 0) {
			Bounded c = Bounded(1.0) / Bounded(3.0);
			const bool preserved = (std::fegetround() == FE_UPWARD);
			// the enclosure does not depend on the mode either
			const bool encloses = (c.lo() <= 1.0 / 3.0 && 1.0 / 3.0 <= c.hi());
			std::fesetround(before);
			fails += expect_true(preserved, "a non-default rounding mode is left alone", reportTestCases);
			fails += expect_true(encloses, "and the enclosure holds under it", reportTestCases);
		}

		return fails;
	}

	// ---- the metrics, including the cases that used to lie ---------------------------

	int VerifyErrorMetrics(bool reportTestCases) {
		int fails = 0;

		// An exactly known value is at full precision whatever it holds, zero included.
		// relative_error() used to divide by the midpoint first and return infinity at
		// zero, so an exact zero reported ZERO valid bits while an exact 1.0 reported
		// 53 (#1546). A zero known exactly is not a value about which nothing is known.
		{
			Bounded z = 0.0;
			fails += expect_true(z.is_exact(), "exact zero is exact", reportTestCases);
			fails += expect_exact(z.error(), 0.0, "exact zero has no error", reportTestCases);
			fails += expect_exact(z.relative_error(), 0.0, "and no relative error", reportTestCases);
			fails += expect_exact(z.valid_bits(), 53.0, "exact zero is at full precision",
				reportTestCases);
			fails += expect_exact(Bounded(1.0).valid_bits(), 53.0, "so is an exact one",
				reportTestCases);
		}

		// An interval spanning zero has no meaningful relative error, and nothing left
		// to report in bits.
		{
			Bounded spanning(-1.0, 1.0);
			fails += expect_true(!std::isfinite(spanning.relative_error()),
				"an interval around zero has no relative error", reportTestCases);
			fails += expect_exact(spanning.valid_bits(), 0.0, "and no valid bits", reportTestCases);
		}

		// valid_bits never goes negative, however wide the interval
		{
			Bounded huge(1.0, 1000.0);
			fails += expect_true(huge.valid_bits() >= 0.0, "valid_bits is never negative",
				reportTestCases);
			fails += expect_true(huge.valid_bits() < 1.0, "a very wide interval has almost none",
				reportTestCases);
		}

		// a narrower interval leaves more bits than a wider one around the same value
		{
			Bounded tight(0.9999, 1.0001), loose(0.9, 1.1);
			fails += expect_true(tight.valid_bits() > loose.valid_bits(),
				"a tighter interval leaves more valid bits", reportTestCases);
		}

		return fails;
	}

	// ---- intervals are partially ordered, and the operators agree with that ----------
	//
	// The comparison operators used to answer on midpoints while operator== answered on
	// bounds, so [0,2] and [1,1] were neither less, nor greater, nor equal -- an
	// ordering no algorithm could rely on (#1547). They now all answer on the bounds.

	int VerifyPartialOrder(bool reportTestCases) {
		int fails = 0;

		// disjoint intervals are ordered, and the ordering agrees with definitely_less
		{
			Bounded a(1.0, 2.0), b(3.0, 4.0);
			fails += expect_true(a < b, "[1,2] < [3,4]", reportTestCases);
			fails += expect_true(b > a, "[3,4] > [1,2]", reportTestCases);
			fails += expect_true(a <= b && b >= a, "and the inclusive forms agree", reportTestCases);
			fails += expect_true(a.definitely_less(b) == (a < b),
				"operator< agrees with definitely_less", reportTestCases);
			fails += expect_true(!(a == b) && (a != b), "they are not the same interval",
				reportTestCases);
		}

		// Overlapping intervals are INCOMPARABLE even when their midpoints are ordered.
		// This is the case a midpoint comparison gets wrong: [0,10] has the lower
		// midpoint, but it reaches above everything in [6,7], so no order holds.
		{
			Bounded spread(0.0, 10.0), narrow(6.0, 7.0);
			fails += expect_true(spread.overlaps(narrow), "[0,10] overlaps [6,7]", reportTestCases);
			fails += expect_true(!(spread < narrow),
				"[0,10] is not less than [6,7], whatever the midpoints say", reportTestCases);
			fails += expect_true(!(narrow > spread), "nor is [6,7] greater", reportTestCases);
			fails += expect_true(!(spread <= narrow) && !(narrow >= spread),
				"and neither inclusive form holds either", reportTestCases);
		}

		// overlapping intervals are INCOMPARABLE: no order holds, in either direction
		{
			Bounded wide(0.0, 2.0), point(1.0, 1.0);
			fails += expect_true(!(wide < point) && !(wide > point),
				"[0,2] and [1,1] are not ordered", reportTestCases);
			fails += expect_true(!(point < wide) && !(point > wide),
				"nor the other way round", reportTestCases);
			fails += expect_true(!(wide == point), "and they are not equal", reportTestCases);
			fails += expect_true(wide.overlaps(point), "because they overlap", reportTestCases);
		}

		// the same interval compares equal, and the inclusive operators follow
		{
			Bounded a(1.0, 2.0), copy(1.0, 2.0);
			fails += expect_true(a == copy, "equal bounds compare equal", reportTestCases);
			fails += expect_true(a <= copy && a >= copy, "and satisfy both inclusive forms",
				reportTestCases);
			fails += expect_true(!(a < copy) && !(a > copy), "but neither strict form",
				reportTestCases);
		}

		// a consistency law that the midpoint comparison broke: two intervals that are
		// equal are never also strictly ordered, and two that are strictly ordered are
		// never equal
		{
			const Bounded samples[] = { Bounded(0.0, 2.0), Bounded(1.0, 1.0), Bounded(1.0, 2.0),
			                            Bounded(3.0, 4.0), Bounded(-1.0, 0.5), Bounded(0.0, 10.0),
			                            Bounded(6.0, 7.0) };
			for (const Bounded& x : samples) {
				for (const Bounded& y : samples) {
					const bool lt = x < y, gt = x > y, eq = x == y;
					if ((eq && (lt || gt)) || (lt && gt)) {
						++fails;
						if (reportTestCases) std::cout << "    FAIL inconsistent ordering\n";
					}
					// an order in either direction rules out an overlap
					if ((lt || gt) && x.overlaps(y)) {
						++fails;
						if (reportTestCases) std::cout << "    FAIL ordered a pair that overlaps\n";
					}
				}
			}
		}

		return fails;
	}

	// ---- the exploratory narrative, kept for manual inspection ---------------------

#if MANUAL_TESTING
	void ReportTrackedBoundedBehaviour() {
		std::cout << "\n=== TrackedBounded<double> walk-through ===\n";

		Bounded third = Bounded(1.0) / Bounded(3.0);
		std::cout << "1/3:\n";
		third.report(std::cout);

		std::cout << "\n1/3 * 3 (the dependency problem keeps it off 1.0):\n";
		auto back = third * Bounded(3.0);
		back.report(std::cout);

		std::cout << "\nx - x for x = [0.9, 1.1]:\n";
		Bounded x(0.9, 1.1);
		auto d = x - x;
		d.report(std::cout);

		std::cout << "\n1 / [0,0]:\n";
		auto unbounded = Bounded(1.0) / Bounded(0.0);
		unbounded.report(std::cout);
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

	std::string test_suite  = "TrackedBounded<T> directed-rounding interval bounds";
	std::string test_tag    = "tracked_bounded";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTrackedBoundedBehaviour();
	nrOfFailedTestCases += VerifyEnclosure(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else  // !MANUAL_TESTING

	// A few hundred interval operations, so it all belongs at level 1: CI configures
	// REGRESSION_LEVEL_1 only, and a contract that is not checked there does not gate.
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConstruction(reportTestCases), test_tag, "construction");
	nrOfFailedTestCases += ReportTestResult(VerifyExactOperationsDoNotWiden(reportTestCases), test_tag, "exact operations");
	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure(reportTestCases), test_tag, "enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyWidthMonotonicity(reportTestCases), test_tag, "width monotonicity");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionByZeroInterval(reportTestCases), test_tag, "division by a zero interval");
	nrOfFailedTestCases += ReportTestResult(VerifyOperationCounts(reportTestCases), test_tag, "operation counts");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperations(reportTestCases), test_tag, "unary operations");
	nrOfFailedTestCases += ReportTestResult(VerifyPredicatesAndSetOperations(reportTestCases), test_tag, "predicates and set operations");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundingModeIsRestored(reportTestCases), test_tag, "rounding mode untouched");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorMetrics(reportTestCases), test_tag, "error metrics");
	nrOfFailedTestCases += ReportTestResult(VerifyPartialOrder(reportTestCases), test_tag, "partial order");
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
