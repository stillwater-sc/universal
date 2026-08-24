#pragma once
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
// sqrt.hpp: sqrt functions for quad-double (qd) cascade floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

#ifndef QD_CASCADE_NATIVE_SQRT
#define QD_CASCADE_NATIVE_SQRT 1
#endif

// square root formulations; see the comment on sqrt() below for the trade
#define UNIVERSAL_QD_CASCADE_SQRT_NEWTON_DIVISION   0
#define UNIVERSAL_QD_CASCADE_SQRT_NEWTON_RECIPROCAL 1
#ifndef UNIVERSAL_QD_CASCADE_SQRT_ALGORITHM
#define UNIVERSAL_QD_CASCADE_SQRT_ALGORITHM UNIVERSAL_QD_CASCADE_SQRT_NEWTON_DIVISION
#endif

namespace sw { namespace universal {

#if QD_CASCADE_NATIVE_SQRT

    // Computes the square root of the quad-double number qd.
    //   NOTE: qd must be a non-negative number
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
    // asked for, by defining UNIVERSAL_QD_CASCADE_SQRT_ALGORITHM.
    //
    // Measured on an i7-12700K, gcc 13.3 -O3, residual of r*r - a in ulps of
    // 2^-212:
    //
    //     NEWTON_DIVISION    0.42 ulps  2982 nsec/op   (default)
    //     NEWTON_RECIPROCAL  1.05 ulps  1562 nsec/op   = classic qd's algorithm,
    //                                                    and classic qd's accuracy
    //
    // Every formulation scales the argument into [0.5, 2) first, exactly, by a
    // power of two. Each of them squares a value of magnitude ~sqrt(a) or
    // ~1/sqrt(a), which leaves the representable range at the extremes: that is
    // why sqrt(maxpos) returns inf or NaN today in dd and qd (universal#1332).
inline qd_cascade sqrt(const qd_cascade& a) {
        if (a.iszero()) return qd_cascade(0.0);

#	if QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw qd_cascade_negative_sqrt_arg();
#else
        if (a.isneg()) {
            std::cerr << "quad-double argument to sqrt is negative: " << a << std::endl;
            return qd_cascade(SpecificValue::qnan);
        }
#endif
        if (a.isnan() || a.isinf()) return a;

        int e{ 0 };
        std::frexp(a[0], &e);
        int k = e >> 1;                  // floor(e/2), correct for negative e
        qd_cascade b = ldexp(a, -2 * k);       // b in [0.5, 2), exact

#if UNIVERSAL_QD_CASCADE_SQRT_ALGORITHM == UNIVERSAL_QD_CASCADE_SQRT_NEWTON_DIVISION

        qd_cascade x = std::sqrt(b[0]);
        x = (x + b / x) * 0.5;
        x = (x + b / x) * 0.5;
        x = (x + b / x) * 0.5;
        return ldexp(x, k);

#else   // NEWTON_RECIPROCAL

        qd_cascade r(1.0 / std::sqrt(b[0]));
        qd_cascade h = mul_pwr2(b, 0.5);
        r = r + (qd_cascade(0.5) - h * sqr(r)) * r;
        r = r + (qd_cascade(0.5) - h * sqr(r)) * r;
        r = r + (qd_cascade(0.5) - h * sqr(r)) * r;
        return ldexp(r * b, k);

#endif
    }

#else

	// sqrt shim for quad-double
	inline qd_cascade sqrt(qd_cascade a) {
#if QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw qd_cascade_negative_sqrt_arg();
#else  // ! QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) std::cerr << "quad-double argument to sqrt is negative: " << a << std::endl;
#endif // ! QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
		if (a.iszero()) return a;
		return qd_cascade(std::sqrt(double(a)));
	}

#endif // ! QD_CASCADE_NATIVE_SQRT

    // Note: sqrt(double) helper removed to avoid namespace pollution
    // Use explicit construction instead: sqrt(qd_cascade(d))

	// reciprocal sqrt
    inline qd_cascade rsqrt(const qd_cascade& a) {
	    qd_cascade v = sw::universal::sqrt(a);
		return reciprocal(v);
	}


    /* Computes the n-th root of the quad-double number a.
       NOTE: n must be a positive integer.
       NOTE: If n is even, then a must not be negative.       */
    inline qd_cascade nroot(const qd_cascade& a, int n) {
        /* Strategy:  Use Newton iteration for the function

                f(x) = x^(-n) - a

            to find its root a^{-1/n}.  The iteration is thus

                x' = x + x * (1 - a * x^n) / n

            which converges quadratically.  We can then find
        a^{1/n} by taking the reciprocal.
        */

#if QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) throw qd_cascade_negative_nroot_arg();

        if (n % 2 == 0 && a.isneg()) throw qd_cascade_negative_nroot_arg();

#else  // ! QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) {
            std::cerr << "quad-double nroot argument is negative: " << n << std::endl;
        }

        if (n % 2 == 0 && a.isneg()) {
            std::cerr << "quad-double nroot argument is negative: " << n << std::endl;
            return qd_cascade(SpecificValue::snan);
        }

#endif // ! QD_CASCADE_THROW_ARITHMETIC_EXCEPTION

        if (n == 1) return a;
        if (n == 2) return sqrt(a);

        if (a.iszero()) return qd_cascade(0.0);

        // Note  a^{-1/n} = exp(-log(a)/n)
	    qd_cascade r = abs(a);
	    qd_cascade x = std::exp(-std::log(r[0]) / n);

        // Perform Newton's iteration.
        x += x * (1.0 - r * npwr(x, n)) / static_cast<double>(n);
        if (a[0] < 0.0) x = -x;

        return 1.0/x;
    }

}} // namespace sw::universal
