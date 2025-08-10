#pragma once
// newtons_iteration.hpp: Newton's Iteration to calculate the square root
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/algorithm/trace_constants.hpp>
#include <universal/native/ieee754.hpp>
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>
#undef BITBLOCK_THROW_ARITHMETIC_EXCEPTION
#undef VALUE_THROW_ARITHMETIC_EXCEPTION

namespace sw { namespace universal {

/*
- Consider the function argument, x, in floating-point form, with a base
(or radix) B, exponent e, and a fraction, f , such that 1/B <= f < 1.
Then we have x = f Be. The number of bits in the exponent and
fraction, and the value of the base, depends on the particular floating
point arithmetic system chosen.

- Use properties of the elementary function to range reduce the argument
x to a small fixed interval.

- Use a small polynomial approximation to produce an initial estimate,
y0, of the function on the small interval. Such an estimate may
be good to perhaps 5 to 10 bits.

- Given a value f, to iterate to the sqrt(f), apply Newton iteration 
  to refine the result. This takes the form 
	       y_k = y_(k-1)/2 + (f /2)/y_(k-1)
       and y_0 = 1

In base 2, the divisions by two can be done by exponent adjustments 
in floating-point computation, or by bit shifting in fixed-point computation.

Convergence of the Newton method is quadratic, so the number of
correct bits doubles with each iteration. Thus, a starting point correct
to 7 bits will produce iterates accurate to 14, 28, 56, ... bits. Since the
number of iterations is very small, and known in advance, the loop is
written as straight-line code.

- Having computed the function value for the range-reduced argument,
make whatever adjustments are necessary to produce the function value
for the original argument; this step may involve a sign adjustment,
and possibly a single multiplication and/or addition.
*/

// reference for fast direct sqrt method
inline float newtons_iteration(float a) {
	if (_trace_sqrt) std::cout << "----------------------- TEST SQRT -----------------------" << std::endl;

	if (std::isinf(a)) return a;

	bool s;
	int e;
	float fr;
	unsigned int _fraction;
	extract_fp_components(a, s, e, fr, _fraction);
	if (_trace_sqrt) std::cout << "f          " << a << std::endl;
	if (_trace_sqrt) std::cout << "e          " << e << std::endl;
	if (_trace_sqrt) std::cout << "fr         " << fr << std::endl;
	// onemme = 1.0 - machine epsilon
	union {
		float f;
		unsigned i;
	} m{ 0 };
	m.i = 0x3f7fffff;
	float onemme = m.f;

	// y0 to 7.04 bits
	double y = 0.41731 + 0.59016 * fr;
	if (_trace_sqrt) std::cout << "y0         " << y << std::endl;

	// y1 to 15.08 bits
	double z = y + fr / y;
	if (_trace_sqrt) std::cout << "y1         " << z << std::endl;

	// y2 to 31.16 bits
	y = 0.25*z + fr / z;
	if (_trace_sqrt) std::cout << "y2         " << y << std::endl;

	// Include sqrt(2) factor for odd exponents, and
	// ensure(0.5 <= y) and (y < 1.0).
	// Otherwise, exponent calculation is incorrect
	if (e % 2) {
		y = y * 0.707106781186547524400844362104;
		if (_trace_sqrt) std::cout << "y*sqrt0.5  " << y << std::endl;
		y = (y < 0.5 ? 0.5 : y);  // max(y, 0.5)
		e = e + 1;
	}
	else {
		y = (y < onemme ? y : onemme); //  min(y, onemme);
	}
	if (_trace_sqrt) std::cout << "y adjusted " << y << std::endl;
	// update exponent to undo range reduction.
	internal::value<23> v(y);
	v.setscale((e >> 1) - 1);
	return v.to_float();
}


}} // namespace sw::universal
