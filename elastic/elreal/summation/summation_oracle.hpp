// summation_oracle.hpp: shared exact-dyadic oracle helpers for the Phase 5
// (#929) summation tests.
//
// These mirror the exact_real / exact_block / exact_value helpers used by the
// Phase 4 addition tests and the #1022 oracle (exact_value_oracle.cpp). They are
// templates (implicitly inline), so including this header in several test TUs is
// ODR-safe. Kept local to elastic/elreal/summation/ -- not part of the library
// include tree.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace sw { namespace universal { namespace elreal_sum_test {

// Exact value of a binary float v as a dyadic, with no precision loss at any
// significand width (identical to the #1022 oracle's exact_real).
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
            "exact_real: wide native hosts not supported yet (add std::frexp path).");
        return dyadic();
    }
}

// Exact value of a block as a dyadic rational: value(b) = v * 2^exp.
template <typename FpType>
inline dyadic exact_block(const block<FpType>& b) {
    if (b.is_zero_block()) return dyadic();
    dyadic d = exact_real(b.v);
    d.scale += b.exp;
    return d;
}

// Exact value of a ZBCL prefix (32-block window, matching the #1022 oracle).
template <typename FpType>
inline dyadic exact_value(const ZBCL<FpType>& z) {
    dyadic acc;
    for (const auto& blk : z.take(32)) acc = acc + exact_block(blk);
    return acc;
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

}}} // namespace sw::universal::elreal_sum_test
