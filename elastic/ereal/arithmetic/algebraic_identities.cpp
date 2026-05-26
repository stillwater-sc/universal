// algebraic_identities.cpp: regression tests for ereal CROSS-operator identities.
//
// This file is the deliberate home for identities that mix more than one
// operator (distributivity, additive/multiplicative inverse round trips). The
// single-operator algebraic invariants (commutativity, associativity,
// anti-commutativity, self-quotient, ...) live in the per-operator files
// addition.cpp / subtraction.cpp / multiplication.cpp / division.cpp, which
// enforce the "one file = one operator" rule. Cross-operator identities cannot
// honor that rule by construction, so they are collected here instead.
//
// Renamed from identities.cpp and brought up to the standard framework as part
// of issue #948.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated cross-operator identities +
//              small fuzz.
//   LEVEL 2 -- property-based fuzzer (~10,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~1,000,000 iterations).
//
// Exact vs. tolerance: the error-free transforms for + and - make the additive
// identities bit-exact; the Newton-reciprocal division and O(m*n) product make
// the multiplicative/distributive identities exact only up to a relative
// tolerance, so those are checked on the projected double.
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

namespace {

	// Compare two ereal values via their double projection within a relative
	// tolerance. Used for identities that hold mathematically but carry a
	// Newton-reciprocal or O(m*n)-product residual (distributivity, the
	// multiplicative and division inverse round trips).
	bool close_rel(const sw::universal::ereal<16>& x,
	               const sw::universal::ereal<16>& y,
	               double relTol, double absTol = 1.0e-15) {
		double a = double(x), b = double(y);
		double diff = std::abs(a - b);
		if (diff == 0.0) return true;
		double scale = std::max(std::abs(a), std::abs(b));
		// Absolute floor so a near-zero result (collapsing scale, e.g. when a
		// fuzz case drives b+c to cancel) does not make a pure relative
		// tolerance spuriously strict.
		return diff <= std::max(absTol, relTol * scale);
	}

	// =========================================================================
	// LEVEL 1: foundational hand-curated cross-operator identities
	// =========================================================================
	int VerifyErealAlgebraicIdentities(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Additive identities (error-free transforms -> exact) ---

		// Additive recovery: (a + b) - a == b exactly.
		if (reportTestCases) std::cout << "  Additive recovery (a+b)-a == b...\n";
		{
			ereal<16> a(1.0); a += 1.0e-15; a += 1.0e-30;
			ereal<16> b(3.14159); b += 1.0e-12;
			if ((a + b) - a != b) {
				if (reportTestCases) std::cout << "    FAIL (a+b)-a != b\n";
				++nrOfFailedTestCases;
			}
		}

		// Additive inverse round trip: (a - b) + b == a exactly.
		if (reportTestCases) std::cout << "  Additive inverse (a-b)+b == a...\n";
		{
			ereal<16> a(15.5); a += 1.0e-14;
			ereal<16> b(7.25); b += 1.0e-16;
			if ((a - b) + b != a) {
				if (reportTestCases) std::cout << "    FAIL (a-b)+b != a\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Distributivity (cross-operator * over +) ---

		// Small integers: a*(b+c) == a*b + a*c exactly (all products exact).
		if (reportTestCases) std::cout << "  Distributivity (exact small int)...\n";
		{
			ereal<16> a(2.0);
			ereal<16> b(3.0);
			ereal<16> c(5.0);
			if (a * (b + c) != (a * b) + (a * c)) {
				if (reportTestCases) std::cout << "    FAIL 2*(3+5) != 2*3+2*5\n";
				++nrOfFailedTestCases;
			}
		}

		// Multi-limb: a*(b+c) ~= a*b + a*c within tolerance (product residual).
		if (reportTestCases) std::cout << "  Distributivity (multi-limb)...\n";
		{
			ereal<16> a(1.5);  a += 1.0e-12;
			ereal<16> b(2.3);  b += 1.0e-13;
			ereal<16> c(4.7);  c += 1.0e-14;
			if (!close_rel(a * (b + c), (a * b) + (a * c), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL a*(b+c) != a*b+a*c\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Multiplicative / division inverse (Newton residual -> tolerance) ---

		// Multiplicative inverse: a * (1/a) ~= 1.
		if (reportTestCases) std::cout << "  Multiplicative inverse a*(1/a) == 1...\n";
		{
			ereal<16> one(1.0);
			for (double v : {3.0, 7.0, 1.0e10}) {
				ereal<16> a(v);
				ereal<16> recip = one / a;
				if (!close_rel(a * recip, one, 1.0e-14)) {
					if (reportTestCases) std::cout << "    FAIL a*(1/a) != 1 for a=" << v << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// Division inverse: (a / b) * b ~= a.
		if (reportTestCases) std::cout << "  Division inverse (a/b)*b == a...\n";
		{
			ereal<16> a(15.5); a += 1.0e-13;
			ereal<16> b(3.5);  b += 1.0e-15;
			if (!close_rel((a / b) * b, a, 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL (a/b)*b != a\n";
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

	// =========================================================================
	// Fuzzer infrastructure
	// =========================================================================

	// Generate a multi-component ereal expansion of varying length, signs, and
	// exponent ranges. Leading magnitude ~1e2 keeps a*(b+c) and a*(1/a) well
	// away from overflow.
	template <unsigned maxlimbs>
	sw::universal::ereal<maxlimbs>
	random_ereal(std::mt19937_64& rng, double leading_magnitude = 1e2) {
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
			mag = std::ldexp(mag, -50);
			if (mag < std::ldexp(1.0, -950)) break;
		}
		return result;
	}

	// Fuzz the cross-operator identities over random expansions: additive
	// recovery and inverse round trips are exact; distributivity and the
	// multiplicative inverse are tolerance-based. Fixed seed for reproducibility.
	int VerifyErealAlgebraicIdentities_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		ereal<16> one(1.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<16> a = random_ereal<16>(rng);
			ereal<16> b = random_ereal<16>(rng);
			ereal<16> c = random_ereal<16>(rng);

			// Additive recovery (exact): (a + b) - a == b
			if ((a + b) - a != b) {
				if (reportTestCases) std::cout << "    FAIL additive recovery (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Additive inverse round trip (exact): (a - b) + b == a
			if ((a - b) + b != a) {
				if (reportTestCases) std::cout << "    FAIL additive inverse (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Distributivity (tolerance): a*(b+c) ~= a*b + a*c
			double p = double(a * (b + c));
			if (std::isfinite(p) && !close_rel(a * (b + c), (a * b) + (a * c), 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL distributivity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Multiplicative inverse (tolerance): a * (1/a) ~= 1. Skip zero a.
			if (!a.iszero() && !close_rel(a * (one / a), one, 1.0e-11)) {
				if (reportTestCases) std::cout << "    FAIL multiplicative inverse (seed=0x"
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

	std::string test_suite          = "ereal algebraic identities";
	std::string test_tag            = "algebraic identities";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErealAlgebraicIdentities(reportTestCases), "ereal", "algebraic identities manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealAlgebraicIdentities(reportTestCases), "ereal", "algebraic identities foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyErealAlgebraicIdentities_Fuzz(reportTestCases, 100), "ereal", "algebraic identities fuzz x100");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyErealAlgebraicIdentities_Fuzz(reportTestCases, 1000), "ereal", "algebraic identities fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyErealAlgebraicIdentities_Fuzz(reportTestCases, 10000), "ereal", "algebraic identities fuzz x10k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyErealAlgebraicIdentities_Fuzz(reportTestCases, 100000), "ereal", "algebraic identities fuzz x100k");
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
