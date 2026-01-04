#pragma once
// pow.hpp: pow functions for triple-double cascade (td_cascade) floating-point
//
// algorithms adapted from quad-double implementation by Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

    // fwd reference
    inline td_cascade exp(const td_cascade&);

    // power function
    inline td_cascade pow(const td_cascade& a, const td_cascade& b) {
        return exp(b * log(a));
    }

	// power function of a td_cascade to double
    inline td_cascade pow(const td_cascade& x, double y) {
        return pow(x, td_cascade(y));
    }

    // Computes the n-th power of a triple-double number.
    //   NOTE:  0^0 causes an error.
    inline td_cascade npwr(const td_cascade& a, int n) {
        if (n == 0) {
#if TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
            if (a.iszero()) throw td_cascade_invalid_argument();
#else // ! TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
            if (a.iszero()) {
                std::cerr << "(npwr): Invalid argument\n";
                return td_cascade(SpecificValue::snan);
            }
#endif // ! TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
            return 1.0;
        }

        td_cascade r = a;
        td_cascade s = 1.0;
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

    inline td_cascade pow(const td_cascade& a, int n) {
        return npwr(a, n);
    }


}} // namespace sw::universal
