// hypot.hpp: hypotenuse function hypot(x, y) = sqrt(x^2 + y^2) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <universal/number/efloat/math/sqrt.hpp>  // sqrt

namespace sw {
namespace universal {

// hypot(x, y) = sqrt(x^2 + y^2), computed with scale-by-max so the intermediate
// x^2 + y^2 neither overflows nor underflows. Unlike the double-delegating shims
// in most number systems, this runs on efloat's own arithmetic and sqrt, so the
// result carries the operand's full working precision. IEEE-754 special cases:
//   - if either argument is +/-inf, the result is +inf (even if the other is NaN)
//   - otherwise if either argument is NaN, the result is NaN
//   - hypot(x, y) == hypot(y, x) == hypot(x, -y); hypot(0, y) == |y|
template<unsigned nlimbs>
constexpr efloat<nlimbs> hypot(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	// IEEE-754: Infinity suppresses NaN inside hypot! Infinity check must run first.
	if (x.isinf() || y.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(false);
		return inf;
	}
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan;
		nan.setnan();
		return nan;
	}

	efloat<nlimbs> abs_x(x);
	abs_x.setsign(false);
	efloat<nlimbs> abs_y(y);
	abs_y.setsign(false);

	// Scale by max to prevent intermediate underflow/overflow
	efloat<nlimbs> max_val = (abs_x < abs_y) ? abs_y : abs_x;
	efloat<nlimbs> min_val = (abs_x < abs_y) ? abs_x : abs_y;

	if (max_val.iszero()) {
		return efloat<nlimbs>(0.0);
	}

	efloat<nlimbs> r = min_val / max_val;
	return max_val * sqrt(efloat<nlimbs>(1.0) + r * r);
}

}  // namespace universal
}  // namespace sw
