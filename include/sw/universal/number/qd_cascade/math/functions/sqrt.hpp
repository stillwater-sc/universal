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
        /* Strategy:  Use Karp's trick:  if x is an approximation
           to sqrt(a), then

              sqrt(a) = a*x + [a - (a*x)^2] * x / 2   (approx)

           The approximation is accurate to twice the accuracy of x.
           Also, the multiplication (a*x) and [-]*x can be done with
           only half the precision.
        */

        if (a.iszero()) return qd_cascade(0.0);

#	if QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw qd_cascade_negative_sqrt_arg();
#else
        if (a.isneg()) std::cerr << "quad-double argument to sqrt is negative: " << a << std::endl;
#endif

        double x = 1.0 / std::sqrt(a[0]);
        double ax = a[0] * x;
        return qd_cascade(ax) + qd_cascade((a - sqr(qd_cascade(ax)))[0] * (x * 0.5));
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
