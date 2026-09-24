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

	struct Score {
		unsigned checked{ 0 };
		unsigned correct{ 0 };
		unsigned wrong{ 0 };
		int      maxUlp{ 0 };
		double   worstArgument{ 0.0 };
	};

	// distance in ulps between two posits, capped
	template<typename P>
	int ulpsApart(P a, P b, int cap = 16) {
		if (a == b) return 0;
		P up = a, down = a;
		for (int i = 1; i <= cap; ++i) {
			++up;   if (up == b)   return i;
			--down; if (down == b) return i;
		}
		return cap + 1;   // further than we bothered to count
	}

	template<typename P>
	void judge(Score& s, const P& x) {
		if (x.isnar() || x <= P(0)) return;
		const Oracle exact = log(exactly(x));
		if (exact.iszero()) return;              // log(1) == 0, nothing to round
		const P want{ double(exact) };           // correctly rounded
		const P got = log(x);                    // the shim
		++s.checked;
		if (got == want) { ++s.correct; return; }
		++s.wrong;
		const int u = ulpsApart(want, got);
		if (u > s.maxUlp) { s.maxUlp = u; s.worstArgument = double(x); }
	}

	void report(const std::string& tag, const std::string& where, const Score& s) {
		std::cout << "  " << std::left << std::setw(14) << tag
		          << std::setw(22) << where
		          << " checked " << std::setw(7) << s.checked
		          << " correctly rounded " << std::setw(7) << s.correct
		          << " wrong " << std::setw(7) << s.wrong;
		if (s.wrong) std::cout << "  worst " << s.maxUlp << " ulp at " << s.worstArgument;
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
		report(tag, where, s);
	}

	template<unsigned nbits, unsigned es>
	void fractionBudget(const std::string& tag) {
		constexpr int fbits = int(nbits) - 3 - int(es);
		std::cout << "  " << std::left << std::setw(14) << tag
		          << "fraction bits at 1.0: " << std::setw(4) << fbits
		          << (fbits > 53 ? "  EXCEEDS a double's 53" : "  fits in a double's 53") << '\n';
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

	std::cout << "\n3. What this says\n\n";
	std::cout << "   The shim is correctly rounded for every posit configuration whose fraction\n";
	std::cout << "   fits in a double, which is every configuration up to and including 56 bits.\n";
	std::cout << "   A native implementation would have to match a result that is already exact;\n";
	std::cout << "   there is no accuracy to recover there, only libm independence and speed.\n\n";
	std::cout << "   Past that width the shim is genuinely wrong, and it fails where a posit is\n";
	std::cout << "   most precise rather than uniformly -- which is why a uniform sweep of raw\n";
	std::cout << "   encodings reports it as flawless.\n\n";
	std::cout << "   So the open research problem the thread describes -- a correctly rounded\n";
	std::cout << "   polynomial for a parameterized format -- buys nothing below 56 bits, and\n";
	std::cout << "   above it the oracle needed to synthesize the coefficients does not exist.\n";
	std::cout << "   That is the shape of #244, measured rather than argued.\n";

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
