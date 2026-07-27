#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for fixpnt, with a single rounding
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The product a*b is formed EXACTLY as a 2*nbits two's-complement value (via
// urmul2, the same primitive fixpnt::operator*= uses), c is aligned into that
// wide fixed-point frame and added exactly, and only THEN is the result rounded
// (round-to-nearest-even at bit rbits) and range-limited (Modulo wrap / Saturate
// clamp). This matches fixpnt::operator*='s rounding model exactly, but with c
// folded in before the single rounding -- so fma is strictly more accurate than
// the two-rounding form fixpnt(a*b) + c whenever the product's low-order bits
// matter (e.g. under cancellation, c ~ -a*b).
//
// Sub-issue of #1189 (universal fma). Relates to #1191.

#include <universal/number/fixpnt/fixpnt_fwd.hpp>

namespace sw { namespace universal {

// fma(a,b,c) = a*b + c with a single fixpnt rounding/overflow step (fused).
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt>
fma(const fixpnt<nbits, rbits, arithmetic, bt>& a,
    const fixpnt<nbits, rbits, arithmetic, bt>& b,
    const fixpnt<nbits, rbits, arithmetic, bt>& c) {
	using Fixpnt = fixpnt<nbits, rbits, arithmetic, bt>;
	constexpr unsigned pbits = 2 * nbits;   // exact product width (carries 2*rbits fractional bits)
	constexpr unsigned wbits = pbits + 2;   // + guard bits for the fused-add carry and a round-up increment
	using Wide      = blockbinary<wbits, bt, BinaryNumberType::Signed>;
	using ProductBB = blockbinary<pbits, bt, BinaryNumberType::Signed>;

	// exact product a*b as a wide two's-complement value with 2*rbits fractional bits.
	// NOTE: blockbinary is trivially constructible (uninitialized storage), so the
	// zero must be explicit -- gcc happens to clear the stack, clang does not.
	Wide acc;
	acc.setzero();
	if (!(a.iszero() || b.iszero())) {
		ProductBB product = urmul2(a.bits(), b.bits());
		acc = product;                      // sign-extend pbits -> wbits
	}

	// align c (rbits fractional bits) into the product's 2*rbits-fractional frame and add exactly
	Wide cwide(c.bits());                   // sign-extend nbits -> wbits
	cwide <<= rbits;                        // rbits -> 2*rbits fractional bits
	acc += cwide;                           // exact fused sum a*b + c

	// single rounding: round-to-nearest-even at bit rbits, then realign to rbits fractional bits
	bool roundUp = acc.roundingMode(rbits);
	acc >>= rbits;                          // arithmetic (sign-propagating) shift: floor(acc / 2^rbits)

	Fixpnt result;
	if constexpr (arithmetic == Modulo) {
		if (roundUp) ++acc;
		result = acc;                       // operator=(blockbinary): keep the low nbits (modulo wrap)
	}
	else {  // Saturate: clamp to [maxneg, maxpos] in the wide frame, mirroring operator*=
		Wide maxposw(Fixpnt(SpecificValue::maxpos).bits());
		Wide maxnegw(Fixpnt(SpecificValue::maxneg).bits());
		if (acc >= maxposw) return Fixpnt(SpecificValue::maxpos);
		if (acc <  maxnegw) return Fixpnt(SpecificValue::maxneg);
		if (roundUp) ++acc;
		if (acc >= maxposw) return Fixpnt(SpecificValue::maxpos);  // a round-up that crossed the top rail
		result = acc;
	}
	return result;
}

}} // namespace sw::universal
