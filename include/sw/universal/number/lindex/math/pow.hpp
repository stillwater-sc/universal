#pragma once
// pow.hpp: pow for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// X^Y = e^t with t = Y ln X.  ln X is the exact point x - 1 for X >= 1 and -ln g below 1;
// |t| is one psi-domain product, and e^t is the exact step up a level for t >= 0, or a
// level-0 value for t < 0.  So pow is one rounding at every level, like the arithmetic.
//
// A negative base has a real power only for an integer exponent, and the only integers a
// lindex holds exactly are 0 and +-1 (any other encoding is transcendental): pow of a
// negative base is X^0 = 1, X^1 = X and X^-1 = 1/X, and NaR otherwise.
#include <universal/number/lindex/math/constants.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> pow(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	using L = lindex<nbits, lbits, bt>;
	L one;
	one.setbits(L::one_bits);
	if (x.isnar() || y.isnar()) return L(SpecificValue::nar);
	if (y.iszero()) return one;
	if (x.iszero()) return y.isneg() ? L(SpecificValue::nar) : x;   // 0^-y divides by zero
	if (y == one) return x;
	if (y == -one) return x.reciprocal();
	if (x.isneg()) return L(SpecificValue::nar);
	const std::uint64_t mx = x.magnitude_bits(), my = y.magnitude_bits();
	if (mx == L::one_bits) return one;
	const bool ln_negative = mx < L::one_bits;
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::log;
		level_index::point<W> lnX = L::template point_of<W>(mx);
		if (ln_negative) lnX = level_index::psi(W(-log(lnX.index)));
		else --lnX.level;
		const auto t = L::template mul_points<W>(L::template point_of<W>(my), lnX);
		return lindex_math::exp_of<L, W>(t.p, y.isneg() != ln_negative);
	}, false);
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> pow(const lindex<nbits, lbits, bt>& x, double y) { return pow(x, lindex<nbits, lbits, bt>(y)); }

}}  // namespace sw::universal
