// wide_arithmetic.cpp: correct-rounding verification for wide takum configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A takum significand is 1 + p bits with p reaching nbits - 2 - rbits, so from
// nbits > 54 + rbits it no longer fits a double -- takum<64,3> carries 60 bits
// against a double's 53.  The operators used to evaluate in a double, which
// quantized BOTH OPERANDS before the operation, and this suite is the evidence
// that they no longer do (issue #1300).
//
// The sibling suites addition.cpp / multiplication.cpp / division.cpp check the
// narrow configurations against a double, which is the correct reference THERE
// and useless here: it is precisely the thing under test.
//
// So the reference is exact instead.  Every takum value is a dyadic rational
// S * 2^e with S < 2^62, so a + b, a - b and a * b are exact integers and a / b is
// an exact ratio of them.  Held in a 1024-bit integer<>, the reference is arrived
// at with no floating-point arithmetic at all, and the check is the one that
// matters: the produced encoding must be at least as close to the exact result as
// either of its neighbours.  Both neighbours are consulted, since consecutive
// encodings are consecutive in value (Prop. 4).
//
// Confirmed by mutation.  Reverting the operators to the double path fails this
// at 64 bits for ~75% of addition pairs -- matching the 75.60% measured
// independently against a 113-bit __float128 reference in #1300 -- and 0% at 16
// and 32 bits, which is the whole shape of the defect.  Disabling the round-to-odd
// step in the shared tail fails a further 834 cases, all of them at 64 bits.
//
// Exact ties are decided, not waved through.  The format rounds to nearest-even,
// so at a tie the chosen encoding must carry an even trailing field; accepting
// either side would leave tie-breaking unverified, and ties are precisely what the
// round-to-odd intermediate width exists to get right.  They are not rare enough
// to ignore: an addition sweep at 64 bits hits about 1100 of them.  Confirmed by
// mutation -- flipping encode_fraction() to break ties towards odd fails this,
// and fails NOTHING else in the sweep, since a mis-broken tie is by definition
// equidistant and invisible to a nearest-neighbour comparison.
//
// Coverage is a checked property, not an assumption: report_coverage() fails the
// sweep if it compared nothing, or if any pair was skipped because the exact
// reference could not reach it.
//
// Saturated results -- zero, maxpos, maxneg -- are skipped.  Those are range
// decisions the format makes on the caller's behalf, not rounding decisions, and
// the nearest encoding to an out-of-range value is not what the type promises.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <cstdint>
#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/math/fma.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// 1024 bits carries any comparison this suite forms at rbits = 3, where the
// characteristic spans [-255, 254] and the widest span between two operands is
// therefore about 630 bits.  Configurations whose span does not fit are skipped
// rather than mis-verified; see fits() below.
using BigInt = sw::universal::integer<1024, std::uint64_t>;
constexpr int64_t SPAN_LIMIT = 820;

enum class Op { add, sub, mul, div };

// The largest encoding as a bit pattern, and the largest POSITIVE one as the
// signed integer it denotes (NaR is the most negative encoding, so the negative
// range stops one short of it).
//
// Both are right shifts on purpose.  The natural left-shift forms are 1ull << 64
// and int64_t(1) << 63 at nbits == 64, which is the width this suite exists for;
// guarding them behind a runtime ternary leaves the shift in the source even
// though the branch never runs, and cppcheck is right to call it out.  A right
// shift by 64 - nbits is total over the whole supported range.
constexpr std::uint64_t last_encoding(unsigned nbits) noexcept {
	return (~0ull) >> (64u - nbits);
}
constexpr std::int64_t max_positive_encoding(unsigned nbits) noexcept {
	return static_cast<std::int64_t>((~0ull) >> (64u - nbits + 1u));
}

// The step that walks `samples` values across the encoding space, or 1 when the
// space is small enough to enumerate.
//
// Derived from the width rather than passed in as a literal.  A fixed stride is
// silently width-dependent: 0x0123456789ABCDEF samples a 64-bit space about 225
// times, but it is a third of a 58-bit space, and takum<58,3> was getting NINE
// comparisons out of a sweep that reported PASS.  A sweep that covers almost
// nothing and a sweep that covers everything report identically, so the sample
// count is the thing to fix in place, not the stride.
//
// Odd, so it is coprime with the power-of-two field boundaries and the walk
// crosses every DR rather than revisiting one layout.
constexpr std::uint64_t sample_stride(std::uint64_t last, unsigned samples) noexcept {
	const std::uint64_t step = last / samples;
	return (step < 2ull) ? 1ull : (step | 1ull);
}

