#pragma once
// fractional.hpp: fmod and remainder for lindex (#1715)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fmod(X, Y) = X - nY with n = trunc(X / Y), and remainder(X, Y) with n the nearest integer,
// ties to even.  n is astronomically large for most pairs, and the result can cancel below
// minpos, so the domain is:
//
//   |X| <  |Y|                    fmod is X itself; remainder is X, or X -+ |Y| past |Y| / 2,
//                                 a difference rounded as lindex subtraction rounds it
//   |X| >= |Y|, X in double range X and Y are formed in efloat to the bits X / Y needs, n and
//                                 X - nY exactly, and the result rounded once (round_exact)
//   |X| >= |Y|, X past the range  NaR: X - nY needs X to more bits than phi(5) has digits
//
// and NaR for Y = 0.  The result of two level-0 operands is exact.
#include <universal/number/lindex/math/value_domain.hpp>

namespace sw { namespace universal {

namespace lindex_math {
// |X| - n |Y| for the magnitudes a >= b within the double range, and n odd, with n = trunc(|X| / |Y|)
template<typename L>
exact_float fmod_magnitude(std::uint64_t a, std::uint64_t b, unsigned bits, unsigned scale_x, bool& n_odd) {
	using E = exact_float;
	const unsigned p = bits + scale_x + 8u;
	const E X = phi_value<L>(a, p), Y = phi_value<L>(b, p);
	E n = trunc(X / Y);   // at p bits, enough for n's integer part; a rounded-up n is corrected below
	// n Y and X - n Y exactly: n has at most scale(X) - scale(Y) + 1 bits
	const unsigned exact = 2u * p + static_cast<unsigned>(std::max<std::int64_t>(0, n.scale())) + 128u;
	E ny(n);
	ny.set_precision(exact);
	ny *= Y;
	E r(X);
	r.set_precision(exact);
	r -= ny;
	const E zero(0);
	while (r < zero) { r += Y; n -= 1; }
	while (r >= Y)   { r -= Y; n += 1; }
	E half_n(n);
	if (!half_n.iszero()) half_n.setexponent(half_n.scale() - 1);
	n_odd = !n.iszero() && trunc(half_n) != half_n;
	return r;
}

inline const level_index::point<qd_cascade>& psi_of_two() {
	static const level_index::point<qd_cascade> two = level_index::psi(qd_cascade(2.0));
	return two;
}

template<typename L>
unsigned value_scale_of(std::uint64_t mag) noexcept {
	const double X = level_index::phi(L::template point_of<double>(mag));
	return (X > 1.0) ? static_cast<unsigned>(std::ceil(std::log2(X))) : 0u;
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> fmod(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	using L = lindex<nbits, lbits, bt>;
	if (x.isnar() || y.isnar() || y.iszero()) return L(SpecificValue::nar);
	if (x.iszero()) return x;
	const std::uint64_t a = x.magnitude_bits(), b = y.magnitude_bits();
	if (a < b) return x;
	if (lindex_math::past_double_range<L>(a)) return L(SpecificValue::nar);
	const bool negative = x.isneg();
	const unsigned scale = lindex_math::value_scale_of<L>(a);
	return lindex_math::round_exact<L>([&](unsigned bits) {
		bool n_odd = false;
		lindex_math::exact_float r = lindex_math::fmod_magnitude<L>(a, b, bits, scale, n_odd);
		if (negative && !r.iszero()) r.setsign(true);
		return r;
	}, L::fbits + 64u);
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> remainder(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	using L = lindex<nbits, lbits, bt>;
	if (x.isnar() || y.isnar() || y.iszero()) return L(SpecificValue::nar);
	if (x.iszero()) return x;
	const std::uint64_t a = x.magnitude_bits(), b = y.magnitude_bits();
	const bool negative = x.isneg();
	if (a < b) {
		// n is 0 or +-1: |X| against |Y| / 2, the tie to n = 0
		const level_index::point<qd_cascade> half = L::template div_points<qd_cascade>(L::template point_of<qd_cascade>(b), lindex_math::psi_of_two()).p;
		if (!L::template point_less<qd_cascade>(half, L::template point_of<qd_cascade>(a))) return x;
		const L d = abs(y) - abs(x);   // X - sign(X) |Y| = -sign(X) (|Y| - |X|)
		return negative ? d : -d;
	}
	if (lindex_math::past_double_range<L>(a)) return L(SpecificValue::nar);
	const unsigned scale = lindex_math::value_scale_of<L>(a);
	return lindex_math::round_exact<L>([&](unsigned bits) {
		using E = lindex_math::exact_float;
		bool n_odd = false;
		E r = lindex_math::fmod_magnitude<L>(a, b, bits, scale, n_odd);
		const E Y = lindex_math::phi_value<L>(b, bits + scale + 8u);
		E twice(r);
		if (!twice.iszero()) twice.setexponent(twice.scale() + 1);
		if (twice > Y || (twice == Y && n_odd)) r -= Y;
		if (negative) r = -r;
		return r;
	}, L::fbits + 64u);
}

}}  // namespace sw::universal
