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

namespace sw { namespace universal {

    // forward declaration
inline dd_cascade nroot(const dd_cascade&, int);

#if DD_CASCADE_NATIVE_SQRT

    // Computes the square root of the double-double number dd.
    //   NOTE: dd must be a non-negative number
    inline dd_cascade sqrt(const dd_cascade& a) {
        /* Strategy: Karp's trick, which is what classic dd uses.

              sqrt(a) ~ a*x + [a - (a*x)^2] * x / 2

           where x is a double approximation to 1/sqrt(a). The correction term
           needs only half the working precision, so the whole square root costs
           about one double-double multiply - far less than any full-precision
           iteration.

           This replaced two Newton steps on (x + a/x)/2, one DIVISION each
           (universal#1331). The old formulation was reasonable when division
           was the cheap operation here; universal#1326 made division correct,
           and correct division costs 219 nsec/op against 25 for a multiply.

           Note the trade, because it goes the other way from the wider types:
           Karp is faithful rather than correctly rounded, so this is 5.5 ulps
           of 2^-106 where the iteration it replaced was 1.0 - but that is
           exactly classic dd's accuracy, because it is exactly classic dd's
           algorithm, at 99 nsec/op instead of 512. A type meant to be a drop-in
           for dd should cost and deliver what dd does. The wider widths keep an
           iteration because Karp doubles a double seed once, which reaches 106
           bits and no further.

           The argument is scaled into [0.5, 2) first, exactly, by a power of
           two: (a*x)^2 overflows for a near maxpos otherwise, which is why
           sqrt(maxpos) is broken today in dd itself (universal#1332).
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
        dd_cascade b = ldexp(a, -2 * k);

        double x  = 1.0 / std::sqrt(b[0]);
        double ax = b[0] * x;
        dd_cascade axd(ax);
        dd_cascade correction = b - axd * axd;

        return ldexp(axd + dd_cascade(correction[0] * (x * 0.5)), k);
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
