#pragma once
// minmax.hpp: min, max, fmin and fmax for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// min and max follow std::min and std::max, through the encoding order, in which NaR is
// below every value.  fmin and fmax follow IEEE: a NaR operand is ignored.

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> min(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) { return (y < x) ? y : x; }
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> max(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) { return (x < y) ? y : x; }

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> fmin(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	if (x.isnar()) return y;
	if (y.isnar()) return x;
	return (y < x) ? y : x;
}
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> fmax(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	if (x.isnar()) return y;
	if (y.isnar()) return x;
	return (x < y) ? y : x;
}

}}  // namespace sw::universal
