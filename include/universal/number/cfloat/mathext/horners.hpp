#pragma once
// horners.hpp: Horner's polynomial evaluation and root finding functions for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <universal/number/cfloat/cfloat_fwd.hpp>

namespace sw { namespace universal {

    /// <summary>
    /// polyeval evaluates a given n-th degree polynomial at x using Horner's rule.
    /// The polynomial is given by the array of (n+1) coefficients.
    /// </summary>
    /// <param name="c">polynomial coefficients</param>
    /// <param name="n">portion of the polynomial to evaluate</param>
    /// <param name="x">value to evaluate</param>
    /// <returns>polynomial at x</returns>
    template<unsigned nbits, unsigned es, typename BlockType, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
    inline cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> polyeval(const std::vector<cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>>& coefficients, int n, const cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>& x) {
        // Horner's method of polynomial evaluation
        assert(coefficients.size() > n);
        cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> r = coefficients[n];

        for (int i = n - 1; i >= 0; --i) {
            r *= x;
            r += coefficients[i];
        }

        return r;
    }

    /* polyroot(c, n, x0)
       Given an n-th degree polynomial, finds a root close to
       the given guess x0.  Note that this uses simple Newton
       iteration scheme, and does not work for multiple roots.  */

    template<unsigned nbits, unsigned es, typename BlockType, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
    inline cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> polyroot(const std::vector<cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>>& c, const cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>& x0, int n, int max_iter, double threshold) {
	    using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;
        if (threshold == 0.0) threshold = std::numeric_limits<Cfloat>::epsilon();

        int n = c.size() - 1;
        double max_c = std::abs(double(c[0]));
        // Compute the coefficients of the derivatives
        std::vector<Cfloat> derivatives{ c };
        for (int i = 1; i <= n; i++) {
            double v = std::abs(double(c[i]));
            if (v > max_c) max_c = v;
            derivatives[i - 1] = c[i] * static_cast<double>(i);
        }
        threshold *= max_c;

        // Newton iteration
        bool converged{ false };
        Cfloat x = x0;
        for (int i = 0; i < max_iter; i++) {
            Cfloat f = polyeval(c, n, x);

            if (abs(f) < threshold) {
                converged = true;
                break;
            }
            x -= (f / polyeval(derivatives, n - 1, x));
        }

        if (!converged) {
           std::cerr << "polyroot: failed to converge\n";
            return Cfloat(SpecificValue::snan);
        }

        return x;
    }

}} // namespace sw::universal
