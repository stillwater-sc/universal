// sqrt.hpp: McCleeary LFPERA square root on ZBCL<FpType> (Phase 7.2, #931).
//
// Newton-Raphson on the lazy stream:  x_{n+1} = (x_n + a/x_n) / 2.
// The iteration converges quadratically (the number of correct bits doubles
// each step), so a few iterations reach the target depth. We seed from the
// host-double square root of the leading approximation (good to ~52 bits) and
// grow the working depth geometrically (1, 2, 4, ..., depth) so early, low-
// accuracy steps stay cheap.
//
// Domain: a < 0 is undefined and returns the empty co-list (0); sqrt(0) = 0.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstddef>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>   // from_native, to_double_approx
#include <universal/number/elreal/threeAdd.hpp>        // add
#include <universal/number/elreal/multiply.hpp>        // mul_scalar
#include <universal/number/elreal/divide.hpp>          // div

namespace sw { namespace universal {

// sqrt(a, depth): square root of a non-negative ZBCL, refined to ~depth blocks.
template <typename FpType>
inline ZBCL<FpType> sqrt(ZBCL<FpType> a, std::size_t depth = 64) {
    using B = block<FpType>;

    if (a.is_empty()) return ZBCL<FpType>{};            // sqrt(0) = 0
    if (a.head().sign() < 0) return ZBCL<FpType>{};     // sqrt(negative): undefined -> empty

    // Initial guess from the host-double approximation (accurate to ~52 bits).
    const double a0 = to_double_approx(a, 2);
    ZBCL<FpType> x = from_native<FpType>(std::sqrt(a0));
    if (x.is_empty()) return ZBCL<FpType>{};            // defensive (a0 underflowed)

    const B half{ static_cast<FpType>(0.5), 0 };
    const int iters = 3 + static_cast<int>(std::ceil(std::log2(static_cast<double>(depth) + 1.0)));

    std::size_t d = 1;
    for (int i = 0; i < iters; ++i) {
        d = (d * 2 < depth) ? d * 2 : depth;            // grow working depth: 2,4,...,depth
        // x <- (x + a/x) / 2
        x = mul_scalar(half, add(x, div(a, x, d)), d);
    }
    return x;
}

}} // namespace sw::universal
