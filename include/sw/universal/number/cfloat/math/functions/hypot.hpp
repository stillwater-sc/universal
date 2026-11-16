#pragma once
// hypot.hpp: hypotenuse functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>         // the <cmath> functions used below
/*
Computes the square root of the sum of the squares of x and y, without undue overflow or underflow at intermediate stages of the computation.

Parameters
x	-	floating point value
y	-	floating point value
Return value
If no errors occur, the hypotenuse of a right-angled triangle, sqrt(x^2 + y^2), is returned.

If a range error due to overflow occurs, +HUGE_VAL, +HUGE_VALF, or +HUGE_VALL is returned.

If a range error due to underflow occurs, the correct result (after rounding) is returned.

Error handling
Errors are reported as specified in math_errhandling.

If the implementation supports IEEE floating-point arithmetic (IEC 60559),

hypot(x, y), hypot(y, x), and hypot(x, -y) are equivalent
if one of the arguments is +/-0, hypot is equivalent to fabs called with the non-zero argument
if one of the arguments is +/-inf, hypot returns +inf even if the other argument is NaN
otherwise, if any of the arguments is NaN, NaN is returned

Notes
Implementations usually guarantee precision of less than 1 ulp (units in the last place): GNU, BSD, Open64.

hypot(x, y) is equivalent to cabs(x + I*y).

POSIX specifies that underflow may only occur when both arguments are subnormal and the correct result is also subnormal (this forbids naive implementations).

hypot(INFINITY, NAN) returns +8, but sqrt(INFINITY*INFINITY+NAN*NAN) returns NaN.
*/

#include <universal/number/cfloat/math/efloat_bridge.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> hypot(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		if (x.isinf() || y.isinf()) return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(SpecificValue::infpos);   // inf even with a NaN (C 7.12.7.3)
		if (x.isnan()) return x.quieted();
		if (y.isnan()) return y.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) { return hypot(cfloat_math::argument(x, W), cfloat_math::argument(y, W)); });
	}
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::hypot(double(x),double(y)));
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> hypotf(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return hypot(x, y);   // the cfloat's own hypot: hypotf narrowed a wide layout to its native type (#2088)
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> hypotl(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return hypot(x, y);   // the cfloat's own hypot: hypotl narrowed a wide layout to its native type (#2088)
}

}} // namespace sw::universal
