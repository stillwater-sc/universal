#pragma once
// pow.hpp: pow functions for quad-double cascade (qd_cascade) floating-point
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
    inline qd_cascade exp(const qd_cascade&);

    // power function
    inline qd_cascade pow(const qd_cascade& a, const qd_cascade& b) {
        return exp(b * log(a));
    }

	// power function of a qd_cascade to double
    inline qd_cascade pow(const qd_cascade& x, double y) {
        return pow(x, qd_cascade(y));
    }

    // Computes the n-th power of a quad-double number.
    //   NOTE:  0^0 is 1, as std::pow
    inline qd_cascade npwr(const qd_cascade& a, int n) {
        if (n == 0) return 1.0;   // 0^0 included, as std::pow(0, 0) = 1

        qd_cascade r = a;
        qd_cascade s = 1.0;
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

    inline qd_cascade pow(const qd_cascade& a, int n) {
        return npwr(a, n);
    }


}} // namespace sw::universal
