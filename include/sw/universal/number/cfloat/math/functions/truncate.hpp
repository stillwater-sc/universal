#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// These operate directly on the cfloat encoding (no round-trip through double).
// A double-based implementation is wrong for cfloats with more than 53
// significand bits: any integer above 2^53 loses its low bits in the conversion,
// so e.g. floor(2^60 + 8) would not return 2^60 + 8 (issue #1026). trunc() masks
// the fraction bits below the binary point according to the unbiased scale;
// floor, ceil, and round are derived from trunc plus cfloat's (correctly
// rounded) arithmetic and comparisons.

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> trunc(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	// subnormals have magnitude < 1 (the smallest normal is well below 1 for any
	// es), so truncation toward zero yields signed zero.
	if (!x.isnormal()) {
		Cfloat z; z.setzero(); if (x.sign()) z.setsign(true);
		return z;
	}
	constexpr int fbits = static_cast<int>(Cfloat::fbits);
	const int scale = x.scale();                 // unbiased exponent, floor(log2|x|)
	if (scale >= fbits) return x;                // no fraction bits below 2^0: integral
	if (scale < 0) {                             // |x| < 1
		Cfloat z; z.setzero(); if (x.sign()) z.setsign(true);
		return z;
	}
	// clear the fraction bits whose weight is below 2^0, i.e. indices
	// [0 .. fbits - scale - 1]; the exponent and hidden bit are untouched, so the
	// result stays a valid normal value equal to the truncated integer.
	Cfloat r(x);
	for (int i = 0; i < fbits - scale; ++i) r.setbit(static_cast<unsigned>(i), false);
	return r;
}

// Round x downward, returning the largest integral value that is not greater than x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> floor(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan() || x.isinf()) return x;
	Cfloat t = trunc(x);
	// negative, non-integral -> the largest integer <= x is one below trunc
	if (x.sign() && t != x) t = t - Cfloat(1);
	return t;
}

// Round x upward, returning the smallest integral value that is greater than x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> ceil(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan() || x.isinf()) return x;
	Cfloat t = trunc(x);
	// positive, non-integral -> the smallest integer >= x is one above trunc
	if (!x.sign() && t != x) t = t + Cfloat(1);
	return t;
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> round(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	Cfloat t = trunc(x);
	if (t == x) return x;                        // already integral
	// Decide whether to round away from zero WITHOUT constructing 0.5 -- some
	// low-range cfloats cannot represent it. For |x| >= 1 (normal, scale >= 0)
	// the discarded fraction is >= 0.5 iff its leading bit (weight 2^-1, i.e.
	// fraction-bit index fbits-scale-1) is set; that gives ties-away-from-zero.
	// For |x| < 1 (normal scale < 0, or subnormal), test |x| >= 0.5 via the
	// exact 2*|x| >= 1.
	constexpr int fbits = static_cast<int>(Cfloat::fbits);
	bool away;
	const int scale = x.scale();
	if (x.isnormal() && scale >= 0) {
		away = x.test(static_cast<unsigned>(fbits - scale - 1));
	}
	else {
		Cfloat ax = x.sign() ? -x : x;           // |x|
		away = (ax + ax) >= Cfloat(1);           // 2|x| >= 1  <=>  |x| >= 0.5
	}
	if (away) t = x.sign() ? t - Cfloat(1) : t + Cfloat(1);
	return t;
}

}} // namespace sw::universal
