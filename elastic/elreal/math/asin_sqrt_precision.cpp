// asin_sqrt_precision.cpp: diagnostic + characterization for the sin(asin(x))/
//                          cos(acos(x)) round-trip precision (#1076, RESOLVED).
//
// This is a DIAGNOSTIC regression test. It carries, under the MANUAL_TESTING
// guard, the instrumented experiments that were used to chase why the inverse
// round-trip capped at ~234 digits (issue #1076, the last Phase-7 / #931
// acceptance gate), and under the regression path it locks the RESOLVED
// behaviour so the cap cannot come back unnoticed.
//
// ============================================================================
// RESOLUTION (double host; measured 2026-08-21)
// ============================================================================
// sin(asin(0.5)) and cos(acos(0.5)) now agree with 0.5 to 321 digits at depth
// 20, and the agreement SCALES with depth -- 256 at depth 16, 321 at 20, 385 at
// 24, 512 at 32, 768 at 48, a straight line at 16.0 digits per unit of depth
// against a theoretical k * log10(2) = 15.95, with no ceiling in sight --
// tracking a clean pi/6 (= pi_zbcl/6) digit for digit. The gate
// ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION in transcendentals_highprecision.cpp is
// back on.
//
// Fixed by commit 386737c5 (#1362), part of v4.9.0's narrow-host work, whose
// rule is: normalise the OPERANDS of an error-free transform, not its result.
// #1361 had applied that to the narrow hosts but deliberately exempted double;
// #1362 removed the exemption. Bisected: at #1361 the round-trip is still 234 at
// both depth 20 and 24; at #1362 it is 321 and 385.
//
// WHY THE ORIGINAL DIAGNOSIS MISSED IT. Every investigation in #1076 concluded
// the loss was a CONDITIONING defect in the sin/cos Maclaurin recurrence, on this
// reasoning: sqrt(0.75) plateaued at 19 blocks regardless of depth while staying
// 308-digit accurate, so the plateau "must be" the ~2^-1022 double-host precision
// floor -- something sqrt physically could not push past; and since a clean pi/6
// truncated to 19 blocks still gave sin = 306 while asin's own 19-block form gave
// 234, the fault "must be" the series' handling of a ragged deep tail.
//
// The premise was wrong. That 2^-1022 floor was not physics, it was the bug: a
// block is (v: FpType, exp: integer<256>) and its scale belongs in the WIDE
// exponent, but un-normalised EFT operands carried the scale in the host
// significand instead, so the chain bottomed out on double's own exponent range.
// Once #1362 lifted it, sqrt(0.75) resolves one block per unit of depth --
// 8 blocks/130 digits, 16/260, 20/320, and 40 blocks at depth 40, with no
// plateau -- and the series consumes the deeper argument without complaint. The
// sin/cos recurrence never needed changing.
//
// The lesson worth keeping: a "physical floor" that was only ever inferred from
// a plateau is a hypothesis, not a measurement. It named the observed symptom
// (nothing gets deeper than 2^-1022) as its own cause, which made the real
// defect -- that values were being kept in the wrong field of the block -- look
// like a law of the host type.
//
// Historical detail retained for the record: the 234-digit cap showed up in the
// series as PREMATURE TERMINATION -- the asin argument drove the term stream to
// EMPTY at term 60 where a clean pi/6 ran to 76, and the dropped terms 60-76
// carry exactly the 10^-234..10^-306 contributions that were missing. That
// observation was correct; it was the terms losing their deep blocks to the
// host floor, one multiply at a time, not the recurrence mis-handling them.
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
	// These are the traces used to chase #1076. The commentary records what each
	// one showed BEFORE the fix (#1362); re-running them on a current tree now
	// shows the healthy behaviour instead -- the term stream runs to completion,
	// the block counts track depth, and the truncation sweep gains ~16 digits per
	// block for the asin argument just as it does for a clean pi/6. They are kept
	// because they are the instrumentation that localises this class of loss.
	const int D = 20;
	ZBCL<double> t_asin = asin(from_native<double>(0.5), D);
	ZBCL<double> t_pi6  = clean_pi6(D);
	std::printf("asin(0.5) vs clean pi/6 agree to %d digits\n\n", agreed_decimal_digits(t_asin, t_pi6, 340));

	std::printf("[1] sin-series term streams (pre-fix: premature termination):\n");
	int na = trace_term_stream("asin", t_asin, D);   // pre-fix: stopped at 60
	int np = trace_term_stream("pi6 ", t_pi6,  D);   // pre-fix: ran to 76
	std::printf("    => pre-fix, asin stopped %d terms early (the dropped terms carry\n"
	            "       10^-234..10^-306, i.e. exactly the 234-digit cap); both now run to %d\n\n",
	            np - na, np);

	std::printf("[2] block counts along asin = atan(x/sqrt(1-x^2)) vs depth\n"
	            "    (pre-fix sqrt plateaued at 19 for every depth; now one block per depth):\n");
	for (int d : {16, 20, 24, 32, 40}) trace_chain_blocks(d);
	std::printf("\n");

	std::printf("[3] sqrt Newton convergence (quadratic; pre-fix it stalled at 19 blocks /\n"
	            "    ~308 digits, which was misread as double's physical floor):\n");
	trace_sqrt_newton(D);
	std::printf("\n");

	std::printf("[4] asin's decomposition vs a clean-prefix pi/6 of the same width\n"
	            "    (pre-fix: 234 vs 306 -- the difference that was blamed on the series):\n");
	ZBCL<double> half = from_native<double>(0.5);
	ZBCL<double> pi6_trunc = detail::take_while_above(t_pi6, static_cast<int>(t_pi6.head().exponent()) - block_count(t_asin) * block<double>::k + 1);
	std::printf("    sin(asin,%zublk)      vs 0.5 = %d\n", block_count(t_asin), agreed_decimal_digits(sin(t_asin, D), half, 340));
	std::printf("    sin(pi6-trunc,%zublk) vs 0.5 = %d  (clean prefix works)\n\n", block_count(pi6_trunc), agreed_decimal_digits(sin(pi6_trunc, D), half, 340));

	std::printf("[5] argument-truncation sweep (pre-fix, asin's deep blocks 15..19 added\n"
	            "    nothing -- a flat 234 -- while pi/6's each added ~16 digits):\n");
	trace_truncation_sweep(D);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // diagnostic: ignore failures
