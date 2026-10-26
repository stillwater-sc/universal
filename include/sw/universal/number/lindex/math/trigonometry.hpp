#pragma once
// trigonometry.hpp: sin, cos, tan, asin, acos, atan and atan2 for lindex (#1715)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sin, cos and tan of X past the double range (psi > psi(DBL_MAX) = 4.632) are NaR by design:
// X mod 2 pi is not determined by anything the format can compute, since phi(5) already has
// 1.6 million decimal digits in its exponent and Payne-Hanek would need that many bits of pi.
// Within the double range X is formed in efloat to 1024 + fbits bits and more as the argument
// grows (value_domain.hpp, round_exact), so the reduction is exact and the result is
// correctly rounded on the level-0 grid even next to a zero of the function.
//
// asin and acos take |X| <= 1, an exact level-0 value, through the atan2 forms
//     asin x = atan2(x, sqrt((1 - x)(1 + x))),   acos x = 2 atan2(sqrt(1 - x), sqrt(1 + x))
// whose factors 1 +- x are exact, and are NaR past 1.  atan of a huge X is pi/2 - 1/X, and
// 1/X is far below the grid from level 5 up.  atan2 forms the ratio |Y| / |X| in the psi
// domain, so neither operand's value need be formed.
#include <universal/number/lindex/math/value_domain.hpp>

namespace sw { namespace universal {

namespace lindex_math {
// sin, cos or tan of the lindex of magnitude mag and sign negative, X within the double range
template<typename L, typename F>
L trigonometric(std::uint64_t mag, bool negative, F f) {
	const double X = level_index::phi(L::template point_of<double>(mag));
	const unsigned scale = (X > 1.0) ? static_cast<unsigned>(std::ceil(std::log2(X))) : 0u;
	return round_exact<L>([&](unsigned bits) {
		exact_float x = phi_value<L>(mag, bits + scale + 8u);   // X to 2^-(bits + 8) absolute
		if (negative) x.setsign(true);
		return f(x);
	}, L::fbits + 64u);
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> sin(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (lindex_math::past_double_range<L>(mag)) return L(SpecificValue::nar);
	return lindex_math::trigonometric<L>(mag, v.isneg(), [](const lindex_math::exact_float& x) { return sin(x); });
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> cos(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar()) return v;
	if (v.iszero()) return L(1);
	const std::uint64_t mag = v.magnitude_bits();
	if (lindex_math::past_double_range<L>(mag)) return L(SpecificValue::nar);
	return lindex_math::trigonometric<L>(mag, false, [](const lindex_math::exact_float& x) { return cos(x); });
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> tan(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (lindex_math::past_double_range<L>(mag)) return L(SpecificValue::nar);
	return lindex_math::trigonometric<L>(mag, v.isneg(), [](const lindex_math::exact_float& x) { return tan(x); });
}

// asin of |X| > 1 is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> asin(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (mag > L::one_bits) return L(SpecificValue::nar);
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::atan2;
		using std::sqrt;
		const W x = (mag == L::one_bits) ? W(1) : L::template index_in<W>(mag);
		const W r = atan2(x, sqrt((W(1) - x) * (W(1) + x)));
		return negative ? W(-r) : r;
	});
}

// acos of |X| > 1 is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> acos(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (mag > L::one_bits) return L(SpecificValue::nar);
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::atan2;
		using std::sqrt;
		W x = (mag == L::one_bits) ? W(1) : L::template index_in<W>(mag);
		if (negative) x = -x;
		return W(W(2) * atan2(sqrt(W(1) - x), sqrt(W(1) + x)));
	});
}

namespace lindex_math {
// atan of the psi point p >= 0 in W: pi/2 once the value is past W, where 1/X is far below
// every grid
template<typename W>
W atan_of_point(const level_index::point<W>& p) {
	using std::atan;
	if (p.level >= 5) return pi<W>() / W(2);
	const W X = level_index::phi(p);
	// past 1e150, 1/X < 2^-498 is far below every grid, and the cascades' reciprocal of a value
	// near the top of the double range is subnormal
	if (!(X < W(1.0e150))) return pi<W>() / W(2);
	return atan(X);
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> atan(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		const W r = lindex_math::atan_of_point<W>(L::template point_of<W>(mag));
		return negative ? W(-r) : r;
	});
}

// atan2(Y, X) with the conventions of std::atan2; lindex has no signed zero, so
// atan2(0, X) is 0 for X >= 0 and pi for X < 0
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> atan2(const lindex<nbits, lbits, bt>& y, const lindex<nbits, lbits, bt>& x) {
	using L = lindex<nbits, lbits, bt>;
	if (y.isnar() || x.isnar()) return L(SpecificValue::nar);
	const bool sy = y.isneg(), sx = x.isneg();
	if (y.iszero()) {
		if (!sx) return L(0);
		return lindex_math::round_value<L>([](auto w) { return lindex_math::pi<decltype(w)>(); });
	}
	if (x.iszero()) {
		return lindex_math::round_value<L>([&](auto w) {
			using W = decltype(w);
			const W r = lindex_math::pi<W>() / W(2);
			return sy ? W(-r) : r;
		});
	}
	const std::uint64_t a = y.magnitude_bits(), b = x.magnitude_bits();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		// theta = atan(|Y| / |X|), the ratio's psi point formed in the psi domain
		const level_index::point<W> ratio = L::template div_points<W>(L::template point_of<W>(a), L::template point_of<W>(b)).p;
		const W theta = lindex_math::atan_of_point<W>(ratio);
		const W r = sx ? W(lindex_math::pi<W>() - theta) : theta;
		return sy ? W(-r) : r;
	});
}

}}  // namespace sw::universal