// |value| = S * 2^e, with S = 2^p + M the significand and its implicit bit.
struct fields {
	std::uint64_t S;
	std::int64_t  e;
	bool          sign;
	bool          zero;
};

template<typename T>
fields decode(const T& x) {
	if (x.iszero()) return fields{ 0ull, 0, x.sign(), true };
	auto d = T::Codec::decode(x.magnitude_bits());
	return fields{ (1ull << d.p) | d.M_bits,
	               d.c - static_cast<std::int64_t>(d.p),
	               x.sign(), false };
}

// S * 2^(e - base), signed.  Pre: e >= base, and the shift is within SPAN_LIMIT.
BigInt scaled(std::uint64_t S, std::int64_t e, bool sign, std::int64_t base) {
	BigInt v(S);
	v <<= static_cast<int>(e - base);
	return sign ? -v : v;
}

BigInt babs(const BigInt& a) { return (a < 0) ? -a : a; }

// The signed integer the encoding denotes, sign extended to 64 bits.  Encodings
// are ordered by value, so +/-1 on this is the neighbouring representable value.
template<typename T>
std::int64_t signed_bits(const T& x) {
	std::uint64_t raw = x.raw_bits();
	if constexpr (T::nbits < 64) {
		if (raw & (1ull << (T::nbits - 1))) raw |= ~((1ull << T::nbits) - 1);
	}
	return static_cast<std::int64_t>(raw);
}

// The neighbour v steps away, or false when that would leave the finite range.
// NaR is the most negative encoding and is excluded by construction.
template<typename T>
bool neighbour(std::int64_t v, T& out) {
	constexpr std::int64_t hi = max_positive_encoding(T::nbits);
	if (v < -hi || v > hi) return false;
	out.setbits(static_cast<std::uint64_t>(v) & T::Codec::nbits_mask());
	return true;
}

template<typename T>
bool saturated(const T& x) {
	T maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);
	return x.iszero() || x.raw_bits() == maxpos.raw_bits() || x.raw_bits() == maxneg.raw_bits();
}

// Two-sided on purpose.  The upper bound keeps the shift inside BigInt; the lower
// bound is what stops scaled() from being handed a negative shift, which integer<>
// silently turns into a right shift and would quietly truncate a candidate's value
// rather than fail.  neighbour_margin() below is sized so this never trips, and
// this check is what makes that reasoning falsifiable rather than load-bearing.
bool fits(std::int64_t e, std::int64_t base) {
	return e >= base && (e - base) <= SPAN_LIMIT;
}

// How far below the operands' exponents `base` has to sit so that the RESULT'S
// NEIGHBOURS are also placeable.
//
// A neighbour is one encoding step from the result, so its characteristic c' is
// within 1 of the result's c, but its trailing width p' is not within 1 of p --
// crossing a DR boundary moves p by a lot.  With e = c - p and p' at most
// maxCharBits,  e' >= (c - 1) - maxCharBits = e + p - 1 - maxCharBits, so a
// neighbour's exponent can sit maxCharBits + 1 below the result's own.  Taking the
// minimum over only the operands and the result, as an earlier draft did, left the
// neighbours unaccounted for.
constexpr std::int64_t neighbour_margin(unsigned maxCharBits) noexcept {
	return static_cast<std::int64_t>(maxCharBits) + 2;
}

// What a single comparison established.
struct tally {
	long skipped = 0;   // the exact reference could not be formed; NOT a pass
	long ties    = 0;   // the exact value fell exactly between two encodings
	long tiesTested = 0;  // ...and the two shared a layout, so the rule was checked
};

