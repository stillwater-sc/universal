#pragma once
// correctly_rounded.hpp: the engine of the correctly rounded b-posit math library (#1653, phase 2)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The posit standard requires every function to be correctly rounded for every input:
// the result is the true value rounded once, to nearest with ties to the even encoding,
// saturating at minpos and maxpos.  The b-posit math library meets that with Ziv's strategy:
//
//   1. The fast path evaluates the function in double through libm.  libm's result is
//      within a few ulps of the true value (BPOSIT_MATH_LIBM_ULPS, default 4: glibc's
//      documented bounds are 1 to 2 ulps for these functions).  If both ends of that
//      error interval round to the same b-posit, so does the true value, and that is
//      the answer.  This decides all but about 2^-20 of the inputs of a 32-bit b-posit.
//
//   2. Otherwise the function is evaluated in quad-double (qd, about 212 bits) from the
//      exact argument, with an error far below 2^-170 relative, and the same test is
//      applied to that interval.
//
//   3. If even that interval straddles a rounding boundary, the true value lies within
//      2^-170 of the boundary.  For a b-posit of at most 64 bits, whose significand has at
//      most 59 bits, no value of these functions at a b-posit argument comes that close to
//      a midpoint without being exactly on it; the hardest cases for a p-bit format need
//      about 2p bits.  The value is the midpoint, and it rounds to the even neighbor.
//
// The fast path needs the argument to convert to double exactly, which holds when the
// significand fits (fbits + 1 <= 53).  The quad-double engine needs the b-posit's range,
// and the 212 bits below it, inside double's normal range: |scale| <= 600.  That covers
// every standard and DSP-fitted configuration, 64-bit ones included.  A configuration
// outside it falls back to the phase 1 evaluation in double (see shim.hpp).
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <universal/number/qd/qd.hpp>
#include <universal/number/bposit/math/shim.hpp>

#if !defined(BPOSIT_MATH_FAST_PATH)
#define BPOSIT_MATH_FAST_PATH 1
#endif
#if !defined(BPOSIT_MATH_LIBM_ULPS)
#define BPOSIT_MATH_LIBM_ULPS 4
#endif

namespace sw { namespace universal { namespace bposit_math {

// which evaluation a function may use: the regression suite forces qd_only to verify the
// quad-double engine on the inputs the fast path would otherwise decide
enum class engine { libm_then_qd, qd_only };

template<typename B>
inline constexpr bool exact_in_double = (B::fbits + 1 <= 53) && (B::maxscale < 1000) && (B::minscale > -1000);

template<typename B>
inline constexpr bool qd_engine = (B::maxscale <= 600) && (B::minscale >= -600);

// relative error budget of the quad-double kernels, and the half-width of the snap test
inline constexpr int qd_error_exponent = -170;
inline constexpr int snap_exponent     = -160;

////////////////////////////////////////////////////////////////////////////////////////
// exact conversions between b-posits, dyadic values and quad-doubles

// a double pair holding the dyadic m 2^e exactly, m < 2^63: two_sum makes it a normalized qd
inline qd dyadic_to_qd(bool negative, std::uint64_t m, int e) {
	if (m == 0) return qd(0.0);
	const double hi = std::ldexp(static_cast<double>(m & ~std::uint64_t(0x7FF)), e);   // the top 52 bits, exact
	const double lo = std::ldexp(static_cast<double>(m & std::uint64_t(0x7FF)), e);    // the low 11 bits, exact
	const double s  = hi + lo;
	const double bb = s - hi;
	const double err = (hi - (s - bb)) + (lo - bb);
	const qd v(s, err, 0.0, 0.0);
	return negative ? -v : v;
}

// the exact value of a b-posit; NaR maps to NaN
template<typename B>
qd to_qd(const B& x) {
	if (x.iszero()) return qd(0.0);
	if (x.isnar())  return qd(SpecificValue::qnan);
	const auto d = x.decode_fields();
	const std::uint64_t sig = (std::uint64_t(1) << d.nf) | d.frac;
	return dyadic_to_qd(d.negative, sig, d.scale - static_cast<int>(d.nf));
}

// 1 + s x exactly, s = +1 or -1, for |x| < 4: the cancellation near x = -s is where it matters
template<typename B>
qd one_plus(const B& x, int s) {
	if (x.iszero()) return qd(1.0);
	const auto d = x.decode_fields();
	const std::uint64_t sig = (std::uint64_t(1) << d.nf) | d.frac;
	const int e = d.scale - static_cast<int>(d.nf);              // >= -61 when |x| >= 2^-2
	if (d.scale < -2 || d.scale > 1) {                           // |x| < 1/4 or >= 4: no cancellation, qd rounding is harmless
		const qd v = to_qd(x);
		return (s > 0) ? qd(1.0) + v : qd(1.0) - v;
	}
	const bool subtract = d.negative != (s < 0);                 // 1 - |x|
	const std::int64_t one = std::int64_t(1) << (-e);
	const std::int64_t m = subtract ? one - static_cast<std::int64_t>(sig) : one + static_cast<std::int64_t>(sig);
	return dyadic_to_qd(m < 0, static_cast<std::uint64_t>(m < 0 ? -m : m), e);
}

// A two's complement fixed-point accumulator, wide enough to hold a quad-double's limbs
// exactly relative to its leading limb.
struct wide_accumulator {
	static constexpr unsigned words = 8;
	std::uint64_t w[words]{};

