#pragma once
// takum_log_arithmetic.hpp: extended-precision addition and subtraction for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// WHY THIS EXISTS
//
// takum_log<> stores l = c + m and denotes |value| = e^(l/2).  Multiplication and
// division are exact integer operations on l (#1312), but addition and
// subtraction have no logarithmic shortcut, and they were evaluated by converting
// both operands to a double.  The fraction of l is p bits wide with p reaching
// maxCharBits, so takum_log<64,3> carries 59 against a double's 53: both operands
// were quantized before the addition, and 99.22% of 64-bit sums came back
// incorrectly rounded against a 113-bit reference.  Issue #1300.
//
// THE IDENTITY
//
// Never form e^(lx/2) and e^(ly/2) and add them.  Factor out the larger operand:
//
//     |x| + |y| = e^(lb/2) * (1 + e^(d/2))        d = ls - lb <= 0
//     l_result  = lb + 2 log(1 + e^(d/2))         same sign
//     l_result  = lb + 2 log(1 - e^(d/2))         opposite signs
//
// This is not merely more accurate than the naive route, it is the only one with
// the format's range.  e^(l/2) overflows a double for l beyond ~1420, and at
// rbits = 5 the characteristic reaches 2^32, so the naive formulation cannot even
// represent its own operands.  Here d is a DIFFERENCE of characteristics, the
// transcendentals only ever see a bounded argument, and the huge exponent stays
// in lb, which is added back at the end.  Every configuration works the same way.
//
// WHY A LOCAL DOUBLE-DOUBLE
//
// #1300 planned to evaluate this in dd_cascade.  That type is ACCURATE enough --
// measured against an 80-digit reference its exp is good to ~103 bits and its log
// to ~105, which is the full width two doubles can hold and ~45 bits of margin
// over what a 64-bit takum_log needs.  The problem is entirely speed:
//
//     dd_cascade exp        11.2 us
//     dd_cascade log/log1p  25.8 us
//     dd_cascade multiply    0.30 us
//     dd_cascade floor       0.004 us
//
// An addition built on those is 229x more expensive than the double path it
// replaces -- 27 us against 117 ns -- and the cost is in the arithmetic itself,
// not just the transcendentals, so writing better series on top of dd_cascade
// would not recover it.
//
// So this file carries its own two-double type and its own exp and log, each
// specialised to the one domain this identity needs -- and nothing more.  It is
// NOT a general-purpose double-double: no NaN handling, no rounding modes, and
// every entry point documents the range it is valid on.  It reaches the same
// ~106-bit accuracy as dd_cascade; see the measurements in
// static/tapered/takum_log/arithmetic/addition.cpp, which check it directly
// rather than taking it on trust.
//
// THE DOMAIN, which is what makes the transcendentals short
//
//     exp   argument y = d/2 <= 0, and callers shortcut below -80, so y in [-80, 0]
//     log   argument w in (0, 2].  It cannot approach zero: w = 1 - e^(d/2) with
//           |d| at least one ulp of l, so w >= ~2^-63, and log stays above -44.
//
// No overflow, no underflow, no subnormal path, and the argument reduction never
// has to be defensive.

#include <cmath>
#include <cstdint>

#include <universal/number/takum/takum_codec.hpp>

