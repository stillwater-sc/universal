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

// ---------------------------------------------------------------------------
// takum_log: fused multiply-add.  The multiply is exact in the logarithmic
// domain but the addition is not, so this follows the linear takum and fuses in
// double.
//
// That bridge is only faithful while the operands AND the exact product-sum are
// representable in binary64, which is not the whole of the type: at rbits = 5
// the characteristic reaches ~2^32 and about a third of all finite encodings
// convert to infinity as a double.  Once that happens the result is whatever
// std::fma makes of an infinity -- with b == 0 that is NaN, and the constructor
// turns it into NaR.  A range-safe native path, which would add in the linear
// domain without leaving the type, is follow-up work.
// ---------------------------------------------------------------------------

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> fma(const takum_log<nbits, rbits, bt>& a,
                                       const takum_log<nbits, rbits, bt>& b,
                                       const takum_log<nbits, rbits, bt>& c) {
	return takum_log<nbits, rbits, bt>(std::fma(double(a), double(b), double(c)));
}

}} // namespace sw::universal
