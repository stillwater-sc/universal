// block_eft.hpp: error-free transforms on elreal block<FpType>.
//
// Phase 3 (#927). Six primitives:
//
//   block_two_sum(a, b)   -> (high, low)         exact: high + low = a + b
//   block_two_sum_rn(a,b) -> high                rounded: just a + b
//   block_two_mult(a, b)  -> (high, low)         exact: high + low = a * b
//   block_two_mult_rn(a,b)-> high                rounded: just a * b
//   block_two_div(a, b)   -> (high, low)         a/b = high + low (approx)
//   block_two_div_rn(a,b) -> high                rounded: just a / b
//
// Implementation rule (binding per epic #923):
//   Use the host FpType's arithmetic directly. Do NOT promote to a wider
//   type, compute, and snap back. The simulation must reflect what the
//   eventual hardware would compute on the chosen FpType, including the
//   accumulated rounding behaviour of the host's EFT chain.
//
// References:
//   Knuth, Vol. 2 (twoSum)
//   Dekker (1971): twoProd via Veltkamp split
//   Bailey/Hida QD library: two_div formulation
//   McCleeary 2019 dissertation, Section 4.1 (EFTs on blocks)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

#include <universal/number/elreal/block.hpp>
#include <universal/numerics/error_free_ops.hpp>