// Is `got` at least as close to num/den as both of its neighbours?  num and den
// are exact, den > 0, and every value is scaled by the same 2^base, so comparing
// |num - value(candidate) * den| across candidates decides it outright.
//
// An EXACT TIE is decided too, not waved through.  The format rounds to
// nearest-even -- encode_fraction() breaks a tie towards an even trailing field --
// so at a tie the chosen encoding must have an even M.  Accepting either side, as
// an earlier draft did, left the entire tie-breaking path unverified, and ties are
// the whole reason the shared tail rounds to odd at an intermediate width: they
// are the case it exists to get right.
//
// The rule is only checked when the two tied encodings share a trailing width.
// Across a DR boundary the candidates carry different p and "even M" is not a
// statement about the same quantity, so those are counted and reported rather than
// judged.
template<typename T>
bool nearest(const T& got, const BigInt& num, const BigInt& den, std::int64_t base, tally& t) {
	auto distance = [&](const T& cand, bool& ok) -> BigInt {
		fields f = decode(cand);
		if (f.zero) { ok = true; return babs(num); }
		if (!fits(f.e, base)) { ok = false; return BigInt(0); }
		ok = true;
		return babs(num - scaled(f.S, f.e, f.sign, base) * den);
	};

	bool ok = false;
	const BigInt mine = distance(got, ok);
	if (!ok) { ++t.skipped; return true; }

	const std::int64_t g = signed_bits(got);
	for (int k = -1; k <= 1; k += 2) {
		T nb;
		if (!neighbour<T>(g + k, nb)) continue;
		const BigInt d = distance(nb, ok);
		if (!ok) { ++t.skipped; return true; }
		if (d < mine) return false;
		if (d == mine && !nb.iszero() && !got.iszero()) {
			++t.ties;
			const auto dg = T::Codec::decode(got.magnitude_bits());
			const auto dn = T::Codec::decode(nb.magnitude_bits());
			if (dg.p == dn.p && dg.p > 0) {
				++t.tiesTested;
				if ((dg.M_bits & 1ull) != 0ull) return false;   // tie went to odd
			}
		}
	}
	return true;
}

// Did the sweep actually verify anything, and did it verify everything it walked?
//
// A sweep that compares nothing and a sweep that compares everything report the
// same PASS, so the coverage has to be a checked property rather than an assumed
// one.  Two ways it can quietly evaporate:
//
//   exercised == 0   nothing was compared at all
//   skipped > 0      individual pairs fell outside the exact reference's reach and
//                    were passed over.  Every configuration in this suite is sized
//                    to fit, so any skip at all is a defect in the suite -- either
//                    BigInt is too narrow or neighbour_margin() is wrong -- and
//                    NOT something to absorb silently.  A deliberately span-limited
//                    configuration would have to relax this, loudly.
//
// Ties are reported rather than required: they are rare, and a sweep that happens
// to produce none is not thereby broken.  Reporting them is what tells a reader
// whether the tie rule was actually exercised or merely available.
int report_coverage(const char* what, unsigned nbits, unsigned rbits,
                    long exercised, const tally& t, bool reportTestCases) {
	int failures = 0;
	if (exercised == 0) {
		++failures;
		std::cout << "FAIL " << what << " takum<" << nbits << ',' << rbits
		          << "> compared nothing\n";
	}
	if (t.skipped != 0) {
		++failures;
		std::cout << "FAIL " << what << " takum<" << nbits << ',' << rbits << "> skipped "
		          << t.skipped << " of " << (exercised + t.skipped)
		          << " pairs: the exact reference did not reach them\n";
	}
	if (reportTestCases) {
		std::cout << "      takum<" << nbits << ',' << rbits << "> " << what
		          << ": exercised " << exercised << ", ties " << t.ties
		          << " (" << t.tiesTested << " with the rule checked)\n";
	}
	return failures;
}

// ---------------------------------------------------------------------------
// a OP b, correctly rounded
// ---------------------------------------------------------------------------

