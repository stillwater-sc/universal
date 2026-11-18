#pragma once
#include <limits>        // std::numeric_limits
#include <cmath>         // the <cmath> functions used below
#include <cassert>       // assert()
#include <cstddef>       // std::size_t
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
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
    /// <param name="coefficients">polynomial coefficients</param>
    /// <param name="n">portion of the polynomial to evaluate</param>
    /// <param name="x">value to evaluate</param>
    /// <returns>polynomial at x</returns>
    template<unsigned nbits, unsigned es, typename BlockType, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
    inline cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating> polyeval(const std::vector<cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>>& coefficients, int n, const cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>& x) {
        // Horner's method of polynomial evaluation
        assert(n >= 0 && coefficients.size() > static_cast<size_t>(n));
        cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating> r = coefficients[n];

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

    template<unsigned nbits, unsigned es, typename BlockType, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
    inline cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating> 
        polyroot(const std::vector<cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>>& c, const cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>& x0, int max_iter, double threshold) {
	    using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;
        // a cfloat does not convert to double implicitly: this did not compile (#2089)
        // threshold 0 is numeric_limits<Cfloat>::epsilon(), scaled below by the largest coefficient.
        // For a layout without subnormals whose 2^-fbits lies below the smallest normal, epsilon()
        // is min() (#2089), larger than the gap above 1.0, and that is deliberate here (USER decision,
        // #2133): in such a layout every Newton correction f/f' smaller than min() flushes to 0, so
        // the iteration cannot refine past about min() * |f'|. A threshold below that stalls and
        // reports "failed to converge"; min() stops at the attainable root (cfloat<24,4> without
        // subnormals: x^2 - 2 from 1 gives 1.41667, error 2.5e-3).
        if (threshold == 0.0) threshold = double(std::numeric_limits<Cfloat>::epsilon());

        // a constant polynomial has no derivative to iterate on: polyeval(derivatives, -1, x)
        // read coefficients[-1], and an empty one read c[0].  A zero constant has every x as a
        // root; any other constant, and no polynomial at all, has none (#2089).
        if (c.empty()) return Cfloat(SpecificValue::qnan);
        if (c.size() == 1) return c[0].iszero() ? x0 : Cfloat(SpecificValue::qnan);

        int n = static_cast<int>(c.size()) - 1;
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
