#pragma once
// logarithm.hpp: log, log2, log10 and log1p for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// log is exact for X >= 1: ln phi(x) = phi(x - 1), so it subtracts 1 from the encoded l + f.
// Below 1, ln X = -ln(1/X) is negative with magnitude -ln g, an ordinary computed value.
// log(0) and the log of a negative value are NaR: there is no -inf.
#include <universal/number/lindex/math/constants.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> log(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero() || v.isneg()) return L(SpecificValue::nar);
	const std::uint64_t mag = v.magnitude_bits();
	if (mag >= L::one_bits) {
		L r;
		r.setbits(mag - L::one_bits);   // exact
		return r;
	}
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::log;
		return L::template from_point<W>(level_index::psi(W(-log(L::template index_in<W>(mag)))));
	}, true);
}

namespace lindex_math {
// log_b v = ln v / ln b for a constant ln b > 0
template<typename L, typename Lnb>
L log_base(const L& v, Lnb ln_b) {
	if (v.isnar() || v.iszero() || v.isneg()) return L(SpecificValue::nar);
	const std::uint64_t mag = v.magnitude_bits();
	if (mag >= L::one_bits) {
		// ln X = phi(x - 1) is the exact point one level down
		return L::round_psi([&](auto w) {
			using W = decltype(w);
			level_index::point<W> p = L::template point_of<W>(mag);
			--p.level;
			return L::template div_points<W>(p, level_index::psi(ln_b(w)));
		}, false);
	}
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::log;
		return L::template from_point<W>(level_index::psi(W(-log(L::template index_in<W>(mag)) / ln_b(w))));
	}, true);
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> log2(const lindex<nbits, lbits, bt>& v) {
	return lindex_math::log_base(v, [](auto w) { return lindex_math::ln2<decltype(w)>(); });
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> log10(const lindex<nbits, lbits, bt>& v) {
	return lindex_math::log_base(v, [](auto w) { return lindex_math::ln10<decltype(w)>(); });
}

// ln(1 + X).  For X > 0, 1 + X is a sum of points, and its log is the exact step down; for
// -1 < X < 0 the magnitude -ln(1 - g) is computed.  log1p(-1) and below are NaR.
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> log1p(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (!v.isneg()) {
		return L::round_psi([&](auto w) {
			using W = decltype(w);
			return L::template down_a_level<W>(L::template add_points<W>(L::template point_of<W>(mag), level_index::point<W>{ 1, W(0) }));
		}, false);
	}
	if (mag >= L::one_bits) return L(SpecificValue::nar);
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::log1p;
		return L::template from_point<W>(level_index::psi(W(-log1p(-L::template index_in<W>(mag)))));
	}, true);
}

}}  // namespace sw::universal
