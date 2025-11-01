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

namespace sw { namespace universal {

#if TD_CASCADE_NATIVE_SQRT

    // Computes the square root of the triple-double number td.
    //   NOTE: td must be a non-negative number
inline td_cascade sqrt(const td_cascade& a) {
        /* Strategy:  Use Karp's trick:  if x is an approximation
           to sqrt(a), then

              sqrt(a) = a*x + [a - (a*x)^2] * x / 2   (approx)

           The approximation is accurate to twice the accuracy of x.
           Also, the multiplication (a*x) and [-]*x can be done with
           only half the precision.
        */

        if (a.iszero()) return td_cascade(0.0);

#	if TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw td_cascade_negative_sqrt_arg();
#else
        if (a.isneg()) std::cerr << "triple-double argument to sqrt is negative: " << a << std::endl;
#endif

        double x = 1.0 / std::sqrt(a[0]);
        double ax = a[0] * x;
        return td_cascade(ax) + td_cascade((a - sqr(td_cascade(ax)))[0] * (x * 0.5));
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