// Is a OP b correctly rounded?  Counts the case in `exercised` when the exact
// reference reached it and returns false only for a wrong rounding.
template<typename T>
bool CheckCorrectlyRounded(Op op, const T& a, const T& b, tally& t, long& exercised, bool reportTestCases) {
	const fields fa = decode(a);
	const fields fb = decode(b);

	T got;
	switch (op) {
	case Op::add: got = a + b; break;
	case Op::sub: got = a - b; break;
	case Op::mul: got = a * b; break;
	default:      got = a / b; break;
	}
	if (got.isnar() || saturated(got)) return true;

	// Assemble the exact result as num/den, everything scaled by 2^base.
	// den is 1 except for division, where the quotient is not dyadic and
	// the comparison is cross-multiplied by the divisor's significand.
	const fields fg = decode(got);
	BigInt num(0), den(1);
	std::int64_t base = 0;

	if (op == Op::add || op == Op::sub) {
		base = std::min(std::min(fa.e, fb.e), fg.e) - neighbour_margin(T::maxCharBits);
		if (!fits(fa.e, base) || !fits(fb.e, base)) { ++t.skipped; return true; }
		const BigInt A = scaled(fa.S, fa.e, fa.sign, base);
		const BigInt B = scaled(fb.S, fb.e, (op == Op::sub) ? !fb.sign : fb.sign, base);
		num = A + B;
	}
	else if (op == Op::mul) {
		const std::int64_t ep = fa.e + fb.e;
		base = std::min(ep, fg.e) - neighbour_margin(T::maxCharBits);
		if (!fits(ep, base)) { ++t.skipped; return true; }
		BigInt P(fa.S);
		P = P * BigInt(fb.S);
		P <<= static_cast<int>(ep - base);
		num = (fa.sign != fb.sign) ? -P : P;
	}
	else {
		const std::int64_t eq = fa.e - fb.e;
		base = std::min(eq, fg.e) - neighbour_margin(T::maxCharBits);
		if (!fits(eq, base)) { ++t.skipped; return true; }
		num = scaled(fa.S, eq, fa.sign != fb.sign, base);
		den = BigInt(fb.S);
	}

	const long skipsBefore = t.skipped;
	const bool ok = nearest(got, num, den, base, t);
	if (t.skipped != skipsBefore) return true;
	++exercised;
	if (!ok && reportTestCases) {
		std::cout << "FAIL not correctly rounded: op=" << int(op)
		          << " a=" << to_binary(a) << " b=" << to_binary(b)
		          << " got=" << to_binary(got) << '\n';
	}
	return ok;
}

template<unsigned nbits, unsigned rbits>
int VerifyCorrectlyRounded(Op op, unsigned samples, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	long exercised = 0;
	tally t;
	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);

	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar() || a.iszero()) continue;
		for (std::uint64_t j = 0; j <= LAST && j + stride > j; j += stride) {
			T b; b.setbits(j);
			if (b.isnar() || b.iszero()) continue;
			if (!CheckCorrectlyRounded(op, a, b, t, exercised, reportTestCases)) ++nrOfFailedTests;
		}
	}
	nrOfFailedTests += report_coverage("correctly rounded", nbits, rbits, exercised, t, reportTestCases);
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// fma(a, b, c), correctly rounded
//
// The point of an fma is that the product is NOT rounded before the addend joins
// it, which is exactly what a double intermediate destroyed here: it rounded both
// factors first and then computed an exact product of the wrong numbers.
// ---------------------------------------------------------------------------
// Is fma(a, b, c) correctly rounded?  Same contract as CheckCorrectlyRounded().
template<typename T>
bool CheckFmaCorrectlyRounded(const T& a, const T& b, const T& c, tally& t, long& exercised, bool reportTestCases) {
	const fields fa = decode(a);
	const fields fb = decode(b);
	const fields fc = decode(c);

	T got = sw::universal::fma(a, b, c);
	if (got.isnar() || saturated(got)) return true;

	const fields fg = decode(got);
	const std::int64_t ep = fa.e + fb.e;
	const std::int64_t base = std::min(std::min(ep, fc.e), fg.e) - neighbour_margin(T::maxCharBits);
	if (!fits(ep, base) || !fits(fc.e, base)) { ++t.skipped; return true; }

	BigInt P(fa.S);
	P = P * BigInt(fb.S);
	P <<= static_cast<int>(ep - base);
	if (fa.sign != fb.sign) P = -P;
	const BigInt num = P + scaled(fc.S, fc.e, fc.sign, base);

	const long skipsBefore = t.skipped;
	const bool ok = nearest(got, num, BigInt(1), base, t);
	if (t.skipped != skipsBefore) return true;
	++exercised;
	if (!ok && reportTestCases) {
		std::cout << "FAIL fma not correctly rounded: a=" << to_binary(a)
		          << " b=" << to_binary(b) << " c=" << to_binary(c)
		          << " got=" << to_binary(got) << '\n';
	}
	return ok;
}

