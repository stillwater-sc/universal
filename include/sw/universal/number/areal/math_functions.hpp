#pragma once
// math_functions.hpp: definition of arbitrary real mathematical functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>

namespace sw { namespace universal {

// fma(a,b,c) = a*b + c, faithfully rounded into areal with the uncertainty bit.
//
// areal is a faithful floating-point encoding: the least-significant bit is the
// uncertainty bit (ubit), which marks whether a value is exact (ubit = 0) or is a
// representative of the open interval to the adjacent encoding (ubit = 1). fma forms
// the fused product-sum in a wide double intermediate and converts the result once
// into areal via the value constructor, which performs the faithful rounding:
//   - if the fused product-sum is exactly representable, the areal is exact (ubit = 0);
//   - otherwise the areal stores the truncated representative and sets ubit = 1,
//     denoting the open interval (v, v + ulp) for v > 0 (or (v - ulp, v) for v < 0)
//     that contains the true value of a*b + c.
// So the fusion is a single faithful rounding of a*b + c into areal -- there is no
// intermediate rounding to an exact areal product before adding c. Inputs are taken
// by their represented value double(x), and the IEEE special cases (inf*0 -> NaN,
// NaN / inf propagation) come from std::fma.
//
// NOTE on width: for an areal whose significand is narrow enough that a product is
// exact in double (2*fbits <= 52) -- which covers the practical configurations -- the
// double intermediate carries the correctly-rounded a*b + c, so the areal ubit is the
// faithful-rounding uncertainty of the true fused result. Wider significands would
// incur a benign extra rounding in the double intermediate before the areal rounding.
//
// Sub-issue of #1189 (universal fma). Relates to #1194.
//
// An inexact input carries over: double(x) of an areal with its ubit set is the lower end of its
// interval, so the fused result of those values is marked inexact as well. fma ignored the input
// ubits: fma(areal<16,5>(0.1f), 1, 0) was exact while 0.1 * 1 + 0 was not (#1970). Like + - * /,
// this flags the result inexact; it does not promise that the result's interval encloses the
// exact interval result (see areal_impl.hpp).
template<unsigned nbits, unsigned es, typename bt>
areal<nbits, es, bt> fma(const areal<nbits, es, bt>& a, const areal<nbits, es, bt>& b, const areal<nbits, es, bt>& c) {
	areal<nbits, es, bt> r(std::fma(double(a), double(b), double(c)));
	const bool inexactInput = (a.at(0) && !a.isnan() && !a.isinf()) || (b.at(0) && !b.isnan() && !b.isinf()) || (c.at(0) && !c.isnan() && !c.isinf());
	if (inexactInput && !r.isnan() && !r.isinf()) r.set(0);
	return r;
}

}} // namespace sw::universal
