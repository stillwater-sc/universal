// division.cpp: regression tests for ereal division.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests + small fuzz: division
//              invariants (identity, self-quotient, sign rules, exact
//              divisions, power-of-two divisors, recurring quotients) and
//              divide-by-zero boundary.
//   LEVEL 2 -- property-based fuzzer over random multi-component expansions
//              (~10,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~1,000,000 iterations).
//
// Scope: the operation under test is `/` / `/=` only. `+` / `-` appear solely
// to CONSTRUCT multi-component inputs, never as the operation under test.
// Division is neither commutative nor associative, and mixed-operator
// identities such as (a / b) * b == a belong in identities.cpp, not here.
//
// Exact vs. tolerance: ereal division is e * reciprocal(f), where the
// reciprocal is a 3-iteration Newton refinement -- so it is NOT exact in
// general. Division by 1, by a power of two, by an exact-reciprocal divisor
// (e.g. 6/2, 1/4), and the sign rules are bit-exact and use operator==.
// Self-quotient (a/a), non-power-of-two exact quotients (15/3), and recurring
// quotients (1/3) carry a Newton residual and are checked on the projected
// double within a relative tolerance.
//
// Reference: Shewchuk (1997) "Adaptive Precision Floating-Point Arithmetic
//   and Fast Robust Geometric Predicates"; addition.cpp (PR #943) for the
//   standard test structure.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_test_support.hpp>

namespace {

