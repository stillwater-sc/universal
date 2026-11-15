#pragma once
// fractional.hpp: fractional functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// |x| fmod |y| for finite x, y with y != 0, and the parity of the truncated quotient; exact in a
// type with subnormals.
// Shift-and-subtract: double |y| while the double is exact and stays <= |x| (a doubling below
// the top binade is exact; checking the scale first keeps a saturating type from looping on
// maxpos + maxpos == maxpos, #2088), then walk back down by halves, which are exact because
// each half is a value the doubling produced.  Every subtraction r - s has s <= r < 2s, so it
// is exact (Sterbenz).  O(scale(x) - scale(y)) operations.
template<typename Real>
Real cfloat_fmod_magnitude(Real r, Real ay, bool& quotientOdd) {
	quotientOdd = false;
	if (r < ay) return r;
	Real scaled = ay;
	while (scaled.scale() < r.scale()) {
		Real next = scaled + scaled;
		if (next > r) break;
		scaled = next;
	}
	const Real two(2);
	for (;;) {
		bool subtracted = false;
		if (r >= scaled) { r = r - scaled; subtracted = true; }
		if (scaled == ay) { quotientOdd = subtracted; break; }
		scaled = scaled / two;
	}
	return r;
}

// the arithmetic type of fmod and remainder: one more exponent bit and subnormals, so that every
// value of the layout, twice it, and every partial remainder are exact.  The result is rounded into
// the layout once -- which is exact, except in a layout without subnormals, where a remainder below
// the smallest normal rounds like any other value instead of being flushed by a subtraction.
template<typename Real>
using cfloat_remainder_t = cfloat<Real::nbits + 2u, Real::es + 1u, typename Real::BlockType, true, false, false>;

// x - n*y for finite x != 0 and finite y != 0; n is x/y truncated, or rounded half to even when
// nearest is true
template<typename Real>
Real cfloat_remainder_finite(const Real& x, const Real& y, bool nearest) {
	using Wide = cfloat_remainder_t<Real>;
	const bool negative = x.sign();
	const Wide ax = negative ? -Wide(x) : Wide(x);
	const Wide ay = y.sign() ? -Wide(y) : Wide(y);
	bool odd = false;
	Wide r = cfloat_fmod_magnitude(ax, ay, odd);
	if (nearest) {
		const Wide twoR = r + r;                                 // exact: Wide has a spare exponent bit
		if (twoR > ay || (twoR == ay && odd)) r = r - ay;        // exact: |y|/2 <= r < |y|
	}
	if (negative) r = -r;                                        // a zero result takes the sign of x
	return Real(r);
}

// fmod returns x - n*y, where n is x/y with its fractional part truncated, exactly (IEEE-754
// and C semantics: fmod(+-0, y) = +-0, fmod(x, +-inf) = x, a NaN for y = 0 or an infinite x)
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cfloatmod(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (y.iszero() || x.isinf() || x.isnan() || y.isnan()) {
		Real nan;
		nan.setnan(false);  // quiet NaN
		return nan;
	}
	if (y.isinf() || x.iszero()) {
		return x;
	}
	return cfloat_remainder_finite(x, y, false);
}

// fmod retuns x - n*y where n = x/y with the fractional part truncated
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> fmod(cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return cfloatmod(x, y);
}

// remainder returns x - n*y, where n is x/y rounded to the nearest integer, ties to even, exactly
// (IEEE-754 remainder): fmod's remainder r, less |y| when r is past |y|/2, or at it with an odd
// quotient.  It used to go through double, so a wide type lost its result: cfloat<80,11>
// remainder(1 + 2^-60, 1) was 0 (#2088).
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> remainder(cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (y.iszero() || x.isinf() || x.isnan() || y.isnan()) {
		Real nan;
		nan.setnan(false);  // quiet NaN
		return nan;
	}
	if (y.isinf() || x.iszero()) {
		return x;
	}
	return cfloat_remainder_finite(x, y, true);
}

// frac returns the fraction of a cfloat value, x - trunc(x), as an unsigned quantity.  The
// subtraction is exact.  It used to cast to long long, which is undefined past 2^63, for a NaN
// and for inf, and lost the fraction of every value past 2^53 (frac(1e19) was 7.77e17, #2088).
// frac(NaN) is a quiet NaN; frac(+-inf) is 0, as modf gives.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> frac(cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan()) return x.quieted();
	if (x.isinf()) return Real(0);
	return abs(x - trunc(x));  // with the logic that fractions are unsigned quantities
}


}} // namespace sw::universal
