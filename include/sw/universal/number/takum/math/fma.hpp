#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for takum (linear takum encoding)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// For configurations with nbits <= 54 + rbits, fma widens the operands to
// double, forms a*b + c with std::fma (one IEEE rounding to double), and rounds
// the result once into takum via the value constructor. A takum's significand is
// 1 + p bits with p reaching nbits - 2 - rbits, so up to and including that
// threshold -- 57 bits for the specified rbits = 3, where the significand is
// exactly 53 -- it still fits a double, and double(x) is the exact represented
// value, the product is exact in double, and the subsequent double -> takum
// rounding is the single rounding that determines the result. takum's non-real
// state (NaR) absorbs the IEEE specials: inf*0 -> NaN -> NaR, and any inf / NaN
// operand -> NaR.
//
// ABOVE that threshold the reasoning fails, and it fails before the fma
// begins: takum<64,3> carries a 60-bit significand, so double(a) is already a
// rounded operand and std::fma then delivers an exact product of the wrong
// numbers. Those configurations take the exact integer path instead
// (takum_wide_arithmetic.hpp): the significand product is exact in 128 bits, c is
// aligned into the same window, and the single rounding happens once at the end
// in the codec -- which is what fma is FOR. Issue #1300.
//
// Sub-issue of #1189 (universal fma, linear takum epic #592). Relates to #1195.

#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> fma(const takum<nbits, rbits, bt>& a, const takum<nbits, rbits, bt>& b,
                            const takum<nbits, rbits, bt>& c) {
	using Takum = takum<nbits, rbits, bt>;
	if constexpr (Takum::wide_significand) {
		Takum result;
		if (a.isnar() || b.isnar() || c.isnar()) { result.setnar(); return result; }
		// takum has no signed zero, so a vanishing product is simply absent from
		// the sum and there is no -0 + 0 sign convention to preserve.
		if (a.iszero() || b.iszero()) return c;
		auto product = takum_wide::multiply(a.to_wide_operand(), b.to_wide_operand());
		if (c.iszero()) return result.assign_wide(product);
		return result.assign_wide(
			takum_wide::sum(product, takum_wide::widen(c.to_wide_operand()), false));
	}
	else {
		return Takum(std::fma(double(a), double(b), double(c)));
	}
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
