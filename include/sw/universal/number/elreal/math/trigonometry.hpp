// trigonometry.hpp: McCleeary LFPERA trig on ZBCL<FpType> (Phase 7.5/7.6, #931).
//
// Inverse trig (7.5):
//   atan(x) = 2*atan( x / (1 + sqrt(1 + x^2)) )   -- argument halving until small,
//             then the alternating Taylor series, scaled back by 2^k.
//   asin(x) = atan( x / sqrt(1 - x^2) )           (|x| < 1; |x|=1 -> +/- pi/2)
//   acos(x) = pi/2 - asin(x)
//
// Forward trig (7.6) via octant reduction x = n*(pi/2) + t, |t| <= pi/4:
//   sin/cos(t) from their Maclaurin series (no cancellation for |t| <= pi/4),
//   recombined per n mod 4; tan(x) = sin(x) / cos(x).
//   A value near a zero (e.g. cos(pi/2)) becomes the series of a tiny reduced
//   argument t, so it relies on priestRenorm separating the cancellation in
//   x - n*(pi/2) to a 0-overlap list (see #1044).
//
// Same modest default depth (4) and ~O(depth^4) time/precision profile as the
// rest of the math suite (see exponent.hpp and #1040).
//
// Host range: atan and sin/cos/tan are robust on a wide host (double/float).
// asin/acos are the deepest compositions here (sqrt . atan . div . pi); on a
// narrow exponent range the intermediate expansions underflow for arguments
// toward the domain boundary (where 1 - x^2 also cancels), so asin/acos are
// intended for a double host.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/threeAdd.hpp>     // add
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>     // mul, mul_scalar
#include <universal/number/elreal/divide.hpp>       // div
#include <universal/number/elreal/series.hpp>       // series_from_vector
#include <universal/number/elreal/sum.hpp>          // sum
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

namespace detail {

// Raw Maclaurin series for sin(t)/cos(t), well-conditioned only for small |t|
// (the octant reduction below keeps |t| <= pi/4, where there is no catastrophic
// cancellation -- cos(pi/4)=sin(pi/4)=0.707, both O(1)).
template <typename FpType>
inline ZBCL<FpType> sin_series(ZBCL<FpType> t, std::size_t depth) {
    using B = block<FpType>;
    if (t.is_empty()) return ZBCL<FpType>{};
    const int stop_exp = -static_cast<int>(depth) * block<FpType>::k - 8;
    ZBCL<FpType> neg_t2 = negate(mul(t, t, depth));
    std::vector<ZBCL<FpType>> terms{ t };                    // t^1 / 1!
    ZBCL<FpType> term = t;
    for (std::size_t nn = 1; nn < 8 * depth; ++nn) {
        const double denom = static_cast<double>(2 * nn) * static_cast<double>(2 * nn + 1);
        term = mul_scalar(B{ static_cast<FpType>(1.0 / denom), 0 }, mul(term, neg_t2, depth), depth);
        if (term.is_empty()) break;
        terms.push_back(term);
        if (term.head().exponent() < stop_exp) break;
    }
    return sum(series_from_vector<FpType>(terms), terms.size() + 1);
}
template <typename FpType>
inline ZBCL<FpType> cos_series(ZBCL<FpType> t, std::size_t depth) {
    using B = block<FpType>;
    const int stop_exp = -static_cast<int>(depth) * block<FpType>::k - 8;
    ZBCL<FpType> neg_t2 = t.is_empty() ? ZBCL<FpType>{} : negate(mul(t, t, depth));
    std::vector<ZBCL<FpType>> terms{ from_native<FpType>(1.0) };   // 1
    ZBCL<FpType> term = from_native<FpType>(1.0);
    for (std::size_t nn = 1; nn < 8 * depth; ++nn) {
        const double denom = static_cast<double>(2 * nn - 1) * static_cast<double>(2 * nn);
        term = mul_scalar(B{ static_cast<FpType>(1.0 / denom), 0 }, mul(term, neg_t2, depth), depth);
        if (term.is_empty()) break;
        terms.push_back(term);
        if (term.head().exponent() < stop_exp) break;
    }
    return sum(series_from_vector<FpType>(terms), terms.size() + 1);
}

// Octant reduction: x = n*(pi/2) + t with |t| <= pi/4. Returns {sin(x), cos(x)}
// built from sin(t)/cos(t) per n mod 4, so a value near a zero (e.g. cos(pi/2))
// is computed as the series of a tiny argument -- no large-term cancellation.
// n is taken from the host-double estimate; very large |x| (Payne-Hanek) is
// deferred, so callers should keep |x| modest.
template <typename FpType>
inline std::pair<ZBCL<FpType>, ZBCL<FpType>> sincos(ZBCL<FpType> x, std::size_t depth) {
    using B = block<FpType>;
    ZBCL<FpType> halfpi = mul_scalar(B{ static_cast<FpType>(0.5), 0 }, pi_zbcl<FpType>(depth), depth);
    const double xa = x.is_empty() ? 0.0 : to_double_approx(x, 2);
    const double nd = std::round(xa / 1.5707963267948966);   // round(x / (pi/2))
    ZBCL<FpType> t = x;
    if (nd != 0.0) {
        // t = x - nd*(pi/2). This subtraction *cancels* (x is near a multiple of
        // pi/2), so a lazy add() would leave the collapsed leading term closer than
        // k to the following limb -- a non-0-overlap ZBCL that trips the invariant
        // when the series forces it (#1044). Combine the operand blocks and
        // renormalise eagerly instead: priestRenorm separates the cancellation
        // residual to a 0-overlap stream.
        ZBCL<FpType> nhalf = negate(mul_scalar(B{ static_cast<FpType>(nd), 0 }, halfpi, depth));
        std::vector<B> pool;
        for (const auto& b : x.take(depth)) pool.push_back(b);
        for (const auto& b : nhalf.take(depth)) pool.push_back(b);
        t = zbcl_from_blocks<FpType>(priestRenorm(pool));
    }
    ZBCL<FpType> st = sin_series(t, depth);
    ZBCL<FpType> ct = cos_series(t, depth);
    const int n = ((static_cast<long long>(nd) % 4) + 4) % 4;
    switch (n) {
        case 0:  return { st, ct };
        case 1:  return { ct, negate(st) };
        case 2:  return { negate(st), negate(ct) };
        default: return { negate(ct), st };                  // n == 3
    }
}

} // namespace detail

// sin(x, depth) -- reduced to [-pi/4, pi/4] by octant before the series.
template <typename FpType>
inline ZBCL<FpType> sin(ZBCL<FpType> x, std::size_t depth = 4) {
    if (x.is_empty()) return ZBCL<FpType>{};                 // sin(0) = 0
    return detail::sincos(x, depth).first;
}

// cos(x, depth).
template <typename FpType>
inline ZBCL<FpType> cos(ZBCL<FpType> x, std::size_t depth = 4) {
    return detail::sincos(x, depth).second;                  // cos(0) = 1
}

// tan(x, depth) = sin(x) / cos(x).
template <typename FpType>
inline ZBCL<FpType> tan(ZBCL<FpType> x, std::size_t depth = 4) {
    auto sc = detail::sincos(x, depth);
    return div(sc.first, sc.second, depth);
}

}} // namespace sw::universal
