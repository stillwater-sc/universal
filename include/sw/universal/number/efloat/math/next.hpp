// next.hpp: next-representable-value functions (nextafter, nexttoward) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw {
namespace universal {

// nextafter: the value adjacent to x, one unit-in-the-last-place toward y, at
// x's WORKING precision (get_precision(), the type's bit width -- NOT the number
// of limbs currently occupied by x's mantissa, which for a value like 1.0 is a
// single limb and would make the step a fixed 2^-31 regardless of precision).
//
// The ULP of the binade [2^k, 2^(k+1)) is 2^(k-P+1) for precision P. Stepping
// TOWARD ZERO from an exact power of two 2^k crosses into the lower binade
// [2^(k-1), 2^k), whose spacing is half as large (2^(k-P)); that neighbor is
// computed at one extra limb of precision so its trailing bit is not rounded off
// when aligned against 2^k. Every step round-trips: nextafter(nextafter(x,a),x)
// == x in both directions, across power-of-two boundaries included.
template<unsigned nlimbs>
constexpr efloat<nlimbs> nextafter(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan;
		nan.setnan();
		return nan;
	}
	if (x == y)
		return y;

	if (x.iszero()) {
		// Step away from zero. An arbitrary-precision type has no fixed smallest
		// subnormal, so use the double smallest-subnormal exponent (2^-1074) as a
		// sensible minimum floor.
		efloat<nlimbs> ulp(1.0);
		ulp.setexponent(-1074);
		return (y.sign() == -1) ? -ulp : ulp;
	}

	const int64_t P = static_cast<int64_t>(x.get_precision());
	const int64_t k = x.scale();

	// Work in magnitude, then re-attach x's sign. "magnitudeUp" is true when the
	// step moves |x| away from zero (x>0 stepping up, or x<0 stepping down).
	const bool xNeg        = (x.sign() == -1);
	const bool magnitudeUp = ((y > x) != xNeg);

	efloat<nlimbs> absx(x);
	absx.setsign(false);

	efloat<nlimbs> newmag;
	if (magnitudeUp) {
		// away from zero: ULP of |x|'s own binade
		efloat<nlimbs> ulp(1.0);
		ulp.setexponent(k - P + 1);
		newmag = absx + ulp;
	} else {
		// toward zero: if |x| is an exact power of two, the neighbor lies in the
		// lower binade a half-ULP away; otherwise it is one ULP of this binade.
		efloat<nlimbs> pow2(1.0);
		pow2.setexponent(k);
		if (absx == pow2) {
			efloat<nlimbs> half(1.0);
			half.setexponent(k - P);
			newmag = absx;
			newmag.set_precision(static_cast<unsigned>(P) + 32);  // keep the trailing bit
			newmag = newmag - half;
			newmag.set_precision(static_cast<unsigned>(P));
		} else {
			efloat<nlimbs> ulp(1.0);
			ulp.setexponent(k - P + 1);
			newmag = absx - ulp;
		}
	}

	newmag.setsign(xNeg);
	return newmag;
}

// nexttoward: same as nextafter, but the direction argument is long double.
//
// The target is built directly from long double so no direction information is
// lost. On platforms where LONG_DOUBLE_SUPPORT is off, long double aliases double
// and the efloat(double) constructor is selected instead -- also exact.
// (efloat's long double conversion was fixed in #1160; this previously routed
// through double as a workaround for the constructor returning 0.)
template<unsigned nlimbs>
constexpr efloat<nlimbs> nexttoward(const efloat<nlimbs>& x, long double y) {
	return nextafter(x, efloat<nlimbs>(y));
}

}  // namespace universal
}  // namespace sw
