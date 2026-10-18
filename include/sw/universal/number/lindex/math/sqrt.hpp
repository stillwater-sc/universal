#pragma once
// sqrt.hpp: sqrt and cbrt for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// For X >= 1, the root X^(1/n) = e^(ln X / n) is one psi-domain product one level down:
//     psi(X^(1/n)) = 1 + psi(phi(x - 1) / n)
// which holds at every level.  Below 1 the root is computed directly, at level 0.
#include <universal/number/lindex/math/constants.hpp>

namespace sw { namespace universal {

namespace lindex_math {
// |v|^(1/n) for nonzero |v|, the reciprocal 1/n given in W
template<typename L, typename Inverse>
L root_magnitude(std::uint64_t mag, bool negative, Inverse inverse_n) {
	if (mag >= L::one_bits) {
		return L::round_psi([&](auto w) {
			using W = decltype(w);
			level_index::point<W> p = L::template point_of<W>(mag);
			--p.level;
			return L::template up_a_level<W>(L::template mul_points<W>(p, level_index::point<W>{ 0, inverse_n(w) }));
		}, negative);
	}
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::exp;
		using std::log;
		return L::template from_point<W>(level_index::psi(W(exp(log(L::template index_in<W>(mag)) * inverse_n(w)))));
	}, negative);
}
}  // namespace lindex_math

// sqrt of a negative value is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> sqrt(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	if (v.isneg()) return L(SpecificValue::nar);
	return lindex_math::root_magnitude<L>(v.magnitude_bits(), false, [](auto w) { return decltype(w)(0.5); });
}

// cbrt is odd: cbrt(-X) = -cbrt(X)
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> cbrt(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	return lindex_math::root_magnitude<L>(v.magnitude_bits(), v.isneg(), [](auto w) { using W = decltype(w); return W(W(1) / W(3)); });
}

}}  // namespace sw::universal
