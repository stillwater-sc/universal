#pragma once
// pow.hpp: pow functions for quad-double (qd) floating-point
//
// algorithms courtesy Scibuilders, Jack Poulson
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

    // fwd reference
    inline qd exp(const qd&);

    // power function
    inline qd pow(const qd& a, const qd& b) {
        return exp(b * log(a));
    }
	
	// power function of a qd to double
    inline qd pow(const qd& x, double y) {
        return pow(x, qd(y));
    }

    // Computes the n-th power of a quad-double number. 
    //   NOTE:  0^0 causes an error.
    inline qd npwr(const qd& a, int n) {
        if (n == 0) {
#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
            if (a.iszero()) throw qd_invalid_argument();
#else // ! QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
            if (a.iszero()) {
                std::cerr << "(npwr): Invalid argument\n";
                return qd(SpecificValue::snan);
            }
#endif // ! QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
            return 1.0;
        }

        qd r = a;
        qd s = 1.0;
        int N = std::abs(n);

        if (N > 1) {
            // Use binary exponentiation
            while (N > 0) {
                if (N % 2 == 1) {
                    s *= r;
                }
                N /= 2;
                if (N > 0) r = sqr(r);
            }
        } else {
            s = r;
        }

        // if n is negative then compute the reciprocal 
        if (n < 0) return (1.0 / s);
        return s;
    }

    inline qd pow(const qd& a, int n) {
        return npwr(a, n);
    }


}} // namespace sw::universal
