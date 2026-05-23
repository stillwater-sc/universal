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
// IEEE-754 round-to-nearest-compliant FpType. Universal wrapper types satisfy
// this because their arithmetic ops are explicit functions the compiler can't
// reorder through; native types need volatile (see double specialisation).
template <typename T>
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

// Generic Veltkamp split.
template <typename T>
inline void split_host(T a, T& hi, T& lo) {
    const T S = eft_splitter<T>();
    T temp = S * a;
    hi   = temp - (temp - a);
    lo   = a - hi;
}

// Generic Dekker two_prod via Veltkamp split.
template <typename T>
inline void two_prod_host(T a, T b, T& p, T& r) {
    p = a * b;
    T a_hi, a_lo, b_hi, b_lo;
    split_host(a, a_hi, a_lo);
    split_host(b, b_hi, b_lo);
    r = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
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
// Precondition: a.exp_offset == b.exp_offset. The result blocks share the same
// exp_offset. Cross-exp_offset addition is handled at a higher level
// (Phase 4 threeAdd).
template <typename FpType>
inline std::pair<block<FpType>, block<FpType>>
block_two_sum(const block<FpType>& a, const block<FpType>& b) {
    assert(a.exp_offset == b.exp_offset
           && "block_two_sum precondition: inputs must share exp_offset");
    FpType s, r;
    detail::two_sum_host(a.v, b.v, s, r);
    return { block<FpType>{s, a.exp_offset}, block<FpType>{r, a.exp_offset} };
}

// block_two_sum_rn(a, b): rounded sum only (no residual computed).
template <typename FpType>
inline block<FpType>
block_two_sum_rn(const block<FpType>& a, const block<FpType>& b) {
    assert(a.exp_offset == b.exp_offset
           && "block_two_sum_rn precondition: inputs must share exp_offset");
    return block<FpType>{ a.v + b.v, a.exp_offset };
}

// block_two_mult(a, b): exact decomposition of a * b into (high, low).
//
// No exp_offset precondition; the result blocks have exp_offset =
// a.exp_offset + b.exp_offset. The host product `a.v * b.v` must not
// overflow or underflow the host range -- caller's responsibility.
template <typename FpType>
inline std::pair<block<FpType>, block<FpType>>
block_two_mult(const block<FpType>& a, const block<FpType>& b) {
    FpType p, r;
    detail::two_prod_host(a.v, b.v, p, r);
    std::int32_t out_off = a.exp_offset + b.exp_offset;
    return { block<FpType>{p, out_off}, block<FpType>{r, out_off} };
}

// block_two_mult_rn(a, b): rounded product only.
template <typename FpType>
inline block<FpType>
block_two_mult_rn(const block<FpType>& a, const block<FpType>& b) {
    std::int32_t out_off = a.exp_offset + b.exp_offset;
    return block<FpType>{ a.v * b.v, out_off };
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
    std::int32_t out_off = a.exp_offset - b.exp_offset;
    return { block<FpType>{q, out_off}, block<FpType>{r, out_off} };
}

// block_two_div_rn(a, b): rounded quotient only.
template <typename FpType>
inline block<FpType>
block_two_div_rn(const block<FpType>& a, const block<FpType>& b) {
    assert(!b.is_zero_block() && "block_two_div_rn: divisor is zero");
    std::int32_t out_off = a.exp_offset - b.exp_offset;
    return block<FpType>{ a.v / b.v, out_off };
}

}} // namespace sw::universal
