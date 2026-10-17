#pragma once
// truncate.hpp: floor, ceil, trunc and round for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The integer part of a lindex value, rounded back to the nearest lindex.  The only
// integers a lindex holds exactly are 0 and +-1, so floor(2.5) is the lindex nearest 2,
// not 2 itself.  Level 0 is decided exactly on the encoding.  Above it the value is formed
// in dd_cascade, about 2^-104 relative, where floor is exact for any value below 2^100 that
// is not within that distance of an integer; from 2^100 up a lindex is far coarser than 1,
// and the integer part rounds back to the value itself.
#include <universal/number/dd_cascade/math/functions/truncate.hpp>

namespace sw { namespace universal {

namespace lindex_math {
enum class integral_mode { floor, ceil, trunc, round };

template<typename L>
L integral(const L& v, integral_mode mode) {
	using W = typename L::Working;
	if (v.isnar() || v.iszero()) return v;
	const bool negative = v.isneg();
	const std::uint64_t mag = v.magnitude_bits();
	L one;
	one.setbits(L::one_bits);
	// toward +inf for ceil of a positive and floor of a negative value, toward zero for
	// floor of a positive and ceil of a negative one and for trunc, to nearest for round
	const bool away = (mode == integral_mode::ceil && !negative) || (mode == integral_mode::floor && negative);
	if (mag < L::one_bits) {
		// level 0: |v| = g in (0, 1)
		if (mode == integral_mode::round) return (mag >= (L::one_bits >> 1)) ? (negative ? -one : one) : L(0);
		return away ? (negative ? -one : one) : L(0);
	}
	const W V = level_index::phi(L::template point_of<W>(mag));
	if (!(V.high() < 0x1p100)) return v;
	W I;
	if (mode == integral_mode::round) I = floor(W(V + W(0.5)));   // half away from zero on |v|
	else I = away ? ceil(V) : floor(V);
	if (I == W(0)) return L(0);
	return L::from_value(I, negative);
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> floor(const lindex<nbits, lbits, bt>& v) { return lindex_math::integral(v, lindex_math::integral_mode::floor); }
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> ceil(const lindex<nbits, lbits, bt>& v) { return lindex_math::integral(v, lindex_math::integral_mode::ceil); }
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> trunc(const lindex<nbits, lbits, bt>& v) { return lindex_math::integral(v, lindex_math::integral_mode::trunc); }
// halfway cases away from zero, as std::round
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> round(const lindex<nbits, lbits, bt>& v) { return lindex_math::integral(v, lindex_math::integral_mode::round); }

}}  // namespace sw::universal
