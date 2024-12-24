#pragma once
// sqrt.hpp: sqrt functions for rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
//#include <universal/number/lns/math/sqrt_tables.hpp>

#ifndef LNS_NATIVE_SQRT
#define LNS_NATIVE_SQRT 0
#endif

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

	- Apply Newton iteration to refine the result. This takes the form yk =
	yk?1/2 + (f /2)/yk?1. In base 2, the divisions by two can be done by
	exponent adjustments in floating-point computation, or by bit shifting
	in fixed-point computation.

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


	// sqrt for arbitrary rational
	template<unsigned nbits, typename bt>
	inline rational<nbits, bt> sqrt(const rational<nbits, bt>& a) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw rational_negative_sqrt_arg();
#else
		if (a.isneg()) std::cerr << "rationalns argument to sqrt is negative: " << a << std::endl;
#endif
		if (a.iszero()) return a;
		return rational<nbits, bt>(std::sqrt((double)a));  // TBD
	}

	// reciprocal sqrt
	template<unsigned nbits, typename bt>
	inline rational<nbits, bt> rsqrt(const rational<nbits, bt>& a) {
		rational<nbits, bt> v = sqrt(a);
		return v.reciprocate();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
