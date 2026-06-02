// arithmetic_oracle.hpp: shared exact-dyadic oracle helpers for the Phase 6
// (#930) negate/mul/div tests. Template helpers (implicitly inline) so including
// this header in several test TUs is ODR-safe. Mirrors the helpers in
// addition.cpp / summation_oracle.hpp (consolidation tracked by #1035).
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

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace sw { namespace universal { namespace elreal_arith_test {

// Exact value of a binary float as a dyadic, lossless at any significand width.
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
        dyadic::bigint M(1); M <<= fbits; M = M + F;
        return dyadic(v.sign() ? -M : M, v.scale() - fbits);
    } else {
        static_assert(has_universal_fp_api_v<T>, "exact_real: unsupported wide host");
        return dyadic();
    }
}

template <typename FpType>
inline dyadic exact_block(const block<FpType>& b) {
    if (b.is_zero_block()) return dyadic();
    dyadic d = exact_real(b.v);
    d.scale += b.exp;
    return d;
}

template <typename FpType>
inline dyadic exact_value(const ZBCL<FpType>& z) {
    dyadic acc;
    for (const auto& blk : z.take(64)) acc = acc + exact_block(blk);
    return acc;
}

// Approximate value as double (for tolerance-based checks, e.g. division).
template <typename FpType>
inline double approx(const ZBCL<FpType>& z) {
    double s = 0.0;
    for (const auto& b : z.take(64)) s += b.template value_as<double>();
    return s;
}

// 0-overlap over the first n blocks. Returns failure count.
template <typename FpType>
inline int check_zero_overlap(const ZBCL<FpType>& z, std::size_t n, const std::string& tag) {
    auto b = z.take(n);
    int fails = 0;
    for (std::size_t i = 0; i + 1 < b.size(); ++i) {
        if (!zero_overlap(b[i], b[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at block " << i << '\n';
            ++fails;
        }
    }
    return fails;
}

}}} // namespace sw::universal::elreal_arith_test
