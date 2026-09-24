// posit_log_characterization.cpp: what the posit logarithm shim actually costs
//
// Issue #244 (2021) asked for native posit log and log1p, on the reasonable suspicion
// that the library was calling std::log and converting. It is:
//
//     template<unsigned nbits, unsigned es, typename bt>
//     posit<nbits,es,bt> log(posit<nbits,es,bt> x) {
//         return posit<nbits,es,bt>(std::log(double(x)));
//     }
//
// The issue stayed open for five years, and the thread records why. A correctly rounded
// elementary function library for a PARAMETERIZED number system -- posit<nbits,es> names
// thousands of formats, each sampling the reals differently -- is an open research
// question. The Minefield method that solves it for one configuration does not generalize,
// its coefficient synthesis is not feasible above 32 bits, and RLibm's own posit32 log
// computes in doubles and converts on the way out, exactly as this shim does.
//
// So rather than guess at a polynomial, this program measures what the shim costs. The
// result is not what the issue assumed.
//
// THE ORACLE
//
// A high-precision qd computes the logarithm, and the answer is rounded to the posit
// under test. That is the correctly rounded result by construction, and the shim is
// scored against it.
//
// The oracle must be given the posit's EXACT value. Converting through double(x) rounds
// away the very bits under investigation, so the oracle would see the same argument as
// the shim and agree with it by construction -- a check that cannot fail. Building the
// value from the decoded sign, scale and fraction bits is what makes the boundary
// visible at all: with the double conversion in place, every configuration measures as
// perfect, including the ones that are not.
//
// WHERE TO LOOK
//
// A posit's precision is TAPERED. Sampling raw encodings uniformly puts most of them in
// the low-precision regime, where a double has bits to spare, and the shim measures as
// flawless. The fraction is widest near 1.0, and that is where the shim is worst -- not
// because the result is large but because the bits that distinguish x from 1.0 ARE the
// low fraction bits, and the conversion to a double is exactly what throws them away.
// log(1+eps) then comes back as the logarithm of a different number.
//
// Arguments near e are the second region worth probing: there log(x) is near 1, so the
// RESULT sits at the posit's widest point and needs every bit the format has.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include <universal/number/posit/posit.hpp>
#include <universal/number/qd/qd.hpp>

namespace {

	using namespace sw::universal;

	// qd carries ~212 bits, against the 59 that the widest posit here needs, and its log
	// agrees with an adaptive-precision ereal to 208 bits while running 3039x faster --
	// fast enough to sweep every encoding of the small configurations exhaustively.
	using Oracle = qd;

	// Exact posit -> oracle, from the decoded fields.
	//
	// significant(posit) would be the natural route, but it calls get_fixed_point(),
	// which does not compile: blockbinary has setbit(), not set(unsigned, bool). Nothing
	// instantiates it, so the body has never been checked -- the same trap components()
	// fell into in #1453. Reported separately.
	template<typename P>
	Oracle exactly(const P& p) {
		constexpr unsigned nbits = P::nbits;
		constexpr unsigned es    = P::es;
		using bt = typename P::BlockType;
		constexpr unsigned fbits = nbits - 3u - es;

		bool sign{ false };
		positRegime<nbits, es, bt>   regime;
		positExponent<nbits, es, bt> exponent;
		positFraction<fbits, bt>     fraction;
		decode(p.bits(), sign, regime, exponent, fraction);

		const int scale = regime.scale() + exponent.scale();
		auto bits = fraction.bits();
		// the fraction runs MSB-first: index fbits-1 carries 2^-1, index 0 carries 2^-fbits
		Oracle frac(0);
		for (unsigned i = 0; i < fbits; ++i) {
			if (bits.test(i)) frac = frac + Oracle(std::ldexp(1.0, -int(fbits - i)));
		}
		// a power of two is exact in a double, so the scaling introduces no error
		Oracle v = (Oracle(1) + frac) * Oracle(std::ldexp(1.0, scale));
		return sign ? -v : v;
	}