#else
	// ---- characterization: lock the RESOLVED behaviour (expensive: LEVEL_4) ----
#if REGRESSION_LEVEL_4
	const int D = 20;
	ZBCL<double> half = from_native<double>(0.5);

	// (a) the acceptance criterion itself: the round-trip clears 300 digits.
	//     This is the check that was gated off in transcendentals_highprecision.cpp
	//     for the life of #1076.
	int rt20 = 0;
	{
		rt20 = agreed_decimal_digits(sin(asin(half, D), D), half, 340);
		if (rt20 < 300) {
			std::cout << "  FAIL sin(asin(0.5)) round-trip = " << rt20
			          << " digits (want >= 300; #1076 has regressed)\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases)
			std::cout << "  ok   sin(asin(0.5)) round-trip = " << rt20 << " digits\n";
	}

	// (b) it SCALES with depth. This is the check that would have caught #1076:
	//     the bug's signature was a FLAT line -- a pre-#1362 tree returns 234
	//     digits at depth 16 and at depth 20 alike (and #1076 recorded the same
	//     234 at 24 and beyond) -- so a round-trip that merely clears 300 at one
	//     depth does not prove the ceiling is gone. Depth 16 now reaches 256 and
	//     depth 20 reaches 321, so a gain of >= 40 digits is expected; a returned
	//     ceiling shows up here as ~0.
	{
		int rt16 = agreed_decimal_digits(sin(asin(half, 16), 16), half, 340);
		if (rt20 - rt16 < 40) {
			std::cout << "  FAIL round-trip does not scale with depth: " << rt16
			          << " digits @D16 -> " << rt20 << " @D20 (expected a gain >= 40;"
			          << " a flat line is the #1076 ceiling signature)\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases)
			std::cout << "  ok   round-trip scales with depth: " << rt16 << " @D16 -> " << rt20 << " @D20\n";
	}

	// (c) sqrt resolves one block per unit of depth. The #1076 investigation
	//     read sqrt's depth-independent 19-block plateau as the double host's
	//     physical precision floor; it was the un-normalised-operand bug, and
	//     lifting it (#1362) removed the plateau. Cheap, and it pins the fact
	//     that was mis-read.
	{
		auto sqrt075 = [](int d) {
			ZBCL<double> x = from_native<double>(0.5);
			return sqrt(add(from_native<double>(1.0), negate(mul_online(x, x))), d);   // sqrt(0.75)
		};
		ZBCL<double> s16 = sqrt075(16), s40 = sqrt075(40);
		std::size_t b16 = block_count(s16, 128), b40 = block_count(s40, 128);
		// Demand PROPORTIONALITY to depth, not merely "more blocks than at D16":
		// the pre-fix plateau sat at 19 blocks, which does exceed D16's 16 and so
		// slips past a strict-increase test. sqrt now returns one block per unit
		// of depth (40 blocks at depth 40); requiring 32 fails the 19-block
		// plateau decisively while leaving room for a block or two of slack.
		constexpr std::size_t kMinBlocksAtD40 = 32;
		if (b40 < kMinBlocksAtD40) {
			std::cout << "  FAIL sqrt(0.75) plateaus below its depth: " << b16
			          << " blocks @D16, " << b40 << " @D40 (want >= " << kMinBlocksAtD40
			          << "; the #1076 plateau sat at 19 for every depth)\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases)
			std::cout << "  ok   sqrt(0.75) resolves with depth: " << b16 << " blocks @D16 -> " << b40 << " @D40\n";

		// and it is accurate at that resolution, against an independent 320-digit
		// reference (depth 16 lands at ~260 digits, well short of the reference's
		// own length, so this measures sqrt and not the reference).
		int acc16 = agreed_decimal_digits(s16, s_sqrt_075, 320);
		if (acc16 < 250) {
			std::cout << "  FAIL sqrt(0.75) @D16 only " << acc16 << " digits accurate (expected ~260)\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases)
			std::cout << "  ok   sqrt(0.75) @D16 is " << acc16 << "-digit accurate\n";
	}

	// (d) asin's own decomposition agrees with a clean pi/6 to the round-trip's
	//     precision. The old diagnosis blamed the DIFFERENCE between these two
	//     equally-accurate representations; keeping the comparison documents that
	//     they now feed sin equally well.
	{
		int agree = agreed_decimal_digits(asin(half, D), clean_pi6(D), 340);
		if (agree < 300) {
			std::cout << "  FAIL asin(0.5) vs clean pi/6: " << agree << " digits (want >= 300)\n";
			++nrOfFailedTestCases;
		}
		else if (reportTestCases)
			std::cout << "  ok   asin(0.5) agrees with clean pi/6 to " << agree << " digits\n";
	}
#else
	if (reportTestCases)
		std::cout << "  (diagnostic + characterization run at REGRESSION_LEVEL_4 / MANUAL_TESTING; see the file header for the #1076 resolution)\n";
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
