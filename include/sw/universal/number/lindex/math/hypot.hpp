#pragma once
// hypot.hpp: hypot for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sqrt(X^2 + Y^2) composed in the psi domain and rounded once: each square is a product one
// level down and a step up, the sum is the kernel's, and the root is the product by 1/2 one
// level down.  No square is formed as a value, so hypot never overflows an intermediate.
#include <universal/number/lindex/math/constants.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> hypot(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	using L = lindex<nbits, lbits, bt>;
	if (x.isnar() || y.isnar()) return L(SpecificValue::nar);
	if (x.iszero()) return abs(y);
	if (y.iszero()) return abs(x);
	const std::uint64_t a = x.magnitude_bits(), b = y.magnitude_bits();
	const std::uint64_t top = (a > b) ? a : b;
	// the square of the top level is past the kernel's levels; hypot rounds to the larger there
	if (static_cast<int>(top >> L::fbits) + 1 > level_index::max_level) return abs(a > b ? x : y);
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::exp;
		using std::log;
		auto square = [](const level_index::point<W>& P) {
			if (P.level == 0) return level_index::point<W>{ 0, W(P.index * P.index) };
			// psi(X^2) = 1 + psi(2 phi(x - 1)), and psi(2) = 1 + ln 2
			return L::template up_a_level<W>(L::template mul_points<W>(level_index::point<W>{ P.level - 1, P.index },
			                                                          level_index::point<W>{ 1, lindex_math::ln2<W>() })).p;
		};
		const level_index::point<W> S = L::template add_points<W>(square(L::template point_of<W>(a)), square(L::template point_of<W>(b))).p;
		if (S.level == 0) return L::template from_point<W>(level_index::psi(W(exp(log(S.index) * W(0.5)))));
		return L::template up_a_level<W>(L::template mul_points<W>(level_index::point<W>{ S.level - 1, S.index }, level_index::point<W>{ 0, W(0.5) }));
	}, false);
}

}}  // namespace sw::universal
