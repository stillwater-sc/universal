#pragma once
// sqrt.hpp: sqrt functions for double-double (dd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>
#include <cmath>

namespace sw { namespace universal {

    // Computes the square root of the double-double number dd.
    //   NOTE: dd must be a non-negative number
    inline dd sqrt(dd a) {
        /* Strategy:  Use Karp's trick:  if x is an approximation
           to 1/sqrt(a), then

              sqrt(a) = a*x + [a - (a*x)^2] * x / 2   (approx)

           The approximation is accurate to twice the accuracy of x.
           Also, the multiplication (a*x) and [-]*x can be done with
           only half the precision.

           Then one Newton correction (#1883). Relative error against a qd
           reference, in units of 2^-106, 200000 arguments in [1,4) and
           200000 across 2^[-1000,1000], i7-12700K, -O2:

                                        worst   mean   > 1 unit   gcc 15.2   clang
              Karp alone (before #1883)  7.92   0.67     26 %     35 ns/op   31 ns/op
              Karp + Newton correction   1.95   0.19      1 %     85 ns/op   64 ns/op
              dd_cascade NEWTON_DIVISION 0.75   0.14      0 %    539 ns/op  562 ns/op

           Over 10^6 arguments per range the worst is 1.99 units, still below
           epsilon() = 2^-105. It comes from the dd rounding of sqr(r) in the
           residual and of the final add.
           Correct rounding takes dd_cascade's two Newton divisions.
        */

        if (a.iszero()) return dd{};

#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw dd_negative_sqrt_arg();
#else
        if (a.isneg()) { errno = EDOM; return dd(SpecificValue::qnan); }
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
        dd r = add(bx, (b - sqr(dd(bx))).high() * (x * 0.5));
        // One Newton correction, r' = r + (b - r^2) / (2r), with 1/(2r) taken as x/2 (#1883).
        // Karp's step alone leaves up to 7.9 units of 2^-106: its correction uses the double
        // x in place of 1/sqrt(b) and only the leading limb of the residual. Here the residual
        // is ~2^-104, so a double correction is plenty, and what remains is the rounding of
        // the dd sqr(r) and of the final add.
        r += (b - sqr(r)).high() * (x * 0.5);
        return ldexp(r, k);
    }

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
            errno = EDOM;
            return dd(SpecificValue::qnan);
        }

        if (n % 2 == 0 && a.isneg()) {
            errno = EDOM;
            return dd(SpecificValue::qnan);
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
