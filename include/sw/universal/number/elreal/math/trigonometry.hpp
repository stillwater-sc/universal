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
//   For |x| < 2^50 the octant n comes from a host-double estimate of x/(pi/2);
//   for larger |x| that estimate loses its low bits, so a Payne-Hanek reduction
//   (#1050) computes the quotient in full ZBCL precision instead -- sin/cos/tan
//   stay accurate for arbitrarily large |x|. Large-|x| reduction is validated on
//   the double host (to 1e20); a narrow host (float) cannot even represent
//   integers past 2^24, so its large-|x| arguments carry no meaningful low bits
//   and are out of scope, like asin/acos below.
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
#include <universal/number/elreal/online_multiply.hpp>  // mul_online (Payne-Hanek t = f*(pi/2))
#include <universal/number/elreal/divide.hpp>       // div
#include <universal/number/elreal/online_divide.hpp>    // div_online (Payne-Hanek q = |x|/(pi/2))
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

// sincos_term_stream: the shared lazy term co-list for the sin/cos Maclaurin series.
// term_{n+1} = term_n * (-t^2) / (a*(a+1)); `a` starts at 2 for sin (denominators
// 2*3, 4*5, ...) and at 1 for cos (1*2, 3*4, ...). Online realisation (#1061 Phase 3b):
// each next term is materialised only to its significance window (take_while_above,
// from constants.hpp) and the alternating-sign terms fold through the streaming infSum's
// carry-arrest. Each term divides by the integer denom EXACTLY (not a host-double
// reciprocal, which capped the series at ~17 digits, #1058 / Phase 3a).
template <typename FpType>
inline series<FpType> sincos_term_stream(ZBCL<FpType> term, ZBCL<FpType> neg_t2,
                                         double a, int floor_exp) {
    if (term.is_empty() || static_cast<int>(term.head().exponent()) < floor_exp)
        return series<FpType>{};
    ZBCL<FpType> next = take_while_above(
        div_online(mul_online(term, neg_t2), from_native<FpType>(a * (a + 1.0))), floor_exp);
    return series<FpType>::cons(term, [next, neg_t2, a, floor_exp]() {
        return sincos_term_stream(next, neg_t2, a + 2.0, floor_exp);
    });
}

// Raw Maclaurin series for sin(t)/cos(t), well-conditioned only for small |t|
// (the octant reduction below keeps |t| <= pi/4, where there is no catastrophic
// cancellation -- cos(pi/4)=sin(pi/4)=0.707, both O(1)). Summed online (#1061 Phase 3b).
template <typename FpType>
inline ZBCL<FpType> sin_series(ZBCL<FpType> t, std::size_t depth) {
    if (t.is_empty()) return ZBCL<FpType>{};                 // sin(0) = 0
    const int floor_exp = -static_cast<int>(depth) * block<FpType>::k - 8;
    ZBCL<FpType> neg_t2 = take_while_above(negate(mul_online(t, t)), floor_exp);
    // sin = t - t^3/3! + t^5/5! - ... : first term t, denominators 2*3, 4*5, ...
    return infsum(detail::sincos_term_stream(t, neg_t2, 2.0, floor_exp));
}
template <typename FpType>
inline ZBCL<FpType> cos_series(ZBCL<FpType> t, std::size_t depth) {
    const int floor_exp = -static_cast<int>(depth) * block<FpType>::k - 8;
    ZBCL<FpType> neg_t2 = t.is_empty() ? ZBCL<FpType>{}
                                       : take_while_above(negate(mul_online(t, t)), floor_exp);
    // cos = 1 - t^2/2! + t^4/4! - ... : first term 1, denominators 1*2, 3*4, ...
    return infsum(detail::sincos_term_stream(from_native<FpType>(1.0), neg_t2, 1.0, floor_exp));
}

// zbcl_round_to_int(q): the integer nearest a non-negative ZBCL value q, as a
// wide integer<256>. Accumulates q's exact value scaled by 2^F into the wide
// integer, then rounds. Each block's value is M * 2^(exponent - (k-1)) with M
// the k-bit significand; blocks whose bits fall wholly below the 2^-F guard
// window are dropped (they cannot change the rounding). q must be >= 0 (callers
// reduce |x|). F guard bits only need to pick the round direction: an off-by-one
// in N is harmless because t = (q - N)*(pi/2) is formed exactly and the octant
// tracks whichever N is chosen.
template <typename FpType>
inline integer<256, std::uint32_t> zbcl_round_to_int(const ZBCL<FpType>& q, std::size_t depth) {
    using Int = integer<256, std::uint32_t>;
    constexpr int k = block<FpType>::k;
    constexpr int F = 64;   // fractional guard bits below the units place
    Int acc(0);
    for (const auto& b : q.take(depth)) {
        if (b.is_zero_block()) continue;
        // block value = v * 2^exp. Split v (a full-range double, NOT pre-scaled to
        // [1,2)) into its signed k-bit significand m and power p: value = m * 2^p.
        int e2 = 0;
        const double f = std::frexp(static_cast<double>(b.v), &e2);  // v = f * 2^e2, |f| in [0.5,1)
        const long long m = std::llround(std::ldexp(f, k));         // exact signed k-bit significand
        const int shift = (e2 - k) + static_cast<int>(b.exp) + F;   // value * 2^F = m * 2^shift
        if (shift + k <= 0) continue;                               // wholly below the guard window
        Int term(m);
        if (shift >= 0) term <<= shift; else term >>= (-shift);
        acc += term;
    }
    // round to nearest: add half a unit (2^(F-1)) then floor by >> F. q >= 0 so
    // acc >= 0; ties round up, which is harmless for range reduction.
    acc += (Int(1) << (F - 1));
    acc >>= F;
    return acc;
}

