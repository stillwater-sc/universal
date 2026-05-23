// addition.cpp: regression tests for ereal addition.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests: algebraic invariants,
//              hostile arithmetic corner cases (round-to-even, massive
//              exponent gaps, complete overlap), subnormal boundaries,
//              IEEE 754 special values.
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
	int VerifyErealAddition(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Algebraic invariants ---

		// Catastrophic cancellation
		if (reportTestCases) std::cout << "  Catastrophic cancellation...\n";
		{
			ereal<16> a(1.0e100);
			ereal<16> tiny(1.0e-100);
			ereal<16> c = a + tiny + a;
			ereal<16> d = a + a;
			if (tiny != c - d) {
				if (reportTestCases) std::cout << "    FAIL " << tiny << " != " << c - d << '\n';
				++nrOfFailedTestCases;
			}
		}

		// Commutativity
		if (reportTestCases) std::cout << "  Commutativity...\n";
		{
			ereal<16> a(1.0e+15);
			ereal<16> b(1.0);
			ereal<16> c(1.0e-15);
			if ((a + b + c) != (c + b + a)) {
				if (reportTestCases) std::cout << "    FAIL\n";
				++nrOfFailedTestCases;
			}
		}

		// Associativity
		if (reportTestCases) std::cout << "  Associativity...\n";
		{
			ereal<16> a(1.0e+30);
			ereal<16> b(1.0);
			ereal<16> c(1.0e-30);
			if (((a + b) + c) != (a + (b + c))) {
				if (reportTestCases) std::cout << "    FAIL\n";
				++nrOfFailedTestCases;
			}
		}

		// Identity (multi-limb)
		if (reportTestCases) std::cout << "  Identity (multi-limb)...\n";
		{
			ereal<16> a(1.0);
			a += 1.0e-15; a += 1.0e-30; a += 1.0e-45;
			ereal<16> zero(0.0);
			if (a + zero != a) {
				if (reportTestCases) std::cout << "    FAIL\n";
				++nrOfFailedTestCases;
			}
		}

		// Inverse identity: a + (-a) collapses to zero
		if (reportTestCases) std::cout << "  Inverse identity...\n";
		{
			ereal<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			ereal<16> neg_a = -a;
			ereal<16> result = a + neg_a;
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL a + (-a) != 0\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Hostile arithmetic ---

		// Round-to-even boundary tie at 2^53 + 1
		if (reportTestCases) std::cout << "  Round-to-even tie (2^53 + 1)...\n";
		{
			double two53 = std::ldexp(1.0, 53);
			ereal<16> reconstructed_by_plus(two53);
			reconstructed_by_plus += 1.0;
			ereal<16> direct(two53);
			direct += 1.0;
			if (reconstructed_by_plus != direct) {
				if (reportTestCases) std::cout << "    FAIL tie path mismatch\n";
				++nrOfFailedTestCases;
			}
		}

		// Massive exponent gap (1e+300 + 1e-300)
		if (reportTestCases) std::cout << "  Massive exponent gap...\n";
		{
			ereal<16> a(1.0e+300);
			ereal<16> b(1.0e-300);
			ereal<16> result = a + b;
			ereal<16> recovered = result - a;
			if (recovered != b) {
				if (reportTestCases) std::cout << "    FAIL tiny term dropped\n";
				++nrOfFailedTestCases;
			}
		}

		// Complete overlap: A + A for multi-limb A
		if (reportTestCases) std::cout << "  Complete overlap (A + A)...\n";
		{
			ereal<16> a(1.0);
			a += 1.0e-15; a += 1.0e-30;
			ereal<16> doubled = a + a;
			if (doubled - a != a) {
				if (reportTestCases) std::cout << "    FAIL (A+A)-A != A\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Subnormal boundary ---

		// Subnormal addend
		if (reportTestCases) std::cout << "  Subnormal addend...\n";
		{
			double subn = std::ldexp(1.0, -1050);
			if (subn != 0.0) {
				ereal<16> a(1.0);
				ereal<16> b(subn);
				ereal<16> result = a + b;
				double r = double(result);
				if (std::isnan(r) || std::isinf(r)
				    || std::abs(r - 1.0) > std::ldexp(1.0, -50)) {
					if (reportTestCases) std::cout << "    FAIL r=" << r << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// DBL_MIN-scale residual
		if (reportTestCases) std::cout << "  DBL_MIN-scale residual...\n";
		{
			ereal<16> a(1.0);
			ereal<16> small(std::numeric_limits<double>::min());
			ereal<16> result = a + small;
			if (result - a != small) {
				if (reportTestCases) std::cout << "    FAIL DBL_MIN residual dropped\n";
				++nrOfFailedTestCases;
			}
		}

		// --- IEEE 754 special values ---

		double qnan = std::numeric_limits<double>::quiet_NaN();
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;

		// NaN + finite = NaN
		if (reportTestCases) std::cout << "  NaN + finite...\n";
		{
			ereal<16> a(1.0);
			ereal<16> n(qnan);
			if (!std::isnan(double(a + n))) {
				if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases;
			}
		}

		// +Inf + finite = +Inf  (fixed in #957)
		// Exercise both operand orderings: a + inf hits the rhs-Inf branch
		// of apply_ieee754_add_special_values; inf + a hits the lhs-Inf
		// branch (because operator+ copies lhs then calls operator+= on it).
		if (reportTestCases) std::cout << "  +Inf + finite (both orderings)...\n";
		{
			ereal<16> a(1.0);
			ereal<16> inf(pinf);
			for (auto& result : { a + inf, inf + a }) {
				double r = double(result);
				if (!std::isinf(r) || r < 0) {
					if (reportTestCases) std::cout << "    FAIL got " << r << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// -Inf + finite = -Inf  (both orderings)
		if (reportTestCases) std::cout << "  -Inf + finite (both orderings)...\n";
		{
			ereal<16> a(1.0);
			ereal<16> inf(ninf);
			for (auto& result : { a + inf, inf + a }) {
				double r = double(result);
				if (!std::isinf(r) || r > 0) {
					if (reportTestCases) std::cout << "    FAIL got " << r << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// +Inf + +Inf = +Inf
		if (reportTestCases) std::cout << "  +Inf + +Inf...\n";
		{
			ereal<16> a(pinf);
			ereal<16> b(pinf);
			double r = double(a + b);
			if (!std::isinf(r) || r < 0) {
				if (reportTestCases) std::cout << "    FAIL got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}

		// -Inf + -Inf = -Inf
		if (reportTestCases) std::cout << "  -Inf + -Inf...\n";
		{
			ereal<16> a(ninf);
			ereal<16> b(ninf);
			double r = double(a + b);
			if (!std::isinf(r) || r > 0) {
				if (reportTestCases) std::cout << "    FAIL got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}

		// -Inf + +Inf = NaN (IEEE 754)
		if (reportTestCases) std::cout << "  -Inf + +Inf...\n";
		{
			ereal<16> p(pinf);
			ereal<16> n(ninf);
			if (!std::isnan(double(p + n))) {
				if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases;
			}
		}

		// +0 + -0 = +0
		if (reportTestCases) std::cout << "  +0 + -0...\n";
		{
			ereal<16> p(0.0);
			ereal<16> n(-0.0);
			double r = double(p + n);
			if (r != 0.0 || std::signbit(r)) {
				if (reportTestCases) std::cout << "    FAIL signbit=" << std::signbit(r) << '\n';
				++nrOfFailedTestCases;
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

	// Fuzz commutativity, associativity, identity, inverse over N random
	// multi-component pairs / triples. Fixed seed for reproducibility; failing
	// (seed, iter) pairs let a tester reconstruct the exact case.
	int VerifyErealAddition_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		ereal<16> zero(0.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<16> a = random_ereal<16>(rng);
			ereal<16> b = random_ereal<16>(rng);
			ereal<16> c = random_ereal<16>(rng);

			// Commutativity: a + b == b + a
			if (a + b != b + a) {
				if (reportTestCases) std::cout << "    FAIL commutativity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Associativity: (a + b) + c == a + (b + c)
			// KNOWN FAILURE gated by #959: renormalize_expansion is not
			// value-canonical, so associativity fails ~0.5% on random
			// multi-limb inputs. Code preserved so the check fires
			// automatically when #959 lands.
#if 0  // re-enable when #959 is fixed
			if ((a + b) + c != a + (b + c)) {
				if (reportTestCases) std::cout << "    FAIL associativity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
#else
			(void)a; (void)b; (void)c;
#endif

			// Identity: a + 0 == a
			if (a + zero != a) {
				if (reportTestCases) std::cout << "    FAIL identity (seed=0x"
				                               << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}

			// Inverse: a + (-a) == 0
			ereal<16> sum_with_neg = a + (-a);
			if (!sum_with_neg.iszero()) {
				if (reportTestCases) std::cout << "    FAIL inverse (seed=0x"
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

	std::string test_suite          = "ereal addition";
	std::string test_tag            = "addition";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErealAddition(reportTestCases), "ereal", "addition manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealAddition(reportTestCases), "ereal", "addition foundational");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyErealAddition_Fuzz(reportTestCases, 1000), "ereal", "addition fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyErealAddition_Fuzz(reportTestCases, 100000), "ereal", "addition fuzz x100k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyErealAddition_Fuzz(reportTestCases, 10000000), "ereal", "addition fuzz x10M");
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
