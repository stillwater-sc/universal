#pragma once
// hypot.hpp: hypotenuse functions for posits
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

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

template<unsigned nbits, unsigned es>
posit<nbits,es> hypot(posit<nbits,es> x, posit<nbits,es> y) {
	return posit<nbits,es>(std::hypot(double(x),double(y)));
}

template<unsigned nbits, unsigned es>
posit<nbits,es> hypotf(posit<nbits,es> x, posit<nbits,es> y) {
	return posit<nbits,es>(std::hypotf(float(x),float(y)));
}

template<unsigned nbits, unsigned es>
posit<nbits,es> hypotl(posit<nbits,es> x, posit<nbits,es> y) {
	return posit<nbits,es>(std::hypotl((long double)(x),(long double)(y)));
}

// _MSVC_LANG Defined as an integer literal that specifies the C++ language standard targeted by the compiler.
// It's set only in code compiled as C++. 
// The macro is the integer literal value 201402L by default, or when the /std:c++14 compiler option is specified. 
// The macro is set to 201703L if the /std:c++17 compiler option is specified. 
// The macro is set to 202002L if the /std:c++20 compiler option is specified. 
// It's set to a higher, unspecified value when the / std:c++latest option is specified.Otherwise, the macro is undefined.

#ifdef LATER
// since C++17
template<unsigned nbits, unsigned es>
posit<nbits,es> hypot(posit<nbits,es> x, posit<nbits,es> y, posit<nbits,es> z) {
	return posit<nbits,es>(std::hypot(double(x),double(y),double(z)));
}

template<unsigned nbits, unsigned es>
posit<nbits,es> hypotf(posit<nbits,es> x, posit<nbits,es> y, posit<nbits,es> z) {
	return posit<nbits,es>(std::hypotf(float(x),float(y),float(z)));
}

template<unsigned nbits, unsigned es>
posit<nbits,es> hypotl(posit<nbits,es> x, posit<nbits,es> y, posit<nbits,es> z) {
	return posit<nbits,es>(std::hypotl((long double)(x),(long double)(y),(long double)(z)));
}
#endif

}} // namespace sw::universal
