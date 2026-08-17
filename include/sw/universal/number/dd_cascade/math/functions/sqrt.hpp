#pragma once
// sqrt.hpp: sqrt functions for double-double (dd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

#ifndef DD_CASCADE_NATIVE_SQRT
#define DD_CASCADE_NATIVE_SQRT 1
#endif

// selects Karp's trick over the reciprocal Newton iteration; normally set through
// dd_cascade.hpp, defaulted here so this header stands on its own
#ifndef UNIVERSAL_DD_CASCADE_FAST_SQRT
#define UNIVERSAL_DD_CASCADE_FAST_SQRT 0
#endif

namespace sw { namespace universal {

    // forward declaration
inline dd_cascade nroot(const dd_cascade&, int);

#if DD_CASCADE_NATIVE_SQRT

    // Computes the square root of the double-double number dd.
    //   NOTE: dd must be a non-negative number
    inline dd_cascade sqrt(const dd_cascade& a) {
        /* Two algorithms, selected by UNIVERSAL_DD_CASCADE_FAST_SQRT.

           Both replaced Newton iteration on (x + a/x)/2, one DIVISION per
           step (universal#1331). That was reasonable when division was the
           cheap operation here; universal#1326 made division correct, and
           correct division costs 219 nsec/op against 25 for a multiply.

           DEFAULT - Newton iteration on the reciprocal square root:

               r' = r + r * (0.5 - (a/2) * r^2)

           converging to 1/sqrt(a) with multiplication only, then one multiply
           by a. Two iterations from a 53-bit seed reach past the format's 106
           bits. Measured 3.8 ulps of 2^-106 at 376 nsec/op; a third iteration
           was measured too and is dominated - it reaches only 3.3 ulps for 532
           nsec/op, because what limits this path is the rounding inside the
           iteration and the closing multiply by a, not the iteration count.

           UNIVERSAL_DD_CASCADE_FAST_SQRT - Karp's trick, which is what classic
           dd uses:

               sqrt(a) ~ a*x + [a - (a*x)^2] * x / 2

           where x is a double approximation to 1/sqrt(a). The correction needs
           only half the working precision, so the whole square root costs about
           one double-double multiply. Measured 5.5 ulps at 142 nsec/op - which
           is exactly classic dd's accuracy, because it is exactly dd's
           algorithm.

           The default keeps most of the accuracy, at 2.6x the cost: 3.8 ulps
           against Karp's 5.5 - and against 1.4 for the division iteration both
           of these replaced, which is the part of the trade worth knowing. Karp is faithful
           rather than correctly rounded, and 5.5 ulps of a 106-bit significand
           is a real loss for a type whose reason to exist is precision; the
           guard is there for code that would rather have dd's speed and has
           decided it can afford dd's error.

           Both paths scale the argument into [0.5, 2) first, exactly, by a
           power of two. The iteration squares a value of magnitude ~sqrt(a) or
           ~1/sqrt(a), which leaves the representable range at the extremes:
           sqrt(maxpos) returns inf or NaN today in dd itself for exactly this
           reason (universal#1332).
        */

        if (a.iszero()) return dd_cascade(0.0);

#	if DD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw dd_cascade_negative_sqrt_arg();
#else
        if (a.isneg()) {
            std::cerr << "double-double argument to sqrt is negative: " << a << std::endl;
            return dd_cascade(SpecificValue::qnan);
        }
#endif
        if (a.isnan() || a.isinf()) return a;

        int e{ 0 };
        std::frexp(a[0], &e);
        int k = e >> 1;                       // floor(e/2), correct for negative e
        dd_cascade b = ldexp(a, -2 * k);      // b in [0.5, 2), exact

#if UNIVERSAL_DD_CASCADE_FAST_SQRT

        double x  = 1.0 / std::sqrt(b[0]);
        double ax = b[0] * x;
        dd_cascade axd(ax);
        dd_cascade correction = b - axd * axd;

        return ldexp(axd + dd_cascade(correction[0] * (x * 0.5)), k);

#else

        dd_cascade r(1.0 / std::sqrt(b[0]));  // ~53 bits
        dd_cascade h = mul_pwr2(b, 0.5);

        r = r + (dd_cascade(0.5) - h * sqr(r)) * r;   // ~106 bits
        r = r + (dd_cascade(0.5) - h * sqr(r)) * r;   // carries the rounding

        return ldexp(r * b, k);

#endif
    }

#else

	// sqrt shim for double-double
	inline dd_cascade sqrt(dd_cascade a) {
#if DD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw dd_cascade_negative_sqrt_arg();
#else  // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) std::cerr << "double-double argument to sqrt is negative: " << a << std::endl;
#endif // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.iszero()) return a;
		return dd_cascade(std::sqrt(double(a)));
	}

#endif // ! DD_CASCADE_NATIVE_SQRT

    // Note: sqrt(double) helper removed to avoid namespace pollution
    // Use explicit construction instead: sqrt(dd_cascade(d))

	// reciprocal sqrt
    inline dd_cascade rsqrt(const dd_cascade& a) {
	    dd_cascade v = sw::universal::sqrt(a);
		return reciprocal(v);
	}


    /* Computes the n-th root of the double-double number a.
       NOTE: n must be a positive integer.  
       NOTE: If n is even, then a must not be negative.       */
    inline dd_cascade nroot(const dd_cascade& a, int n) {
        /* Strategy:  Use Newton iteration for the function

                f(x) = x^(-n) - a

            to find its root a^{-1/n}.  The iteration is thus

                x' = x + x * (1 - a * x^n) / n

            which converges quadratically.  We can then find 
        a^{1/n} by taking the reciprocal.
        */

#if DD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) throw dd_cascade_negative_nroot_arg();

        if (n % 2 == 0 && a.isneg()) throw dd_cascade_negative_nroot_arg();

#else  // ! DD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) {
            std::cerr << "double-double nroot argument is negative: " << n << std::endl;
        }

        if (n % 2 == 0 && a.isneg()) {
            std::cerr << "double-double nroot argument is negative: " << n << std::endl;
            return dd_cascade(SpecificValue::snan);
        }

#endif // ! DD_CASCADE_THROW_ARITHMETIC_EXCEPTION

        if (n == 1) return a;
        if (n == 2) return sqrt(a);

        if (a.iszero()) return dd_cascade(0.0);

        // Note  a^{-1/n} = exp(-log(a)/n)
	    dd_cascade r = abs(a);
	    dd_cascade x = std::exp(-std::log(r.high()) / n);

        // Perform Newton's iteration.
        x += x * (1.0 - r * npwr(x, n)) / static_cast<double>(n);
        if (a.high() < 0.0) x = -x;

        return 1.0/x;
    }

}} // namespace sw::universal
