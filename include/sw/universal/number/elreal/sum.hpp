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
// caller for non-trivial (e.g. transcendental) ones. We add a guard rail: if
// the budget is reached *before* the series terminates on its own AND the term
// magnitudes are not trending toward zero (the leading exponent of the most
// recent term is not below that of the first), the series is not Cauchy-stable
// within budget and we throw elreal_sum_budget_exceeded. A convergent-but-slow
// series (term magnitudes decreasing) is simply truncated at the budget and its
// partial sum returned -- trusting the caller, per the dissertation.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstddef>
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

    // Track the leading-block exponent of the first and most-recent NON-zero
    // terms to assess convergence. An empty ZBCL term represents 0.
    bool haveFirstLead = false;
    int  firstLead = 0;
    int  lastLead  = 0;
    bool sawNonzero = false;

    series<FpType> cur = s;
    bool naturalEnd = false;
    std::size_t nterms = 0;
    while (true) {
        if (cur.is_empty()) { naturalEnd = true; break; }
        if (nterms == max_depth) break;            // budget reached
        ZBCL<FpType> t = cur.head();
        ++nterms;
        if (!t.is_empty()) {
            int lead = t.head().exponent();
            if (!haveFirstLead) { firstLead = lead; haveFirstLead = true; }
            lastLead = lead;
            sawNonzero = true;
            // flatten the term's blocks into the renormalisation pool
            ZBCL<FpType> bcur = t;
            std::size_t taken = 0;
            while (!bcur.is_empty() && taken < per_term_cap) {
                components.push_back(bcur.head());
                bcur = bcur.tail();
                ++taken;
            }
        }
        cur = cur.tail();
    }

    // Convergence guard (only when we stopped at the budget, not on natural end).
    if (!naturalEnd && sawNonzero && lastLead >= firstLead) {
        throw elreal_sum_budget_exceeded(
            "sum: series did not converge within max_depth terms "
            "(term magnitudes not decreasing); not Cauchy-stable.");
    }

    // Renormalise the exact total of all collected blocks into the unique
    // 0-overlap (DBL_k) representation, then wrap as a ZBCL.
    std::vector<B> renorm = priestRenorm(components);
    return zbcl_from_blocks<FpType>(std::move(renorm));
}

}} // namespace sw::universal
