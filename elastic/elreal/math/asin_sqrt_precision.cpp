// asin_sqrt_precision.cpp: diagnostic + characterization for the sin(asin(x))/
//                          cos(acos(x)) round-trip precision cap (#1076).
//
// This is a DIAGNOSTIC regression test. It carries, under the MANUAL_TESTING
// guard, the instrumented experiments that root-caused why the inverse round-trip
// caps at ~234 digits (issue #1076, the sole remaining Phase-7 / #931 acceptance
// gate), together with their measured results and the assessment below; and under
// the regression path it locks the diagnosis facts so a future change to sqrt /
// the inverse-trig chain / the sin-cos series is noticed.
//
// ============================================================================
// ASSESSMENT (double host, depth 20; measured 2026-07-21)
// ============================================================================
// Symptom: `sin(asin(0.5))` agrees with 0.5 to only ~234 digits, while every
// other transcendental identity reaches 305-320 (e.g. sin(pi/6) itself = 306,
// tan(atan(0.5)) = 320). Gated off in transcendentals_highprecision.cpp via
// ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION 0.
//
// Chain of cause, established by the experiments below:
//
// 1. PREMATURE TERMINATION of the sin Maclaurin series. Replicating
//    detail::sincos_term_stream and logging each term: the asin(0.5) argument
//    drives the stream to EMPTY at term 60, while a clean pi/6 (= pi_zbcl/6)
//    runs to term 76. The dropped terms 60-76 have magnitude 10^-234 .. 10^-306
//    -- losing exactly them is the 234-digit cap.
//
// 2. The terms progressively lose DEEP BLOCKS in the asin chain (value
//    cancellation, not floor truncation). neg_t2 = -t^2 starts EQUAL for both
//    inputs (19 blocks each, agree to 305); mul_online/div_online preserve block
//    count per step; but the asin terms carry fewer and fewer deep blocks
//    (term 40: 8 vs pi6's 10; term 52: 3 vs 7; term 58: 1 vs 5; term 60: EMPTY).
//    A deeper internal recurrence floor (out_floor - extra*k, extra up to 12) has
//    NO effect, confirming the deep blocks cancel in value rather than being cut.
//
// 3. sqrt IS NOT THE BOTTLENECK (sqrt-Newton hypothesis DISPROVEN 2026-07-21).
//    sqrt(0.75) plateaus at 19 blocks, but it is 308-digit ACCURATE there: the
//    Newton iteration converges 33 -> 65 -> 129 -> 260 -> 309 digits, and stays
//    308 for depth 20..40. The 19-block/308-digit plateau is the ~2^-1022
//    double-host precision FLOOR, not an under-resolution -- sqrt physically
//    cannot go deeper on a double host. (div_online inside the Newton step is
//    strictly WORSE: 224 digits. asin/atan inherit sqrt's 308-digit result.)
//    pi_zbcl (odd_power_series, no sqrt) happens to pack pi/6 into 20 blocks at
//    the same floor -- a decomposition difference, not a resolution difference.
//
// 4. It is the DECOMPOSITION, not accuracy or block count -- and the sin SERIES
//    is where the loss happens. A clean pi/6 TRUNCATED to 19 blocks still gives
//    sin = 306, but asin's own 19-block decomposition gives 234. Truncating the
//    argument to k blocks and taking sin:
//        k    14  15  16  17  18  19
//        pi6 231 247 264 280 297 306      (each block adds ~16 digits -- usable)
//        asin 231 234 234 234 234 234      (blocks 15..19 add NOTHING -- unusable)
//    asin's value agrees with pi/6 to 306 and sin is well-conditioned at pi/6, so
//    sin(asin) OUGHT to reach ~306; it reaches 234 because the sin Maclaurin
//    recurrence cannot extract precision from asin's deep-block decomposition
//    (blocks 15..19 are "dead weight"). The bug is in the series machinery's
//    conditioning against a non-prefix deepest block, NOT in sqrt/asin accuracy.
//
// Fix directions RULED OUT: deeper series floor; div_online inside sqrt (worse);
// wider sqrt working depth; and -- now added -- making sqrt "resolve deeper"
// (sqrt is already at the double-host floor and 308-digit accurate). Candidate
// fixes for a follow-up: (a) make the sin/cos series recurrence robust to a
// non-prefix deep tail (the hard, real fix -- e.g. a different summation/EFT that
// does not compound the deep-block error term-to-term); (b) a wider intermediate
// host for the inverse-trig -> sin round-trip so the deep tail sits above the
// float floor with margin. When one lands, flip
// ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION to 1 and update the CHARACTERIZATION
// checks in this file.
// ============================================================================
//
// Cost: the depth-20 diagnostic runs a full asin + sin per experiment (~seconds
// each), so the characterization is gated to REGRESSION_LEVEL_4 and the rich
// MANUAL_TESTING trace is hand-run. Fast tiers do a near no-op.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/trigonometry.hpp>
#include <universal/number/elreal/math/sqrt.hpp>
#include <universal/number/elreal/online_multiply.hpp>
#include <universal/number/elreal/online_divide.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/reference_constants.hpp>

