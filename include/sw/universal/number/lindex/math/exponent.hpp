#pragma once
// exponent.hpp: exp, exp2, exp10 and expm1 for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// exp is exact for X >= 0: e^phi(x) = phi(x + 1), so it adds 1 to the encoded l + f, and
// saturates only from the top level.  For X < 0 it is the reciprocal e^-X, at level 0.
// exp2 and exp10 are e^(X ln b): a product one level down, then that exact step up.
#include <universal/number/lindex/math/constants.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> exp(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (!v.isneg()) {
		L r;
		r.setbits(mag > L::maxpos_bits - L::one_bits ? L::maxpos_bits : mag + L::one_bits);   // exact
		return r;
	}
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		return lindex_math::exp_of<L, W>(L::template point_of<W>(mag), true);
	}, false);
}

namespace lindex_math {
// b^v = e^(v ln b) for a constant ln b > 0 given as a psi point
template<typename L, typename Lnb>
L exp_base(const L& v, Lnb ln_b) {
	if (v.isnar()) return v;
	if (v.iszero()) { L one; one.setbits(L::one_bits); return one; }
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		const auto t = L::template mul_points<W>(L::template point_of<W>(mag), level_index::psi(ln_b(w)));
		return exp_of<L, W>(t.p, negative);
	}, false);
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> exp2(const lindex<nbits, lbits, bt>& v) {
	return lindex_math::exp_base(v, [](auto w) { return lindex_math::ln2<decltype(w)>(); });
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> exp10(const lindex<nbits, lbits, bt>& v) {
	return lindex_math::exp_base(v, [](auto w) { return lindex_math::ln10<decltype(w)>(); });
}

// e^X - 1.  For X > 0 it is the difference phi(x + 1) - phi(1) of two exact points, which
// the kernel's expm1 form keeps free of cancellation; for X < 0 its magnitude 1 - e^-X is
// below 1
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> expm1(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (!v.isneg()) {
		if (static_cast<unsigned>(mag >> L::fbits) + 1u >= L::nr_levels) return L(SpecificValue::maxpos);
		return L::round_psi([&](auto w) {
			using W = decltype(w);
			level_index::point<W> p = L::template point_of<W>(mag);
			++p.level;
			return L::template sub_points<W>(p, level_index::point<W>{ 1, W(0) });
		}, false);
	}
	return L::round_psi([&](auto w) {
		using W = decltype(w);
		using std::expm1;
		const W A = level_index::phi(L::template point_of<W>(mag));   // inf past W: the magnitude is 1
		return L::template from_point<W>(level_index::psi(W(-expm1(-A))));
	}, true);
}

}}  // namespace sw::universal