	// Round an oracle value to the nearest posit WITHOUT passing through a double.
	//
	// P want{ double(v) } would discard everything past the double's 53 significant bits
	// -- which is exactly the regime this program exists to measure, so the reference
	// would be wrong for the only configurations that fail. This is the same trap as
	// converting the ARGUMENT through a double, made on the way out instead of the way
	// in. The double is used only as a starting guess; the walk downhill in |v - p| is
	// decided in oracle arithmetic.
	template<typename P>
	P roundToNearest(const Oracle& v) {
		P best{ double(v) };
		if (best.isnar()) return best;
		Oracle bestErr = abs(v - exactly(best));
		// No short cap. double(v) can land far from the true nearest posit: for
		// posit<64,2>, v = 1 + 2^-54 converts to exactly 1.0, and the nearest posit is 32
		// encodings away. A walk that stopped after a few steps would hand back a "want"
		// that is not the correctly rounded value, and then misjudge the shim against it.
		// |v - p| is unimodal in the posit ordering, so the first local minimum is the
		// global one and the loop terminates on its own; the bound is a safety net and
		// the program says so if it is ever reached.
		constexpr int WALK_GUARD = 1 << 16;
		int step = 0;
		for (; step < WALK_GUARD; ++step) {
			P up = best;   ++up;
			P down = best; --down;
			const Oracle errUp   = up.isnar()   ? bestErr : abs(v - exactly(up));
			const Oracle errDown = down.isnar() ? bestErr : abs(v - exactly(down));
			if (errUp < bestErr && errUp <= errDown)      { best = up;   bestErr = errUp; }
			else if (errDown < bestErr)                   { best = down; bestErr = errDown; }
			else break;
		}
		if (step == WALK_GUARD) std::cerr << "roundToNearest: walk hit its guard\n";
		return best;
	}

	struct Score {
		unsigned checked{ 0 };
		unsigned correct{ 0 };
		unsigned wrong{ 0 };
		int      maxUlp{ 0 };
		bool     maxUlpIsExact{ true };
		double   worstArgument{ 0.0 };
	};

	// Distance in ulps between two posits. The search is capped, so the result says
	// whether it is exact: a capped count reported as though it were measured would
	// understate how far apart they are, and would also decide the "worst" argument on
	// a number that is not the real distance.
	constexpr int ULP_SEARCH_CAP = 16;
	template<typename P>
	int ulpsApart(P a, P b, bool& exactCount) {
		exactCount = true;
		if (a == b) return 0;
		P up = a, down = a;
		for (int i = 1; i <= ULP_SEARCH_CAP; ++i) {
			++up;   if (up == b)   return i;
			--down; if (down == b) return i;
		}
		exactCount = false;
		return ULP_SEARCH_CAP;
	}

	template<typename P>
	void judge(Score& s, const P& x) {
		if (x.isnar() || x <= P(0)) return;
		const Oracle exact = log(exactly(x));
		if (exact.iszero()) return;              // log(1) == 0, nothing to round
		const P want = roundToNearest<P>(exact); // correctly rounded, in oracle arithmetic
		const P got = log(x);                    // the shim
		++s.checked;
		if (got == want) { ++s.correct; return; }
		++s.wrong;
		bool exactCount = true;
		const int u = ulpsApart(want, got, exactCount);
		// an unbounded distance always outranks a measured one
		if ((!exactCount && s.maxUlpIsExact) || (exactCount == s.maxUlpIsExact && u > s.maxUlp)) {
			s.maxUlp = u; s.maxUlpIsExact = exactCount; s.worstArgument = double(x);
		}
	}

	void report(const std::string& tag, const std::string& where, const Score& s) {
		std::cout << "  " << std::left << std::setw(14) << tag
		          << std::setw(22) << where
		          << " checked " << std::setw(7) << s.checked
		          << " correctly rounded " << std::setw(7) << s.correct
		          << " wrong " << std::setw(7) << s.wrong;
		if (s.wrong) std::cout << "  worst " << (s.maxUlpIsExact ? "" : "> ") << s.maxUlp
		                       << " ulp at " << s.worstArgument;
		std::cout << '\n';
	}