namespace {

using sw::universal::ZBCL;
using sw::universal::block;
using sw::universal::agreed_decimal_digits;
namespace su = sw::universal;

// number of blocks a ZBCL materialises to (bounded pull)
std::size_t block_count(const ZBCL<double>& z, std::size_t cap = 96) { return z.take(cap).size(); }

// mpmath 320-digit reference for sqrt(0.75) = sqrt(3)/2 (exact-double argument).
constexpr const char* s_sqrt_075 =
    "0.86602540378443864676372317075293618347140262690519031402790348972596650845440001854057309337862428783781307070770335151498497254749947623940582775604718682426404661595115279103398741005054233746163250765617163345166144332533612733446091898561352356583018393079400952499326868992969473382517375328802537830917406480305047";

// a clean pi/6 = pi_zbcl / 6 (odd_power_series, no sqrt in the chain)
ZBCL<double> clean_pi6(int depth) {
	return su::div_online(su::pi_zbcl<double>(depth), su::from_native<double>(6.0));
}

// -------------------------------------------------------------------- diagnostics
// [1] Replicate detail::sincos_term_stream and report the term count + the block
//     count of each term, showing the asin chain terminating early. (MANUAL only.)
[[maybe_unused]] int trace_term_stream(const char* tag, ZBCL<double> t, int depth) {
	const int floor_exp = -depth * block<double>::k - 8;
	ZBCL<double> neg_t2 = su::detail::take_while_above(su::negate(su::mul_online(t, t)), floor_exp);
	std::printf("[%s] t blocks=%zu ; neg_t2 blocks=%zu (floor_exp=%d)\n",
	            tag, block_count(t), block_count(neg_t2), floor_exp);
	ZBCL<double> term = t;
	double a = 2.0;
	int n = 0;
	while (n < 100) {
		if (term.is_empty()) { std::printf("[%s] term %d: EMPTY -> stream stops\n", tag, n); break; }
		if (static_cast<int>(term.head().exponent()) < floor_exp) {
			std::printf("[%s] term %d: below floor -> stream stops\n", tag, n); break;
		}
		if (n >= 50 && n % 2 == 0)
			std::printf("[%s] term %2d: headExp=%5d blocks=%zu\n",
			            tag, n, static_cast<int>(term.head().exponent()), block_count(term));
		term = su::detail::take_while_above(
		    su::div_online(su::mul_online(term, neg_t2), su::from_native<double>(a * (a + 1.0))), floor_exp);
		a += 2.0;
		++n;
	}
	std::printf("[%s] total terms = %d\n", tag, n);
	return n;
}

// [2] Block counts along the inverse-trig chain vs depth, exposing the sqrt cap.
[[maybe_unused]] void trace_chain_blocks(int depth) {
	ZBCL<double> x = su::from_native<double>(0.5);
	ZBCL<double> s = su::sqrt(su::add(su::from_native<double>(1.0), su::negate(su::mul_online(x, x))), depth); // sqrt(0.75)
	ZBCL<double> q = su::div_online(x, s);      // 1/sqrt(3)
	ZBCL<double> at = su::atan(q, depth);       // pi/6
	ZBCL<double> as = su::asin(x, depth);
	std::printf("D=%2d: sqrt=%2zu div=%2zu atan=%2zu asin=%2zu | pi_zbcl=%2zu pi6=%2zu\n",
	            depth, block_count(s), block_count(q), block_count(at), block_count(as),
	            block_count(su::pi_zbcl<double>(depth)), block_count(clean_pi6(depth)));
}

// [3] sqrt Newton convergence per iteration: sqrt is 308-digit ACCURATE (converges
//     quadratically), NOT under-resolving -- disproves the sqrt-Newton fix. (MANUAL.)
[[maybe_unused]] void trace_sqrt_newton(int depth) {
	ZBCL<double> a = su::from_native<double>(0.75);
	const block<double> half{ static_cast<double>(0.5), 0 };
	const int iters = 3 + static_cast<int>(std::ceil(std::log2(static_cast<double>(depth) + 1.0)));
	ZBCL<double> x = su::from_native<double>(std::sqrt(su::to_double_approx(a, 2)));
	std::size_t d = 1;
	for (int i = 0; i < iters; ++i) {
		d = (d * 2 < static_cast<std::size_t>(depth)) ? d * 2 : static_cast<std::size_t>(depth);
		x = su::mul_scalar(half, su::add(x, su::div(a, x, d)), d);
		std::printf("    iter %d (d=%2zu): blocks=%2zu accuracy=%3d digits\n",
		            i, d, block_count(x), agreed_decimal_digits(x, s_sqrt_075, 320));
	}
}

// [4] Truncate the argument to k blocks and take sin: pi/6's deep blocks each add
//     ~16 digits (usable), asin's blocks 15..19 add NOTHING (unusable) -- the loss
//     is the series' inability to use asin's deep-block decomposition. (MANUAL.)
[[maybe_unused]] void trace_truncation_sweep(int depth) {
	ZBCL<double> half = su::from_native<double>(0.5);
	ZBCL<double> as = su::asin(half, depth);
	ZBCL<double> p6 = clean_pi6(depth);
	const int hea = static_cast<int>(as.head().exponent());
	const int hep = static_cast<int>(p6.head().exponent());
	std::printf("     k | sin(asin) | sin(pi6)\n");
	for (int k = 14; k <= 20; ++k) {
		ZBCL<double> at = su::detail::take_while_above(as, hea - k * block<double>::k + 1);
		ZBCL<double> pt = su::detail::take_while_above(p6, hep - k * block<double>::k + 1);
		std::printf("    %2d |   %3d     |  %3d\n", k,
		            agreed_decimal_digits(su::sin(at, depth), half, 330),
		            agreed_decimal_digits(su::sin(pt, depth), half, 330));
	}
}

} // anonymous

