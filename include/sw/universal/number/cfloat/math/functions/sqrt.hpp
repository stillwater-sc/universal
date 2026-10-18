#pragma once
#include <cerrno>
#include <cmath>         // the <cmath> functions used below
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
// sqrt.hpp: sqrt functions for cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/numerics/error_free_ops.hpp>   // two_prod: the exact squares of the midpoint test
#include <universal/number/cfloat/math/functions/sqrt_tables.hpp>

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


	// sign of m*m - A, exactly, for doubles near 1 with m*m within a factor of two of A:
	// two_prod gives m*m = p + e exactly (Dekker's split unless a trusted fms is configured),
	// p - A is exact by Sterbenz, and rounding (p - A) + e cannot change its sign
	inline int sqrt_compare_square(double m, double A) {
		double e{ 0.0 };
		const double p = two_prod(m, m, e);
		const double d = (p - A) + e;
		return (d > 0.0) ? 1 : ((d < 0.0) ? -1 : 0);
	}

	// sqrt_correctly_rounded for a format of at most 51 fraction bits whose a and x a double
	// holds: the midpoints then have at most 53 bits and the whole test runs in doubles.  Both
	// sides are scaled by a power of four, exactly, so no square underflows or overflows.
	template<typename Cfloat>
	Cfloat sqrt_correctly_rounded_in_double(const Cfloat& a, Cfloat x) {
		static_assert(Cfloat::fbits <= 51, "a midpoint must fit a double");
		const double ad = double(a);
		for (int step = 0; step < 16; ++step) {
			const double xd = double(x);
			const int s = std::ilogb(xd);
			const double A = std::ldexp(ad, -2 * s);
			Cfloat down(x);
			--down;                                           // x > 0, so down >= 0
			const double lo = std::ldexp(0.5 * (double(down) + xd), -s);   // exact
			const int cl = sqrt_compare_square(lo, A);
			if (cl > 0) { x = down; continue; }
			if (cl == 0 && x.at(0)) return down;              // tie: the even neighbour
			Cfloat up(x);
			++up;
			if (up.isinf() || up.isnan()) return x;           // x is maxpos
			const double hi = std::ldexp(0.5 * (xd + double(up)), -s);     // exact
			const int ch = sqrt_compare_square(hi, A);
			if (ch < 0) { x = up; continue; }
			if (ch == 0 && x.at(0)) return up;                // tie: the even neighbour
			return x;
		}
		return x;
	}

	// x is within a few ulps of sqrt(a), for a finite a > 0.  Return the correctly rounded root:
	// x is that root exactly when a lies between the squares of the midpoints to x's
	// neighbours, and otherwise the root is a neighbour, so step until it holds.  The test runs
	// in a cfloat of the same exponent width and 2 fbits + 4 fraction bits, where it is exact:
	//   - a converts exactly (the wider fraction refines the subnormals too);
	//   - a midpoint x +- ulp/2 is formed as x + (neighbour - x)/2, which cannot overflow at
	//     maxpos, and has at most fbits + 2 significant bits;
	//   - both sides are scaled by 2^-2k and 2^-k for k = scale(x), exactly (ldexp), so the
	//     squares sit near 1 and stay normal whatever the exponent range -- es == 20 needs no
	//     wider exponent, which the cfloat limit of es < 21 would not allow;
	//   - a square of fbits + 2 bits has at most 2 fbits + 4.
	// Ties go to even; a tie needs a midpoint whose square has no more bits than the format,
	// which only the subnormals of a narrow exponent can produce.
	template<typename Cfloat>
	Cfloat sqrt_correctly_rounded(const Cfloat& a, Cfloat x) {
		constexpr unsigned fbits = Cfloat::fbits;
		constexpr unsigned es = Cfloat::es;
		constexpr unsigned fbitsW = 2u * fbits + 4u;
		using Wide = cfloat<1u + es + fbitsW, es, typename Cfloat::BlockType, true, true, false>;
		const Wide A(a), half(0.5);
		for (int step = 0; step < 16; ++step) {
			const int k = x.scale();
			const Wide X(x);
			const Wide As = ldexp(A, -2 * k);                 // exact
			Cfloat down(x);
			--down;                                           // x > 0, so down >= 0
			const Wide lo = ldexp(X + (Wide(down) - X) * half, -k);   // exact
			const Wide lo2 = lo * lo;                         // exact
			if (As < lo2) { x = down; continue; }
			if (As == lo2 && x.at(0)) return down;            // tie: the even neighbour
			Cfloat up(x);
			++up;
			if (up.isinf() || up.isnan()) return x;           // x is maxpos
			const Wide hi = ldexp(X + (Wide(up) - X) * half, -k);     // exact
			const Wide hi2 = hi * hi;                         // exact
			if (As > hi2) { x = up; continue; }
			if (As == hi2 && x.at(0)) return up;              // tie: the even neighbour
			return x;
		}
		return x;
	}

	// sqrt for arbitrary cfloat, correctly rounded at every width (#1725).
	//
	// This used to be `cfloat(std::sqrt((double)a))` (#1589), then a host answer for every
	// format of up to 52 fraction bits and Newton-Raphson above that.  The host answer is
	// rounded twice, once to a double and once to the format, and that is only innocuous when
	// the double carries at least 2p + 2 bits for a p-bit significand: p <= 25, fbits <= 24.
	// Past that it misrounded -- 9 in 10^5 arguments of cfloat<48,8>, a quarter of them for
	// cfloat<60,8> -- and the Newton iteration stopped at an oscillation without proving the
	// last ulp.  Both now end in sqrt_correctly_rounded.  The CFLOAT_NATIVE_SQRT switch that
	// forced the iteration is gone: the configuration alone picks the path.
	//
	// Newton-Raphson on x = (x + a/x)/2 doubles the correct bits each step, so a 53-bit seed
	// reaches 112 bits in two steps and 236 in three.  The seed is std::sqrt(double(a))
	// whenever a is inside a double's range; outside it, 2^floor(scale/2), which is within a
	// factor of sqrt(2) of the answer and costs a few more iterations rather than a wrong
	// result.
	template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
	inline cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sqrt(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
		using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
		// Order matters. isneg() is true for -0 (the sign bit is set), so testing it first
		// sent -0 down the negative branch; IEEE 754 requires sqrt(-0) to be -0. NaN and zero
		// are therefore settled before the sign test, and -inf is left to it because
		// sqrt(-inf) IS NaN (#1589).
		if (a.isnan())  return a;                     // sqrt(nan) is nan
		if (a.iszero()) return a;                     // sqrt(+-0) is +-0, sign preserved
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw cfloat_negative_sqrt_arg();
#else
		if (a.isneg()) {                              // catches -inf as well: EDOM and a NaN, as std::sqrt
			errno = EDOM;
			Cfloat nan; nan.setnan(NAN_TYPE_QUIET);
			return nan;
		}
#endif
		if (a.isinf()) return a;                      // sqrt(+inf) is +inf

		constexpr unsigned fbits = Cfloat::fbits;

		// Whether the host can answer is a property of the VALUE, not of the format's
		// fraction width: cfloat<64,15> has fbits = 48 but its maxpos is inf as a double.
		const double d = double(a);
		const bool hostUsable = (d > 0.0 && std::isfinite(d));

		if constexpr (fbits <= 24) {
			// p <= 25 bits: the double's 53 >= 2p + 2, so rounding its correctly rounded root
			// once more to the format is the correctly rounded root (Figueroa).
			if (hostUsable) return Cfloat(std::sqrt(d));
		}
		else if constexpr (fbits <= 51) {
			// the double root is within an ulp of the format's root: correct it, in doubles
			if (hostUsable) return sqrt_correctly_rounded_in_double(a, Cfloat(std::sqrt(d)));
		}
		else if constexpr (fbits == 52 && (es >= 7 || (es >= 3 && !hasSubnormals))) {
			// the format's significand is a double's: std::sqrt's correctly rounded root is
			// rounded once, provided the root is a normal of the format.  sqrt(minpos) >=
			// minnormal needs MIN_EXP_NORMAL <= -52, es >= 7, when there are subnormals;
			// below that a subnormal root would be rounded a second time.
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
		// whatever the slot happened to hold and returned the seed.
		const Cfloat half(0.5);
		Cfloat previous(0), twoAgo(0);
		for (unsigned i = 0; i < 64u; ++i) {
			twoAgo = previous;
			previous = x;
			x = (x + a / x) * half;
			// a repeat means the format cannot hold a closer value; a match two steps back is
			// the 1-ulp oscillation that ends a converged Newton
			if (x == previous) break;
			if (i > 0 && x == twoAgo) break;
		}
		return sqrt_correctly_rounded(a, x);
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
