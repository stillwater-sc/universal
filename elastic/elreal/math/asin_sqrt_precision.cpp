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
// 3. UPSTREAM ROOT: sqrt(0.75) (elreal/math/sqrt.hpp, Newton-Raphson) plateaus at
//    19 blocks and stops resolving deeper regardless of depth (D=20..40) or
//    working width. That cap propagates div -> atan -> asin, so asin's argument
//    to sin is a shallow 19-block decomposition of pi/6. pi_zbcl (odd_power_series,
//    no sqrt in its chain) reaches 20+ blocks -- which is why sin(pi/6) runs deep.
//
// 4. It is the DECOMPOSITION, not accuracy or block count. A clean pi/6 TRUNCATED
//    to 19 blocks still gives sin = 306, but asin's own 19-block decomposition
//    gives 234. So the sin recurrence is ill-conditioned specifically against a
//    NON-PREFIX / "ragged" deepest block -- the kind the sqrt Newton fixed-point
//    produces (its last block is an iteration residual, not the next true digit).
//
// Fix directions RULED OUT here: deeper series floor; div_online inside the sqrt
// Newton step; wider sqrt working depth (none lift the 19-block cap or the 234
// round-trip). Candidate fixes for a follow-up (NOT attempted): (a) make sqrt
// resolve past the plateau; (b) re-express the argument as a canonical greedy-
// prefix Priest expansion before the series (priestRenorm does NOT -- it keeps
// the ragged tail). When either lands, flip ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION
// to 1 and update the CHARACTERIZATION checks in this file.
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

	std::printf("[3] decomposition, not count:\n");
	ZBCL<double> half = from_native<double>(0.5);
	ZBCL<double> pi6_trunc = detail::take_while_above(t_pi6, static_cast<int>(t_pi6.head().exponent()) - block_count(t_asin) * block<double>::k + 1);
	std::printf("    sin(asin,19blk)      vs 0.5 = %d\n", agreed_decimal_digits(sin(t_asin, D), half, 340));
	std::printf("    sin(pi6-trunc,%zublk) vs 0.5 = %d  (clean prefix works)\n", block_count(pi6_trunc), agreed_decimal_digits(sin(pi6_trunc, D), half, 340));

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // diagnostic: ignore failures
#else
	// ---- characterization: lock the diagnosis facts (expensive: LEVEL_4) ----
#if REGRESSION_LEVEL_4
	const int D = 20;
	ZBCL<double> half = from_native<double>(0.5);

	// (a) sqrt(0.75) plateaus: its block count is the same at depth 20 and 40
	//     (the depth-independent cap that starves the asin argument).
	{
		auto sqrt075 = [](int d) {
			ZBCL<double> x = from_native<double>(0.5);
			return sqrt(add(from_native<double>(1.0), negate(mul_online(x, x))), d);
		};
		std::size_t b20 = block_count(sqrt075(20)), b40 = block_count(sqrt075(40));
		if (b20 != b40) {
			std::cout << "  NOTE sqrt(0.75) block count changed with depth: " << b20 << " (D20) vs " << b40
			          << " (D40) -- the #1076 sqrt plateau may have moved; re-examine this test\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases) std::cout << "  ok   sqrt(0.75) plateaus at " << b20 << " blocks (D20==D40) [#1076 root]\n";
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
