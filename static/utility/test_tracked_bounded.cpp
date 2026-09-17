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
// WHAT THIS SUITE CANNOT CHECK TODAY, AND WHY: the enclosure of a result that ROUNDS.
// The header switches the rounding mode with fesetround but never declares
// "#pragma STDC FENV_ACCESS ON" for GCC or Clang, so an optimizing compiler may fold the
// arithmetic under whatever mode it likes. Measured on this tree at -O2:
//
//   Clang   1/3 -> [0.33333333333333337034, 0.33333333333333337034]  (a single point,
//           the UPWARD-rounded value -- it excludes the true 1/3 entirely)
//   GCC     1/3 -> a correct enclosure, but 1.0 + 1e-17 collapses to a point
//   both    correct at -O0
//
// So on a Release build the rigorous-bounds guarantee does not hold, and any assertion
// of it would be red on Clang and green on GCC. The checks below are the ones that hold
// whatever the optimizer does: ordering, exactness where nothing rounds, widths that
// come from the OPERANDS, operation counts, the unbounded-divisor case, and the
// restoration of the caller's rounding mode. This is #1544; when it is fixed, the
// enclosure of rounded results belongs back in this file.
//
// Two further checks pin behaviour that is wrong (#1546, #1547); each says so, so that a
// fix is noticed here rather than silently changing what callers see.
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

		// The enclosure of a result that ROUNDS cannot be asserted here at all -- see the
		// note at the top of this file. What survives optimization is the structure: the
		// bounds stay ordered, the midpoint stays between them, and the operations are
		// counted.
		Bounded third = Bounded(1.0) / Bounded(3.0);
		fails += expect_true(third.lo() <= third.hi(), "the quotient's bounds are ordered",
			reportTestCases);
		fails += expect_true(third.lo() <= third.value() && third.value() <= third.hi(),
			"the midpoint lies inside the interval", reportTestCases);
		{
			auto chain = third / Bounded(3.0) / Bounded(3.0);
			fails += expect_true(chain.lo() <= chain.hi(), "the chained bounds are ordered",
				reportTestCases);
			fails += expect_count(chain.operations(), 3, "chain operations", reportTestCases);
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

	// ---- the rounding mode is left as it was found --------------------------------
	//
	// RoundingGuard changes the global FP rounding mode. If it ever failed to restore
	// it, every later floating-point computation in the process would be quietly wrong,
	// which is the kind of damage that shows up far from its cause.

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

		fails += expect_true(std::fegetround() == before, "the rounding mode is restored", reportTestCases);

		// and it is restored from a non-default mode too
		if (std::fesetround(FE_UPWARD) == 0) {
			Bounded c = Bounded(1.0) / Bounded(3.0);
			(void)c;
			fails += expect_true(std::fegetround() == FE_UPWARD,
				"a non-default rounding mode is restored", reportTestCases);
			std::fesetround(before);
		}

		return fails;
	}

	// ---- behaviour that is pinned although it looks wrong --------------------------
	//
	// Both checks below assert what the header does TODAY. They are here so that a fix
	// shows up as a failing test rather than as a silent change in what callers see.

	int VerifyKnownDefects(bool reportTestCases) {
		int fails = 0;

		// An exact zero reports ZERO valid bits, while an exact 1.0 reports 53.
		// relative_error() returns infinity when the midpoint is zero (there is no
		// relative error to speak of), and valid_bits() maps a non-finite relative
		// error to 0. A zero known exactly is not a value about which nothing is known.
		{
			Bounded z = 0.0;
			fails += expect_true(z.is_exact(), "exact zero is exact", reportTestCases);
			fails += expect_exact(z.error(), 0.0, "exact zero has no error", reportTestCases);
			fails += expect_exact(z.valid_bits(), 0.0,
				"exact zero reports no valid bits (known defect #1546)", reportTestCases);
			fails += expect_exact(Bounded(1.0).valid_bits(), 53.0,
				"exact one reports full precision", reportTestCases);
		}

		// operator== compares BOUNDS while operator< and operator> compare MIDPOINTS,
		// so two intervals can be neither less, nor greater, nor equal.
		{
			Bounded wide(0.0, 2.0), point(1.0, 1.0);
			fails += expect_true(!(wide < point), "[0,2] is not less than [1,1]", reportTestCases);
			fails += expect_true(!(wide > point), "[0,2] is not greater than [1,1]", reportTestCases);
			fails += expect_true(!(wide == point),
				"[0,2] is not equal to [1,1] either (known defect #1547)", reportTestCases);
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
	nrOfFailedTestCases += ReportTestResult(VerifyRoundingModeIsRestored(reportTestCases), test_tag, "rounding mode restored");
	nrOfFailedTestCases += ReportTestResult(VerifyKnownDefects(reportTestCases), test_tag, "pinned known defects");
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