#define MANUAL_TESTING 0
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
	std::string test_suite = "elreal asin/sqrt round-trip precision diagnostic (#1076)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// ---- full instrumented diagnostic (hand-run: flip MANUAL_TESTING to 1) ----
	const int D = 20;
	ZBCL<double> t_asin = asin(from_native<double>(0.5), D);
	ZBCL<double> t_pi6  = clean_pi6(D);
	std::printf("asin(0.5) vs clean pi/6 agree to %d digits\n\n", agreed_decimal_digits(t_asin, t_pi6, 340));

	std::printf("[1] sin-series term streams (premature termination):\n");
	int na = trace_term_stream("asin", t_asin, D);   // measured: stops at 60
	int np = trace_term_stream("pi6 ", t_pi6,  D);   // measured: stops at 76
	std::printf("    => asin drops terms %d..%d (mag 10^-234..10^-306) = the 234-digit cap\n\n", na, np);

	std::printf("[2] block counts along asin = atan(x/sqrt(1-x^2)) vs depth (sqrt caps at 19):\n");
	for (int d : {16, 20, 24, 32, 40}) trace_chain_blocks(d);
	std::printf("\n");

	std::printf("[3] sqrt Newton convergence (sqrt is 308-digit ACCURATE, not under-resolving):\n");
	trace_sqrt_newton(D);
	std::printf("\n");

	std::printf("[4] decomposition, not count:\n");
	ZBCL<double> half = from_native<double>(0.5);
	ZBCL<double> pi6_trunc = detail::take_while_above(t_pi6, static_cast<int>(t_pi6.head().exponent()) - block_count(t_asin) * block<double>::k + 1);
	std::printf("    sin(asin,19blk)      vs 0.5 = %d\n", agreed_decimal_digits(sin(t_asin, D), half, 340));
	std::printf("    sin(pi6-trunc,%zublk) vs 0.5 = %d  (clean prefix works)\n\n", block_count(pi6_trunc), agreed_decimal_digits(sin(pi6_trunc, D), half, 340));

	std::printf("[5] argument-truncation sweep (asin's deep blocks 15..19 are unusable by sin):\n");
	trace_truncation_sweep(D);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // diagnostic: ignore failures