template<unsigned nbits, unsigned rbits>
int VerifyFmaCorrectlyRounded(unsigned samples, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	long exercised = 0;
	tally t;
	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);
	// a third operand from the same sweep, offset so it is not a repeat of a or b
	const std::uint64_t skew = stride / 3ull + 1ull;

	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar() || a.iszero()) continue;
		for (std::uint64_t j = 0; j <= LAST && j + stride > j; j += stride) {
			T b; b.setbits(j);
			if (b.isnar() || b.iszero()) continue;
			T c; c.setbits(i + j + skew);
			if (c.isnar() || c.iszero()) continue;
			if (!CheckFmaCorrectlyRounded(a, b, c, t, exercised, reportTestCases)) ++nrOfFailedTests;
		}
	}
	nrOfFailedTests += report_coverage("correctly rounded fma", nbits, rbits, exercised, t, reportTestCases);
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// Double rounding (#1616)
//
// Random operands almost never expose it -- 0 in 10^8 pairs per operator for
// takum<32,3> -- so the sweeps above cannot see it.  These construct the cases.
//
// Products: for operands 1 + i/2^p and 1 + j/2^p in [1,2) the exact product is
// 1 + (i+j)/2^p + ij/2^(2p).  Choosing ij == 2^(p-1) +/- 1 (mod 2^p) puts it one
// 2^-2p unit off a midpoint, which a double's 53 bits cannot hold once 2p > 53:
// the double lands ON the midpoint and the second rounding breaks the tie by
// evenness.  Half of them break it the wrong way.
//
// fma: a product that is exactly a midpoint plus a tiny addend.  std::fma drops
// the addend entirely, at every width, and the tie goes to even regardless of
// which side the addend was on.
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits>
int VerifyNearMidpointProducts(unsigned count, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	long exercised = 0;
	tally t;
	const T one(1.0);
	const unsigned p = T::Codec::decode(one.magnitude_bits()).p;
	const std::uint64_t mask = (1ull << p) - 1ull;
	auto inverse = [&](std::uint64_t x) {         // x odd: Newton's iteration mod 2^p
		std::uint64_t y = x;
		for (int k = 0; k < 6; ++k) y *= 2ull - x * y;
		return y & mask;
	};
	std::uint64_t state = 0x9E3779B97F4A7C15ull;
	for (unsigned n = 0; n < count; ++n) {
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		const std::uint64_t i = ((state >> 11) & mask) | 1ull;
		const std::uint64_t target = (1ull << (p - 1)) + ((n & 1u) ? 1ull : mask);   // 2^(p-1) +/- 1
		const std::uint64_t j = (target * inverse(i)) & mask;
		T a, b;
		a.setbits(one.raw_bits() + i);
		b.setbits(one.raw_bits() + j);
		if (!CheckCorrectlyRounded(Op::mul, a, b, t, exercised, reportTestCases)) ++nrOfFailedTests;
	}
	// the three products quoted in the issue
	if constexpr (nbits == 32 && rbits == 3) {
		const std::uint64_t quoted[3][2] = { { 0x44000001, 0x40000001 }, { 0x43fffffe, 0x40000001 }, { 0x46aaaaab, 0x40000003 } };
		for (const auto& q : quoted) {
			T a, b; a.setbits(q[0]); b.setbits(q[1]);
			if (!CheckCorrectlyRounded(Op::mul, a, b, t, exercised, reportTestCases)) ++nrOfFailedTests;
		}
	}
	nrOfFailedTests += report_coverage("near-midpoint products", nbits, rbits, exercised, t, reportTestCases);
	return nrOfFailedTests;
}

