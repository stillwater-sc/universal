#pragma once
// horners.hpp: Horner's polynomial evaluation and root finding functions for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <universal/number/dd/dd_fwd.hpp>

namespace sw { namespace universal {

    /// <summary>
    /// polyeval evaluates a given n-th degree polynomial at x using Horner's rule.
    /// The polynomial is given by the array of (n+1) coefficients.
    /// </summary>
    /// <param name="c">polynomial coefficients</param>
    /// <param name="n">portion of the polynomial to evaluate</param>
    /// <param name="x">value to evaluate</param>
    /// <returns>polynomial at x</returns>
    inline qd polyeval(const std::vector<qd>& coefficients, int n, const qd& x) {
        // Horner's method of polynomial evaluation
        assert(coefficients.size() > n);
        qd r = coefficients[n];

        for (int i = n - 1; i >= 0; i--) {
            r *= x;
            r += coefficients[i];
        }

        return r;
    }

    /* polyroot(c, n, x0)
       Given an n-th degree polynomial, finds a root close to
       the given guess x0.  Note that this uses simple Newton
       iteration scheme, and does not work for multiple roots.  */

   
    inline qd polyroot(const std::vector<qd>& c, const qd& x0, int n, int max_iter, double threshold) {
        qd x = x0;

        std::vector<qd> d{ c };
        bool converged{ false };
        double max_c = std::abs(double(c[0]));
        double v;

        if (threshold == 0.0) threshold = qd_eps;

        // Compute the coefficients of the derivatives
        for (int i = 1; i <= n; i++) {
            v = std::abs(double(c[i]));
            if (v > max_c) max_c = v;
            d[i - 1] = c[i] * static_cast<double>(i);
        }
        threshold *= max_c;

        // Newton iteration
        for (int i = 0; i < max_iter; i++) {
            qd f = polyeval(c, n, x);

            if (abs(f) < threshold) {
                converged = true;
                break;
            }
            x -= (f / polyeval(d, n - 1, x));
        }

        if (!converged) {
           std::cerr << "polyroot: failed to converge\n";
            return qd(SpecificValue::snan);
        }

        return x;
    }

}} // namespace sw::universal
