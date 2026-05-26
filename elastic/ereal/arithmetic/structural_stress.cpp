// structural_stress.cpp: structural stress test of Priest normal form (#954, D3).
//
// First-principles structural oracle (no external reference): every result of
// +, -, *, / on an ereal expansion must be in Priest's normal form
// (decreasing magnitude, non-overlapping limbs, no interior zeros). D1 proved
// this on random multi-limb fuzz inputs; D3 stresses the boundaries of the
// double exponent range and the rounding/representation edge cases that fuzz
// inputs sampled near magnitude 1e6 never exercise:
//
//   - magnitude sweep:        leading magnitudes from 2^-900 to 2^900, so
//                             expansions straddle the whole normal-double range
//                             (and renormalisation drops any sub-DBL_MIN tail).
//   - subnormal boundary:     trailing limbs near DBL_MIN / DBL_TRUE_MIN.
//   - round-to-even ties:     pairs whose sum/difference lands exactly on a
//                             representable midpoint (two_sum tie handling).
//   - near maxpos / near 0:   operands at the top and bottom of the range,
//                             where products and quotients span the most
//                             binary exponents (the #981 regression surface).
//
// REGRESSION_LEVEL convention:
//   LEVEL 1 -- one pass of every generator.
//   LEVEL 2 -- 10x random reps per generator.   LEVEL 3 -- 100x.   LEVEL 4 -- 1000x.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <cmath>
#include <random>
#include <vector>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_test_support.hpp>

namespace {

	using namespace sw::universal;

	// Assert that every arithmetic result on the pair (a, b) is Priest-normal.
	// Returns the number of violations found; emits a localized diagnostic that
	// names the operator, the broken invariant, and the offending limb index.
	int CheckAllOps(const ereal<16>& a, const ereal<16>& b,
	                const char* context, bool reportTestCases) {
		int fails = 0;
		auto probe = [&](const char* op, const ereal<16>& r) {
			// The magnitude sweep deliberately reaches 2^900; sums/products there
			// overflow to inf/nan, which is expected, not a normal-form violation.
			// Only assert Priest-normal form on finite (representable) results.
			if (!std::isfinite(double(r))) return;
			if (auto pn = check_priest_normal(r); !pn.ok()) {
				if (reportTestCases) std::cout << "    FAIL priest-normal " << op
					<< " [" << context << "]: " << pn.what()
					<< " at limb " << pn.index << " (result has "
					<< r.limbs().size() << " limbs)\n";
				++fails;
			}
		};
		probe("a+b", a + b);
		probe("a-b", a - b);
		probe("a*b", a * b);
		if (!b.iszero()) probe("a/b", a / b);
		return fails;
	}

	// Build a multi-limb expansion at a chosen leading magnitude by accumulating
	// random contributions ~50 binary exponents apart, mirroring the fuzz
	// generator but with a caller-controlled scale.
	ereal<16> expansion_at(std::mt19937_64& rng, double leading_magnitude, unsigned n_limbs) {
		std::uniform_int_distribution<int> sign_dist(0, 1);
		std::uniform_real_distribution<double> unit(0.5, 2.0);
		ereal<16> result(0.0);
		double mag = leading_magnitude;
		for (unsigned i = 0; i < n_limbs; ++i) {
			result += unit(rng) * mag * (sign_dist(rng) ? 1.0 : -1.0);
			mag = std::ldexp(mag, -50);
			if (mag < std::ldexp(1.0, -1060)) break;  // below DBL_TRUE_MIN tail
		}
		return result;
	}

	// Sweep leading magnitudes across the whole normal-double range, in 2^50
	// steps, exercising +,-,*,/ at each scale and against a fixed mid-range
	// operand (so products/quotients span the widest exponent gaps).
	int VerifyMagnitudeSweep(bool reportTestCases, unsigned reps) {
		int fails = 0;
		std::mt19937_64 rng(0x5'7E55'00D5ULL);
		for (unsigned r = 0; r < reps; ++r) {
			for (int e = -900; e <= 900; e += 50) {
				double mag = std::ldexp(1.0, e);
				ereal<16> a = expansion_at(rng, mag, 6);
				ereal<16> b = expansion_at(rng, std::ldexp(1.0, -e), 6);  // reciprocal scale
				fails += CheckAllOps(a, b, "sweep", reportTestCases);
				ereal<16> mid = expansion_at(rng, 1.0, 4);
				fails += CheckAllOps(a, mid, "sweep-vs-mid", reportTestCases);
			}
		}
		return fails;
	}

