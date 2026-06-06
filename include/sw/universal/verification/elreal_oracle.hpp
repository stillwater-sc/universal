// elreal_oracle.hpp: shared exact-dyadic oracle helpers for the elreal tests.
//
// Single source of truth for the bit-exact reference the elreal Phase 4-9 tests
// validate against. Built on the dyadic-rational exact arithmetic of
// dyadic_exact.hpp (the #1022 / #987 oracle work). These helpers were historically
// copy-pasted into addition.cpp, exact_value_oracle.cpp, summation_oracle.hpp, and
// arithmetic_oracle.hpp; keeping four copies of the subtle digits<=53 vs wide-cfloat
// bit-path logic in sync by hand was error-prone (the windows had already drifted
// 32 vs 64). This header consolidates them (#1035).
//
// All helpers are templates (implicitly inline), so including this header in many
// test translation units is ODR-safe. Header-only and test-facing; it sits next to
// dyadic_exact.hpp and test_suite.hpp under verification/.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace sw { namespace universal { namespace elreal_oracle {

// Number of leading ZBCL blocks summed when computing an exact value. Finite sums
// of the test inputs settle well within this on a double host (a ZBCL near 1.0
// tops out at ~19-20 non-overlapping components against the 2^-1022 floor), so the
// window only caps pathological streams. 32 matches the #1022 oracle.
constexpr std::size_t ZBCL_EXACT_WINDOW = 32;

// Exact value of a binary floating-point value v as a dyadic rational, with NO
// precision loss at any significand width. Two regimes, chosen at compile time:
//
//   * p = digits <= 53: double represents v exactly, so from_double is exact and
//     cheap. Covers float(24), double(53), half(11), bfloat16(8), cfloat<24,5>(19),
//     cfloat<32,8>(24) -- every <= 53-bit host.
//
//   * p > 53 with the Universal FP API (cfloat quad and up): read the encoding
//     DIRECTLY -- sign, scale, and the fbits stored fraction bits -- and assemble
//         value = (-1)^sign * (2^fbits + F) * 2^(scale - fbits),  fbits = p - 1.
//     Reading the encoding directly keeps this oracle self-contained: it depends on
//     no cfloat math function, only on the bit layout.
template <typename T>
inline dyadic exact_real(T v) {
    if (v == T(0)) return dyadic();
    if constexpr (std::numeric_limits<T>::digits <= 53) {
        return dyadic::from_double(static_cast<double>(v));
    } else if constexpr (has_universal_fp_api_v<T>) {
        constexpr int fbits = std::numeric_limits<T>::digits - 1;
        assert(v.isnormal() && "exact_real bit path expects a normal value");
        dyadic::bigint F(0);
        for (int i = 0; i < fbits; ++i) {
            if (v.test(static_cast<unsigned>(i))) {
                dyadic::bigint bit(1); bit <<= i; F = F + bit;
            }
        }
        dyadic::bigint M(1); M <<= fbits; M = M + F;     // hidden bit + fraction
        return dyadic(v.sign() ? -M : M, v.scale() - fbits);
    } else {
        static_assert(has_universal_fp_api_v<T>,
            "exact_real: wide (>53-bit) native hosts are not supported yet; add a "
            "std::frexp-based extraction (std frexp is correct for native types) "
            "when such a host is introduced.");
        return dyadic();
    }
}

// Exact value of a single block as a dyadic rational: value(b) = v * 2^exp.
// Shares no code with the block/EFT/threeAdd/add algorithms under test.
template <typename FpType>
inline dyadic exact_block(const block<FpType>& b) {
    if (b.is_zero_block()) return dyadic();
    dyadic d = exact_real(b.v);
    // block exp is a wide integer<256>; the oracle verifies host-range test
    // values whose combined exponent fits an int. Fail loud rather than silently
    // truncate if that invariant is ever violated (it would corrupt the reference).
    using EXP = typename block<FpType>::exp_t;
    assert(b.exp <= EXP(std::numeric_limits<int>::max())
        && b.exp >= EXP(std::numeric_limits<int>::min())
        && "elreal oracle: block exponent outside host int range");
    d.scale += static_cast<int>(b.exp);   // multiply by 2^exp exactly (value = v * 2^exp)
    return d;
}

// Exact value of a list of blocks: sum_i exact_block(b_i).
template <typename FpType>
inline dyadic exact_blocks(const std::vector<block<FpType>>& bs) {
    dyadic acc;                // 0
    for (const auto& b : bs) acc = acc + exact_block(b);
    return acc;
}

// Exact value of a ZBCL prefix (ZBCL_EXACT_WINDOW-block window).
template <typename FpType>
inline dyadic exact_value(const ZBCL<FpType>& z) {
    return exact_blocks(z.take(ZBCL_EXACT_WINDOW));
}

// Exact value of a finite list of ZBCL terms: sum_i exact_value(term_i). This is
// the reference value a correct sum() must reproduce bit-for-bit on the terms it
// actually folds (the represented values, not the ideal mathematical series).
template <typename FpType>
inline dyadic exact_series_sum(const std::vector<ZBCL<FpType>>& terms) {
    dyadic acc;
    for (const auto& t : terms) acc = acc + exact_value(t);
    return acc;
}

// Approximate value as double (for tolerance-based checks, e.g. division).
template <typename FpType>
inline double approx(const ZBCL<FpType>& z) {
    double s = 0.0;
    for (const auto& b : z.take(ZBCL_EXACT_WINDOW)) s += b.template value_as<double>();
    return s;
}

// 0-overlap check over the first `n` blocks of a stream. Returns failure count.
template <typename FpType>
inline int check_zero_overlap(const ZBCL<FpType>& z, std::size_t n, const std::string& tag) {
    auto blocks = z.take(n);
    int fails = 0;
    for (std::size_t i = 0; i + 1 < blocks.size(); ++i) {
        if (!zero_overlap(blocks[i], blocks[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at block " << i << '\n';
            ++fails;
        }
    }
    return fails;
}

}}} // namespace sw::universal::elreal_oracle
