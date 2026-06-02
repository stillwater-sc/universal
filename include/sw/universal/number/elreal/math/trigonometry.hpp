// trigonometry.hpp: McCleeary LFPERA inverse trig on ZBCL<FpType> (Phase 7.5, #931).
// (Forward trig sin/cos/tan arrive in Phase 7.6.)
//
//   atan(x) = 2*atan( x / (1 + sqrt(1 + x^2)) )   -- argument halving until small,
//             then the alternating Taylor series, scaled back by 2^k.
//   asin(x) = atan( x / sqrt(1 - x^2) )           (|x| < 1; |x|=1 -> +/- pi/2)
//   acos(x) = pi/2 - asin(x)
//
// Same modest default depth (4) and ~O(depth^4) time/precision profile as the
// rest of the math suite (see exponent.hpp and #1040).
//
// Host range: atan is robust on any host. asin/acos are the deepest compositions
// here (sqrt . atan . div . pi); on a narrow exponent range (float, bfloat16) the
// intermediate expansions underflow for arguments toward the domain boundary
// (where 1 - x^2 also cancels), so asin/acos are intended for a double host.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstddef>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/threeAdd.hpp>     // add
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>     // mul, mul_scalar
#include <universal/number/elreal/divide.hpp>       // div
#include <universal/number/elreal/math/sqrt.hpp>
#include <universal/number/elreal/math/constants.hpp>   // pi_zbcl, detail::odd_power_series

namespace sw { namespace universal {

// atan(x, depth): arctangent over all x.
template <typename FpType>
inline ZBCL<FpType> atan(ZBCL<FpType> x, std::size_t depth = 4) {
    using B = block<FpType>;
    if (x.is_empty()) return ZBCL<FpType>{};                 // atan(0) = 0

    // Halve the argument until |xr| is small (fast series convergence):
    //   atan(x) = 2 * atan( x / (1 + sqrt(1 + x^2)) ).
    int k = 0;
    ZBCL<FpType> xr = x;
    double xa = to_double_approx(x, 2);
    ZBCL<FpType> one = from_native<FpType>(1.0);
    while (std::abs(xa) > 0.25 && k < 48) {
        ZBCL<FpType> s = sqrt(add(one, mul(xr, xr, depth)), depth);     // sqrt(1 + xr^2)
        xr = div(xr, add(one, s), depth);
        xa = to_double_approx(xr, 2);
        ++k;
    }
    ZBCL<FpType> a = detail::odd_power_series(xr, true, depth);          // atan(xr), |xr| <= 0.25
    if (k > 0) a = mul_scalar(B{ static_cast<FpType>(1.0), k }, a, depth);   // * 2^k (exact)
    return a;
}

// asin(x, depth): |x| <= 1; |x| > 1 -> empty (domain).
template <typename FpType>
inline ZBCL<FpType> asin(ZBCL<FpType> x, std::size_t depth = 4) {
    using B = block<FpType>;
    if (x.is_empty()) return ZBCL<FpType>{};                 // asin(0) = 0
    const double xa = to_double_approx(x, 2);
    if (std::abs(xa) > 1.0 + 1e-9) return ZBCL<FpType>{};     // out of domain

    ZBCL<FpType> oneMx2 = add(from_native<FpType>(1.0), negate(mul(x, x, depth)));   // 1 - x^2
    if (oneMx2.is_empty() || to_double_approx(oneMx2, 2) <= 0.0) {
        // |x| == 1 -> +/- pi/2 (avoid the 1/0 in x/sqrt(1-x^2)).
        ZBCL<FpType> halfpi = mul_scalar(B{ static_cast<FpType>(0.5), 0 }, pi_zbcl<FpType>(depth), depth);
        return (xa < 0.0) ? negate(halfpi) : halfpi;
    }

    // For |x| > 1/sqrt(2), x/sqrt(1-x^2) blows up and atan would need a very deep
    // argument reduction (underflowing the host's exponent range). Reflect:
    //   asin(x) = sign(x) * ( pi/2 - asin(sqrt(1-x^2)) ),  sqrt(1-x^2) < 1/sqrt(2),
    // so the recursive asin sees a small, well-conditioned argument.
    if (std::abs(xa) > 0.70710678118654752) {
        ZBCL<FpType> reduced = asin(sqrt(oneMx2, depth), depth);
        ZBCL<FpType> halfpi  = mul_scalar(B{ static_cast<FpType>(0.5), 0 }, pi_zbcl<FpType>(depth), depth);
        ZBCL<FpType> r = add(halfpi, negate(reduced));
        return (xa < 0.0) ? negate(r) : r;
    }
    // |x| <= 1/sqrt(2): asin(x) = atan( x / sqrt(1 - x^2) ), argument <= 1.
    return atan(div(x, sqrt(oneMx2, depth), depth), depth);
}

// acos(x, depth) = pi/2 - asin(x); |x| <= 1.
template <typename FpType>
inline ZBCL<FpType> acos(ZBCL<FpType> x, std::size_t depth = 4) {
    using B = block<FpType>;
    if (to_double_approx(x.is_empty() ? ZBCL<FpType>{} : x, 2) > 1.0 + 1e-9) return ZBCL<FpType>{};
    ZBCL<FpType> halfpi = mul_scalar(B{ static_cast<FpType>(0.5), 0 }, pi_zbcl<FpType>(depth), depth);
    return add(halfpi, negate(asin(x, depth)));
}

}} // namespace sw::universal
