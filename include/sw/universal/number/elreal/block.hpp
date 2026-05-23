// block.hpp: McCleeary LFPERA block<FpType> -- the unit of co-list storage.
//
// A block holds (sign, exponent, k-bit significand) packed into a host
// floating-point type T, plus an explicit int32_t `exp_offset` that lets a
// single block escape T's hardware exponent range. The effective value is
//
//     v * 2^(exp_offset * 2^E_T)
//
// where E_T = exp_field_width_v<T>. The combined exponent
//
//     E(b) = scale_of(v) + exp_offset * 2^E_T
//
// is what the ZBCL 0-overlap predicate compares.
//
// References:
//   McCleeary, R. (2019). Lazy Floating Point Exact Real Arithmetic.
//   Ph.D. dissertation, University of Iowa. Chapter 4.1.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/bfloat16/bfloat16_fwd.hpp>
#include <universal/number/elreal/exp_field_width.hpp>

namespace sw { namespace universal {

// Detect a Universal type that provides .sign() and .scale() member accessors.
// All Universal floating-point wrappers (cfloat, bfloat16, half, posit, ...)
// expose these; native float/double/long double do not.
template <typename T, typename = void>
struct has_universal_fp_api : std::false_type {};

template <typename T>
struct has_universal_fp_api<
    T,
    std::void_t<
        decltype(std::declval<const T&>().sign()),
        decltype(std::declval<const T&>().scale())
    >
> : std::true_type {};

template <typename T>
inline constexpr bool has_universal_fp_api_v = has_universal_fp_api<T>::value;

// block<FpType>: McCleeary's (sign, exp, k-bit significand) packed into FpType,
// plus an int32_t escape multiplier `exp_offset`.
//
// Trivial layout: no in-class member initialisers. Use {value, offset} literal
// construction to initialise. Default construction leaves members indeterminate
// (consistent with Universal's other trivial number types).
template <typename FpType>
struct block {
    using fp_type = FpType;

    // k = total significand-vector width, matching the dissertation's k.
    // For IEEE-style formats this includes the hidden bit. The relationship
    // numeric_limits<T>::digits == k holds when T's numeric_limits is correctly
    // configured (a minor inconsistency exists for Universal's bfloat16 which
    // reports digits=7 -- the McCleeary k for that format would be 8).
    static constexpr int k = std::numeric_limits<FpType>::digits;

    // E = exponent-field width of FpType; exp_step = 2^E.
    static constexpr int          E         = exp_field_width_v<FpType>;
    static constexpr std::int64_t exp_step  = exp_step_v<FpType>;

    FpType        v;
    std::int32_t  exp_offset;

    // sign: -1 for negative, +1 for non-negative. Sign of +0.0 and -0.0 follow
    // signbit / Universal's .sign() respectively.
    constexpr int sign() const noexcept {
        if constexpr (has_universal_fp_api_v<FpType>) {
            return v.sign() ? -1 : 1;
        } else {
            return std::signbit(v) ? -1 : 1;
        }
    }

    // scale_of(v): unbiased binary exponent of v (such that |v| in [2^e, 2^(e+1))).
    // For Universal types this is .scale(); for native types it is ilogb.
    constexpr int scale_of_v() const noexcept {
        if constexpr (has_universal_fp_api_v<FpType>) {
            return v.scale();
        } else {
            if (v == FpType{0}) return 0;
            return std::ilogb(v);
        }
    }

    // Combined exponent E(b) = scale(v) + exp_offset * 2^E.
    // Returned as int64 so exp_offset levering does not overflow int.
    constexpr std::int64_t exponent() const noexcept {
        return static_cast<std::int64_t>(scale_of_v())
             + static_cast<std::int64_t>(exp_offset) * exp_step;
    }

    constexpr bool is_zero_block() const noexcept {
        return v == FpType{0};
    }

    // is_normalised(): for IEEE-style FpTypes the leading significand bit must be
    // set; equivalently, |v| in [1, 2). McCleeary blocks must be normalised so
    // 0-overlap accounting holds.
    constexpr bool is_normalised() const noexcept {
        if (is_zero_block()) return false;
        // For all supported FpType (native and Universal), ilogb / scale returns
        // a valid integer for normalised values. Subnormals are NOT considered
        // normalised here.
        if constexpr (has_universal_fp_api_v<FpType>) {
            // Universal cfloat / bfloat16: scale() works for both normal and
            // subnormal values; check the value range explicitly.
            FpType abs_v = v.sign() ? -v : v;
            return abs_v >= FpType{1} || (v.scale() >= std::numeric_limits<FpType>::min_exponent - 1);
        } else {
            int cls = std::fpclassify(v);
            return cls == FP_NORMAL;
        }
    }

    // value_as<T>(): for testing only. Combines v and exp_offset into a T value.
    // Requires the combined exponent to fit in T's range; otherwise the result
    // is +/-inf or zero depending on rounding.
    template <typename T = double>
    constexpr T value_as() const noexcept {
        static_assert(std::is_floating_point_v<T>,
                      "value_as<T>() requires a native floating-point T");
        T base = static_cast<T>(v);
        if (exp_offset == 0) return base;
        return std::ldexp(base, static_cast<int>(static_cast<std::int64_t>(exp_offset) * exp_step));
    }
};

// zero_overlap(b1, b2): the McCleeary 0-overlap predicate. True iff
// E(b1) >= E(b2) + k, i.e. b2's most significant bit sits at least k+1 bit
// positions below b1's. The intervening `imp` bit is the gap that absorbs
// carry-ups from later refinement.
//
// Zero blocks impose no constraint: any block trivially 0-overlaps a zero
// block, and a zero block trivially 0-overlaps any block.
template <typename FpType>
constexpr bool zero_overlap(const block<FpType>& b1, const block<FpType>& b2) noexcept {
    if (b1.is_zero_block() || b2.is_zero_block()) return true;
    return b1.exponent() >= b2.exponent() + static_cast<std::int64_t>(block<FpType>::k);
}

}} // namespace sw::universal
