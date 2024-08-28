#pragma once
// sqrt.hpp: sqrt functions for quad-double (qd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

#ifndef QUADDOUBLE_NATIVE_SQRT
#define QUADDOUBLE_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

#if QUADDOUBLE_NATIVE_SQRT

    // Computes the square root of the quad-double number qd.
    //   NOTE: qd must be a non-negative number
    inline qd sqrt(qd const &a) {
        /* Strategy:  Use Karp's trick:  if x is an approximation
           to sqrt(a), then

              sqrt(a) = a*x + [a - (a*x)^2] * x / 2   (approx)

           The approximation is accurate to twice the accuracy of x.
           Also, the multiplication (a*x) and [-]*x can be done with
           only half the precision.
        */

        if (a.iszero()) return qd{};

#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw qd_negative_sqrt_arg();
#else
        if (a.isneg()) std::cerr << "quad-double argument to sqrt is negative: " << a << std::endl;
#endif

        double x = 1.0 / std::sqrt(a[0]);
        double ax = a[0] * x;
        return aqd(ax, (a - sqr(qd(ax)))[0] * (x * 0.5));
    }

#else

	// sqrt shim for quad-double
	inline qd sqrt(qd const& a) {
#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw qd_negative_sqrt_arg();
#else  // ! QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) std::cerr << "quad-double argument to sqrt is negative: " << a << std::endl;
#endif // ! QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.iszero()) return a;
		return qd(std::sqrt(double(a)));
	}

#endif // ! QUADDOUBLE_NATIVE_SQRT

	// reciprocal sqrt
	inline qd rsqrt(qd const& a) {
		qd v = sqrt(a);
		return reciprocal(v);
	}


    /* Computes the n-th root of the quad-double number a.
       NOTE: n must be a positive integer.  
       NOTE: If n is even, then a must not be negative.       */
    inline qd nroot(const qd& a, int n) {
        /* Strategy:  Use Newton iteration for the function

                f(x) = x^(-n) - a

            to find its root a^{-1/n}.  The iteration is thus

                x' = x + x * (1 - a * x^n) / n

            which converges quadratically.  We can then find 
        a^{1/n} by taking the reciprocal.
        */

#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) throw qd_negative_nroot_arg();

        if (n % 2 == 0 && a.isneg()) throw qd_negative_nroot_arg();

#else  // ! QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (n <= 0) {
            std::cerr << "quad-double nroot argument is negative: " << n << std::endl;
        }

        if (n % 2 == 0 && a.isneg()) {
            std::cerr << "quad-double nroot argument is negative: " << n << std::endl;
            return qd(SpecificValue::snan);
        }

#endif // ! QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION

        if (n == 1) return a;
        if (n == 2) return sqrt(a);

        if (a.iszero()) return qd(0.0);

        // Note  a^{-1/n} = exp(-log(a)/n)
        qd r = abs(a);
        qd x = std::exp(-std::log(r[0]) / n);

        // Perform Newton's iteration.
        x += x * (1.0 - r * pown(x, n)) / static_cast<double>(n);
        if (a[0] < 0.0) x = -x;

        return 1.0/x;
    }

}} // namespace sw::universal