namespace sw { namespace universal { namespace takum_log_arith {

// ---------------------------------------------------------------------------
// ddouble: an unevaluated sum of two doubles, hi + lo, with |lo| <= ulp(hi)/2
// ---------------------------------------------------------------------------

struct ddouble {
	double hi;
	double lo;
};

constexpr ddouble make(double h, double l = 0.0) noexcept { return ddouble{ h, l }; }

// Exact sum of two doubles (Knuth).  No assumption about relative magnitude.
inline ddouble two_sum(double a, double b) noexcept {
	const double s  = a + b;
	const double bb = s - a;
	const double e  = (a - (s - bb)) + (b - bb);
	return ddouble{ s, e };
}

// Exact sum when |a| >= |b| (Dekker).  Three flops instead of six.
inline ddouble quick_two_sum(double a, double b) noexcept {
	const double s = a + b;
	return ddouble{ s, b - (s - a) };
}

// Exact product.
//
// std::fma would be the obvious way to get the residual, and it is what the first
// draft used -- but without -mfma the compiler cannot assume the instruction
// exists and emits a LIBRARY CALL, which measured 214 ns per double-double
// multiply and made a whole addition 8 us.  Dekker's split is ten flops and no
// call.  FP_FAST_FMA is the platform's own statement that the instruction is
// there, so honour it when it is set and fall back otherwise.
//
// The split scales by 2^27 + 1, so it would overflow for |a| beyond ~6.7e299.
// Nothing here approaches that: the arguments are bounded by the characteristic
// range and the intermediate values by the series' own domain.
inline ddouble two_prod(double a, double b) noexcept {
	const double p = a * b;
#if defined(FP_FAST_FMA)
	return ddouble{ p, std::fma(a, b, -p) };
#else
	constexpr double SPLITTER = 134217729.0;      // 2^27 + 1
	const double at = SPLITTER * a;
	const double bt = SPLITTER * b;
	const double ah = at - (at - a), al = a - ah;
	const double bh = bt - (bt - b), bl = b - bh;
	return ddouble{ p, ((ah * bh - p) + ah * bl + al * bh) + al * bl };
#endif
}

inline ddouble renorm(double h, double l) noexcept { return quick_two_sum(h, l); }

inline ddouble add(const ddouble& a, const ddouble& b) noexcept {
	ddouble s = two_sum(a.hi, b.hi);
	ddouble t = two_sum(a.lo, b.lo);
	s.lo += t.hi;
	s = quick_two_sum(s.hi, s.lo);
	s.lo += t.lo;
	return quick_two_sum(s.hi, s.lo);
}

inline ddouble neg(const ddouble& a) noexcept { return ddouble{ -a.hi, -a.lo }; }
inline ddouble sub(const ddouble& a, const ddouble& b) noexcept { return add(a, neg(b)); }

inline ddouble mul(const ddouble& a, const ddouble& b) noexcept {
	ddouble p = two_prod(a.hi, b.hi);
	p.lo += a.hi * b.lo + a.lo * b.hi;
	return quick_two_sum(p.hi, p.lo);
}

inline ddouble mul_d(const ddouble& a, double b) noexcept {
	ddouble p = two_prod(a.hi, b);
	p.lo += a.lo * b;
	return quick_two_sum(p.hi, p.lo);
}

inline ddouble div(const ddouble& a, const ddouble& b) noexcept {
	// One Newton correction on a double-precision quotient is enough for the
	// full width: the first estimate is already good to ~53 bits.
	const double q1 = a.hi / b.hi;
	const ddouble r = sub(a, mul_d(b, q1));
	const double q2 = r.hi / b.hi;
	return quick_two_sum(q1, q2);
}

// Scaling by a power of two is exact on both limbs.
inline ddouble ldexp2(const ddouble& a, int e) noexcept {
	return ddouble{ std::ldexp(a.hi, e), std::ldexp(a.lo, e) };
}

inline bool is_zero(const ddouble& a) noexcept { return a.hi == 0.0 && a.lo == 0.0; }
inline bool less(const ddouble& a, const ddouble& b) noexcept {
	return (a.hi != b.hi) ? (a.hi < b.hi) : (a.lo < b.lo);
}
inline double to_double(const ddouble& a) noexcept { return a.hi + a.lo; }

// floor, exact: the result is an integer and fits the high limb whenever the
// value does, which it always does here (|l| stays far inside 2^53 for rbits <= 3
// and is an exact integer plus a fraction for wider regimes).
inline ddouble floor_dd(const ddouble& a) noexcept {
	double fh = std::floor(a.hi);
	if (fh == a.hi) {                       // hi was already integral
		const double fl = std::floor(a.lo);
		return quick_two_sum(fh, fl);
	}
	return ddouble{ fh, 0.0 };
}

// ln 2 to three limbs, so the exp argument reduction is exact well past what two
// limbs can hold.  Generated to 60 digits and split by repeated subtraction.
inline constexpr double ln2_hi  = 6.93147180559945286e-01;
inline constexpr double ln2_lo  = 2.31904681384629956e-17;
inline constexpr double ln2_lo2 = 5.70770843841621207e-34;
inline constexpr double invln2  = 1.44269504088896339e+00;

// ---------------------------------------------------------------------------
// exp, for y in [-80, 0]
//
// y = k ln2 + r with |r| <= ln2/2, then r is halved SPLITS times so the Taylor
// series converges in a dozen terms, then the result is squared back up.  ln2 is
// subtracted in three pieces so the reduction itself introduces no error the
// series would have to carry.
// ---------------------------------------------------------------------------
inline ddouble exp_nonpos(const ddouble& y) noexcept {
	if (is_zero(y)) return make(1.0);

	const double kd = std::floor(to_double(y) * invln2 + 0.5);
	const int    k  = static_cast<int>(kd);

	// r = y - k ln2, in three exact steps
	ddouble r = sub(y, mul_d(make(ln2_hi), kd));
	r = sub(r, mul_d(make(ln2_lo), kd));
	r = sub(r, mul_d(make(ln2_lo2), kd));

	// |r| <= ln2/2; halve it SPLITS times, exactly
	constexpr int SPLITS = 6;
	r = ldexp2(r, -SPLITS);

	// Taylor: e^r - 1 = r + r^2/2! + ...  Summing the MINUS ONE form keeps the
	// leading 1 out of the accumulation, so the small terms are not shifted away.
	//
	// Divide by i rather than multiplying by 1.0/i: the reciprocal of a
	// non-power-of-two is not exact, and scaling every term by a value already
	// wrong in its 54th bit caps the whole series at ~2^-62 no matter how many
	// terms it runs.  Measured: that one substitution cost 33 bits.
	ddouble term = r;
	ddouble sum  = r;
	for (int i = 2; i <= 14; ++i) {
		term = mul(term, r);
		term = div(term, make(static_cast<double>(i)));
		sum  = add(sum, term);
	}

	// undo the halving: e^(2x) - 1 = (e^x - 1)^2 + 2(e^x - 1)
	for (int i = 0; i < SPLITS; ++i) {
		sum = add(mul(sum, sum), ldexp2(sum, 1));
	}

	return ldexp2(add(sum, make(1.0)), k);
}

// exp(y) - 1 for y <= 0.  Separate from exp() because the caller needs it when y
// is tiny, where exp(y) - 1 would cancel away everything the value carries: |d|
// is at least one ulp of l, so y reaches 2^-62 and the difference would keep only
// ~44 bits of the ~106 available.
inline ddouble expm1_nonpos(const ddouble& y) noexcept {
	if (is_zero(y)) return make(0.0);
	if (to_double(y) <= -0.5) return sub(exp_nonpos(y), make(1.0));  // no cancellation there

	// |y| < 0.5: sum the series directly, which is exact in the relative sense
	ddouble term = y;
	ddouble sum  = y;
	for (int i = 2; i <= 26; ++i) {
		term = mul(term, y);
		term = div(term, make(static_cast<double>(i)));
		sum  = add(sum, term);
	}
	return sum;
}

// ---------------------------------------------------------------------------
// log, for w in (0, 2]
//
// w = f 2^E with f in [1,2), so log w = E ln2 + log f, and log f is refined from
// a double-precision seed by ONE Newton step -- the correction is computed as
// log(f e^-z0), whose argument is within 2^-53 of 1, so three series terms carry
// it to the full width.
// ---------------------------------------------------------------------------
inline ddouble log_pos(const ddouble& w) noexcept {
	int E = 0;
	const double fh = std::frexp(w.hi, &E);      // fh in [0.5, 1)
	ddouble f = ldexp2(w, -E);                   // exact
	// frexp gives [0.5,1); shift to [1,2) so the seed and the series agree
	f = ldexp2(f, 1);
	--E;
	(void)fh;

	const double z0 = std::log(f.hi);            // ~53 bits

	// u = f e^(-z0), within about 2^-53 of 1
	const ddouble u = mul(f, exp_nonpos(make(-z0)));
	const ddouble x = sub(u, make(1.0));         // |x| ~ 2^-53

	// log(1+x) = x - x^2/2 + x^3/3, ample at |x| ~ 2^-53
	const ddouble x2 = mul(x, x);
	ddouble corr = sub(x, ldexp2(x2, -1));
	corr = add(corr, div(mul(x2, x), make(3.0)));

	ddouble logf = add(make(z0), corr);
	if (E == 0) return logf;

	// E ln2, with ln2 carried to two limbs
	const double Ed = static_cast<double>(E);
	ddouble eln2 = mul_d(make(ln2_hi), Ed);
	eln2 = add(eln2, mul_d(make(ln2_lo), Ed));
	return add(eln2, logf);
}

// ---------------------------------------------------------------------------
// Bridging the encoding
// ---------------------------------------------------------------------------

// Fraction width handed to encode_fraction().  Two wider than the widest trailing
// field a 64-bit layout produces (p reaches 61 at rbits = 1), which is what makes
// the round-to-odd below equivalent to a single rounding.
inline constexpr unsigned qbits = 63;

// l = c + M/2^p, exactly.  M reaches 2^61, past a double, so it is split into
// halves that are each exactly representable.
inline ddouble exact_l(int64_t c, uint64_t M, unsigned p) noexcept {
	ddouble r = make(static_cast<double>(c));
	if (p == 0 || M == 0) return r;
	const uint64_t hi = M >> 32;
	const uint64_t lo = M & 0xFFFFFFFFull;
	const double scale = std::ldexp(1.0, -static_cast<int>(p));
	if (hi != 0) r = add(r, make(static_cast<double>(hi) * std::ldexp(1.0, 32) * scale));
	if (lo != 0) r = add(r, make(static_cast<double>(lo) * scale));
	return r;
}

// Split l into an integer part and a qbits fraction numerator, ROUNDED TO ODD, so
// that encode_fraction()'s subsequent round-to-nearest-even agrees with rounding
// the exact value once.  Same argument as takum_cross_conversion.hpp.
struct split_result {
	int64_t  c;
	uint64_t N;
};
// The fraction is extracted in TWO pieces of 31 and 32 bits rather than one of
// qbits.  A double-double's low limb is routinely negative -- 2^63 - 15 is held as
// (2^63, -15) -- and casting that limb straight to an unsigned type is undefined,
// which UBSan duly reported.  Splitting the extraction keeps every intermediate
// under 2^32, so each limb converts through a signed type that comfortably holds
// it, and the two pieces are reassembled in the unsigned domain where they belong.
inline split_result to_integer_fraction(const ddouble& v) noexcept {
	constexpr unsigned HIBITS = 31;
	constexpr unsigned LOBITS = qbits - HIBITS;            // 32

	const ddouble fl = floor_dd(v);
	int64_t c = static_cast<int64_t>(fl.hi) + static_cast<int64_t>(fl.lo);
	const ddouble frac = sub(v, fl);                       // in [0,1), exact

	// top HIBITS bits: frac < 1, so this is at most 2^31 - 1
	const ddouble s1 = ldexp2(frac, static_cast<int>(HIBITS));
	const ddouble f1 = floor_dd(s1);
	const uint64_t nhi = static_cast<uint64_t>(
		static_cast<int64_t>(f1.hi) + static_cast<int64_t>(f1.lo));

	// the remaining LOBITS, from what is left over
	const ddouble s2 = ldexp2(sub(s1, f1), static_cast<int>(LOBITS));
	const ddouble f2 = floor_dd(s2);
	const uint64_t nlo = static_cast<uint64_t>(
		static_cast<int64_t>(f2.hi) + static_cast<int64_t>(f2.lo));

	const bool sticky = !is_zero(sub(s2, f2));

	uint64_t N = (nhi << LOBITS) | nlo;
	if (sticky) N |= 1ull;                                 // round to odd
	return split_result{ c, N };
}

// ---------------------------------------------------------------------------
// The combination itself
// ---------------------------------------------------------------------------

// Beyond this the smaller operand cannot move the result: |u| < 2 e^-80 < 2^-114,
// while the finest grid a 64-bit layout has is 2^-61.  Returning the larger
// operand is not an approximation -- lb is exactly on the grid, so lb +/- a
// quantity below half an ulp rounds back to lb, and there is no tie to break.
inline constexpr double negligible_d = -160.0;

struct combined {
	bool     zero;     // the operands cancelled exactly
	bool     take_big; // the smaller operand was negligible; result IS the larger operand
	ddouble  l;        // otherwise, the result's logarithmic value
	bool     sign;
};

// Pre: both operands are non-zero and non-NaR; la, lb are their exact l values;
// sa, sb their signs (sb already flipped by the caller for subtraction).
inline combined combine(const ddouble& la, bool sa, const ddouble& lb, bool sb) noexcept {
	const bool aBig = !less(la, lb);          // |x| = e^(l/2) is monotone in l
	const ddouble& big = aBig ? la : lb;
	const ddouble& sml = aBig ? lb : la;
	const bool sBig = aBig ? sa : sb;
	const bool same = (sa == sb);

	const ddouble d = sub(sml, big);          // <= 0
	if (!same && is_zero(d)) return combined{ true, false, make(0.0), false };
	if (to_double(d) < negligible_d) return combined{ false, true, big, sBig };

	const ddouble y = ldexp2(d, -1);          // d/2, exact
	ddouble u;
	if (same) {
		// 2 log(1 + e^y).  Forming 1 + t loses only bits below the value's own
		// scale, and the result is needed to absolute, not relative, accuracy.
		u = ldexp2(log_pos(add(make(1.0), exp_nonpos(y))), 1);
	}
	else {
		// 2 log(1 - e^y) = 2 log(-expm1(y)), which is where the cancellation would
		// have been had expm1 not been carried separately.
		u = ldexp2(log_pos(neg(expm1_nonpos(y))), 1);
	}
	return combined{ false, false, add(big, u), sBig };
}

}}} // namespace sw::universal::takum_log_arith