namespace sw { namespace universal {

namespace detail {

// Prevent the optimiser from algebraically eliminating the EFT residual
// computation. Clang in particular will treat sub-expressions like `(s - a)`
// after `s = a + b` as algebraically equal to `b`, which is true in real
// arithmetic but NOT in rounded floating-point. We mark the helpers with
// noinline so the optimiser can't see through the function boundary and apply
// such rewrites. The double specialisation uses error_free_ops.hpp's
// volatile-based guard so it remains inlineable.
#if defined(__GNUC__) || defined(__clang__)
#  define UNIVERSAL_ELREAL_EFT_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define UNIVERSAL_ELREAL_EFT_NOINLINE __declspec(noinline)
#else
#  define UNIVERSAL_ELREAL_EFT_NOINLINE
#endif

// Veltkamp splitter for FpType: S = 2^ceil(p/2) + 1, where p = precision.
// The result is exactly representable in FpType for all IEEE-compliant types
// with p >= 3. Computed at first call and cached as a static local.
template <typename T>
inline T eft_splitter() {
    static const T s = static_cast<T>(
        std::ldexp(1.0, (std::numeric_limits<T>::digits + 1) / 2) + 1.0);
    return s;
}

// Generic Knuth two_sum, computed entirely in host arithmetic. Works for any
// IEEE-754 round-to-nearest-compliant FpType. noinline keeps the optimiser
// from algebraically rewriting `s - a` as `b` across the function boundary.
template <typename T>
UNIVERSAL_ELREAL_EFT_NOINLINE
inline void two_sum_host(T a, T b, T& s, T& r) {
    s = a + b;
    T bb = s - a;
    r = (a - (s - bb)) + (b - bb);
}

// Specialisation for double: reuse the existing volatile-protected EFT so we
// inherit the constexpr / non-reassociation guarantees of error_free_ops.hpp.
template <>
inline void two_sum_host<double>(double a, double b, double& s, double& r) {
    s = sw::universal::two_sum(a, b, r);
}

// Generic Veltkamp split. noinline for the same reason as two_sum_host:
// `temp - (temp - a)` is algebraically `a` in real arithmetic but produces a
// non-trivial result in rounded FP.
template <typename T>
UNIVERSAL_ELREAL_EFT_NOINLINE
inline void split_host(T a, T& hi, T& lo) {
    const T S = eft_splitter<T>();
    T temp = S * a;
    hi   = temp - (temp - a);
    lo   = a - hi;
}

// two_prod returning the rounded product p = a*b and the residual r = a*b - p.
//
// The generic Veltkamp/Dekker split is exact only when the precision p is EVEN:
// the partial product a_hi*b_hi must fit in p bits, which needs 2*ceil(p/2) <= p.
// For odd p the split leaks error that grows with p -- negligible for small p
// (bfloat16 p=7 still rounds exactly) but ~80 ulp^2 for cfloat<24,5> p=19
// (issue #942). Two paths:
//
//   - even p: Veltkamp/Dekker residual is exact (and stays in host arithmetic).
//   - odd p:  compute a*b in a `double` intermediate (>= 2p bits for every host
//             FpType used in elreal blocks, p <= 26), so `wp` is the EXACT
//             product and `wp - double(p)` is the EXACT residual; only the
//             residual uses the wider type, the rounded product p = a*b stays in
//             host arithmetic.
//
// IMPORTANT representability caveat: two_prod is bit-exact only when the residual
// r is itself representable in T. That holds for wide-exponent hosts (bfloat16,
// float, double -> measured 0 ulp^2). It does NOT hold for a narrow-exponent,
// high-precision host such as cfloat<24,5> (es=5, p=19): the residual (~ulp of
// the product) underflows the format's subnormal range and loses bits (~32
// ulp^2). That is an inherent representability limit of the result type, not an
// EFT-algorithm defect -- no Dekker/FMA/wider-intermediate variant can recover
// it, since T simply cannot hold the value. This path computes the closest
// achievable residual.
//
// Why a wider intermediate rather than a fused fma? Not every odd-p host has fma
// (bfloat16 does not), so the double intermediate is the uniform path. For hosts
// that do -- cfloat<> has a correctly-rounded fused fma() (verified: it computes
// x*y+z in extended precision with a single rounding) -- fma(a,b,-p) yields the
// IDENTICAL correctly-rounded residual, bounded by the same representability
// floor, so the double intermediate loses nothing. A fused fma would only win
// for a host with 2p > 53, where double cannot hold the exact product (none in
// elreal blocks today).
template <typename T>
UNIVERSAL_ELREAL_EFT_NOINLINE
inline void two_prod_host(T a, T b, T& p, T& r) {
    p = a * b;
    if constexpr ((std::numeric_limits<T>::digits & 1) == 0) {
        T a_hi, a_lo, b_hi, b_lo;
        split_host(a, a_hi, a_lo);
        split_host(b, b_hi, b_lo);
        r = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
    }
    else {
        const double wp = static_cast<double>(a) * static_cast<double>(b);
        r = static_cast<T>(wp - static_cast<double>(p));
    }
}

// Specialisation for double: reuse the existing two_prod (uses FMA when
// available, falls back to Veltkamp/Dekker otherwise).
template <>
inline void two_prod_host<double>(double a, double b, double& p, double& r) {
    p = sw::universal::two_prod(a, b, r);
}

// two_div: compute q = a/b and the correction `(a - q*b) / b` such that
// q + correction approximates a/b. Uses two_prod to compute q*b exactly.
template <typename T>
UNIVERSAL_ELREAL_EFT_NOINLINE
inline void two_div_host(T a, T b, T& q, T& r) {
    q = a / b;
    T p, p_err;
    two_prod_host(q, b, p, p_err);
    // Sterbenz: (a - p) is exact when a and p are within a factor of 2.
    // p_err is the residual from two_prod, so we subtract it to recover the
    // exact remainder of q*b, then divide back by b for the correction term.
    r = ((a - p) - p_err) / b;
}

} // namespace detail

// =============================================================================
// Block-level EFTs
// =============================================================================

// block_two_sum(a, b): exact decomposition of a + b into (high, low).
//
// Precondition: a.exp == b.exp. The result blocks share the same
// exp. Cross-exp addition is handled at a higher level
// (Phase 4 threeAdd).
template <typename FpType>
inline std::pair<block<FpType>, block<FpType>>
block_two_sum(const block<FpType>& a, const block<FpType>& b) {
    assert(a.exp == b.exp
           && "block_two_sum precondition: inputs must share exp");
    FpType s, r;
    detail::two_sum_host(a.v, b.v, s, r);
    return { block<FpType>{s, a.exp}, block<FpType>{r, a.exp} };
}

// block_two_sum_rn(a, b): rounded sum only (no residual computed).
template <typename FpType>
inline block<FpType>
block_two_sum_rn(const block<FpType>& a, const block<FpType>& b) {
    assert(a.exp == b.exp
           && "block_two_sum_rn precondition: inputs must share exp");
    return block<FpType>{ a.v + b.v, a.exp };
}

// block_two_mult(a, b): exact decomposition of a * b into (high, low).
//
// No exp precondition; the result blocks have exp =
// a.exp + b.exp. The host product `a.v * b.v` must not
// overflow or underflow the host range -- caller's responsibility.
template <typename FpType>
inline std::pair<block<FpType>, block<FpType>>
block_two_mult(const block<FpType>& a, const block<FpType>& b) {
    FpType p, r;
    detail::two_prod_host(a.v, b.v, p, r);
    std::int32_t out_exp = a.exp + b.exp;
    return { block<FpType>{p, out_exp}, block<FpType>{r, out_exp} };
}

// block_two_mult_rn(a, b): rounded product only.
template <typename FpType>
inline block<FpType>
block_two_mult_rn(const block<FpType>& a, const block<FpType>& b) {
    std::int32_t out_exp = a.exp + b.exp;
    return block<FpType>{ a.v * b.v, out_exp };
}

// block_two_div(a, b): 2-block expansion of a/b. q + correction ~= a/b, with
// the correction's magnitude bounded by ulp(q)/2 (so they are normally 0-overlap
// for typical inputs). Caller must guarantee b is non-zero.
template <typename FpType>
inline std::pair<block<FpType>, block<FpType>>
block_two_div(const block<FpType>& a, const block<FpType>& b) {
    assert(!b.is_zero_block() && "block_two_div: divisor is zero");
    FpType q, r;
    detail::two_div_host(a.v, b.v, q, r);
    std::int32_t out_exp = a.exp - b.exp;
    return { block<FpType>{q, out_exp}, block<FpType>{r, out_exp} };
}

// block_two_div_rn(a, b): rounded quotient only.
template <typename FpType>
inline block<FpType>
block_two_div_rn(const block<FpType>& a, const block<FpType>& b) {
    assert(!b.is_zero_block() && "block_two_div_rn: divisor is zero");
    std::int32_t out_exp = a.exp - b.exp;
    return block<FpType>{ a.v / b.v, out_exp };
}

}} // namespace sw::universal