	void add_shifted(std::uint64_t m, unsigned shift, bool negative) {
		std::uint64_t part[words]{};
		const unsigned word = shift / 64, bit = shift % 64;
		if (word < words) part[word] = m << bit;
		if (bit != 0 && word + 1 < words) part[word + 1] = m >> (64 - bit);
		if (negative) {                                          // two's complement of the shifted magnitude
			std::uint64_t carry = 1;
			for (unsigned i = 0; i < words; ++i) {
				const std::uint64_t v = ~part[i] + carry;
				carry = (carry && v == 0) ? 1 : 0;
				part[i] = v;
			}
		}
		std::uint64_t carry = 0;
		for (unsigned i = 0; i < words; ++i) {
			const std::uint64_t s = w[i] + part[i];
			const std::uint64_t c1 = s < w[i] ? 1 : 0;
			const std::uint64_t t = s + carry;
			const std::uint64_t c2 = t < s ? 1 : 0;
			w[i] = t;
			carry = c1 | c2;
		}
	}
	bool test(unsigned i) const { return ((w[i / 64] >> (i % 64)) & 1u) != 0; }
	int msb() const {
		for (int i = static_cast<int>(words) - 1; i >= 0; --i) {
			if (w[i] != 0) {
				int b = 63;
				while (((w[i] >> b) & 1u) == 0) --b;
				return i * 64 + b;
			}
		}
		return -1;
	}
	bool any_below(int i) const {                                // any bit in [0, i)
		for (int k = 0; k < i; ++k) if (test(static_cast<unsigned>(k))) return true;
		return false;
	}
};

// The quad-double rounded once, correctly, into the b-posit: the limbs are summed exactly in
// fixed point below the leading limb, then encoded with a sticky bit.  NaN is NaR.
template<typename B>
B round_to(const qd& v) {
	const double v0 = v[0];
	if (v0 != v0 || std::isinf(v0)) return B(SpecificValue::nar);
	if (v0 == 0.0) return B(0);
	const bool negative = v0 < 0.0;
	const int top  = std::ilogb(v0);
	const int unit = top - 360;                                  // the accumulator's bit 0
	wide_accumulator acc;
	for (int i = 0; i < 4; ++i) {
		const double li = negative ? -v[i] : v[i];
		if (li == 0.0) continue;
		const int ei = std::ilogb(li) - 52;                      // li = m 2^ei with m a 53-bit integer
		if (ei < unit) {
			// 300 and more bits below the leading bit: only the sign of this tail matters, and
			// a quad-double's tail has the sign of its leading limb
			acc.add_shifted(1u, 0u, li < 0.0);
			break;
		}
		const double m = std::ldexp(li, -ei);
		const bool mneg = m < 0.0;
		acc.add_shifted(static_cast<std::uint64_t>(mneg ? -m : m), static_cast<unsigned>(ei - unit), mneg);
	}
	const int msb = acc.msb();
	if (msb < 0) return B(0);
	constexpr unsigned q = 62;
	std::uint64_t frac = 0;
	for (unsigned k = 1; k <= q; ++k) {
		const int pos = msb - static_cast<int>(k);
		frac = (frac << 1) | ((pos >= 0 && acc.test(static_cast<unsigned>(pos))) ? 1u : 0u);
	}
	const bool sticky = acc.any_below(msb - static_cast<int>(q));
	B r;
	r.setbits(B::encode(negative, msb + unit, frac, q, sticky));
	return r;
}

// the b-posit that every value within err of y rounds to, if there is one
template<typename B>
std::optional<B> decide(const qd& y, const qd& err) {
	const B lo = round_to<B>(y - err);
	const B hi = round_to<B>(y + err);
	if (lo == hi) return lo;
	return std::nullopt;
}

// The quad-double value is within 2^-170 of a rounding boundary and the true value is on
// it: it is a midpoint (a representable value is never ambiguous), so round to the even one.
template<typename B>
B snap(const qd& y) {
	const qd d = qd(std::ldexp(std::fabs(y[0]), snap_exponent));
	const B lo = round_to<B>(y - d);
	const B hi = round_to<B>(y + d);
	if (lo == hi) return lo;
	return (lo.raw() & 1u) == 0 ? lo : hi;
}

// the correctly rounded value of a quad-double result with a relative error below 2^-170
template<typename B>
B from_qd(const qd& y) {
	if (y[0] != y[0]) return B(SpecificValue::nar);
	if (y[0] == 0.0) return B(0);
	if (auto r = decide<B>(y, qd(std::ldexp(std::fabs(y[0]), qd_error_exponent)))) return *r;
	return snap<B>(y);
}

// The fast path: a libm result in double, within BPOSIT_MATH_LIBM_ULPS ulps of the true
// value.  y - err is exact in double; y + err can round down by half an ulp where it crosses
// into the next binade, which still leaves the interval wider than libm's error by margin.
// The b-posit's conversion from double is correctly rounded, so the two ends are rounded
// exactly as round_to would, at a fraction of the cost.
template<typename B>
std::optional<B> from_libm(double y) {
	if (!(std::fabs(y) >= std::numeric_limits<double>::min()) || std::isinf(y)) return std::nullopt;   // 0, subnormal, inf, NaN
	const double err = std::ldexp(static_cast<double>(BPOSIT_MATH_LIBM_ULPS), std::ilogb(y) - 52);
	const B lo(y - err), hi(y + err);
	if (lo == hi) return lo;
	return std::nullopt;
}

// Ziv: the libm result if it decides the rounding, the quad-double one otherwise
template<engine E, typename B, typename Libm, typename Kernel>
B evaluate(const B& x, Libm libm, Kernel kernel) {
	if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<B>) {
		if (auto r = from_libm<B>(libm(double(x)))) return *r;
	}
	return from_qd<B>(kernel(to_qd(x)));
}

template<engine E, typename B, typename Libm, typename Kernel>
B evaluate(const B& x, const B& y, Libm libm, Kernel kernel) {
	if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<B>) {
		if (auto r = from_libm<B>(libm(double(x), double(y)))) return *r;
	}
	return from_qd<B>(kernel(to_qd(x), to_qd(y)));
}

// does a value of this magnitude, 2^k, leave the b-posit's range: the true value then
// saturates, and the kernels are spared arguments that would leave double's
template<typename B>
constexpr bool above_range(double log2_magnitude) { return log2_magnitude > B::maxscale + 2; }
template<typename B>
constexpr bool below_range(double log2_magnitude) { return log2_magnitude < B::minscale - 2; }

}}} // namespace sw::universal::bposit_math