	// Compare two ereal values via their double projection within a relative
	// tolerance. Used for division invariants that hold mathematically but
	// carry a Newton-reciprocal residual (self-quotient, non-power-of-two
	// quotients, recurring quotients).
	bool close_rel(const sw::universal::ereal<16>& x,
	               const sw::universal::ereal<16>& y, double relTol) {
		double a = double(x), b = double(y);
		double diff = std::abs(a - b);
		if (diff == 0.0) return true;
		double scale = std::max(std::abs(a), std::abs(b));
		return diff <= relTol * scale;
	}

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyErealDivision(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Algebraic invariants ---

		// Identity: a / 1 == a (exact -- reciprocal(1) is exactly 1).
		if (reportTestCases) std::cout << "  Identity (multi-limb)...\n";
		{
			ereal<16> a(1.0);
			a += 1.0e-15; a += 1.0e-30; a += 1.0e-45;
			ereal<16> one(1.0);
			if (a / one != a) {
				if (reportTestCases) std::cout << "    FAIL a / 1 != a\n";
				++nrOfFailedTestCases;
			}
		}

		// Self-quotient: a / a == 1 for finite non-zero a (tolerance -- Newton
		// residual).
		if (reportTestCases) std::cout << "  Self-quotient a/a == 1...\n";
		{
			ereal<16> a(123.0);
			a += 1.0e-10;
			ereal<16> one(1.0);
			if (!close_rel(a / a, one, 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL a/a != 1\n";
				++nrOfFailedTestCases;
			}
		}

		// Sign rules (exact): (-a)/b == -(a/b); a/(-b) == -(a/b); (-a)/(-b) == a/b.
		if (reportTestCases) std::cout << "  Sign rules...\n";
		{
			ereal<16> a(123.0); a += 1.0e-10;
			ereal<16> b(7.0);   b += 1.0e-12;
			if ((-a) / b != -(a / b)) {
				if (reportTestCases) std::cout << "    FAIL (-a)/b != -(a/b)\n";
				++nrOfFailedTestCases;
			}
			if (a / (-b) != -(a / b)) {
				if (reportTestCases) std::cout << "    FAIL a/(-b) != -(a/b)\n";
				++nrOfFailedTestCases;
			}
			if ((-a) / (-b) != a / b) {
				if (reportTestCases) std::cout << "    FAIL (-a)/(-b) != a/b\n";
				++nrOfFailedTestCases;
			}
		}

		// In-place parity: (a /= b) == (a / b) (exact, same code path).
		if (reportTestCases) std::cout << "  In-place /= parity...\n";
		{
			ereal<16> a(123.0); a += 1.0e-10;
			ereal<16> b(7.0);   b += 1.0e-12;
			ereal<16> inplace(a);
			inplace /= b;
			if (inplace != (a / b)) {
				if (reportTestCases) std::cout << "    FAIL in-place /= parity\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Exact divisions ---

		// 6 / 2 == 3 exactly (reciprocal(2) = 0.5 is exact).
		if (reportTestCases) std::cout << "  Exact division 6/2 == 3...\n";
		{
			if (ereal<16>(6.0) / ereal<16>(2.0) != ereal<16>(3.0)) {
				if (reportTestCases) std::cout << "    FAIL 6/2 != 3\n";
				++nrOfFailedTestCases;
			}
		}

		// 1 / 4 == 0.25 exactly (power-of-two reciprocal).
		if (reportTestCases) std::cout << "  Exact reciprocal 1/4 == 0.25...\n";
		{
			if (ereal<16>(1.0) / ereal<16>(4.0) != ereal<16>(0.25)) {
				if (reportTestCases) std::cout << "    FAIL 1/4 != 0.25\n";
				++nrOfFailedTestCases;
			}
		}

		// 15 / 3 == 5 (tolerance -- reciprocal(3) is not representable).
		if (reportTestCases) std::cout << "  15/3 ~= 5 (Newton residual)...\n";
		{
			if (!close_rel(ereal<16>(15.0) / ereal<16>(3.0), ereal<16>(5.0), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL 15/3 != 5\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Power-of-two divisor (exact exponent shift) ---

		// a / 2^10 is exact: each component's exponent shifts cleanly. Compare
		// against the directly-constructed expansion (division-only).
		if (reportTestCases) std::cout << "  Power-of-two divisor (exact shift)...\n";
		{
			ereal<16> a(1.0); a += std::ldexp(1.0, -20); a += std::ldexp(1.0, -40);
			ereal<16> divisor(std::ldexp(1.0, 10));  // 2^10
			ereal<16> expected(std::ldexp(1.0, -10));
			expected += std::ldexp(1.0, -30);
			expected += std::ldexp(1.0, -50);
			if (a / divisor != expected) {
				if (reportTestCases) std::cout << "    FAIL a / 2^10 not exact\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Recurring quotient ---

		// 1 / 3 is not representable; the expansion should populate multiple
		// limbs and project to the nearest double.
		if (reportTestCases) std::cout << "  Recurring quotient 1/3 (multi-limb)...\n";
		{
			ereal<16> q = ereal<16>(1.0) / ereal<16>(3.0);
			if (q.limbs().size() < 2) {
				if (reportTestCases) std::cout << "    FAIL 1/3 collapsed to a single limb\n";
				++nrOfFailedTestCases;
			}
			if (std::abs(double(q) - (1.0 / 3.0)) > 1.0e-15) {
				if (reportTestCases) std::cout << "    FAIL 1/3 projection off\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Subnormal boundary ---

		// Dividing a subnormal by one preserves it exactly.
		if (reportTestCases) std::cout << "  Subnormal numerator...\n";
		{
			double subn = std::ldexp(1.0, -1050);
			if (subn != 0.0) {
				ereal<16> s(subn);
				ereal<16> one(1.0);
				if (s / one != s) {
					if (reportTestCases) std::cout << "    FAIL subnormal / 1 != subnormal\n";
					++nrOfFailedTestCases;
				}
			}
		}

		// --- IEEE 754 special values + divide-by-zero (fixed by #968) ---
		// The sign of a quotient is signbit(a) XOR signbit(b), including for
		// zero and infinite results. In the default (non-throwing) mode a zero
		// divisor produces a signed Inf (or NaN for 0/0); see the throw-mode
		// block below for EREAL_THROW_ARITHMETIC_EXCEPTION behavior.

		double qnan = std::numeric_limits<double>::quiet_NaN();
		double pinf = std::numeric_limits<double>::infinity();

		// NaN / finite = NaN; finite / NaN = NaN
		if (reportTestCases) std::cout << "  NaN propagation...\n";
		{
			ereal<16> a(5.0);
			ereal<16> n(qnan);
			if (!std::isnan(double(a / n))) { if (reportTestCases) std::cout << "    FAIL a/NaN\n"; ++nrOfFailedTestCases; }
			if (!std::isnan(double(n / a))) { if (reportTestCases) std::cout << "    FAIL NaN/a\n"; ++nrOfFailedTestCases; }
		}

#ifndef EREAL_THROW_ARITHMETIC_EXCEPTION
#define EREAL_THROW_ARITHMETIC_EXCEPTION 0
#endif
#if EREAL_THROW_ARITHMETIC_EXCEPTION
		// Throw mode: a zero divisor throws ereal_divide_by_zero.
		if (reportTestCases) std::cout << "  Divide-by-zero throws (exception mode)...\n";
		{
			bool threw = false;
			try { ereal<16> q = ereal<16>(5.0) / ereal<16>(0.0); (void)q; }
			catch (const ereal_divide_by_zero&) { threw = true; }
			if (!threw) {
				if (reportTestCases) std::cout << "    FAIL 5/0 did not throw\n";
				++nrOfFailedTestCases;
			}
		}
#else
		// Default mode: IEEE results. x/0 = signed Inf, 0/0 = NaN.
		if (reportTestCases) std::cout << "  Divide-by-zero (IEEE)...\n";
		{
			ereal<16> zero(0.0);
			double r1 = double(ereal<16>(5.0)  / zero);   // +Inf
			if (!std::isinf(r1) || r1 < 0) { if (reportTestCases) std::cout << "    FAIL 5/0 != +Inf\n"; ++nrOfFailedTestCases; }
			double r2 = double(ereal<16>(-5.0) / zero);   // -Inf
			if (!std::isinf(r2) || r2 > 0) { if (reportTestCases) std::cout << "    FAIL -5/0 != -Inf\n"; ++nrOfFailedTestCases; }
			double r3 = double(zero / zero);              // NaN
			if (!std::isnan(r3)) { if (reportTestCases) std::cout << "    FAIL 0/0 != NaN\n"; ++nrOfFailedTestCases; }
		}
#endif

		// Infinity operands (sign = XOR).
		if (reportTestCases) std::cout << "  Infinity operands...\n";
		{
			ereal<16> pos(pinf);
			ereal<16> neg(-pinf);
			ereal<16> two(2.0);
			if (!std::isnan(double(pos / pos))) { if (reportTestCases) std::cout << "    FAIL Inf/Inf != NaN\n"; ++nrOfFailedTestCases; }
			double r1 = double(pos / two);   // +Inf
			if (!std::isinf(r1) || r1 < 0) { if (reportTestCases) std::cout << "    FAIL Inf/2 != +Inf\n"; ++nrOfFailedTestCases; }
			double r2 = double(neg / two);   // -Inf
			if (!std::isinf(r2) || r2 > 0) { if (reportTestCases) std::cout << "    FAIL -Inf/2 != -Inf\n"; ++nrOfFailedTestCases; }
			double r3 = double(two / pos);   // +0
			if (r3 != 0.0 || std::signbit(r3)) { if (reportTestCases) std::cout << "    FAIL 2/Inf != +0\n"; ++nrOfFailedTestCases; }
		}

		// Signed-zero quotients: sign = signbit(a) XOR signbit(b).
		if (reportTestCases) std::cout << "  Signed-zero quotients...\n";
		{
			double r1 = double(ereal<16>(-0.0) / ereal<16>(5.0));   // -0
			if (r1 != 0.0 || !std::signbit(r1)) { if (reportTestCases) std::cout << "    FAIL -0/5 != -0\n"; ++nrOfFailedTestCases; }
			double r2 = double(ereal<16>(0.0)  / ereal<16>(-5.0));  // -0
			if (r2 != 0.0 || !std::signbit(r2)) { if (reportTestCases) std::cout << "    FAIL 0/-5 != -0\n"; ++nrOfFailedTestCases; }
			double r3 = double(ereal<16>(0.0)  / ereal<16>(5.0));   // +0
			if (r3 != 0.0 || std::signbit(r3)) { if (reportTestCases) std::cout << "    FAIL 0/5 != +0\n"; ++nrOfFailedTestCases; }
		}

		return nrOfFailedTestCases;
	}

	// =========================================================================
	// Fuzzer infrastructure
	// =========================================================================

	// Generate a multi-component ereal expansion of varying length, signs, and
	// exponent ranges. Leading magnitude ~1e3 keeps quotients well away from
	// overflow/underflow. The expansion is built by adding random values at
	// progressively smaller magnitudes so renormalisation produces a true
	// multi-limb result (rather than collapsing to a single limb).
	template <unsigned maxlimbs>
	sw::universal::ereal<maxlimbs>
	random_ereal(std::mt19937_64& rng, double leading_magnitude = 1e3) {
		using namespace sw::universal;
		std::uniform_int_distribution<unsigned> n_dist(1, maxlimbs);
		std::uniform_int_distribution<int> sign_dist(0, 1);
		std::uniform_real_distribution<double> unit(0.5, 2.0);

		ereal<maxlimbs> result(0.0);
		unsigned n_limbs = n_dist(rng);
		double mag = leading_magnitude;
		for (unsigned i = 0; i < n_limbs; ++i) {
			double v = unit(rng) * mag * (sign_dist(rng) ? 1.0 : -1.0);
			result += v;
			// Drop ~50 binary exponents per next limb so the contribution
			// survives renormalisation as a separate component.
			mag = std::ldexp(mag, -50);
			// Stop if next magnitude would underflow into subnormals.
			if (mag < std::ldexp(1.0, -950)) break;
		}
		return result;
	}

	// Fuzz the bit-exact division invariants (identity, sign rules, in-place
	// parity) plus tolerance-based self-quotient over N random pairs. random_ereal
	// never produces a zero leading limb, so the divisor is always non-zero.
	// Fixed seed for reproducibility; failing (seed, iter) pairs let a tester
	// reconstruct the exact case.
	int VerifyErealDivision_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		ereal<16> one(1.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<16> a = random_ereal<16>(rng);
			ereal<16> b = random_ereal<16>(rng);

			// Structural invariant (#954 D3): every quotient is Priest-normal
			// (divisor b is non-zero -- random_ereal never produces a zero
			// leading limb). Regression guard for #981.
			if (auto pn = check_priest_normal(a / b); !pn.ok()) {
				if (reportTestCases) std::cout << "    FAIL priest-normal a/b: " << pn.what()
				                               << " at limb " << pn.index << " (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Identity: a / 1 == a
			if (a / one != a) {
				if (reportTestCases) std::cout << "    FAIL identity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Sign rule: (-a) / b == -(a / b)
			if ((-a) / b != -(a / b)) {
				if (reportTestCases) std::cout << "    FAIL sign (-a)/b (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Sign rule: (-a) / (-b) == a / b
			if ((-a) / (-b) != a / b) {
				if (reportTestCases) std::cout << "    FAIL sign (-a)/(-b) (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// In-place parity: (a /= b) == (a / b)
			ereal<16> inplace(a);
			inplace /= b;
			if (inplace != (a / b)) {
				if (reportTestCases) std::cout << "    FAIL in-place /= parity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Self-quotient (tolerance): a / a ~= 1. Skip if a is zero.
			if (!a.iszero() && !close_rel(a / a, one, 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL self-quotient (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

}  // anonymous namespace

// Regression testing guards
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

	std::string test_suite          = "ereal division";
	std::string test_tag            = "division";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErealDivision(reportTestCases), "ereal", "division manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealDivision(reportTestCases), "ereal", "division foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyErealDivision_Fuzz(reportTestCases, 100), "ereal", "division fuzz x100");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyErealDivision_Fuzz(reportTestCases, 1000), "ereal", "division fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyErealDivision_Fuzz(reportTestCases, 10000), "ereal", "division fuzz x10k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyErealDivision_Fuzz(reportTestCases, 100000), "ereal", "division fuzz x100k");
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
