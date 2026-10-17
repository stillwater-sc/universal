#pragma once
// classify.hpp: classification functions for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A lindex is zero, NaR or a normal value: level 0 is evenly spaced, so there are no
// subnormals, and there is no infinity.
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
int fpclassify(const lindex<nbits, lbits, bt>& v) {
	if (v.isnar()) return FP_NAN;
	if (v.iszero()) return FP_ZERO;
	return FP_NORMAL;
}
template<unsigned nbits, unsigned lbits, typename bt>
bool isnan(const lindex<nbits, lbits, bt>& v) { return v.isnar(); }
template<unsigned nbits, unsigned lbits, typename bt>
bool isinf(const lindex<nbits, lbits, bt>&) { return false; }
template<unsigned nbits, unsigned lbits, typename bt>
bool isfinite(const lindex<nbits, lbits, bt>& v) { return !v.isnar(); }
template<unsigned nbits, unsigned lbits, typename bt>
bool isnormal(const lindex<nbits, lbits, bt>& v) { return !v.isnar() && !v.iszero(); }
// the sign bit of the encoding; NaR, 1000...0, has it set
template<unsigned nbits, unsigned lbits, typename bt>
bool signbit(const lindex<nbits, lbits, bt>& v) { return v.sign(); }

}}  // namespace sw::universal
