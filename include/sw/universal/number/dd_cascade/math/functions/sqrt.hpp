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
        /* Strategy:  Use Newton-Raphson iteration:

              x' = (x + a/x) / 2

           Starting with x = sqrt(a[0]), each iteration doubles the
           number of correct digits. This method is numerically stable
           across the entire range, including near-max values where
           Karp's trick (a*x) would overflow.

           For dd_cascade (106 bits precision):
           - Initial guess: ~53 bits
           - After iteration 1: ~106 bits
           - After iteration 2: ~212 bits (sufficient)
        */

        if (a.iszero()) return dd_cascade(0.0);

#	if DD_CASCADE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw dd_cascade_negative_sqrt_arg();
#else
        if (a.isneg()) std::cerr << "double-double argument to sqrt is negative: " << a << std::endl;
#endif

        // Initial approximation from high component
        dd_cascade x = std::sqrt(a.high());

        // Newton iteration 1: x = (x + a/x) / 2
        x = (x + a / x) * 0.5;

        // Newton iteration 2: doubles precision again
        x = (x + a / x) * 0.5;

        return x;
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
