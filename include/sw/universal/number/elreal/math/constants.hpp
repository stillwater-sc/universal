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

// Guard blocks the series helpers carry internally. A truncated Taylor/artanh
// series run at the output depth tops out ~2 components below what div() alone
// delivers, because mul/div/sum accumulation error consumes the deepest output
// blocks. Running the internal mul/div/sum a few blocks deeper (output + guard)
// absorbs that loss, so the series reaches the same ceiling as div (#1052).
constexpr std::size_t kSeriesGuard = 4;

// Wide-host series stop_exp: on a wide host (k>=24) the EFTs are scale-invariant
// and div has no denormal floor, so refine to the working-depth precision; on a
// narrow host keep the denormal floor.
template <typename FpType>
constexpr int series_stop_exp(std::size_t wdepth) {
    constexpr int k = block<FpType>::k;
    const int target = -static_cast<int>(wdepth) * k - 8;
    const int floor  = (k >= 24) ? (std::numeric_limits<int>::min() / 2)
                                 : (std::numeric_limits<FpType>::min_exponent + 2 * k);
    return target > floor ? target : floor;
}

// odd_power_series(x, alternating, depth):
//   alternating=false -> artanh(x) = sum_n x^(2n+1)/(2n+1)
//   alternating=true  -> atan(x)   = sum_n (-1)^n x^(2n+1)/(2n+1)
// Converges for |x| < 1; terms are accumulated until the running power falls
// below the target precision (depth*k bits) and summed via priestRenorm.
template <typename FpType>
inline ZBCL<FpType> odd_power_series(ZBCL<FpType> x, bool alternating, std::size_t depth) {
    // Carry kSeriesGuard extra working blocks internally (see above) and stop at
    // the working-depth precision (wide host) / denormal floor (narrow host).
    const std::size_t wdepth = depth + kSeriesGuard;
    const int stop_exp = series_stop_exp<FpType>(wdepth);

    ZBCL<FpType> step = mul(x, x, wdepth);           // x^2
    if (alternating) step = negate(step);            // -x^2 for atan

    std::vector<ZBCL<FpType>> terms;
    ZBCL<FpType> power = x;                           // x^(2n+1), starts at x^1
    for (std::size_t n = 0; ; ++n) {
        if (power.is_empty() || power.head().exponent() < stop_exp) break;   // converged
        const double denom = 2.0 * static_cast<double>(n) + 1.0;
        terms.push_back(div(power, from_native<FpType>(denom), wdepth));
        power = mul(power, step, wdepth);            // *x^2 (or *-x^2)
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
    // precision. Carry kSeriesGuard extra working blocks (like odd_power_series) so
    // accumulation error does not eat the deepest output components, and stop at the
    // working-depth precision / denormal floor.
    const std::size_t wdepth = depth + detail::kSeriesGuard;
    const int stop_exp = detail::series_stop_exp<FpType>(wdepth);
    std::vector<ZBCL<FpType>> terms;
    ZBCL<FpType> term = from_native<FpType>(1.0);    // 1/0! = 1
    terms.push_back(term);
    const std::size_t maxTerms = 64 * depth + 64;    // generous; convergence breaks first
    for (std::size_t n = 1; n < maxTerms; ++n) {
        term = div(term, from_native<FpType>(static_cast<double>(n)), wdepth);   // 1/n!
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

// euler_gamma via Brent-McMillan B1 (#1053): gamma = A(n)/B(n) - ln(n), with
//   B(n) = sum_{k>=0} (n^k/k!)^2,  A(n) = sum_{k>=0} (n^k/k!)^2 * H_k,  H_0 = 0,
//   |error| ~ pi*exp(-4n)  ->  n >= D*ln(10)/4 for D digits.
// The Euler-Mascheroni constant has no rapidly-converging elementary series, so
// (unlike pi/e/ln2) it needs a real algorithm. The per-term work is dominated by
// the product w_k * H_k; the long tail of terms contributes only a few limbs each.
//
// HOST RANGE: the intermediate w_k = (n^k/k!)^2 peaks at ~exp(2n) near k=n. n grows
// with the requested precision (n ~ D*ln(10)/4), and the block significand carries
// that magnitude, so a NARROW-exponent host (float/bfloat16, max ~10^38) overflows
// once n exceeds ~44 (a few blocks of depth). High-precision euler_gamma is therefore
// a double-host generator; on narrow hosts keep depth shallow (n below the overflow).
// Validated to 307 digits on double (#1053); eager like the other constants -- the
// online-ops speedup for the whole series layer is #1061 Phase 3.
template <typename FpType>
inline ZBCL<FpType> euler_gamma_zbcl(std::size_t depth = 16) {
    using B = block<FpType>;
    const std::size_t wdepth = depth + detail::kSeriesGuard;
    const int wbits = static_cast<int>(wdepth) * block<FpType>::k;
    const double targetDigits = static_cast<double>(wbits) * 0.30102999566398 + 16.0;
    const int n = static_cast<int>(std::ceil(targetDigits * 0.57564627324851)) + 5; // ln(10)/4
    const double n2 = static_cast<double>(n) * static_cast<double>(n);
    const ZBCL<FpType> one = from_native<FpType>(1.0);
    // running, priestRenorm'd sums capped to working depth (terms > wdepth limbs
    // below the running peak are negligible for the O(1) ratio A/B).
    const std::size_t cap = wdepth + 2;
    auto fold = [cap](std::vector<B>& acc, const ZBCL<FpType>& term, std::size_t take) {
        std::vector<B> pool = acc;
        for (const auto& b : term.take(take)) pool.push_back(b);
        acc = priestRenorm(pool);
        if (acc.size() > cap) acc.resize(cap);
    };
    ZBCL<FpType> w = one;
    int peakExp = static_cast<int>(w.head().exponent());
    std::vector<B> Hblocks, Bblocks = w.take(cap), Ablocks;
    for (std::size_t k = 1; ; ++k) {
        w = div(mul_scalar(B{ static_cast<FpType>(n2), 0 }, w, wdepth),
                from_native<FpType>(static_cast<double>(k) * static_cast<double>(k)), wdepth); // *n^2/k^2
        if (w.is_empty()) break;
        peakExp = std::max(peakExp, static_cast<int>(w.head().exponent()));
        fold(Hblocks, div(one, from_native<FpType>(static_cast<double>(k)), wdepth), wdepth);   // H_k += 1/k
        ZBCL<FpType> Hk = zbcl_from_blocks<FpType>(Hblocks);
        fold(Bblocks, w, cap);
        fold(Ablocks, mul(w, Hk, wdepth), cap);
        if (k > static_cast<std::size_t>(n) &&
            static_cast<int>(w.head().exponent()) < peakExp - wbits - 8) break;
    }
    ZBCL<FpType> ratio = div(zbcl_from_blocks<FpType>(Ablocks), zbcl_from_blocks<FpType>(Bblocks), wdepth);
    // ln(n) = e*ln2 + 2*artanh((m-1)/(m+1)), m = n / 2^e in [1,2). Pass `depth` (NOT
    // wdepth): the helpers add their own kSeriesGuard, and ln2_zbcl=artanh(1/3) is
    // costly over-deep.
    const ZBCL<FpType> N = from_native<FpType>(static_cast<double>(n));
    const int e = static_cast<int>(N.head().exponent());
    ZBCL<FpType> m = mul_scalar(B{ static_cast<FpType>(1.0), -e }, N, depth);
    ZBCL<FpType> u = div(add(m, negate(one)), add(m, one), depth);
    ZBCL<FpType> lnN = mul_scalar(B{ static_cast<FpType>(2.0), 0 }, detail::odd_power_series(u, false, depth), depth);
    if (e != 0) lnN = add(lnN, mul_scalar(B{ static_cast<FpType>(static_cast<double>(e)), 0 }, ln2_zbcl<FpType>(depth), depth));
    return add(ratio, negate(lnN)); // gamma = A/B - ln(n)
}

}} // namespace sw::universal
