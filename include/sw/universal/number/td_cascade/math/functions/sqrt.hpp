#pragma once
// sqrt.hpp: sqrt functions for triple-double (td) cascade floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

#ifndef TD_CASCADE_NATIVE_SQRT
#define TD_CASCADE_NATIVE_SQRT 1
#endif

// square root formulations; see the comment on sqrt() below for the trade
#define UNIVERSAL_TD_CASCADE_SQRT_NEWTON_DIVISION   0
#define UNIVERSAL_TD_CASCADE_SQRT_NEWTON_RECIPROCAL 1
#ifndef UNIVERSAL_TD_CASCADE_SQRT_ALGORITHM
#define UNIVERSAL_TD_CASCADE_SQRT_ALGORITHM UNIVERSAL_TD_CASCADE_SQRT_NEWTON_RECIPROCAL
#endif

namespace sw { namespace universal {

#if TD_CASCADE_NATIVE_SQRT

    // Computes the square root of the triple-double number td.
    //   NOTE: td must be a non-negative number
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
    // The default is always the most accurate. The faster formulations have to be
    // asked for, by defining UNIVERSAL_TD_CASCADE_SQRT_ALGORITHM.
    //
    // Measured on an i7-12700K, gcc 13.3 -O3, residual of r*r - a in ulps of
    // 2^-159:
    //
    //     NEWTON_RECIPROCAL  0.72 ulps   692 nsec/op   (default: at this width
    //                                                  it is the more accurate
    //                                                  AND the faster of the two)
    //     NEWTON_DIVISION    1.10 ulps   957 nsec/op
    //
    // Every formulation scales the argument into [0.5, 2) first, exactly, by a
    // power of two. Each of them squares a value of magnitude ~sqrt(a) or
    // ~1/sqrt(a), which leaves the representable range at the extremes: that is
    // why sqrt(maxpos) returns inf or NaN today in dd and qd (universal#1332).
inline td_cascade sqrt(const td_cascade& a) {
        if (a.iszero()) return td_cascade(0.0);

#	if TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw td_cascade_negative_sqrt_arg();
#else
        if (a.isneg()) {
            std::cerr << "triple-double argument to sqrt is negative: " << a << std::endl;
            return td_cascade(SpecificValue::qnan);
        }
#endif
        if (a.isnan() || a.isinf()) return a;

        int e{ 0 };
        std::frexp(a[0], &e);
        int k = e >> 1;                  // floor(e/2), correct for negative e
        td_cascade b = ldexp(a, -2 * k);       // b in [0.5, 2), exact

#if UNIVERSAL_TD_CASCADE_SQRT_ALGORITHM == UNIVERSAL_TD_CASCADE_SQRT_NEWTON_DIVISION

        td_cascade x = std::sqrt(b[0]);
        x = (x + b / x) * 0.5;
        x = (x + b / x) * 0.5;
        return ldexp(x, k);

#else   // NEWTON_RECIPROCAL

        td_cascade r(1.0 / std::sqrt(b[0]));
        td_cascade h = mul_pwr2(b, 0.5);
        r = r + (td_cascade(0.5) - h * sqr(r)) * r;
        r = r + (td_cascade(0.5) - h * sqr(r)) * r;
        return ldexp(r * b, k);

#endif
    }

#else

	// sqrt shim for triple-double
	inline td_cascade sqrt(td_cascade a) {
#if TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw td_cascade_negative_sqrt_arg();
#else  // ! TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) std::cerr << "triple-double argument to sqrt is negative: " << a << std::endl;
#endif // ! TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.iszero()) return a;
		return td_cascade(std::sqrt(double(a)));
	}

#endif // ! TD_CASCADE_NATIVE_SQRT

    // Note: sqrt(double) helper removed to avoid namespace pollution
    // Use explicit construction instead: sqrt(td_cascade(d))

	// reciprocal sqrt
    inline td_cascade rsqrt(const td_cascade& a) {
	    td_cascade v = sw::universal::sqrt(a);
		return reciprocal(v);
	}


    /* Computes the n-th root of the triple-double number a.
       NOTE: n must be a positive integer.
       NOTE: If n is even, then a must not be negative.       */
    inline td_cascade nroot(const td_cascade& a, int n) {
        /* Strategy:  Use Newton iteration for the function

                f(x) = x^(-n) - a

            to find its root a^{-1/n}.  The iteration is thus

                x' = x + x * (1 - a * x^n) / n

            which converges quadratically.  We can then find
        a^{1/n} by taking the reciprocal.
        */

#if TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) throw td_cascade_negative_nroot_arg();

        if (n % 2 == 0 && a.isneg()) throw td_cascade_negative_nroot_arg();

#else  // ! TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) {
            std::cerr << "triple-double nroot argument is negative: " << n << std::endl;
        }

        if (n % 2 == 0 && a.isneg()) {
            std::cerr << "triple-double nroot argument is negative: " << n << std::endl;
            return td_cascade(SpecificValue::snan);
        }

#endif // ! TD_CASCADE_THROW_ARITHMETIC_EXCEPTION

        if (n == 1) return a;
        if (n == 2) return sqrt(a);

        if (a.iszero()) return td_cascade(0.0);

        // Note  a^{-1/n} = exp(-log(a)/n)
	    td_cascade r = abs(a);
	    td_cascade x = std::exp(-std::log(r[0]) / n);

        // Perform Newton's iteration.
        x += x * (1.0 - r * npwr(x, n)) / static_cast<double>(n);
        if (a[0] < 0.0) x = -x;

        return 1.0/x;
    }

}} // namespace sw::universal
