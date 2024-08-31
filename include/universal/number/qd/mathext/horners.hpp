#pragma once
// horners.hpp: Horner's polynomial evaluation and root finding functions for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <universal/number/qd/qd_fwd.hpp>

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
        assert(coefficients.size() > static_cast<unsigned>(n));
        qd r{ coefficients[static_cast<unsigned>(n)] };

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
    /// <returns>root closed to x0</returns>
    inline qd polyroot(const std::vector<qd>& c, const qd& x0, int max_iter, double threshold = 1e-32) {
        if (threshold == 0.0) threshold = qd_eps;

        int n = c.size() - 1;
        double max_c = std::abs(double(c[0]));
        // Compute the coefficients of the derivatives
        std::vector<qd> derivatives{ c };
        for (int i = 1; i <= n; ++i) {
            double v = std::abs(double(c[static_cast<unsigned>(i)]));
            if (v > max_c) max_c = v;
            derivatives[i - 1] = c[static_cast<unsigned>(i)] * static_cast<double>(i);
        }
        threshold *= max_c;

        // Newton iteration
        bool converged{ false };
        qd x = x0;
        for (int i = 0; i < max_iter; ++i) {
            qd f = polyeval(c, n, x);

            if (abs(f) < threshold) {
                converged = true;
                break;
            }
            x -= (f / polyeval(derivatives, n - 1, x));
        }

        if (!converged) {
           std::cerr << "polyroot: failed to converge\n";
            return qd(SpecificValue::snan);
        }

        return x;
    }

}} // namespace sw::universal