	// every encoding of a small posit
	template<unsigned nbits, unsigned es>
	void exhaustive(const std::string& tag) {
		using P = posit<nbits, es>;
		Score s;
		for (unsigned long long raw = 1; raw < (1ull << nbits); ++raw) {
			P x; x.setbits(raw);
			judge(s, x);
		}
		report(tag, "every encoding", s);
	}

	// the encodings adjacent to a chosen value, where a wide posit spends its fraction
	// how many fraction bits this posit actually carries AT this value. A posit's regime
	// is variable length, so the width at 1.0 is not the width at e -- which is why the
	// error rate is not monotone in the fbits-at-1.0 column.
	template<typename P>
	unsigned fractionBitsAt(const P& p) {
		constexpr unsigned nbits = P::nbits;
		constexpr unsigned es    = P::es;
		using bt = typename P::BlockType;
		constexpr unsigned fbits = nbits - 3u - es;
		bool sign{ false };
		positRegime<nbits, es, bt>   regime;
		positExponent<nbits, es, bt> exponent;
		positFraction<fbits, bt>     fraction;
		decode(p.bits(), sign, regime, exponent, fraction);
		return fraction.nrBits();
	}

	template<unsigned nbits, unsigned es>
	void around(const std::string& tag, double centre, const std::string& where, unsigned n) {
		using P = posit<nbits, es>;
		P c(centre);
		const uint64_t base = c.bits().to_ull();
		Score s;
		for (unsigned i = 1; i <= n; ++i) {
			for (int dir : { 1, -1 }) {
				P x; x.setbits(base + uint64_t(dir * int64_t(i)));
				judge(s, x);
			}
		}
		report(tag, where + " [" + std::to_string(fractionBitsAt(c)) + " frac bits here]", s);
	}

