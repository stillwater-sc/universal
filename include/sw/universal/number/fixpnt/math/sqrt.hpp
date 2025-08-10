#pragma once
// sqrt.hpp: sqrt functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/fixpnt/numeric_limits.hpp>
#include <universal/number/fixpnt/math/sqrt_tables.hpp>

#ifndef FIXPNT_NATIVE_SQRT
#define FIXPNT_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

/*
	// straight Babylonian
	inline double babylonian(double v) {
		double x_n = 0.5 * v; // initial guess
		const double eps = 1.0e-7;   // 
		do {
			x_n = (x_n + v / x_n) / 2.0;
		} while (std::abs(x_n * x_n - v) > eps);

		return x_n;
	}
*/

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> BabylonianMethod(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
		using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
		constexpr Fixed eps = std::numeric_limits<Fixed>::epsilon();
		Fixed half(0.5f);
		Fixed x_next;
		Fixed x_n = half * v;
		Fixed diff;
		std::cout << "v    " << v << '\n';
		std::cout << "x_n  " << x_n << '\n';
		std::cout << "half " << half << '\n';
		do {
			x_next = (x_n + v / x_n) * half;
			diff = x_next - x_n;
			std::cout << " x_n+1: " << x_next << " x_n: " << x_n << " diff " << diff << '\n';
			x_n = x_next;
		} while ((sw::universal::abs(x_n*x_n - v) > eps));
		return x_n;
	}

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> BabylonianMethod2(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
		using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
		constexpr Fixed eps = std::numeric_limits<Fixed>::epsilon();

		Fixed y(v);
		Fixed x(v);
		x >>= 1; // divide by 2
		Fixed diff = (x - y);
		while (sw::universal::abs(diff) > eps) {
			x = (x + y);
			x >>= 1;
			y = v / x;
			diff = x - y;
			std::cout << " x: " << x << " y: " << y << " diff " << diff << '\n';
		} 
		return x;
	}

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> BabylonianMethod3(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
		using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
		constexpr Fixed eps = std::numeric_limits<Fixed>::epsilon();

		Fixed y(v);
		Fixed x(v);
		x >>= 1; // divide by 2
		Fixed diff = (x*x - y);
		while (sw::universal::abs(diff) > eps) {
			x = (x + y);
			x >>= 1;
			y = v / x;
			diff = x - y;
			std::cout << " x: " << x << " y: " << y << " diff " << diff << '\n';
		}
		return x;
	}

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
	yk_1/2 + (f /2)/yk_1. In base 2, the divisions by two can be done by
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


#if FIXPNT_NATIVE_SQRT
	// sqrt for arbitrary cfloat
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> sqrt(const fixpnt<nbits, rbits, arithmetic, bt>& a) {
		if (a < 0) std::cout << "sqrt arg is negative: " << a << std::endl;
		if (a < 0) throw fixpnt_arithmetic_exception("argument to sqrt is negative");
		using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
		Fixed eps{ std::numeric_limits<Fixed>::epsilon() };
		Fixed y(a);
		Fixed x(a);
		x >>= 1; // divide by 2
		Fixed diff = (x * x - y);
		int iterations = 0;
		while (sw::universal::abs(diff) > eps) {
			x = (x + y);
			x >>= 1;
			y = a / x;
			diff = x - y;
//			std::cout << " x: " << x << " y: " << y << " diff " << diff << '\n';
			if (++iterations > static_cast<int>(rbits)) break;
		}
		if (iterations > static_cast<int>(rbits)) std::cerr << "sqrt(" << double(a) << ") failed to converge\n";
		return x;
	}
#else
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> sqrt(const fixpnt<nbits, rbits, arithmetic, bt>& a) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) {
			throw fixpnt_negative_sqrt_arg();
		}
#else 
		if (a.isneg()) std::cerr << "fixpnt_negative_sqrt_arg\n";
#endif
			return fixpnt<nbits, rbits, arithmetic, bt>(std::sqrt((double)a));
	}
#endif

	// reciprocal sqrt
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> rsqrt(const fixpnt<nbits, rbits, arithmetic, bt>& f) {
		fixpnt<nbits, rbits, arithmetic, bt> rsqrt = sqrt(f);
		return rsqrt.reciprocate();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
