#pragma once
// error_and_gamma.hpp: erf, erfc, lgamma and tgamma for lindex (#1715)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// erf saturates to +-1 and erfc of a large X underflows into level 0, then to 0 (or rises to
// 2 for X < 0).  For X > 0:
//     lgamma X = X (ln X - 1) (1 + O(1/X)),  psi(lgamma X) = psi(phi(x) phi(psi(ln X - 1)))
// a product of psi points at every level once lgamma is past the working type, and
//     tgamma X = e^lgamma X,  psi(tgamma X) = 1 + psi(lgamma X)
// the exact step up wherever lgamma >= 0, that is outside 1 < X < 2.
//
// For X < 0 the reflection Gamma(X) = pi / (sin(pi X) Gamma(1 - X)) needs sin(pi X) of the
// value X, so it goes through the exact path (value_domain.hpp) and, like sin, is NaR past
// the double range.  The poles are 0 and -1, the only negative integer a lindex holds; there
// lgamma and tgamma are NaR, since lindex has no infinity.
#include <universal/number/lindex/math/value_domain.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> erf(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::erf;
		const W X = level_index::phi(L::template point_of<W>(mag));
		const W r = (X < W(16)) ? W(erf(X)) : W(1);   // erfc(16) < 2^-370
		return negative ? W(-r) : r;
	});
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> erfc(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar()) return v;
	if (v.iszero()) return L(1);
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::erfc;
		W X = level_index::phi(L::template point_of<W>(mag));
		if (X >= W(32)) return negative ? W(2) : W(0);   // erfc(32) < 2^-1480
		if (negative) X = -X;
		return W(erfc(X));
	});
}

namespace lindex_math {
// the magnitude encoding of 1 < X < 2, where lgamma X < 0
template<typename L>
bool between_one_and_two(std::uint64_t mag) noexcept {
	static const level_index::point<qd_cascade> two = level_index::psi(qd_cascade(2.0));
	return mag > L::one_bits && L::template point_less<qd_cascade>(L::template point_of<qd_cascade>(mag), two);
}

// whether lgamma X of the magnitude mag is past the working type: X >= 2^1000
template<typename L>
bool lgamma_past_working(std::uint64_t mag) noexcept {
	static const level_index::point<qd_cascade> edge = level_index::psi(qd_cascade(std::ldexp(1.0, 1000)));
	return !L::template point_less<qd_cascade>(L::template point_of<qd_cascade>(mag), edge);
}

// psi(lgamma X) for X > 0 and lgamma X >= 0
template<typename L, typename W>
typename L::template outcome<W> lgamma_point(std::uint64_t mag, bool past_working) {
	using std::lgamma;
	const level_index::point<W> x = L::template point_of<W>(mag);
	if (!past_working) return L::template from_point<W>(level_index::psi(W(lgamma(level_index::phi(x)))));
	// X (ln X - 1), with ln X - 1 = phi(x - 1) - 1 >= 692 a difference of psi points
	const level_index::point<W> lnx_minus_1 = L::template sub_points<W>(level_index::point<W>{ x.level - 1, x.index }, level_index::point<W>{ 1, W(0) }).p;
	return L::template mul_points<W>(x, lnx_minus_1);
}

// lgamma z for z >= 1: qd_cascade below 2^64, and past it the Stirling series
//     (z - 1/2) ln z - z + ln(2 pi) / 2 + 1/(12 z) - 1/(360 z^3) + 1/(1260 z^5)
// in efloat, whose next term is below 2^-380 and whose value may be past the double range
inline exact_float lgamma_of(const exact_float& z, const exact_float& pi_p) {
	const unsigned p = z.get_precision();
	if (z.scale() < 64) {
		using std::lgamma;
		const qd_cascade lg = lgamma(to_qd(z));
		const bool negative = lg < qd_cascade(0);
		exact_float r = from_qd(negative ? qd_cascade(-lg) : lg, p);
		if (negative) r.setsign(true);
		return r;
	}
	exact_float half(1);
	half.set_precision(p);
	half.setexponent(-1);
	exact_float two_pi(pi_p);
	two_pi.setexponent(two_pi.scale() + 1);
	const exact_float inv = half / z * 2;   // 1 / z
	const exact_float inv2 = inv * inv;
	exact_float series = inv / 12 - inv * inv2 / 360 + inv * inv2 * inv2 / 1260;
	return (z - half) * log(z) - z + log(two_pi) * half + series;
}

// ln |Gamma(-X)| for the value X > 0, -X not a pole, and the sign of Gamma(-X), at `bits` bits:
// ln pi - ln |sin(pi X)| - lgamma(1 + X), the last in qd_cascade
template<typename L>
exact_float reflected_lgamma(std::uint64_t mag, unsigned bits, unsigned scale, bool& gamma_negative) {
	const unsigned p = bits + scale + 8u;
	const exact_float X = phi_value<L>(mag, p);
	const exact_float pi_p = efloat_detail::pi<2>(p + 8u);
	exact_float s = sin(pi_p * X);
	gamma_negative = s.sign() == 1;     // sin(-pi X) = -sin(pi X)
	s.setsign(false);
	exact_float z(1);
	z.set_precision(p);
	z += X;
	return log(pi_p) - log(s) - lgamma_of(z, pi_p);
}

template<typename L>
unsigned value_scale(std::uint64_t mag) noexcept {
	const double X = level_index::phi(L::template point_of<double>(mag));
	return (X > 1.0) ? static_cast<unsigned>(std::ceil(std::log2(X))) : 0u;
}
}  // namespace lindex_math