template<unsigned nbits, unsigned rbits>
int VerifyFmaMidpointPlusTiny(unsigned count, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	long exercised = 0;
	tally t;
	const T one(1.0);
	const T tiny[2] = { T(sw::universal::SpecificValue::minpos), T(sw::universal::SpecificValue::minneg) };
	// Operands 1 + k/2^q in [1,2) with q = ceil((p+1)/2) fraction bits, so a product
	// has at most 2q+2 <= 53 bits: exact in a double, which makes the midpoint
	// filter below exact, and fine enough to reach the result's midpoints.
	const unsigned p = T::Codec::decode(one.magnitude_bits()).p;
	const unsigned q = (p + 2u) / 2u;
	const std::uint64_t step  = 1ull << (p - q);
	const std::uint64_t nr    = 1ull << q;
	const std::uint64_t rowSkip = (nr > 128ull) ? nr / 128ull + 1ull : 1ull;   // ~128 rows, odd stride so ka*kb can be odd
	unsigned found = 0;
	for (std::uint64_t ka = 0; ka < nr && found < count; ka += rowSkip) {
		for (std::uint64_t kb = 0; kb < nr && found < count; ++kb) {
			T a, b;
			a.setbits(one.raw_bits() + ka * step);
			b.setbits(one.raw_bits() + kb * step);
			// keep only products that sit exactly on a midpoint of the result's layout
			const double prod = double(a) * double(b);
			const T lo(prod);
			if (double(lo) == prod) continue;
			T nb = lo; if (double(lo) < prod) ++nb; else --nb;
			if (0.5 * (double(lo) + double(nb)) != prod) continue;
			++found;
			for (const T& c : tiny)
				if (!CheckFmaCorrectlyRounded(a, b, c, t, exercised, reportTestCases)) ++nrOfFailedTests;
		}
	}
	if (found == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL fma midpoint + tiny found no midpoint products\n";
	}
	nrOfFailedTests += report_coverage("fma midpoint + tiny", nbits, rbits, exercised, t, reportTestCases);
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// Exact identities
//
// These need no reference at all and they are what issue #1300 opened on: one
// encoding step above 1.0, takum<64,3> lost its low bit to `x + 0` because the
// operand was quantized on the way in.  Cheap enough to run at every width, and
// they discriminate -- the double path fails a + 0 at 64 bits.
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits>
int VerifyExactIdentities(unsigned samples, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);
	T one(1.0), zero; zero.setzero();
	T nar; nar.setnar();

	auto fail = [&](const char* what, std::uint64_t bits) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << " at bits=" << bits << '\n';
	};

	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar()) continue;

		if ((a + zero).raw_bits() != a.raw_bits()) fail("a + 0 != a", i);
		if ((a - zero).raw_bits() != a.raw_bits()) fail("a - 0 != a", i);
		if ((a * one).raw_bits() != a.raw_bits())  fail("a * 1 != a", i);
		if (!(a * zero).iszero())                  fail("a * 0 != 0", i);
		if (!(a - a).iszero())                     fail("a - a != 0", i);
		if (!(a + nar).isnar())                    fail("a + NaR must be NaR", i);
		if (!(a * nar).isnar())                    fail("a * NaR must be NaR", i);
		if (!(a / zero).isnar())                   fail("a / 0 must be NaR", i);
		if (!a.iszero()) {
			if ((a / one).raw_bits() != a.raw_bits()) fail("a / 1 != a", i);
			if ((a / a).raw_bits() != one.raw_bits()) fail("a / a != 1", i);
			if (!(zero / a).iszero())                 fail("0 / a != 0", i);
			if ((-a + a).raw_bits() != 0ull)          fail("-a + a != 0", i);
		}
	}
	return nrOfFailedTests;
}

