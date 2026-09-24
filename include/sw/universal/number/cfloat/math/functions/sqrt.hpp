#pragma once
#include <cmath>         // the <cmath> functions used below
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
// sqrt.hpp: sqrt functions for cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/cfloat/math/functions/sqrt_tables.hpp>

#ifndef CFLOAT_NATIVE_SQRT
#define CFLOAT_NATIVE_SQRT 0
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

	template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
	inline cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> BabylonianMethod(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& v) {
		const double eps = 1.0e-5;
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> half(0.5);
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x_next;
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x_n = half * v;
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> diff;
		do {
			x_next = (x_n + v / x_n) * half;
			diff = x_next - x_n;
			   std::cout << " x_n+1: " << x_next << " x_n: " << x_n << " diff " << diff << std::endl;
			x_n = x_next;
		} while (double(sw::universal::abs(diff)) > eps);
		return x_n;
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


	// sqrt for arbitrary cfloat, refined to the format's own precision.
	//
	// This used to be `cfloat(std::sqrt((double)a))` -- in BOTH branches of the
	// CFLOAT_NATIVE_SQRT switch, the first of them marked "// TBD" and identical to the
	// second, with the macro defaulting to 0 in three separate headers. So the result
	// carried 53 bits of significand whatever the format asked for: 53 of 112 for quad,
	// 53 of 236 for octo, silently. Arguments outside a double's exponent range were
	// worse than imprecise -- (double)a overflowed to infinity before std::sqrt was ever
	// called, so sqrt(maxpos) returned inf (#1589).
	//
	// Newton-Raphson on x = (x + a/x)/2 doubles the correct bits each step, so a 53-bit
	// seed reaches 112 bits in two steps and 236 in three. The iteration runs in the
	// cfloat's own arithmetic, so the precision is the format's, not the host's.
	//
	// The seed is std::sqrt(double(a)) whenever a is inside a double's range, which is
	// the common case and worth the 53 bits it buys. Outside that range the seed is the
	// power of two 2^floor(scale/2), which is within a factor of sqrt(2) of the answer
	// and costs a few more iterations rather than a wrong result.
	template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
	inline cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sqrt(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
		using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
		// Order matters. isneg() is true for -0 (the sign bit is set), so testing it first
		// sent -0 down the negative branch, which printed an error and returned NaN;
		// IEEE 754 requires sqrt(-0) to be -0. NaN and zero are therefore settled before
		// the sign test, and -inf is left to it because sqrt(-inf) IS NaN (#1589).
		if (a.isnan())  return a;                     // sqrt(nan) is nan
		if (a.iszero()) return a;                     // sqrt(+-0) is +-0, sign preserved
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw cfloat_negative_sqrt_arg();
#else
		if (a.isneg()) {                              // catches -inf as well
			std::cerr << "cfloat argument to sqrt is negative: " << a << std::endl;
			Cfloat nan; nan.setnan(NAN_TYPE_QUIET);
			return nan;
		}
#endif
		if (a.isinf()) return a;                      // sqrt(+inf) is +inf

		constexpr unsigned fbits = Cfloat::fbits;

		// Whether the host can answer is a property of the VALUE, not of the format's
		// fraction width. A narrow fraction says nothing about the exponent range:
		// cfloat<64,15> has fbits = 48 and would take a fbits-only fast path, but its
		// maxpos is inf as a double and its minpos is 0, so that path reproduced exactly
		// the defect this function was rewritten to fix. The host result is used only
		// when the format cannot hold more than a double AND this particular value is
		// one a double can carry; everything else iterates.
		const double d = double(a);
		const bool hostUsable = (d > 0.0 && std::isfinite(d));

		if constexpr (fbits <= 52 && !CFLOAT_NATIVE_SQRT) {
			// a double carries this format's whole fraction, so where it is in range its
			// answer is already the best the format can hold. CFLOAT_NATIVE_SQRT forces
			// the iteration anyway, which is how the two paths are compared in test.
			if (hostUsable) return Cfloat(std::sqrt(d));
		}

		// seed
		Cfloat x;
		if (hostUsable) {
			x = std::sqrt(d);
		}
		else {
			// a is outside a double's range: 2^floor(scale/2) is within sqrt(2) of the
			// root. The shift is arithmetic, so it floors for a negative scale too.
			x = 1.0;
			if (!x.setexponent(a.scale() >> 1)) { x = 1.0; }   // unrepresentable: start at 1
		}
		if (x.iszero() || x.isnan() || x.isinf()) x = 1.0;

		// Newton-Raphson. The bound is the worst case from a 1-bit seed; convergence
		// breaks out long before that for the usual 53-bit one.
		//
		// previous and twoAgo are initialized explicitly. A cfloat is trivially
		// constructible, so `Cfloat previous;` leaves stack garbage, and clang does not
		// zero it: the oscillation guard below then compared the new iterate against
		// whatever the slot happened to hold -- often a leftover from an earlier sqrt
		// call -- matched, and returned the seed. That produced a 53-bit answer on clang
		// while gcc happened to give the right one.
		const Cfloat half(0.5);
		Cfloat previous(0), twoAgo(0);
		for (unsigned i = 0; i < 64u; ++i) {
			twoAgo = previous;
			previous = x;
			x = (x + a / x) * half;
			// the iterate is monotone once it is above the root, so a repeat means the
			// format cannot hold a closer value; a match two steps back is the 1-ulp
			// oscillation that ends a correctly converged Newton. twoAgo is only a real
			// iterate from the second pass on, so the oscillation test waits for it.
			if (x == previous) break;
			if (i > 0 && x == twoAgo) { if (previous < x) x = previous; break; }
		}
		return x;
	}

	// reciprocal sqrt
	template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
	inline cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> rsqrt(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> v = sqrt(a);
		return v.reciprocate();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
