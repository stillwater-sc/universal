#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for bfloat16 (Google Brain float)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Single-rounding fused multiply-add: widen the operands to float, form a*b + c
// with std::fma (one IEEE rounding to float), and round the result once into
// bfloat16 via the RNE bfloat16(float) constructor (magic-bias round-to-nearest-even,
// with NaN preserved and out-of-range collapsing to +/-inf).
//
// This is correctly rounded to bfloat16: a bfloat16 carries 8 significand bits, and
// the product of two of them is EXACT in float (16 <= 24 significand bits), so
// std::fma yields the correctly-rounded-to-float a*b + c. Because float's 24 bits
// satisfy p >= 2*q + 2 for the bfloat16 target q = 8 (24 >= 18), the subsequent
// float -> bfloat16 rounding is innocuous: the double rounding equals a single
// round-to-nearest-even of the exact a*b + c. std::fma also gives the IEEE special
// cases (inf*0 + c -> NaN, NaN propagation) for free.
//
// Sub-issue of #1189 (universal fma). Relates to #1193.

#include <cmath>

namespace sw { namespace universal {

inline bfloat16 fma(bfloat16 a, bfloat16 b, bfloat16 c) {
	return bfloat16(std::fma(float(a), float(b), float(c)));
}

}} // namespace sw::universal
