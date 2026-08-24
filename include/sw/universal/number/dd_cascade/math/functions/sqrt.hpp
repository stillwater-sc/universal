#pragma once
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
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

// square root formulations; see the comment on sqrt() below for the trade
#define UNIVERSAL_DD_CASCADE_SQRT_NEWTON_DIVISION   0
#define UNIVERSAL_DD_CASCADE_SQRT_NEWTON_RECIPROCAL 1
#define UNIVERSAL_DD_CASCADE_SQRT_KARP              2   // Karp's trick, classic dd's algorithm
#ifndef UNIVERSAL_DD_CASCADE_SQRT_ALGORITHM
#define UNIVERSAL_DD_CASCADE_SQRT_ALGORITHM UNIVERSAL_DD_CASCADE_SQRT_NEWTON_DIVISION
#endif

namespace sw { namespace universal {

    // forward declaration
inline dd_cascade nroot(const dd_cascade&, int);

#if DD_CASCADE_NATIVE_SQRT

    // Computes the square root of the double-double number dd.
    //   NOTE: dd must be a non-negative number
        // Square root algorithm selection.
    //
    // Three formulations, kept side by side because the choice between them is a
    // real trade rather than a settled question, and because seeing them together
    // is the clearest statement of what each one costs.
    //
    //   NEWTON_DIVISION     x' = (x + a/x)/2
    //                       one division per step. The most accurate of the three
    //                       at this width, and the slowest.
    //
    //   NEWTON_RECIPROCAL   r' = r + r*(0.5 - (a/2)*r^2), then multiply by a
    //                       converges to 1/sqrt(a) with multiplication only. This
    //                       is what classic qd uses.
    //
    //   KARP                sqrt(a) ~ a*x + [a - (a*x)^2]*x/2
    //                       with x a double approximation to 1/sqrt(a). The
    //                       correction needs only half the working precision, so
    //                       it costs about one double-double multiply. This is
    //                       what classic dd uses, and it delivers dd's accuracy.
    //
    // The default is always the most accurate. The faster formulations have to be
    // asked for, by defining UNIVERSAL_DD_CASCADE_SQRT_ALGORITHM.
    //
    // Measured on an i7-12700K, gcc 13.3 -O3, residual of r*r - a in ulps of
    // 2^-106:
    //
    //     NEWTON_DIVISION     1.4 ulps   580 nsec/op   (default)
    //     NEWTON_RECIPROCAL   3.8 ulps   376 nsec/op
    //     KARP                5.5 ulps   142 nsec/op   = classic dd
    //     (classic dd itself 10.8 ulps at 42 nsec/op)
    //
    // Every formulation scales the argument into [0.5, 2) first, exactly, by a
    // power of two. Each of them squares a value of magnitude ~sqrt(a) or
    // ~1/sqrt(a), which leaves the representable range at the extremes: that is
    // why sqrt(maxpos) returns inf or NaN today in dd and qd (universal#1332).
inline dd_cascade sqrt(const dd_cascade& a) {
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
        int k = e >> 1;                  // floor(e/2), correct for negative e
        dd_cascade b = ldexp(a, -2 * k);       // b in [0.5, 2), exact

#if UNIVERSAL_DD_CASCADE_SQRT_ALGORITHM == UNIVERSAL_DD_CASCADE_SQRT_NEWTON_DIVISION

        dd_cascade x = std::sqrt(b[0]);
        x = (x + b / x) * 0.5;
        x = (x + b / x) * 0.5;
        return ldexp(x, k);

#elif UNIVERSAL_DD_CASCADE_SQRT_ALGORITHM == UNIVERSAL_DD_CASCADE_SQRT_KARP

        double x  = 1.0 / std::sqrt(b[0]);
        double ax = b[0] * x;
        dd_cascade axd(ax);
        dd_cascade correction = b - axd * axd;
        return ldexp(axd + dd_cascade(correction[0] * (x * 0.5)), k);

#else   // NEWTON_RECIPROCAL

        dd_cascade r(1.0 / std::sqrt(b[0]));
        dd_cascade h = mul_pwr2(b, 0.5);
        r = r + (dd_cascade(0.5) - h * sqr(r)) * r;
        r = r + (dd_cascade(0.5) - h * sqr(r)) * r;
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
