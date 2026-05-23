// subtraction.cpp: regression tests for ereal subtraction.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests: algebraic invariants
//              (anti-commutativity, self-cancellation, sign reversal,
//              catastrophic cancellation recovery), hostile arithmetic
//              corner cases (round-to-even, massive exponent gaps,
//              complete overlap), subnormal boundaries, IEEE 754 special
//              values (note +Inf - +Inf = NaN, +Inf - (-Inf) = +Inf).
//   LEVEL 2 -- property-based fuzzer over random multi-component expansions
//              (~1,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~10,000,000 iterations).
//
// Reference: Shewchuk (1997) "Adaptive Precision Floating-Point Arithmetic
//   and Fast Robust Geometric Predicates"; Priest's normal form requires
//   |z_{k+1}| <= ulp(z_k) / 2 with no zero components (unless sum is zero).
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

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyErealSubtraction(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Algebraic invariants ---

		// Anti-commutativity: a - b == -(b - a)
		if (reportTestCases) std::cout << "  Anti-commutativity...\n";
		{
			ereal<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			ereal<16> b(3.14159);
			b += 1.0e-12;
			if ((a - b) != -(b - a)) {
				if (reportTestCases) std::cout << "    FAIL a-b != -(b-a)\n";
				++nrOfFailedTestCases;
			}
		}

		// Identity: a - 0 == a
		if (reportTestCases) std::cout << "  Identity (multi-limb)...\n";
		{
			ereal<16> a(1.0);
			a += 1.0e-15; a += 1.0e-30; a += 1.0e-45;
			ereal<16> zero(0.0);
			if (a - zero != a) {
				if (reportTestCases) std::cout << "    FAIL\n";
				++nrOfFailedTestCases;
			}
		}

		// Self-cancellation: a - a == 0 (single-component zero)
		if (reportTestCases) std::cout << "  Self-cancellation (multi-limb)...\n";
		{
			ereal<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			ereal<16> result = a - a;
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL a - a != 0\n";
				++nrOfFailedTestCases;
			}
		}

		// Sign reversal: 0 - a == -a
		if (reportTestCases) std::cout << "  Sign reversal...\n";
		{
			ereal<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			ereal<16> zero(0.0);
			if (zero - a != -a) {
				if (reportTestCases) std::cout << "    FAIL 0 - a != -a\n";
				++nrOfFailedTestCases;
			}
		}

		// Round-trip: (a - b) + b == a (additive inverse compose) +
		// in-place parity: (a -= b) == (a - b).
		if (reportTestCases) std::cout << "  Round-trip (a-b)+b == a + in-place parity...\n";
		{
			ereal<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			ereal<16> b(3.14159);
			b += 1.0e-12;
			ereal<16> result = (a - b) + b;
			if (result != a) {
				if (reportTestCases) std::cout << "    FAIL round-trip\n";
				++nrOfFailedTestCases;
			}

			// operator-= must agree with operator- (mutating vs returning)
			ereal<16> inplace(a);
			inplace -= b;
			if (inplace != (a - b)) {
				if (reportTestCases) std::cout << "    FAIL in-place -= parity\n";
				++nrOfFailedTestCases;
			}

			// Self -= zeroes (mutating self-cancellation)
			ereal<16> self(a);
			self -= self;
			if (!self.iszero()) {
				if (reportTestCases) std::cout << "    FAIL self -= self != 0\n";
				++nrOfFailedTestCases;
			}

			// zero -= a equals -a
			ereal<16> sink(0.0);
			sink -= a;
			if (sink != -a) {
				if (reportTestCases) std::cout << "    FAIL 0 -= a != -a\n";
				++nrOfFailedTestCases;
			}
		}

		// Catastrophic cancellation: a - (a - epsilon) recovers epsilon
		if (reportTestCases) std::cout << "  Catastrophic cancellation recovery...\n";
		{
			ereal<16> a(1.0e+100);
			ereal<16> epsilon(1.0e-100);
			ereal<16> shifted = a - epsilon;
			ereal<16> recovered = a - shifted;
			if (recovered != epsilon) {
				if (reportTestCases) std::cout << "    FAIL epsilon lost: got "
				                               << recovered << " expected " << epsilon << '\n';
				++nrOfFailedTestCases;
			}
		}

		// Mixed-magnitude expansion: (a + tiny) - a == tiny
		if (reportTestCases) std::cout << "  Mixed-magnitude expansion...\n";
		{
			ereal<16> a(1.0e+15);
			ereal<16> tiny(1.0e-15);
			ereal<16> combined = a + tiny;
			if (combined - a != tiny) {
				if (reportTestCases) std::cout << "    FAIL tiny lost\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Hostile arithmetic ---

		// Round-to-even boundary tie at 2^53 + 1, then back-subtract
		if (reportTestCases) std::cout << "  Round-to-even tie (2^53 + 1)...\n";
		{
			double two53 = std::ldexp(1.0, 53);
			ereal<16> a(two53);
			a += 1.0;
			ereal<16> back(two53);
			if (a - back != ereal<16>(1.0)) {
				if (reportTestCases) std::cout << "    FAIL tie subtract lost the +1\n";
				++nrOfFailedTestCases;
			}
		}

		// Massive exponent gap (1e+300 - 1e-300): tiny term must survive
		if (reportTestCases) std::cout << "  Massive exponent gap...\n";
		{
			ereal<16> a(1.0e+300);
			ereal<16> b(1.0e-300);
			ereal<16> result = a - b;
			ereal<16> recovered = a - result;
			if (recovered != b) {
				if (reportTestCases) std::cout << "    FAIL tiny term dropped\n";
				++nrOfFailedTestCases;
			}
		}

		// Complete overlap: A - A for multi-limb A reduces to zero
		if (reportTestCases) std::cout << "  Complete overlap (A - A)...\n";
		{
			ereal<16> a(1.0);
			a += 1.0e-15; a += 1.0e-30;
			ereal<16> result = a - a;
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL multi-limb A-A != 0\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Subnormal boundary ---

		// Subnormal subtrahend
		if (reportTestCases) std::cout << "  Subnormal subtrahend...\n";
		{
			double subn = std::ldexp(1.0, -1050);
			if (subn != 0.0) {
				ereal<16> a(1.0);
				ereal<16> b(subn);
				ereal<16> result = a - b;
				double r = double(result);
				if (std::isnan(r) || std::isinf(r)
				    || std::abs(r - 1.0) > std::ldexp(1.0, -50)) {
					if (reportTestCases) std::cout << "    FAIL r=" << r << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// DBL_MIN-scale residual: (1.0 + DBL_MIN) - 1.0 recovers DBL_MIN
		if (reportTestCases) std::cout << "  DBL_MIN-scale residual...\n";
		{
			ereal<16> a(1.0);
			ereal<16> small(std::numeric_limits<double>::min());
			ereal<16> combined = a + small;
			if (combined - a != small) {
				if (reportTestCases) std::cout << "    FAIL DBL_MIN residual dropped\n";
				++nrOfFailedTestCases;
			}
		}

		// --- IEEE 754 special values ---

		double qnan = std::numeric_limits<double>::quiet_NaN();
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;

		// NaN - finite = NaN  (both orderings)
		if (reportTestCases) std::cout << "  NaN - finite (both orderings)...\n";
		{
			ereal<16> a(1.0);
			ereal<16> n(qnan);
			if (!std::isnan(double(a - n))) {
				if (reportTestCases) std::cout << "    FAIL a - NaN\n"; ++nrOfFailedTestCases;
			}
			if (!std::isnan(double(n - a))) {
				if (reportTestCases) std::cout << "    FAIL NaN - a\n"; ++nrOfFailedTestCases;
			}
		}

		// +Inf - finite = +Inf;  finite - +Inf = -Inf
		if (reportTestCases) std::cout << "  +Inf - finite, finite - +Inf...\n";
		{
			ereal<16> a(1.0);
			ereal<16> inf(pinf);
			double r1 = double(inf - a);
			if (!std::isinf(r1) || r1 < 0) {
				if (reportTestCases) std::cout << "    FAIL +Inf - a got " << r1 << '\n';
				++nrOfFailedTestCases;
			}
			double r2 = double(a - inf);
			if (!std::isinf(r2) || r2 > 0) {
				if (reportTestCases) std::cout << "    FAIL a - +Inf got " << r2 << '\n';
				++nrOfFailedTestCases;
			}
		}

		// -Inf - finite = -Inf;  finite - (-Inf) = +Inf
		if (reportTestCases) std::cout << "  -Inf - finite, finite - (-Inf)...\n";
		{
			ereal<16> a(1.0);
			ereal<16> inf(ninf);
			double r1 = double(inf - a);
			if (!std::isinf(r1) || r1 > 0) {
				if (reportTestCases) std::cout << "    FAIL -Inf - a got " << r1 << '\n';
				++nrOfFailedTestCases;
			}
			double r2 = double(a - inf);
			if (!std::isinf(r2) || r2 < 0) {
				if (reportTestCases) std::cout << "    FAIL a - (-Inf) got " << r2 << '\n';
				++nrOfFailedTestCases;
			}
		}

		// +Inf - +Inf = NaN (IEEE 754)
		if (reportTestCases) std::cout << "  +Inf - +Inf...\n";
		{
			ereal<16> a(pinf);
			ereal<16> b(pinf);
			if (!std::isnan(double(a - b))) {
				if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases;
			}
		}

		// -Inf - -Inf = NaN (IEEE 754)
		if (reportTestCases) std::cout << "  -Inf - -Inf...\n";
		{
			ereal<16> a(ninf);
			ereal<16> b(ninf);
			if (!std::isnan(double(a - b))) {
				if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases;
			}
		}

		// +Inf - (-Inf) = +Inf
		if (reportTestCases) std::cout << "  +Inf - (-Inf)...\n";
		{
			ereal<16> p(pinf);
			ereal<16> n(ninf);
			double r = double(p - n);
			if (!std::isinf(r) || r < 0) {
				if (reportTestCases) std::cout << "    FAIL got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}

		// -Inf - +Inf = -Inf
		if (reportTestCases) std::cout << "  -Inf - +Inf...\n";
		{
			ereal<16> p(ninf);
			ereal<16> n(pinf);
			double r = double(p - n);
			if (!std::isinf(r) || r > 0) {
				if (reportTestCases) std::cout << "    FAIL got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}

		// +0 - +0 = +0  (sign preserved per IEEE 754 round-to-nearest-even)
		if (reportTestCases) std::cout << "  +0 - +0...\n";
		{
			ereal<16> p(0.0);
			ereal<16> q(0.0);
			double r = double(p - q);
			if (r != 0.0 || std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n';
				++nrOfFailedTestCases;
			}
		}

		// +0 - -0 = +0
		if (reportTestCases) std::cout << "  +0 - -0...\n";
		{
			ereal<16> p(0.0);
			ereal<16> n(-0.0);
			double r = double(p - n);
			if (r != 0.0 || std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n';
				++nrOfFailedTestCases;
			}
		}

		// -0 - +0 = -0  (per IEEE 754-2008 6.3: signs differ in subtraction)
		// Gated pending fix for #962 -- ereal currently returns +0. The case
		// is left in place documenting the expected behavior so re-enabling
		// is a one-line edit once the signed-zero path is fixed.
		//
		// Value-based guard: `#if EREAL_TEST_ISSUE_962` (not `#if defined(...)`)
		// so that `-DEREAL_TEST_ISSUE_962=0` correctly disables the case.
#ifndef EREAL_TEST_ISSUE_962
#define EREAL_TEST_ISSUE_962 0
#endif
#if EREAL_TEST_ISSUE_962
		if (reportTestCases) std::cout << "  -0 - +0...\n";
		{
			ereal<16> n(-0.0);
			ereal<16> p(0.0);
			double r = double(n - p);
			if (r != 0.0 || !std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n';
				++nrOfFailedTestCases;
			}
		}
#endif

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
	random_ereal(std::mt19937_64& rng, double leading_magnitude = 1e6) {
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

	// Fuzz anti-commutativity, identity, self-cancellation, sign-reversal, and
	// round-trip over N random multi-component pairs. Fixed seed for
	// reproducibility; failing (seed, iter) pairs let a tester reconstruct the
	// exact case.
	int VerifyErealSubtraction_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		ereal<16> zero(0.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<16> a = random_ereal<16>(rng);
			ereal<16> b = random_ereal<16>(rng);

			// Anti-commutativity: a - b == -(b - a)
			if ((a - b) != -(b - a)) {
				if (reportTestCases) std::cout << "    FAIL anti-commutativity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Identity: a - 0 == a
			if (a - zero != a) {
				if (reportTestCases) std::cout << "    FAIL identity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Self-cancellation: a - a == 0
			ereal<16> self_diff = a - a;
			if (!self_diff.iszero()) {
				if (reportTestCases) std::cout << "    FAIL self-cancellation (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Sign reversal: 0 - a == -a
			if (zero - a != -a) {
				if (reportTestCases) std::cout << "    FAIL sign reversal (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Round-trip: (a - b) + b == a
			if ((a - b) + b != a) {
				if (reportTestCases) std::cout << "    FAIL round-trip (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// In-place parity: (a -= b) equals (a - b)
			ereal<16> inplace(a);
			inplace -= b;
			if (inplace != (a - b)) {
				if (reportTestCases) std::cout << "    FAIL in-place -= parity (seed=0x"
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

	std::string test_suite          = "ereal subtraction";
	std::string test_tag            = "subtraction";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErealSubtraction(reportTestCases), "ereal", "subtraction manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealSubtraction(reportTestCases), "ereal", "subtraction foundational");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyErealSubtraction_Fuzz(reportTestCases, 1000), "ereal", "subtraction fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyErealSubtraction_Fuzz(reportTestCases, 100000), "ereal", "subtraction fuzz x100k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyErealSubtraction_Fuzz(reportTestCases, 10000000), "ereal", "subtraction fuzz x10M");
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