// int_to_zbcl(N): build the exact value of a non-negative integer<256> N as a
// ZBCL, by summing its (k-1)-bit chunks (each an exactly-representable double,
// normalised by from_native). Used to form -N for the reduced argument.
template <typename FpType>
inline ZBCL<FpType> int_to_zbcl(integer<256, std::uint32_t> N) {
    using Int = integer<256, std::uint32_t>;
    constexpr int k = block<FpType>::k;
    const Int mask = (Int(1) << (k - 1)) - Int(1);
    ZBCL<FpType> acc{};   // 0
    int shift = 0;
    while (!N.iszero()) {
        const long long chunk = static_cast<long long>(N & mask);   // low (k-1) bits
        if (chunk != 0) {
            acc = add(acc, from_native<FpType>(std::ldexp(static_cast<double>(chunk), shift)));
        }
        N >>= (k - 1);
        shift += (k - 1);
    }
    return acc;
}

// Octant reduction: x = n*(pi/2) + t with |t| <= pi/4. Returns {sin(x), cos(x)}
// built from sin(t)/cos(t) per n mod 4, so a value near a zero (e.g. cos(pi/2))
// is computed as the series of a tiny argument -- no large-term cancellation.
//
// For |x| < 2^50 the octant index n comes from a host-double estimate of
// x/(pi/2) -- exact there, since the integer part fits in double's 53-bit range.
// For larger |x| that estimate loses all its low bits, so we switch to a
// Payne-Hanek reduction that computes the quotient q = |x|/(pi/2) in full ZBCL
// precision (enough blocks of pi to cover |x|'s binade), rounds it to the exact
// integer N, and forms t = (q - N)*(pi/2) -- a well-conditioned multiply of the
// small fraction (q - N), never a subtraction of near-equal large quantities.
template <typename FpType>
inline std::pair<ZBCL<FpType>, ZBCL<FpType>> sincos(ZBCL<FpType> x, std::size_t depth) {
    using B = block<FpType>;
    if (x.is_empty()) return { ZBCL<FpType>{}, from_native<FpType>(1.0) };   // sin(0)=0, cos(0)=1

    // |x|'s binary exponent selects the reduction path. The double-estimate path
    // is exact while the integer part of x/(pi/2) fits in double (|x| < ~2^50);
    // beyond that it loses low bits and we switch to Payne-Hanek.
    const int  ex  = static_cast<int>(x.head().exponent());
    const bool big = ex >= 50;

    ZBCL<FpType> t;
    int nmod4;
    if (!big) {
        // ---- fast path: host-double octant estimate (|x| < ~2^50) ----
        ZBCL<FpType> halfpi = mul_scalar(B{ static_cast<FpType>(0.5), 0 }, pi_zbcl<FpType>(depth), depth);
        const double xa = to_double_approx(x, 2);
        const double nd = std::round(xa / 1.5707963267948966);   // round(x / (pi/2))
        t = x;
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
        nmod4 = static_cast<int>(((static_cast<long long>(nd) % 4) + 4) % 4);
    }
    else {
        // ---- Payne-Hanek path: accurate octant + reduced arg for large |x| ----
        // Work on |x| (sin odd, cos even); reapply the sign to sin at the end.
        const bool neg = x.head().sign() < 0;
        ZBCL<FpType> ax = neg ? negate(x) : x;

        // Enough blocks of pi/(2) to cover |x|'s binade plus the output precision:
        // reddepth*k >= ex + depth*k + guard.
        const std::size_t reddepth = depth + 6 + static_cast<std::size_t>(ex / B::k + 2);
        ZBCL<FpType> halfpi = mul_scalar(B{ static_cast<FpType>(0.5), 0 }, pi_zbcl<FpType>(reddepth), reddepth);

        ZBCL<FpType> q = div_online(ax, halfpi);                 // |x| / (pi/2), full precision
        integer<256, std::uint32_t> N = zbcl_round_to_int(q, reddepth);
        nmod4 = static_cast<int>(N % 4);                         // N >= 0

        // f = q - N, renormalised so the cancellation stays 0-overlap (#1044).
        ZBCL<FpType> negN = negate(int_to_zbcl<FpType>(N));
        std::vector<B> pool;
        for (const auto& b : q.take(reddepth))    pool.push_back(b);
        for (const auto& b : negN.take(reddepth)) pool.push_back(b);
        ZBCL<FpType> f = zbcl_from_blocks<FpType>(priestRenorm(pool));

        t = mul_online(f, halfpi);                               // (q - N)*(pi/2) = |x| - N*(pi/2)
        // sin(-|x|) = -sin(|x|): fold the sign back into the octant. Negating the
        // reduced argument negates sin(t) and preserves cos(t), i.e. octant n -> -n.
        if (neg) { t = negate(t); nmod4 = (4 - nmod4) % 4; }
    }

    ZBCL<FpType> st = sin_series(t, depth);
    ZBCL<FpType> ct = cos_series(t, depth);
    switch (nmod4) {
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
