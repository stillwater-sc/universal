// constants.hpp: McCleeary LFPERA named constants as lazy ZBCL<FpType> factories
// (Phase 7.1, #931).
//
// The constants are GENERATED (not baked-in double literals): pi via Machin's
// formula, ln2/ln10 via the artanh series, e via the exp(1) Taylor series, the
// radicals via Newton-Raphson sqrt, phi from sqrt(5), and log2(10) = ln10/ln2.
// Each is computed to ~depth blocks, so on a double host it reaches hundreds of
// bits, far beyond the host double. euler_gamma is the one exception: it has no
// efficient elementary series, so it is seeded from the host double value
// (host-precision only) -- a high-precision expansion is a follow-up.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/threeAdd.hpp>     // add
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>     // mul, mul_scalar
#include <universal/number/elreal/divide.hpp>       // div
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/sum.hpp>
#include <universal/number/elreal/math/sqrt.hpp>

namespace sw { namespace universal {

namespace detail {

// odd_power_series(x, alternating, depth):
//   alternating=false -> artanh(x) = sum_n x^(2n+1)/(2n+1)
//   alternating=true  -> atan(x)   = sum_n (-1)^n x^(2n+1)/(2n+1)
// Converges for |x| < 1; terms are accumulated until the running power falls
// below the target precision (depth*k bits) and summed via priestRenorm.
template <typename FpType>
inline ZBCL<FpType> odd_power_series(ZBCL<FpType> x, bool alternating, std::size_t depth) {
    constexpr int k = block<FpType>::k;
    // Stop at the target precision (depth*k bits), but never refine into the
    // denormal range -- terms below min_exponent + 2k would create denormal
    // blocks that break 0-overlap (the same floor div()/mul() respect). For
    // narrow hosts (bfloat16) the floor bounds the work; for double/float the
    // precision target stops first.
    const int stop_exp = std::max(-static_cast<int>(depth) * k - 8,
                                  std::numeric_limits<FpType>::min_exponent + 2 * k);

    ZBCL<FpType> step = mul(x, x, depth);            // x^2
    if (alternating) step = negate(step);            // -x^2 for atan

    std::vector<ZBCL<FpType>> terms;
    ZBCL<FpType> power = x;                           // x^(2n+1), starts at x^1
    for (std::size_t n = 0; ; ++n) {
        if (power.is_empty() || power.head().exponent() < stop_exp) break;   // converged
        const double denom = 2.0 * static_cast<double>(n) + 1.0;
        terms.push_back(div(power, from_native<FpType>(denom), depth));
        power = mul(power, step, depth);             // *x^2 (or *-x^2)
    }
    return sum(series_from_vector(terms), terms.size() + 1);
}

} // namespace detail

// e = exp(1) = sum_{n>=0} 1/n!
template <typename FpType>
inline ZBCL<FpType> e_zbcl(std::size_t depth = 32) {
    // Each term must carry FULL ZBCL precision. A previous version built the term
    // as a host double 1.0/n!, so every term was rounded to ~k bits and the sum was
    // only good to ~k bits (~16 digits for double) no matter how many terms -- the
    // exact-dyadic oracle caught this. Instead accumulate the reciprocal factorial
    // as a ZBCL: term_n = term_{n-1} / n (exact div), so 1/n! keeps depth-block
    // precision. Stop at the target precision (depth*k bits), clamped above the
    // denormal floor like odd_power_series so we never build a denormal block.
    constexpr int k = block<FpType>::k;
    const int stop_exp = std::max(-static_cast<int>(depth) * k - 8,
                                  std::numeric_limits<FpType>::min_exponent + 2 * k);
    std::vector<ZBCL<FpType>> terms;
    ZBCL<FpType> term = from_native<FpType>(1.0);    // 1/0! = 1
    terms.push_back(term);
    const std::size_t maxTerms = 64 * depth + 64;    // generous; convergence breaks first
    for (std::size_t n = 1; n < maxTerms; ++n) {
        term = div(term, from_native<FpType>(static_cast<double>(n)), depth);   // 1/n!
        if (term.is_empty() || term.head().exponent() < stop_exp) break;
        terms.push_back(term);
    }
    return sum(series_from_vector<FpType>(terms), terms.size() + 1);
}

// ln2 = 2 * artanh(1/3)
template <typename FpType>
inline ZBCL<FpType> ln2_zbcl(std::size_t depth = 16) {
    ZBCL<FpType> oneThird = div(from_native<FpType>(1.0), from_native<FpType>(3.0), depth);
    ZBCL<FpType> at = detail::odd_power_series(oneThird, false, depth);
    return mul_scalar(block<FpType>{ static_cast<FpType>(2.0), 0 }, at, depth);
}

// ln10 = 3*ln2 + 2*artanh(1/9)   (since ln10 = ln2 + ln5, ln5 = 2*ln2 + 2*artanh(1/9))
template <typename FpType>
inline ZBCL<FpType> ln10_zbcl(std::size_t depth = 16) {
    ZBCL<FpType> threeLn2 = mul_scalar(block<FpType>{ static_cast<FpType>(3.0), 0 }, ln2_zbcl<FpType>(depth), depth);
    ZBCL<FpType> oneNinth = div(from_native<FpType>(1.0), from_native<FpType>(9.0), depth);
    ZBCL<FpType> twoAt = mul_scalar(block<FpType>{ static_cast<FpType>(2.0), 0 },
                                    detail::odd_power_series(oneNinth, false, depth), depth);
    return add(threeLn2, twoAt);
}

// log2(10) = ln10 / ln2
template <typename FpType>
inline ZBCL<FpType> log2_10_zbcl(std::size_t depth = 16) {
    return div(ln10_zbcl<FpType>(depth + 2), ln2_zbcl<FpType>(depth + 2), depth);
}

// pi = 16*atan(1/5) - 4*atan(1/239)   (Machin)
template <typename FpType>
inline ZBCL<FpType> pi_zbcl(std::size_t depth = 16) {
    ZBCL<FpType> a5   = detail::odd_power_series(div(from_native<FpType>(1.0), from_native<FpType>(5.0),   depth), true, depth);
    ZBCL<FpType> a239 = detail::odd_power_series(div(from_native<FpType>(1.0), from_native<FpType>(239.0), depth), true, depth);
    ZBCL<FpType> t1 = mul_scalar(block<FpType>{ static_cast<FpType>(16.0), 0 }, a5,   depth);
    ZBCL<FpType> t2 = mul_scalar(block<FpType>{ static_cast<FpType>(4.0),  0 }, a239, depth);
    return add(t1, negate(t2));
}

// sqrt(n) radicals
template <typename FpType>
inline ZBCL<FpType> sqrt2_zbcl(std::size_t depth = 16) { return sqrt(from_native<FpType>(2.0), depth); }
template <typename FpType>
inline ZBCL<FpType> sqrt3_zbcl(std::size_t depth = 16) { return sqrt(from_native<FpType>(3.0), depth); }
template <typename FpType>
inline ZBCL<FpType> sqrt5_zbcl(std::size_t depth = 16) { return sqrt(from_native<FpType>(5.0), depth); }

// phi = (1 + sqrt(5)) / 2
template <typename FpType>
inline ZBCL<FpType> phi_zbcl(std::size_t depth = 16) {
    ZBCL<FpType> s = add(from_native<FpType>(1.0), sqrt5_zbcl<FpType>(depth));
    return mul_scalar(block<FpType>{ static_cast<FpType>(0.5), 0 }, s, depth);
}

// euler_gamma: no efficient elementary series -- host-precision seed only.
template <typename FpType>
inline ZBCL<FpType> euler_gamma_zbcl(std::size_t /*depth*/ = 16) {
    return from_native<FpType>(0.57721566490153286060651209008240243104215933593992);
}

}} // namespace sw::universal
