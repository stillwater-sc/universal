// multiplication.cpp: regression tests for ereal multiplication.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests: algebraic invariants
//              (commutativity, identity, annihilation by zero, associativity,
//              sign rules, in-place parity), hostile arithmetic corner cases
//              (powers of two, cancellation-via-multiplication, multi-limb
//              cross terms), and subnormal boundaries.
//   LEVEL 2 -- property-based fuzzer over random multi-component expansions
//              (~1,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~10,000,000 iterations).
//
// Scope: the operation under test is `*` / `*=` only. `+` / `-` appear solely
// to CONSTRUCT multi-component inputs, never as the operation under test.
// Mixed-operator identities (e.g. distributivity a*(b+c) == a*b + a*c) live in
// identities.cpp, not here.
//
// Exact vs. tolerance: expansion_product scales one operand by each component
// of the other and accumulates (O(m*n)). Consequently a*b and b*a round the
// same true product in different orders and are NOT bit-identical, and neither
// is multi-limb associativity. Those invariants are checked on the projected
// double within a relative tolerance; every other invariant is exact and uses
// operator== on the expansion.
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
#include <cmath>
#include <limits>
#include <random>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// Compare two ereal values via their double projection within a relative
	// tolerance. Used only for invariants that hold mathematically but are not
	// bit-identical because expansion_product accumulates cross terms in an
	// operand-dependent order (commutativity, multi-limb associativity).
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
	int VerifyErealMultiplication(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Algebraic invariants ---

		// Commutativity: a * b == b * a (relative tolerance -- see header).
		if (reportTestCases) std::cout << "  Commutativity...\n";
		{
			ereal<16> a(1.0e+8);
			a += 1.0; a += 1.0e-8;
			ereal<16> b(3.14159);
			b += 1.0e-12;
			if (!close_rel(a * b, b * a, 1.0e-14)) {
				if (reportTestCases) std::cout << "    FAIL a*b != b*a\n";
				++nrOfFailedTestCases;
			}
		}

		// Identity: a * 1 == a and 1 * a == a (exact).
		if (reportTestCases) std::cout << "  Identity (multi-limb)...\n";
		{
			ereal<16> a(1.0);
			a += 1.0e-15; a += 1.0e-30; a += 1.0e-45;
			ereal<16> one(1.0);
			if (a * one != a) {
				if (reportTestCases) std::cout << "    FAIL a * 1 != a\n";
				++nrOfFailedTestCases;
			}
			if (one * a != a) {
				if (reportTestCases) std::cout << "    FAIL 1 * a != a\n";
				++nrOfFailedTestCases;
			}
		}

		// Annihilation: a * 0 == 0 and 0 * a == 0 (exact).
		if (reportTestCases) std::cout << "  Annihilation by zero...\n";
		{
			ereal<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			ereal<16> zero(0.0);
			if (!(a * zero).iszero()) {
				if (reportTestCases) std::cout << "    FAIL a * 0 != 0\n";
				++nrOfFailedTestCases;
			}
			if (!(zero * a).iszero()) {
				if (reportTestCases) std::cout << "    FAIL 0 * a != 0\n";
				++nrOfFailedTestCases;
			}
		}

		// Associativity (small exact integers): (a*b)*c == a*(b*c). Products of
		// small integers are exactly representable, so this holds bit-for-bit.
		if (reportTestCases) std::cout << "  Associativity (exact small int)...\n";
		{
			ereal<16> a(2.0);
			ereal<16> b(3.0);
			ereal<16> c(5.0);
			if ((a * b) * c != a * (b * c)) {
				if (reportTestCases) std::cout << "    FAIL (2*3)*5 != 2*(3*5)\n";
				++nrOfFailedTestCases;
			}
		}

		// Associativity (multi-limb): (a*b)*c ~= a*(b*c) within tolerance.
		if (reportTestCases) std::cout << "  Associativity (multi-limb)...\n";
		{
			ereal<16> a(1.0e+8); a += 1.0;
			ereal<16> b(3.0);    b += 1.0e-12;
			ereal<16> c(7.0);    c += 1.0e-10;
			if (!close_rel((a * b) * c, a * (b * c), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL (a*b)*c != a*(b*c)\n";
				++nrOfFailedTestCases;
			}
		}

		// Sign rules (exact): (-a)*b == -(a*b); (-a)*(-b) == a*b; a*(-1) == -a.
		if (reportTestCases) std::cout << "  Sign rules...\n";
		{
			ereal<16> a(1.0e+8); a += 1.0; a += 1.0e-8;
			ereal<16> b(3.14159); b += 1.0e-12;
			if ((-a) * b != -(a * b)) {
				if (reportTestCases) std::cout << "    FAIL (-a)*b != -(a*b)\n";
				++nrOfFailedTestCases;
			}
			if ((-a) * (-b) != a * b) {
				if (reportTestCases) std::cout << "    FAIL (-a)*(-b) != a*b\n";
				++nrOfFailedTestCases;
			}
			ereal<16> negone(-1.0);
			if (a * negone != -a) {
				if (reportTestCases) std::cout << "    FAIL a*(-1) != -a\n";
				++nrOfFailedTestCases;
			}
		}

		// In-place parity: (a *= b) == (a * b) (exact, same code path).
		if (reportTestCases) std::cout << "  In-place *= parity...\n";
		{
			ereal<16> a(1.0e+8); a += 1.0; a += 1.0e-8;
			ereal<16> b(3.14159); b += 1.0e-12;
			ereal<16> inplace(a);
			inplace *= b;
			if (inplace != (a * b)) {
				if (reportTestCases) std::cout << "    FAIL in-place *= parity\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Hostile arithmetic ---

		// Powers of two multiply exactly (pure exponent shift). The round trip
		// (a * 2^k) * 2^-k recovers a exactly.
		if (reportTestCases) std::cout << "  Powers of two (exact shift round trip)...\n";
		{
			ereal<16> a(1.0); a += 1.0e-15; a += 1.0e-30;
			ereal<16> up(std::ldexp(1.0, 40));
			ereal<16> down(std::ldexp(1.0, -40));
			if ((a * up) * down != a) {
				if (reportTestCases) std::cout << "    FAIL (a*2^40)*2^-40 != a\n";
				++nrOfFailedTestCases;
			}
		}

		// Cancellation via multiplication: (1 + e)(1 - e) == 1 - e^2 exactly,
		// for e = 2^-30 (so 1+/-e and 1-e^2 are exact expansions). The e^2 term
		// must survive in the second limb.
		if (reportTestCases) std::cout << "  Cancellation (1+e)(1-e) == 1-e^2...\n";
		{
			double e = std::ldexp(1.0, -30);  // 2^-30
			ereal<16> plus(1.0);  plus  += e;
			ereal<16> minus(1.0); minus -= e;
			ereal<16> product = plus * minus;
			ereal<16> expected(1.0); expected -= e * e;  // 1 - 2^-60
			if (product != expected) {
				if (reportTestCases) std::cout << "    FAIL e^2 term lost\n";
				++nrOfFailedTestCases;
			}
		}

		// Multi-limb * multi-limb cross terms: (1 + 2^-20)(1 + 2^-40) expands to
		// 1 + 2^-20 + 2^-40 + 2^-60 exactly (every cross term representable).
		if (reportTestCases) std::cout << "  Multi-limb cross terms...\n";
		{
			double a20 = std::ldexp(1.0, -20);
			double a40 = std::ldexp(1.0, -40);
			double a60 = std::ldexp(1.0, -60);
			ereal<16> a(1.0); a += a20;
			ereal<16> b(1.0); b += a40;
			ereal<16> product = a * b;
			ereal<16> expected(1.0); expected += a20; expected += a40; expected += a60;
			if (product != expected) {
				if (reportTestCases) std::cout << "    FAIL cross terms lost\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Subnormal boundary ---

		// Multiplying a subnormal by one preserves it exactly.
		if (reportTestCases) std::cout << "  Subnormal factor...\n";
		{
			double subn = std::ldexp(1.0, -1050);
			if (subn != 0.0) {
				ereal<16> one(1.0);
				ereal<16> s(subn);
				if (one * s != s) {
					if (reportTestCases) std::cout << "    FAIL 1 * subnormal != subnormal\n";
					++nrOfFailedTestCases;
				}
			}
		}

		// --- IEEE 754 special values (fixed by #966) ---
		// The sign of a product is signbit(a) XOR signbit(b), including for
		// zero and infinite results. Inf * 0 is NaN.

		double qnan = std::numeric_limits<double>::quiet_NaN();
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;

		// NaN * finite = NaN (both orderings)
		if (reportTestCases) std::cout << "  NaN * finite (both orderings)...\n";
		{
			ereal<16> a(2.0);
			ereal<16> n(qnan);
			if (!std::isnan(double(a * n))) {
				if (reportTestCases) std::cout << "    FAIL a * NaN\n"; ++nrOfFailedTestCases;
			}
			if (!std::isnan(double(n * a))) {
				if (reportTestCases) std::cout << "    FAIL NaN * a\n"; ++nrOfFailedTestCases;
			}
		}

		// Inf * finite-nonzero = signed Inf (sign = XOR)
		if (reportTestCases) std::cout << "  Inf * finite-nonzero (sign = XOR)...\n";
		{
			ereal<16> two(2.0);
			ereal<16> negtwo(-2.0);
			ereal<16> pos(pinf);
			ereal<16> neg(ninf);
			double r1 = double(two * pos);     // +Inf
			if (!std::isinf(r1) || r1 < 0) {
				if (reportTestCases) std::cout << "    FAIL 2 * +Inf got " << r1 << '\n'; ++nrOfFailedTestCases;
			}
			double r2 = double(negtwo * pos);  // -Inf
			if (!std::isinf(r2) || r2 > 0) {
				if (reportTestCases) std::cout << "    FAIL -2 * +Inf got " << r2 << '\n'; ++nrOfFailedTestCases;
			}
			double r3 = double(two * neg);     // -Inf
			if (!std::isinf(r3) || r3 > 0) {
				if (reportTestCases) std::cout << "    FAIL 2 * -Inf got " << r3 << '\n'; ++nrOfFailedTestCases;
			}
		}

		// Inf * Inf = signed Inf (sign = XOR)
		if (reportTestCases) std::cout << "  Inf * Inf (sign = XOR)...\n";
		{
			ereal<16> pos(pinf);
			ereal<16> neg(ninf);
			double r1 = double(pos * pos);  // +Inf
			if (!std::isinf(r1) || r1 < 0) {
				if (reportTestCases) std::cout << "    FAIL +Inf * +Inf got " << r1 << '\n'; ++nrOfFailedTestCases;
			}
			double r2 = double(pos * neg);  // -Inf
			if (!std::isinf(r2) || r2 > 0) {
				if (reportTestCases) std::cout << "    FAIL +Inf * -Inf got " << r2 << '\n'; ++nrOfFailedTestCases;
			}
			double r3 = double(neg * neg);  // +Inf
			if (!std::isinf(r3) || r3 < 0) {
				if (reportTestCases) std::cout << "    FAIL -Inf * -Inf got " << r3 << '\n'; ++nrOfFailedTestCases;
			}
		}

		// Inf * 0 = NaN (both orderings)
		if (reportTestCases) std::cout << "  Inf * 0 = NaN (both orderings)...\n";
		{
			ereal<16> inf(pinf);
			ereal<16> zero(0.0);
			if (!std::isnan(double(inf * zero))) {
				if (reportTestCases) std::cout << "    FAIL Inf * 0\n"; ++nrOfFailedTestCases;
			}
			if (!std::isnan(double(zero * inf))) {
				if (reportTestCases) std::cout << "    FAIL 0 * Inf\n"; ++nrOfFailedTestCases;
			}
		}

		// Signed-zero products: sign = signbit(a) XOR signbit(b)
		// finite * -0 carries the sign through
		if (reportTestCases) std::cout << "  finite * -0 = -0...\n";
		{
			ereal<16> a(3.0);
			ereal<16> n(-0.0);
			double r = double(a * n);
			if (r != 0.0 || !std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n'; ++nrOfFailedTestCases;
			}
		}

		// +0 * +0 = +0
		if (reportTestCases) std::cout << "  +0 * +0 = +0...\n";
		{
			ereal<16> p(0.0);
			ereal<16> q(0.0);
			double r = double(p * q);
			if (r != 0.0 || std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n'; ++nrOfFailedTestCases;
			}
		}

		// +0 * -0 = -0  and  -0 * +0 = -0
		if (reportTestCases) std::cout << "  +0 * -0 = -0, -0 * +0 = -0...\n";
		{
			ereal<16> p(0.0);
			ereal<16> n(-0.0);
			double r1 = double(p * n);
			if (r1 != 0.0 || !std::signbit(r1)) {
				if (reportTestCases) std::cout << "    FAIL +0 * -0 signbit=" << std::signbit(r1) << '\n'; ++nrOfFailedTestCases;
			}
			double r2 = double(n * p);
			if (r2 != 0.0 || !std::signbit(r2)) {
				if (reportTestCases) std::cout << "    FAIL -0 * +0 signbit=" << std::signbit(r2) << '\n'; ++nrOfFailedTestCases;
			}
		}

		// -0 * -0 = +0
		if (reportTestCases) std::cout << "  -0 * -0 = +0...\n";
		{
			ereal<16> n(-0.0);
			ereal<16> m(-0.0);
			double r = double(n * m);
			if (r != 0.0 || std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n'; ++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

	// =========================================================================
	// Fuzzer infrastructure
	// =========================================================================

	// Generate a multi-component ereal expansion of varying length, signs, and
	// exponent ranges. The expansion is built by adding random values at
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

	// Fuzz the bit-exact multiplication invariants (identity, annihilation, sign
	// rules, in-place parity) plus tolerance-based commutativity over N random
	// multi-component pairs. Fixed seed for reproducibility; failing (seed, iter)
	// pairs let a tester reconstruct the exact case.
	int VerifyErealMultiplication_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		ereal<16> zero(0.0);
		ereal<16> one(1.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<16> a = random_ereal<16>(rng);
			ereal<16> b = random_ereal<16>(rng);

			// Identity: a * 1 == a
			if (a * one != a) {
				if (reportTestCases) std::cout << "    FAIL identity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Annihilation: a * 0 == 0
			if (!(a * zero).iszero()) {
				if (reportTestCases) std::cout << "    FAIL annihilation (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Sign rule: (-a) * b == -(a * b)
			if ((-a) * b != -(a * b)) {
				if (reportTestCases) std::cout << "    FAIL sign (-a)*b (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Sign rule: (-a) * (-b) == a * b
			if ((-a) * (-b) != a * b) {
				if (reportTestCases) std::cout << "    FAIL sign (-a)*(-b) (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// In-place parity: (a *= b) == (a * b)
			ereal<16> inplace(a);
			inplace *= b;
			if (inplace != (a * b)) {
				if (reportTestCases) std::cout << "    FAIL in-place *= parity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Commutativity (tolerance): a * b ~= b * a. Skip non-finite
			// projections (random magnitudes can overflow double on multiply).
			double p = double(a * b);
			if (std::isfinite(p) && !close_rel(a * b, b * a, 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL commutativity (seed=0x"
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
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "ereal multiplication";
	std::string test_tag            = "multiplication";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErealMultiplication(reportTestCases), "ereal", "multiplication manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealMultiplication(reportTestCases), "ereal", "multiplication foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyErealMultiplication_Fuzz(reportTestCases, 1000), "ereal", "multiplication fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyErealMultiplication_Fuzz(reportTestCases, 10000), "ereal", "multiplication fuzz x10k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyErealMultiplication_Fuzz(reportTestCases, 100000), "ereal", "multiplication fuzz x100k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyErealMultiplication_Fuzz(reportTestCases, 1000000), "ereal", "multiplication fuzz x1M");
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
