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

    /* polyeval(c, n, x)
   Evaluates the given n-th degree polynomial at x.
    */

    /// <summary>
    /// polyeval evaluates a given n-th degree polynomial at x using Horner's rule.
    /// The polynomial is given by the array of (n+1) coefficients.
    /// </summary>
    /// <param name="c">polynomial coefficients</param>
    /// <param name="n">portion of the polynomial to evaluate</param>
    /// <param name="x">value to evaluate</param>
    /// <returns>polynomial at x</returns>
    inline dd polyeval(const std::vector<dd>& coefficients, int n, const dd& x) {
        // Horner's method of polynomial evaluation
        assert(coefficients.size() > n);
        dd r{ coefficients[n] };

        for (int i = n - 1; i >= 0; --i) {
            r *= x;
            r += coefficients[i];
        }

        return r;
    }

    /// <summary>
    /// polyroot finds a root close to the initial guess x0.
    /// Will only find a single root as it is using a Newton iteration
    /// </summary>
    /// <param name="c">n-th degree polynomial</param>
    /// <param name="x0">initial guess of the root of interest</param>
    /// <param name="max_iter">stopping criterium</param>
    /// <param name="threshold">stopping cirterium</param>
    /// <returns></returns>
    inline dd polyroot(const std::vector<dd>& c, const dd& x0, int max_iter, double threshold = 1e-16) {
        if (threshold == 0.0) threshold = dd_eps;

        int n = c.size() - 1;
        double max_c = std::abs(double(c[0]));
        // Compute the coefficients of the derivatives
        std::vector<dd> derivatives{ c };
        for (int i = 1; i <= n; ++i) {
            double v = std::abs(double(c[i]));
            if (v > max_c) max_c = v;
            derivatives[i - 1] = c[i] * static_cast<double>(i);
        }
        threshold *= max_c;

        // Newton iteration
        bool converged{ false };
        dd x = x0;
        for (int i = 0; i < max_iter; ++i) {
            dd f = polyeval(c, n, x);

            if (abs(f) < threshold) {
                converged = true;
                break;
            }
            x -= (f / polyeval(derivatives, n - 1, x));
        }

        if (!converged) {
           std::cerr << "polyroot: failed to converge\n";
            return dd(SpecificValue::snan);
        }

        return x;
    }

}} // namespace sw::universal
