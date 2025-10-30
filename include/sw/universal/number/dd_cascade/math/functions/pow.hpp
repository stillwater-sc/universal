#pragma once
// pow.hpp: pow functions for double-double (dd) floating-point
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
    inline dd_cascade exp(const dd_cascade&);

    // power function
    inline dd_cascade pow(const dd_cascade& a, const dd_cascade& b) {
        return exp(b * log(a));
    }
	
	// power function of a dd_cascade to double
    inline dd_cascade pow(const dd_cascade& x, double y) {
	    return pow(x, dd_cascade(y));
    }

    // Computes the n-th power of a double-double number. 
    //   NOTE:  0^0 causes an error.
    inline dd_cascade npwr(const dd_cascade& a, int n) {
        if (n == 0) {
#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
            if (a.iszero()) throw dd_invalid_argument();
#else // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
            if (a.iszero()) {
                std::cerr << "(npwr): Invalid argument\n";
			    return dd_cascade(SpecificValue::snan);
            }
#endif // ! DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
            return 1.0;
        }

        dd_cascade r = a;
        dd_cascade s = 1.0;
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

    inline dd_cascade pow(const dd_cascade& a, int n) {
        return npwr(a, n);
    }

}} // namespace sw::universal