// lgamma at the poles 0 and -1, and of negative X past the double range, is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> lgamma(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return L(SpecificValue::nar);
	const std::uint64_t mag = v.magnitude_bits();
	if (v.isneg()) {
		if (mag == L::one_bits || lindex_math::past_double_range<L>(mag)) return L(SpecificValue::nar);
		const unsigned scale = lindex_math::value_scale<L>(mag);
		return lindex_math::round_exact<L>([&](unsigned bits) {
			bool gamma_negative = false;
			return lindex_math::reflected_lgamma<L>(mag, bits, scale, gamma_negative);
		}, L::fbits + 64u);
	}
	const bool past = lindex_math::lgamma_past_working<L>(mag);
	if (past) return L::round_psi([&](auto w) { return lindex_math::lgamma_point<L, decltype(w)>(mag, true); }, false);
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::lgamma;
		return W(lgamma(level_index::phi(L::template point_of<W>(mag))));
	});
}

// tgamma at the poles 0 and -1, and of negative X past the double range, is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> tgamma(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return L(SpecificValue::nar);
	const std::uint64_t mag = v.magnitude_bits();
	if (v.isneg()) {
		if (mag == L::one_bits || lindex_math::past_double_range<L>(mag)) return L(SpecificValue::nar);
		const unsigned scale = lindex_math::value_scale<L>(mag);
		return lindex_math::round_exact<L>([&](unsigned bits) {
			bool gamma_negative = false;
			const lindex_math::exact_float lg = lindex_math::reflected_lgamma<L>(mag, bits, scale, gamma_negative);
			// below 2^-(fbits + 16) the magnitude rounds to 0, and e^lg may be past efloat's exponent
			if (double(lg) < -0.6931471805599453 * static_cast<double>(L::fbits + 16u)) return lindex_math::exact_float(0);
			lindex_math::exact_float g = exp(lg);
			if (gamma_negative) g.setsign(true);
			return g;
		}, L::fbits + 64u);
	}
	if (lindex_math::between_one_and_two<L>(mag)) {
		// lgamma < 0: tgamma is in (0.885, 1), at level 0
		return lindex_math::round_value<L>([&](auto w) {
			using W = decltype(w);
			using std::tgamma;
			return W(tgamma(level_index::phi(L::template point_of<W>(mag))));
		});
	}
	const bool past = lindex_math::lgamma_past_working<L>(mag);
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		return L::template up_a_level<W>(lindex_math::lgamma_point<L, W>(mag, past));
	}, false);
}

}}  // namespace sw::universal
