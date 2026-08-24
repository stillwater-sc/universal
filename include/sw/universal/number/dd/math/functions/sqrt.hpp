#pragma once
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
// sqrt.hpp: sqrt functions for double-double (dd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

#ifndef DOUBLEDOUBLE_NATIVE_SQRT
#define DOUBLEDOUBLE_NATIVE_SQRT 1
#endif

namespace sw { namespace universal {

#if DOUBLEDOUBLE_NATIVE_SQRT

    // Computes the square root of the double-double number dd.
    //   NOTE: dd must be a non-negative number
    inline dd sqrt(dd a) {
        /* Strategy:  Use Karp's trick:  if x is an approximation
           to sqrt(a), then

              sqrt(a) = a*x + [a - (a*x)^2] * x / 2   (approx)

           The approximation is accurate to twice the accuracy of x.
           Also, the multiplication (a*x) and [-]*x can be done with
           only half the precision.
        */

        if (a.iszero()) return dd{};

#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw dd_negative_sqrt_arg();
#else
        if (a.isneg()) std::cerr << "double-double argument to sqrt is negative: " << a << std::endl;
#endif

        if (a.isnan() || a.isinf()) return a;

        // Scale into [0.5, 2) first. The scaling is by a power of two, so it is exact and
        // costs no accuracy, and it bounds every intermediate wherever the argument sits.
        // Karp's trick forms (a*x)^2, a value of the argument's own magnitude: within a
        // rounding of maxpos that square overflows and the result comes back inf or NaN,
        // and at the bottom of the range the residual a - (a*x)^2 lands in the subnormals
        // and takes the correction term's accuracy with it -- sqrt(1e-300) held 80 bits of
        // its 106 (universal#1332).
        int e{ 0 };
        std::frexp(a.high(), &e);
        int k = e >> 1;                  // floor(e/2), correct for negative e
        dd b = ldexp(a, -2 * k);         // b in [0.5, 2), exact

        double x  = 1.0 / std::sqrt(b.high());
        double bx = b.high() * x;
        return ldexp(add(bx, (b - sqr(dd(bx))).high() * (x * 0.5)), k);
    }

#else

	// sqrt shim for double-double
	inline dd sqrt(dd a) {
#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw dd_negative_sqrt_arg();
#else  // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) std::cerr << "double-double argument to sqrt is negative: " << a << std::endl;
#endif // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.iszero()) return a;
		return dd(std::sqrt(double(a)));
	}

#endif // ! DOUBLEDOUBLE_NATIVE_SQRT

    // Computes the square root of a double in double-double precision. 
    inline dd sqrt(double d) {
        return sw::universal::sqrt(dd(d));
    }

	// reciprocal sqrt
	inline dd rsqrt(dd a) {
		dd v = sw::universal::sqrt(a);
		return reciprocal(v);
	}


    /* Computes the n-th root of the double-double number a.
       NOTE: n must be a positive integer.  
       NOTE: If n is even, then a must not be negative.       */
    inline dd nroot(const dd& a, int n) {
        /* Strategy:  Use Newton iteration for the function

                f(x) = x^(-n) - a

            to find its root a^{-1/n}.  The iteration is thus

                x' = x + x * (1 - a * x^n) / n

            which converges quadratically.  We can then find 
        a^{1/n} by taking the reciprocal.
        */

#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) throw dd_negative_nroot_arg();

        if (n % 2 == 0 && a.isneg()) throw dd_negative_nroot_arg();

#else  // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) {
            std::cerr << "double-double nroot argument is negative: " << n << std::endl;
        }

        if (n % 2 == 0 && a.isneg()) {
            std::cerr << "double-double nroot argument is negative: " << n << std::endl;
            return dd(SpecificValue::snan);
        }

#endif // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION

        if (n == 1) return a;
        if (n == 2) return sqrt(a);

        if (a.iszero()) return dd(0.0);

        // Note  a^{-1/n} = exp(-log(a)/n)
        dd r = abs(a);
        dd x = std::exp(-std::log(r.high()) / n);

        // Perform Newton's iteration.
        x += x * (1.0 - r * npwr(x, n)) / static_cast<double>(n);
        if (a.high() < 0.0) x = -x;

        return 1.0/x;
    }

}} // namespace sw::universal
