// sum.hpp: McCleeary LFPERA infinite summation (dissertation 4.2.3).
//
// Phase 5 (#929). Given a series<FpType> (a co-list of ZBCL<FpType> terms whose
// partial sums form a Cauchy sequence), produce a single ZBCL<FpType> equal to
// their sum. This is the workhorse for Taylor series and transcendentals in
// Phase 7.
//
// Algorithm
// ---------
// The dissertation defines summation as the right fold of the Phase 4 add()
// combinator:
//
//     sum [x0, x1, x2, ...] = add x0 (add x1 (add x2 (... empty)))
//
// Because the ZBCL value of a finite block multiset is exactly the (two-sum
// exact) total of its blocks, this fold equals the canonical 0-overlap
// renormalisation of all the terms' blocks. We realise it that way over a
// depth-bounded prefix of the series:
//   1. Lazily collect up to `max_depth` terms (the budget), flattening each
//      term's blocks into one pool.
//   2. Guard against divergence (see "Convergence guard" below).
//   3. priestRenorm() the pool into the unique 0-overlap (DBL_k) block list and
//      wrap it as a ZBCL.
//
// priestRenorm is the same exact, value-preserving renormalisation add() uses
// internally; computing the sum this way yields a result that is bit-exact on
// the represented terms AND satisfies the 0-overlap invariant. (A naive lazy
// foldr over add() builds a deep lazy tower that, when forced, can emit a
// value-correct but non-0-overlap stream -- add()'s lazy composition does not
// preserve the invariant across many nested calls. Evaluating eagerly here
// avoids that; a future fully-lazy summation would need a Phase 4 add() that
// composes lazily without breaking 0-overlap.)
//
// This makes sum() eager over the budgeted prefix. For the depth budgets used
// by convergent series (and by transcendentals in Phase 7) that is the intended
// behaviour: the budget bounds the work and the result is exact for the terms
// folded.
//
// Convergence guard
// -----------------
// The dissertation defines summation only for Cauchy series and trusts the
// caller for non-trivial (e.g. transcendental) ones. We add a heuristic guard
// rail (full Cauchy-stability is undecidable in general): if the budget is
// reached *before* the series terminates AND the term magnitudes are not
// decaying, we throw elreal_sum_budget_exceeded. "Decaying" is judged over
// windows rather than endpoints, so it rejects constant, growing, AND
// oscillating non-decaying series (e.g. leading exponents 0,-1,0,-1,...): the
// largest leading exponent over the most recent window of terms must be strictly
// below the largest over the first window. A convergent-but-slow series (term
// magnitudes decreasing) is truncated at the budget and its partial sum returned
// -- trusting the caller, per the dissertation.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/threeAdd.hpp>
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/exceptions.hpp>

namespace sw { namespace universal {

// Build a ZBCL from a finite, already-0-overlap block list, dropping trailing
// zero blocks (which do not change the value).
template <typename FpType>
inline ZBCL<FpType> zbcl_from_blocks(std::vector<block<FpType>> blocks) {
    while (!blocks.empty() && blocks.back().is_zero_block()) blocks.pop_back();
    ZBCL<FpType> out{};
    for (std::size_t i = blocks.size(); i-- > 0;) {
        out = ZBCL<FpType>::cons(blocks[i], out);
    }
    return out;
}

// sum(series, max_depth): infinite summation per dissertation 4.2.3.
//
//   max_depth -- maximum number of series terms to absorb. A finite series that
//                terminates before the budget is summed exactly. An infinite
//                series is truncated at the budget (if convergent) or rejected
//                (if not Cauchy-stable; see the convergence guard above).
//
// Throws elreal_sum_budget_exceeded when max_depth == 0, or when the budget is
// reached on a non-terminating, non-converging series.
template <typename FpType>
inline ZBCL<FpType> sum(series<FpType> s, std::size_t max_depth = 1024) {
    using B = block<FpType>;

    if (max_depth == 0) {
        throw elreal_sum_budget_exceeded("sum: max_depth must be >= 1");
    }

    // Per-term block cap: terms are expected to be finite ZBCLs (Phase 5 uses
    // singletons). The cap bounds the flatten of any (pathological) infinite
    // term so the pool stays finite.
    constexpr std::size_t per_term_cap = 256;

    std::vector<B> components;

    // Per-term leading-block exponent, used to assess convergence. An empty ZBCL
    // term represents 0 and gets a -infinity sentinel so it never inflates a
    // window maximum.
    std::vector<int> leads;
    constexpr int neg_inf = std::numeric_limits<int>::min();
    bool sawNonzero = false;

    series<FpType> cur = s;
    bool naturalEnd = false;
    while (true) {
        if (cur.is_empty()) { naturalEnd = true; break; }
        if (leads.size() == max_depth) break;      // budget reached
        ZBCL<FpType> t = cur.head();
        if (t.is_empty()) {
            leads.push_back(neg_inf);
        } else {
            // eager sum operates on host-range blocks, so the combined exponent
            // fits an int (the convergence window only needs relative magnitude).
            leads.push_back(static_cast<int>(t.head().exponent()));
            sawNonzero = true;
            // flatten the term's blocks into the renormalisation pool
            ZBCL<FpType> bcur = t;
            std::size_t taken = 0;
            while (!bcur.is_empty() && taken < per_term_cap) {
                components.push_back(bcur.head());
                bcur = bcur.tail();
                ++taken;
            }
            // Refuse to silently truncate a term that exceeds the block cap --
            // dropping its suffix would return an incorrect sum.
            if (!bcur.is_empty()) {
                throw elreal_sum_budget_exceeded(
                    "sum: a term has more than per_term_cap blocks; refusing to "
                    "silently truncate it (would corrupt the sum). Pre-renormalise "
                    "the term or raise per_term_cap.");
            }
        }
        cur = cur.tail();
    }

    // Convergence guard rail (only when we stopped at the budget, not on natural
    // end). Compare the largest leading exponent over the most recent window of
    // terms to the largest over the first window; if the tail is not strictly
    // smaller, the magnitudes are not decaying -> reject. Window-based so it
    // catches oscillating non-decaying series, not just monotone ones.
    if (!naturalEnd && sawNonzero) {
        const std::size_t n = leads.size();
        const std::size_t w = (n / 4 == 0) ? 1 : n / 4;
        int headMax = neg_inf, tailMax = neg_inf;
        for (std::size_t i = 0; i < w; ++i)     headMax = std::max(headMax, leads[i]);
        for (std::size_t i = n - w; i < n; ++i) tailMax = std::max(tailMax, leads[i]);
        if (tailMax >= headMax) {
            throw elreal_sum_budget_exceeded(
                "sum: series did not converge within max_depth terms "
                "(term magnitudes not decaying); not Cauchy-stable.");
        }
    }

    // Renormalise the exact total of all collected blocks into the unique
    // 0-overlap (DBL_k) representation, then wrap as a ZBCL.
    std::vector<B> renorm = priestRenorm(components);
    return zbcl_from_blocks<FpType>(std::move(renorm));
}

}} // namespace sw::universal
