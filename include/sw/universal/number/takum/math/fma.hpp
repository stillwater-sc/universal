#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for takum (linear takum encoding)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fma rounds once, at every width, by evaluating exactly in integers
// (takum_wide_arithmetic.hpp): the significand product is exact in 128 bits, c is
// aligned into the same window, and the single rounding happens at the end in the
// codec -- which is what fma is FOR.
//
// It used to do that only above nbits = 54 + rbits, where a double cannot even
// hold the operands (takum<64,3>'s significand is 60 bits, #1300), and to evaluate
// narrower widths with std::fma.  That rounds twice -- once to double, once to
// takum -- and unlike + - * / no width makes it safe: the exact a*b + c can sit a
// hair off a takum midpoint by less than a double ulp, so std::fma lands exactly
// ON the midpoint and the second rounding breaks the tie by evenness instead of by
// the discarded remainder.  takum<16,3> fma(a, b, minpos) was wrong for 2822 of
// the 5648 operand pairs in [1,2) whose product is a midpoint (#1616).
//
// takum's non-real state (NaR) absorbs the IEEE specials: a NaR operand gives NaR,
// and a zero product leaves c unchanged.
//
// Sub-issue of #1189 (universal fma, linear takum epic #592). Relates to #1195.

#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> fma(const takum<nbits, rbits, bt>& a, const takum<nbits, rbits, bt>& b,
                            const takum<nbits, rbits, bt>& c) {
	using Takum = takum<nbits, rbits, bt>;
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
