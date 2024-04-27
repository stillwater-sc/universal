#pragma once
// hypot.hpp: hypotenuse functions for logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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
if one of the arguments is ±0, hypot is equivalent to fabs called with the non-zero argument
if one of the arguments is ±8, hypot returns +8 even if the other argument is NaN
otherwise, if any of the arguments is NaN, NaN is returned

Notes
Implementations usually guarantee precision of less than 1 ulp (units in the last place): GNU, BSD, Open64.

hypot(x, y) is equivalent to cabs(x + I*y).

POSIX specifies that underflow may only occur when both arguments are subnormal and the correct result is also subnormal (this forbids naive implementations).

hypot(INFINITY, NAN) returns +8, but sqrt(INFINITY*INFINITY+NAN*NAN) returns NaN.
*/

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> hypot(lns<nbits, rbits, bt, xtra...> x, lns<nbits, rbits, bt, xtra...> y) {
	return lns<nbits, rbits, bt, xtra...>(std::hypot(double(x),double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> hypotf(lns<nbits, rbits, bt, xtra...> x, lns<nbits, rbits, bt, xtra...> y) {
	return lns<nbits, rbits, bt, xtra...>(std::hypotf(float(x),float(y)));
}

#if LONG_DOUBLE_SUPPORT
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> hypotl(lns<nbits, rbits, bt, xtra...> x, lns<nbits, rbits, bt, xtra...> y) {
	return lns<nbits, rbits, bt, xtra...>(std::hypotl((long double)(x),(long double)(y)));
}
#endif

}} // namespace sw::universal
