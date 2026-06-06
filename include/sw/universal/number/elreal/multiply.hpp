// multiply.hpp: McCleeary LFPERA multiplication on ZBCL<FpType> (dissertation 4.2.5).
//
// DEPRECATED SCAFFOLDING -- slated for removal. LFPERA is ONLINE (lazy) by
// definition; the dissertation has no eager/depth/budget mode. This eager,
// finite-prefix mul(...,depth) is a Phase-6 placeholder ("a fully streaming
// product is deferred", per the note below). It is SUPERSEDED by the streaming
// mul_online() (online_multiply.hpp), which is the canonical realization.
// Removal is blocked only on migrating the math suite off mul(...,depth); see
// docs/design/elreal-online-convergence.md and #1061. Do NOT add new callers.
// (mul_scalar -- a single block times a stream -- is retained; it is the lazy
// scalar-multiply the streaming code also uses.)
//
// x * y = sum_{i,j} (x_i * y_j), where each block-pair product x_i * y_j is the
// exact 2-block decomposition from block_two_mult. We gather all partial-product
// blocks and renormalise them into the unique 0-overlap stream with priestRenorm
// (exactly what sum() does internally with these blocks as terms).
//
// Phase 6 model: FINITE-PREFIX multiplication. We materialise the first `depth`
// blocks of each operand and multiply them out. For finite operands (the common
// case: reals with a few blocks) take(depth) returns the whole operand and the
// result is the EXACT product. For infinite operands (general exact reals) this
// truncates both factors at `depth` blocks; a fully streaming anti-diagonal
// product is deferred to the Phase 7 math suite.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstddef>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_eft.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/threeAdd.hpp>   // priestRenorm
#include <universal/number/elreal/sum.hpp>         // zbcl_from_blocks

namespace sw { namespace universal {

// mul_scalar(s, y): single block s times a ZBCL y. Each y_j contributes the
// exact 2-block product block_two_mult(s, y_j); the pool is renormalised to a
// 0-overlap stream. Used by div() (a quotient block times the divisor); also a
// useful primitive on its own. Zero s or empty y -> empty (value 0).
template <typename FpType>
inline ZBCL<FpType> mul_scalar(const block<FpType>& s, ZBCL<FpType> y,
                               std::size_t depth = 256) {
    using B = block<FpType>;
    if (s.is_zero_block() || y.is_empty()) return ZBCL<FpType>{};
    std::vector<B> pool;
    for (const auto& yj : y.take(depth)) {
        if (!yj.is_normalised()) continue;
        auto pr = block_two_mult(s, yj);
        // Keep only normalised product blocks: drop zeros and sub-floor
        // denormals (denormals would break 0-overlap accounting; their value is
        // below FpType's reliable precision).
        if (pr.first.is_normalised())  pool.push_back(pr.first);
        if (pr.second.is_normalised()) pool.push_back(pr.second);
    }
    return zbcl_from_blocks<FpType>(priestRenorm(pool));
}

// mul(x, y, depth): finite-prefix product (see header note). Exact for finite
// operands; truncates each operand to `depth` blocks otherwise.
template <typename FpType>
inline ZBCL<FpType> mul(ZBCL<FpType> x, ZBCL<FpType> y, std::size_t depth = 64) {
    using B = block<FpType>;
    if (x.is_empty() || y.is_empty()) return ZBCL<FpType>{};   // 0 * y = x * 0 = 0
    auto xs = x.take(depth);
    auto ys = y.take(depth);
    std::vector<B> pool;
    pool.reserve(2 * xs.size() * ys.size());
    for (const auto& xi : xs) {
        if (!xi.is_normalised()) continue;
        for (const auto& yj : ys) {
            if (!yj.is_normalised()) continue;
            auto pr = block_two_mult(xi, yj);
            if (pr.first.is_normalised())  pool.push_back(pr.first);
            if (pr.second.is_normalised()) pool.push_back(pr.second);
        }
    }
    return zbcl_from_blocks<FpType>(priestRenorm(pool));
}

}} // namespace sw::universal
