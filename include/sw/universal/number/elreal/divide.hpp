// divide.hpp: McCleeary LFPERA division on ZBCL<FpType> (dissertation 4.2.6).
//
// Long-division-style refinement against the leading divisor block. At each step
// the next quotient block is q_i = block_two_div_rn(rem_0, y_0), where rem_0 is
// the leading remainder block and y_0 the leading divisor block. We then reduce
// the remainder by q_i * y (rem <- rem - q_i*y, via mul_scalar + negate + add)
// and repeat. The committed quotient blocks are renormalised into the unique
// 0-overlap stream.
//
// Termination: the remainder's leading exponent strictly decreases each step
// (by ~k bits), so for exact divisors the remainder reaches 0; for general
// (irrational) quotients the `depth` budget bounds the number of quotient blocks
// emitted. A no-progress guard stops early if the remainder fails to shrink.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstddef>
#include <limits>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_eft.hpp>   // block_two_div_rn, block_two_mult
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/threeAdd.hpp>     // priestRenorm
#include <universal/number/elreal/sum.hpp>          // zbcl_from_blocks
#include <universal/number/elreal/exceptions.hpp>

namespace sw { namespace universal {

// div(x, y, depth): x / y as a 0-overlap ZBCL, refined to at most `depth`
// quotient blocks. Empty divisor (zero) -> elreal_divide_by_zero if
// ELREAL_THROW_ARITHMETIC_EXCEPTION, else the empty co-list. 0 / y = 0.
template <typename FpType>
inline ZBCL<FpType> div(ZBCL<FpType> x, ZBCL<FpType> y, std::size_t depth = 64) {
    using B = block<FpType>;

    if (y.is_empty()) {
#if ELREAL_THROW_ARITHMETIC_EXCEPTION
        throw elreal_divide_by_zero("elreal div: divisor is the empty co-list (zero)");
#else
        return ZBCL<FpType>{};
#endif
    }
    if (x.is_empty()) return ZBCL<FpType>{};   // 0 / y = 0

    const B y0 = y.head();                       // leading divisor block (non-zero)
    const std::vector<B> yb = y.take(depth);     // materialise the divisor blocks

    // Refinement floor on the leading remainder's combined exponent.
    //
    // The block EFTs (block_two_mult / block_two_div) are scale-invariant: they
    // operate on the normalised significand v (in [1,2), always a normal host
    // value) and track the scale symbolically in the int32 exp, so a block at an
    // arbitrarily negative combined exponent is still a normal host double -- the
    // remainder reductions never denormalise. A WIDE host (double/float, k>=24)
    // therefore has the headroom to refine all the way to its natural ~19-component
    // ceiling (~308 decimal digits for double), so the floor is lifted (#1052);
    // the loop still terminates on an exact remainder, no-progress, or a genuinely
    // non-normalised leading block.
    //
    // A NARROW host (bfloat16 k=8, fp16 k=11) has min_exponent ~ -125 and genuinely
    // denormalises a couple of block-widths above it -- there the remainder
    // reductions stop staying normalised and 0-overlap accounting breaks -- so it
    // keeps the floor (this is the same denormal floor #1044 respects).
    constexpr int k = block<FpType>::k;
    constexpr int exp_floor = (k >= 24)
        ? (std::numeric_limits<int>::min() / 2)                       // wide host: no floor
        : (std::numeric_limits<FpType>::min_exponent + 2 * k);        // narrow host: denormal floor

    // Keep only normalised blocks (drop zeros and sub-floor denormals). On a
    // priestRenorm'd (descending, 0-overlap) list this only widens gaps, so the
    // 0-overlap property is preserved.
    auto keep_normalised = [](std::vector<B> v) {
        std::vector<B> out;
        out.reserve(v.size());
        for (const auto& b : v) if (b.is_normalised()) out.push_back(b);
        return out;
    };

    // The remainder is carried as an eager, renormalised block list (long
    // division on expansions) -- never as a lazy ZBCL -- so no lazy tower of
    // add()s forms and no near-floor denormal is ever forced.
    std::vector<B> rem = keep_normalised(x.take(depth));
    std::vector<B> qblocks;
    qblocks.reserve(depth);
    bool havePrev = false;
    int  prevLead = 0;
    for (std::size_t step = 0; step < depth; ++step) {
        if (rem.empty()) break;                  // exact division: remainder is 0
        const B r0 = rem.front();                // leading block (descending order)
        if (!r0.is_normalised() || r0.exponent() < exp_floor) break;
        const int lead = static_cast<int>(r0.exponent());
        if (havePrev && lead >= prevLead) break; // no progress -> stop refining
        prevLead = lead;
        havePrev = true;

        const B q = block_two_div_rn(r0, y0);    // next quotient block
        if (q.is_zero_block()) break;
        qblocks.push_back(q);

        // rem <- rem - q*y : append the (negated) exact q*y product blocks and
        // renormalise the whole pool back to a 0-overlap list.
        std::vector<B> pool = rem;
        for (const auto& yj : yb) {
            if (!yj.is_normalised()) continue;
            auto pr = block_two_mult(q, yj);
            if (pr.first.is_normalised())  pool.push_back(B{ -pr.first.v,  pr.first.exp });
            if (pr.second.is_normalised()) pool.push_back(B{ -pr.second.v, pr.second.exp });
        }
        rem = keep_normalised(priestRenorm(pool));
    }

    return zbcl_from_blocks<FpType>(priestRenorm(qblocks));
}

}} // namespace sw::universal
