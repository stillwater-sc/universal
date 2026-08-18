#pragma once
// sqrt.hpp: sqrt functions for quad-double (qd) floats
//
// algorithm courtesy of Scibuilders, Jack Poulson
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

    /// <summary>
    /// sqrt returns the square root of its input, returns NaN if argument is negative
    /// </summary>
    /// <param name="a">input</param>
    /// <returns>sqrt(a) or NaN</returns>
    qd sqrt(const qd& a) {
        /* Strategy:

           Perform the following Newton iteration:

             x' = x + (1 - a * x^2) * x / 2;

           which converges to 1/sqrt(a), starting with the
           double precision approximation to 1/sqrt(a).
           Since Newton's iteration more or less doubles the
           number of correct digits, we only need to perform it
           twice.
        */

#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
        if (a.isneg()) throw qd_negative_sqrt_arg();
#else
        if (a.isneg()) {
            std::cerr << "quad-double argument to sqrt is negative\n";
            return qd(SpecificValue::snan);
        }
#endif

        // sqrt(0) is 0. Without this the seed below is 1/sqrt(0) = inf and the iteration
        // returns NaN; dd has always guarded it, qd never did (universal#1332).
        if (a.iszero()) return qd(0.0);
        if (a.isnan() || a.isinf()) return a;

        // Scale into [0.5, 2) first. The scaling is by a power of two, so it is exact and
        // costs no accuracy, and it bounds every intermediate wherever the argument sits.
        // The iteration converges to 1/sqrt(a) and then multiplies by a, so it holds values
        // of magnitude 1/sqrt(a) and sqrt(a) and squares them: near maxpos that overflows,
        // and near the bottom of the range the trailing limbs of those squares go subnormal
        // -- sqrt(1e300) held 81 bits of its 212 (universal#1332).
        int e{ 0 };
        std::frexp(a[0], &e);
        int k = e >> 1;                  // floor(e/2), correct for negative e
        qd b = ldexp(a, -2 * k);         // b in [0.5, 2), exact

        qd r = (1.0 / std::sqrt(b[0]));
        qd h = mul_pwr2(b, 0.5);

        r += ((0.5 - h * sqr(r)) * r);
        r += ((0.5 - h * sqr(r)) * r);
        r += ((0.5 - h * sqr(r)) * r);

        r *= b;
        return ldexp(r, k);
}

#else

	// sqrt shim for quad-double
	inline qd sqrt(const qd& a) {
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
	inline qd rsqrt(const qd& a) {
		qd v = sqrt(a);
		return reciprocal(v);
	}


    /// <summary>
    /// nroot returns the n-th root of its argument
    /// n must be a positive integer.  
    /// If n is even, then argument _a_ must not be negative.
    /// </summary>
    /// <param name="a">input</param>
    /// <param name="n">n-th root to get</param>
    /// <returns>n-th root of (a) or NaN</returns>
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
            std::cerr << "quad-double nroot argument is negative\n";
            return qd(SpecificValue::snan);
        }

        if (n % 2 == 0 && a.isneg()) {
            std::cerr << "quad-double nroot argument is negative\n";
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
