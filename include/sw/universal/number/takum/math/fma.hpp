#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for takum (linear takum encoding)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum has no extended-precision blocktriple product path, so fma widens the
// operands to double, forms a*b + c with std::fma (one IEEE rounding to double),
// and rounds the result once into takum via the value constructor. A takum's
// significand precision is well under double's 53 bits for the practical
// configurations, so double(x) is the exact represented value, the product is
// exact in double, and the subsequent double -> takum rounding is the single
// rounding that determines the result -- the double intermediate carries far more
// precision than the takum target. takum's non-real state (NaR) absorbs the IEEE
// specials: inf*0 -> NaN -> NaR, and any inf / NaN operand -> NaR.
//
// Sub-issue of #1189 (universal fma, linear takum epic #592). Relates to #1195.

#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> fma(const takum<nbits, rbits, bt>& a, const takum<nbits, rbits, bt>& b,
                            const takum<nbits, rbits, bt>& c) {
	return takum<nbits, rbits, bt>(std::fma(double(a), double(b), double(c)));
}

}} // namespace sw::universal