	// Trailing limbs pushed against the subnormal boundary: a normal leading
	// limb with a tail near DBL_MIN and DBL_TRUE_MIN. Renormalisation must
	// either keep them non-overlapping or drop them; either way the result is
	// Priest-normal.
	int VerifySubnormalBoundary(bool reportTestCases, unsigned reps) {
		int fails = 0;
		std::mt19937_64 rng(0xDEAD'5EEDULL);
		std::uniform_real_distribution<double> unit(0.5, 2.0);
		const double tails[] = { DBL_MIN, std::ldexp(DBL_MIN, 1), DBL_TRUE_MIN,
		                         std::ldexp(DBL_TRUE_MIN, 4), std::ldexp(1.0, -1040) };
		for (unsigned r = 0; r < reps; ++r) {
			for (double lead : { 1.0, 1e6, std::ldexp(1.0, -200) }) {
				for (double tail : tails) {
					ereal<16> a(0.0);
					a += unit(rng) * lead;
					a += tail;
					ereal<16> b(0.0);
					b += unit(rng) * lead;
					b += std::ldexp(tail, 1);
					fails += CheckAllOps(a, b, "subnormal-tail", reportTestCases);
				}
			}
		}
		return fails;
	}

	// Round-to-even ties: pairs whose sum lands exactly on a representable
	// midpoint, stressing two_sum's error term. p + (p + ulp) and p - (ulp/2)
	// style constructions across a range of leading exponents.
	int VerifyRoundToEvenTies(bool reportTestCases, unsigned reps) {
		int fails = 0;
		for (unsigned r = 0; r < reps; ++r) {
			for (int e = -200; e <= 200; e += 25) {
				double p = std::ldexp(1.0, e);
				double ulp = std::ldexp(1.0, e - 52);
				ereal<16> a(p);
				a += ulp;                 // p + 1 ulp
				ereal<16> b(p);
				b += std::ldexp(ulp, -1); // p + 1/2 ulp (exact tie target)
				fails += CheckAllOps(a, b, "tie", reportTestCases);
				ereal<16> c(p);
				c += -std::ldexp(ulp, -1);
				fails += CheckAllOps(a, c, "tie-neg", reportTestCases);
			}
		}
		return fails;
	}

	// Operands near maxpos and near zero: products span almost the entire
	// exponent range, the surface that exposed #981 (unrenormalized products).
	int VerifyNearExtremes(bool reportTestCases, unsigned reps) {
		int fails = 0;
		std::mt19937_64 rng(0xBEEF'CAFEULL);
		std::uniform_real_distribution<double> unit(0.5, 2.0);
		ereal<16> big   = ereal<16>(SpecificValue::maxpos);
		ereal<16> tiny  = ereal<16>(SpecificValue::minpos);
		for (unsigned r = 0; r < reps; ++r) {
			ereal<16> near_big(0.0);
			near_big += std::ldexp(1.0, 900);
			near_big += unit(rng) * std::ldexp(1.0, 850);
			ereal<16> near_zero(0.0);
			near_zero += std::ldexp(1.0, -900);
			near_zero += unit(rng) * std::ldexp(1.0, -950);
			fails += CheckAllOps(near_big, near_zero, "big-x-tiny", reportTestCases);
			fails += CheckAllOps(near_big, big, "near-maxpos", reportTestCases);
			fails += CheckAllOps(near_zero, tiny, "near-minpos", reportTestCases);
		}
		return fails;
	}

	int RunAll(bool reportTestCases, unsigned reps) {
		int fails = 0;
		fails += VerifyMagnitudeSweep(reportTestCases, reps);
		fails += VerifySubnormalBoundary(reportTestCases, reps);
		fails += VerifyRoundToEvenTies(reportTestCases, reps);
		fails += VerifyNearExtremes(reportTestCases, reps);
		return fails;
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

	std::string test_suite          = "ereal structural stress (Priest normal form)";
	std::string test_tag            = "structural stress";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(RunAll(reportTestCases, 1), "ereal", "structural stress manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(RunAll(reportTestCases, 1), "ereal", "structural stress x1");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(RunAll(reportTestCases, 10), "ereal", "structural stress x10");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(RunAll(reportTestCases, 100), "ereal", "structural stress x100");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(RunAll(reportTestCases, 1000), "ereal", "structural stress x1000");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