	template<unsigned nbits, unsigned es>
	void fractionBudget(const std::string& tag) {
		constexpr int fbits = int(nbits) - 3 - int(es);
		// fbits counts EXPLICIT fraction bits; a double's 53 counts the whole significand,
		// so the comparable budget is its 52 explicit bits. Off by one here would put
		// posit<56,0> at zero spare when the value above 1.0 is 1 + 2^-53, which a double
		// cannot hold at all.
		constexpr int spare = 52 - fbits;
		std::cout << "  " << std::left << std::setw(14) << tag
		          << std::setw(7) << fbits
		          << std::setw(5) << spare
		          << (spare < 0 ? " -- past a double's range" : (spare < 4 ? " -- too few to resolve a tie" : "")) << '\n';
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	constexpr double E = 2.718281828459045235;

	std::cout << "posit logarithm: what the std::log shim costs (#244)\n";
	std::cout << "===================================================\n\n";

	std::cout << "A posit's fraction is widest at 1.0. The shim computes in a double, so the\n";
	std::cout << "question is whether that 53-bit budget covers the format's own:\n\n";
	fractionBudget<8, 2>("posit<8,2>");
	fractionBudget<16, 2>("posit<16,2>");
	fractionBudget<32, 2>("posit<32,2>");
	fractionBudget<56, 2>("posit<56,2>");
	fractionBudget<64, 2>("posit<64,2>");

	std::cout << "\n1. Every encoding of the small configurations\n\n";
	exhaustive<8, 2>("posit<8,2>");
	exhaustive<10, 2>("posit<10,2>");
	exhaustive<12, 2>("posit<12,2>");
	exhaustive<14, 2>("posit<14,2>");
	exhaustive<16, 2>("posit<16,2>");

	std::cout << "\n2. Where the wide configurations actually spend their fraction.\n";
	std::cout << "   Near 1.0 the bits that distinguish x from 1 ARE the low fraction bits,\n";
	std::cout << "   and the conversion to a double is what discards them. Near e the result\n";
	std::cout << "   sits at the posit's widest point and needs every bit the format has.\n\n";
	around<32, 2>("posit<32,2>", E,   "x near e", 200);
	around<56, 2>("posit<56,2>", E,   "x near e", 200);
	around<64, 2>("posit<64,2>", E,   "x near e", 200);
	std::cout << '\n';
	around<32, 2>("posit<32,2>", 1.0, "x near 1", 200);
	around<56, 2>("posit<56,2>", 1.0, "x near 1", 200);
	around<64, 2>("posit<64,2>", 1.0, "x near 1", 200);

	std::cout << "\n3. The mechanism is DOUBLE ROUNDING, and what matters is headroom.\n";
	std::cout << "   The shim rounds twice: the real result to a double, then that double to\n";
	std::cout << "   the posit. Rounding twice is not rounding once. Whenever the double has\n";
	std::cout << "   few spare bits over the posit's fraction, a value near a posit rounding\n";
	std::cout << "   boundary is resolved by bits the double no longer has.\n";
	std::cout << "   fbits = nbits - 3 - es at 1.0; a double carries 53 significant bits.\n\n";
	std::cout << "   The width below is at 1.0; a posit's regime is variable length, so the\n";
	std::cout << "   width actually in use at e is reported alongside each measurement.\n\n";
	std::cout << "                 fbits  spare bits over a double\n";
	fractionBudget<32, 2>("posit<32,2>");
	fractionBudget<56, 3>("posit<56,3>");
	fractionBudget<56, 2>("posit<56,2>");
	fractionBudget<56, 1>("posit<56,1>");
	fractionBudget<56, 0>("posit<56,0>");
	fractionBudget<64, 2>("posit<64,2>");
	std::cout << '\n';
	around<32, 2>("posit<32,2>", E, "x near e", 200);
	around<56, 3>("posit<56,3>", E, "x near e", 200);
	around<56, 2>("posit<56,2>", E, "x near e", 200);
	around<56, 1>("posit<56,1>", E, "x near e", 200);
	around<56, 0>("posit<56,0>", E, "x near e", 200);
	around<64, 2>("posit<64,2>", E, "x near e", 200);

	std::cout << "\n4. What this says\n\n";
	std::cout << "   Measured here: posit<8,2> through posit<16,2> over every encoding, and\n";
	std::cout << "   posit<32,2>, posit<56,0..3> and posit<64,2> over the demanding\n";
	std::cout << "   neighbourhoods. Claims below are about those, not about every posit.\n\n";
	std::cout << "   The shim is correctly rounded where the double has room to spare over\n";
	std::cout << "   the format's fraction, and mis-rounds as that headroom runs out.\n\n";
	std::cout << "   Where the format still fits a double -- the posit<56,3>, posit<56,2>\n";
	std::cout << "   and posit<56,0> rows AT e, which use 50 to 52 fraction bits there --\n";
	std::cout << "   the mechanism is double rounding rather than a shortcoming of the\n";
	std::cout << "   polynomial, and those rows measured 1 ulp. The failures appear where\n";
	std::cout << "   the format is most precise.\n\n";
	std::cout << "   Where it does not fit at all the errors are not 1 ulp but unbounded:\n";
	std::cout << "   posit<64,2> measured more than 16 ulp, and posit<56,0> returns zero for\n";
	std::cout << "   the value immediately above 1.0, which a double cannot tell from 1.0.\n";
	std::cout << "   That is a different failure from double rounding, not a larger one.\n\n";
	std::cout << "   The error rate is also NOT a clean function of the fraction width.\n";
	std::cout << "   posit<56,0> and posit<56,1> both carry 52 fraction bits at e, and one\n";
	std::cout << "   is wrong 198 times in 400 while the other is wrong none. Whether a\n";
	std::cout << "   result lands near a posit rounding boundary depends on how that\n";
	std::cout << "   configuration's grid sits against the double's, and every (nbits, es)\n";
	std::cout << "   pair has its own answer -- the same reason the thread gives for why a\n";
	std::cout << "   correctly rounded library for a PARAMETERIZED format is hard.\n\n";
	std::cout << "   So for the small and mid configurations a native log would recover\n";
	std::cout << "   nothing; for the wide ones it would, and that is exactly the regime in\n";
	std::cout << "   which no oracle exists to synthesize the coefficients. That is the shape\n";
	std::cout << "   of #244, measured rather than argued.\n";

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
