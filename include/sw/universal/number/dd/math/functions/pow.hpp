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
    inline dd exp(const dd&);

    // power function
    inline dd pow(const dd& a, const dd& b) {
        return exp(b * log(a));
    }
	
	// power function of a dd to double
    inline dd pow(dd x, double y) {
        return pow(x, dd(y));
    }

    // Computes the n-th power of a double-double number. 
    //   NOTE:  0^0 is 1, as std::pow
    inline dd npwr(const dd& a, int n) {
        if (n == 0) return 1.0;   // 0^0 included, as std::pow(0, 0) = 1

        dd r = a;
        dd s = 1.0;
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

    inline dd pow(const dd& a, int n) {
        return npwr(a, n);
    }

}} // namespace sw::universal
