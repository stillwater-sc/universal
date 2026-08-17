#pragma once
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

namespace sw { namespace universal {

#if QD_CASCADE_NATIVE_SQRT

    // Computes the square root of the quad-double number qd.
    //   NOTE: qd must be a non-negative number
inline qd_cascade sqrt(const qd_cascade& a) {
        /* Strategy: Newton iteration on the RECIPROCAL square root,
           
              r' = r + r * (0.5 - (a/2) * r^2)
           
           which converges to 1/sqrt(a) using multiplication only, and one
           final multiply by a to reach sqrt(a). This is the schedule classic
           qd uses, and it replaced an iteration on (x + a/x)/2 that spent one
           DIVISION per step (universal#1331).
           
           Division used to be the cheaper operation here, which is why the old
           formulation was reasonable when it was written. universal#1326 made
           division correct, and correct division costs 640 nsec/op against 89
           for a multiply, so the arithmetic that motivated the choice reversed:
           the reciprocal form is now 1.9x faster.
           
           From a 53-bit seed each iteration doubles the correct digits, so
           53 -> 106 -> 212 reaches the format and a third iteration carries the
           rounding. Measured residual 0.67 ulps of 2^-212, against 1.05 for qd.
           
           The argument is scaled into [0.5, 2) first. That is not cosmetic: the
           iteration squares r ~ 1/sqrt(a), and for a near maxpos that square
           underflows while for a near minpos it overflows. Scaling by a power
           of two is exact, so it costs nothing in accuracy, and it makes the
           whole range work - sqrt(maxpos) is broken today in dd, dd_cascade and
           qd for exactly this reason (universal#1332).
        */

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

        // exact power-of-two scaling into [0.5, 2)
        int e{ 0 };
        std::frexp(a[0], &e);
        int k = e >> 1;                       // floor(e/2), correct for negative e
        qd_cascade b = ldexp(a, -2 * k);

        qd_cascade r(1.0 / std::sqrt(b[0]));  // ~53 bits
        qd_cascade h = mul_pwr2(b, 0.5);

        r = r + (qd_cascade(0.5) - h * sqr(r)) * r;   // ~106 bits
        r = r + (qd_cascade(0.5) - h * sqr(r)) * r;   // ~212 bits
        r = r + (qd_cascade(0.5) - h * sqr(r)) * r;   // carries the rounding

        return ldexp(r * b, k);
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