#else
	// ---- characterization: lock the diagnosis facts (expensive: LEVEL_4) ----
#if REGRESSION_LEVEL_4
	const int D = 20;
	ZBCL<double> half = from_native<double>(0.5);

	// (a) sqrt is NOT the bottleneck: sqrt(0.75) is >= 300-digit ACCURATE at its
	//     19-block floor, and does not resolve deeper with depth (the plateau is
	//     the ~2^-1022 double-host floor, not an under-resolution). This locks the
	//     disproven sqrt-Newton hypothesis: a fix must be in the sin/cos series,
	//     not in sqrt.
	{
		auto sqrt075 = [](int d) {
			ZBCL<double> x = from_native<double>(0.5);
			return sqrt(add(from_native<double>(1.0), negate(mul_online(x, x))), d);   // sqrt(0.75)
		};
		int acc20 = agreed_decimal_digits(sqrt075(20), s_sqrt_075, 320);
		int acc40 = agreed_decimal_digits(sqrt075(40), s_sqrt_075, 320);
		if (acc20 < 300) {
			std::cout << "  FAIL sqrt(0.75) only " << acc20 << " digits accurate (expected >= 300; sqrt regressed)\n";
			++nrOfFailedTestCases;
		}
		else if (acc40 > acc20 + 8) {
			std::cout << "  NOTE sqrt(0.75) now resolves deeper with depth (" << acc20 << " @D20 -> " << acc40
			          << " @D40) -- the #1076 host-floor plateau may have moved; re-examine the assessment\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases)
			std::cout << "  ok   sqrt(0.75) is " << acc20 << "-digit accurate, floored (D20==D40) -- not the bottleneck\n";
	}

	// (b) a CLEAN pi/6 truncated to asin's block count still reaches >= 300 --
	//     proves the loss is the decomposition, not the block count. This must
	//     hold regardless of any future fix.
	{
		ZBCL<double> t_asin = asin(from_native<double>(0.5), D);
		ZBCL<double> t_pi6  = clean_pi6(D);
		ZBCL<double> pi6_trunc = detail::take_while_above(
		    t_pi6, static_cast<int>(t_pi6.head().exponent()) - static_cast<int>(block_count(t_asin)) * block<double>::k + 1);
		int got = agreed_decimal_digits(sin(pi6_trunc, D), half, 340);
		if (got < 300) {
			std::cout << "  FAIL clean pi/6 truncated to " << block_count(t_asin) << " blocks: sin = " << got
			          << " digits (expected >= 300; the 'clean prefix works' fact broke)\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases) std::cout << "  ok   clean pi/6 @ " << block_count(t_asin) << " blocks: sin = " << got << " digits (decomposition, not count)\n";
	}

	// (c) the known cap: sin(asin(0.5)) currently reaches ~234 digits. Assert it
	//     is not WORSE than the documented floor, and shout if it has been FIXED
	//     (so the gate in transcendentals_highprecision.cpp gets flipped).
	{
		int got = agreed_decimal_digits(sin(asin(from_native<double>(0.5), D), D), half, 340);
		if (reportTestCases) std::cout << "  --   sin(asin(0.5)) round-trip = " << got << " digits (known cap ~234, #1076)\n";
		if (got < 200) {
			std::cout << "  FAIL round-trip regressed below the documented floor (" << got << " < 200)\n";
			++nrOfFailedTestCases;
		}
		if (got >= 300) {
			std::cout << "  **#1076 APPEARS FIXED**: sin(asin(0.5)) now reaches " << got
			          << " digits. Flip ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION to 1 in "
			          << "transcendentals_highprecision.cpp and retire this characterization.\n";
			++nrOfFailedTestCases;   // fail loudly so the fix is not silently un-tracked
		}
	}
#else
	if (reportTestCases)
		std::cout << "  (diagnostic + characterization run at REGRESSION_LEVEL_4 / MANUAL_TESTING; see the file header for the #1076 assessment)\n";
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