// Addition and multiplication commute bit for bit.  Independent of any reference,
// and it reaches the operand-ordering logic in the wide path's alignment step.
template<unsigned nbits, unsigned rbits>
int VerifyCommutative(unsigned samples, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);

	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar()) continue;
		for (std::uint64_t j = 0; j <= LAST && j + stride > j; j += stride) {
			T b; b.setbits(j);
			if (b.isnar()) continue;
			if ((a + b).raw_bits() != (b + a).raw_bits()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL addition not commutative at " << i << ',' << j << '\n';
			}
			if ((a * b).raw_bits() != (b * a).raw_bits()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL multiplication not commutative at " << i << ',' << j << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// The reported symptom of #1300, verbatim: one encoding step above 1.0, then add
// zero.  A named regression so the report stays legible in the suite output.
int VerifyIssue1300Repro(bool reportTestCases) {
	using namespace sw::universal;
	using T64 = takum<64, 3, std::uint64_t>;
	int nrOfFailedTests = 0;

	T64 one(1.0), eps, zero;
	zero.setzero();
	eps.setbits(one.raw_bits() + 1ull);
	if ((eps + zero).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps + 0 lost the low bit\n";
	}
	if ((eps - zero).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps - 0 lost the low bit\n";
	}
	if ((eps * one).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps * 1 lost the low bit\n";
	}
	if ((eps / one).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps / 1 lost the low bit\n";
	}
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "takum wide-configuration arithmetic verification";
	std::string test_tag    = "wide arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Sample counts, not strides -- sample_stride() derives the step from the width
	// so every configuration gets the same coverage regardless of nbits.  The
	// pairwise sweeps square: 225 samples is ~50k operand pairs, each of which
	// builds an exact 1024-bit reference.  maybe_unused because which of them a
	// build reaches depends on the regression level.
	[[maybe_unused]] constexpr unsigned rounding_samples = 225;    // ~50k pairs
	[[maybe_unused]] constexpr unsigned identity_samples = 4096;   // single ops, so cheap
	[[maybe_unused]] constexpr unsigned commutative_samples = 300; // ~90k pairs, no reference

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIssue1300Repro(true), "takum<64,3>", "issue 1300 repro");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::add, rounding_samples, true), "takum<64,3>", "correctly rounded add");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyIssue1300Repro(true), "takum<64,3>", "issue 1300 repro");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 3>(identity_samples, reportTestCases), "takum<64,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::add, rounding_samples, reportTestCases), "takum<64,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::mul, rounding_samples, reportTestCases), "takum<64,3>", "correctly rounded mul");
	// #1616: double rounding, constructed rather than sampled
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearMidpointProducts<32, 3>(4000, reportTestCases), "takum<32,3>", "near-midpoint products");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearMidpointProducts<40, 3>(4000, reportTestCases), "takum<40,3>", "near-midpoint products");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFmaMidpointPlusTiny<16, 3>(2000, reportTestCases), "takum<16,3>", "fma midpoint + tiny");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFmaMidpointPlusTiny<32, 3>(2000, reportTestCases), "takum<32,3>", "fma midpoint + tiny");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::sub, rounding_samples, reportTestCases), "takum<64,3>", "correctly rounded sub");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::div, rounding_samples, reportTestCases), "takum<64,3>", "correctly rounded div");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<64, 3>(commutative_samples, reportTestCases), "takum<64,3>", "commutativity");
	// takum<32,3> is on the exact path since #1616; takum<16,3> below keeps the
	// double path, where the double rounding is provably innocuous.  Verified
	// against the SAME exact reference, so the boundary the gate draws is measured
	// rather than asserted.
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::add, rounding_samples, reportTestCases), "takum<32,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::mul, rounding_samples, reportTestCases), "takum<32,3>", "correctly rounded mul");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyFmaCorrectlyRounded<64, 3>(rounding_samples, reportTestCases), "takum<64,3>", "correctly rounded fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::sub, rounding_samples, reportTestCases), "takum<32,3>", "correctly rounded sub");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::div, rounding_samples, reportTestCases), "takum<32,3>", "correctly rounded div");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<16, 3>(Op::add, rounding_samples, reportTestCases), "takum<16,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<16, 3>(Op::div, rounding_samples, reportTestCases), "takum<16,3>", "correctly rounded div");
#endif

#if REGRESSION_LEVEL_4
	// takum<58,3> was the narrowest configuration on the wide path before #1616
	// moved the gate to nbits > 26 + rbits; it still exercises a p > 53 layout
	// and takum<64,1> the widest significand the format allows at 64 bits, p = 61,
	// which is what fixes the round-to-odd width at 63.
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<58, 3>(identity_samples, reportTestCases), "takum<58,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<58, 3>(Op::add, rounding_samples, reportTestCases), "takum<58,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 1>(identity_samples, reportTestCases), "takum<64,1>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 1>(Op::mul, rounding_samples, reportTestCases), "takum<64,1>", "correctly rounded mul");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 1>(Op::div, rounding_samples, reportTestCases), "takum<64,1>", "correctly rounded div");
	// rbits = 5 pushes the characteristic past 2^31, an exponent range no binary
	// floating-point type can hold.  The exact integer path carries the exponent
	// as an int64_t outside the significand, so it is indifferent; the reference
	// above is not, and skips the pairs whose span exceeds its width, which is why
	// this configuration is covered structurally.
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 5>(identity_samples, reportTestCases), "takum<64,5>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<64, 5>(commutative_samples, reportTestCases), "takum<64,5>", "commutativity");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
