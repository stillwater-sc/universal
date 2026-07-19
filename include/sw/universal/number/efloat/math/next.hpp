// next.hpp: next-representable-value functions (nextafter, nexttoward) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw {
namespace universal {

// nextafter: the next value representable at x's WORKING precision, in the
// direction of y. The step is one unit-in-the-last-place, i.e. 2^(scale(x) -
// precision + 1) where precision = get_precision() (the type's working precision
// in bits), NOT the number of limbs currently occupied by x's mantissa -- a
// value like 1.0 stores in a single limb, so keying the ULP off the stored limb
// count would make the step 2^-31 regardless of the type's real precision.
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

	// ULP at x's working precision: 2^(scale - precision + 1)
	efloat<nlimbs> ulp(1.0);
	ulp.setexponent(x.scale() - static_cast<int64_t>(x.get_precision()) + 1);

	return (y > x) ? (x + ulp) : (x - ulp);
}

// nexttoward: same as nextafter, but the direction argument is long double.
// It is cast to double first to avoid empty-limb parsing of a wider type.
template<unsigned nlimbs>
constexpr efloat<nlimbs> nexttoward(const efloat<nlimbs>& x, long double y) {
	return nextafter(x, efloat<nlimbs>(static_cast<double>(y)));
}

}  // namespace universal
}  // namespace sw
