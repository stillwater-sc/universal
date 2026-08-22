// infsum.hpp: McCleeary LFPERA streaming infinite summation (dissertation 4.2.3).
//
// This is the ONLINE (pull-driven) infinite sum: given a series<FpType> (a lazy
// co-list of ZBCL terms whose leading-block exponents are strictly decreasing), it
// produces a single ZBCL equal to their sum, emitting one block per tail() pull and
// pulling input terms on demand. It is a direct translation of FCL.hs Appendix A.4
// (infSum / infSumRec / addition), the streaming summation the dissertation builds
// multiplication and division on top of (4.2.5 / 4.2.6).
//
// Relationship to sum.hpp: sum() is the EAGER stand-in (it take(depth)s a budgeted
// prefix and priestRenorm's the pool). This infsum() is the streaming form the sum.hpp
// header noted as deferred ("a future fully-lazy summation"). Phase 1 of #1061 lands
// this; mul/div then become thin wrappers over it. The two must agree on finite series
// (the eager sum() is the equivalence oracle).
//
// The state machine mirrors add()'s addRec_step (threeAdd.hpp): a step function that
// emits the next block or nullopt, looping internally until it either emits or
// terminates, wrapped as cons(block, thunk). It reuses add's isSafe / createZero
// verbatim -- there is no separate summation carry-arrest.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/threeAdd.hpp>   // add, isSafe, createZero

namespace sw { namespace universal {

// addition(fs, gs): add, then drop a leading zero block if add produced one. FCL.hs
// `addition` -- used everywhere infSumRec folds a term into the accumulator so a
// cancellation that yields a leading zero does not stall the stream. (add() now keeps
// the 0-overlap canonical form under composition, #1057, so no extra renormalisation
// of the accumulator is needed.)
template <typename FpType>
inline ZBCL<FpType> addition(ZBCL<FpType> fs, ZBCL<FpType> gs) {
    ZBCL<FpType> s = add(std::move(fs), std::move(gs));
    if (s.is_empty()) return s;
    if (s.head().is_zero_block()) return s.tail();
    return s;
}

// Streaming state: the remaining input terms, the accumulator ZBCL (FCL `prevs`), and
// the emission frontier `bound`. (Compare addRec_state in threeAdd.hpp.)
template <typename FpType>
struct infsumRec_state {
    series<FpType> inputs;
    ZBCL<FpType>   prevs;
    typename block<FpType>::exp_t bound;   // wide: the division frontier outgrows int
};

// infsumRec_step: produce the next emitted block, mutating the state; nullopt at end.
// Direct translation of FCL.hs infSumRec (Appendix A.4 / dissertation 4.2.3). The
// Haskell is tail-recursive without always emitting, so this loops internally until an
// output is produced or the stream terminates.
template <typename FpType>
inline std::optional<block<FpType>>
infsumRec_step(infsumRec_state<FpType>& st) {
    using B = block<FpType>;
    constexpr int k = block<FpType>::k;

    int safety = 1000000;
    while (--safety > 0) {
        // infSumRec [] prevs _ = prevs : stream out the remaining accumulator.
        if (st.inputs.is_empty()) {
            if (st.prevs.is_empty()) return std::nullopt;
            B h = st.prevs.head();
            st.prevs = st.prevs.tail();
            return h;
        }

        ZBCL<FpType> as = st.inputs.head();
        series<FpType> rest1 = st.inputs.tail();

        // infSumRec (as:[]) prevs _ = add as prevs : fold the last term, then stream.
        if (rest1.is_empty()) {
            st.prevs  = add(as, st.prevs);
            st.inputs = series<FpType>{};
            continue;
        }

        ZBCL<FpType> bs = rest1.head();

        // infSumRec (as:rest) [] bound = infSumRec rest as bound : seed the accumulator.
        if (st.prevs.is_empty()) {
            st.prevs  = as;
            st.inputs = rest1;
            continue;
        }

        // Main case: infSumRec (as:bs:rest) (prev:prevs) bound.
        const B prev = st.prevs.head();
        // isSafe's `es` argument is the next-input list; isSafe only reads its head, so a
        // 1-block view of bs suffices (empty when bs is the zero co-list).
        const std::vector<B> es = bs.is_empty() ? std::vector<B>{} : std::vector<B>{ bs.head() };

        if (st.bound > prev.exponent() + k + 2) {
            // Cancellation region: try to emit an explicit zero at `bound`.
            const B zero = createZero<FpType>(st.bound);
            if (isSafe(zero, as, bs, es, prev)) {
                st.bound = zero.exponent() - k;
                return zero;                              // emit zero; inputs/prevs unchanged
            }
            st.prevs  = add(as, st.prevs);                // nPrevs = add as (prev:prevs)
            st.inputs = rest1;                            // (bs:rest)
            continue;
        }

        // Normal region: fold the next term into the accumulator and try to emit its head.
        // FCL.hs: let (high:highs) = add (prev:prevs) as -- PLAIN add, no zero-dropping;
        // McCleeary keeps leading zeros (they are valid ZBCL blocks handled by isSafe /
        // createZero). Earlier this used a drop-leading-zero wrapper ("addition") plus an
        // extra high.is_zero_block() skip -- both our own additions, removed to match the
        // dissertation (they broke 0-overlap for the correlated terms division generates).
        ZBCL<FpType> s = add(st.prevs, as);               // add (prev:prevs) as
        if (s.is_empty()) {
            // prevs + as == 0 exactly, so the accumulator is now zero. McCleeary's
            // pattern match (high:highs) = add ... assumes this cannot occur, but it
            // does: a term may be an all-zero ZBCL (the multiply emits one whenever an
            // operand block is zero) and add(zero, zero) is empty.
            //
            // Two things must hold here, and the original recovery had neither.
            // (1) Do NOT drop `bs`. Advancing to rest1.tail() discarded an unconsumed
            //     term, truncating any sum with an interior zero term -- a product
            //     against [1@0, Z@-100, Z@-150, Z@-200, 1@-1700] lost the 2^-1700 term
            //     and stopped at 2^-588 where the same VALUE without the zero blocks
            //     reached 2^-2088 (universal#1373). Newton's reciprocal hit this
            //     because 2 - b*r emits a growing run of zero blocks as it converges.
            // (2) Stay PRODUCTIVE. Merely consuming a term without emitting turns an
            //     infinite input whose terms keep cancelling into a non-emitting loop:
            //     the old double-drop accidentally broke that cycle, and keeping bs
            //     without emitting hangs on operands like 1/7 that never terminate.
            //     Emitting an explicit zero at the frontier is value-neutral and gives
            //     the consumer a block per pull, exactly as the cancellation region
            //     above does.
            // prevs + as == 0 exactly, so the accumulator is now zero.
            //
            // McCleeary's pattern match (high:highs) = add ... assumes this cannot
            // occur, and the branch was written as defensive dead code for "full
            // cancellation". It is NOT dead: a term may be an all-zero ZBCL -- zero
            // blocks are legitimate, and the multiply emits an all-zero term whenever
            // an operand block is zero -- and add(zero, zero) is empty.
            //
            // The old recovery advanced the input cursor to rest1.tail(), discarding
            // `bs` along with `as`. `bs` had not been consumed, so an interior zero
            // term silently DROPPED THE FOLLOWING TERM from the sum. That truncated
            // any product against an operand carrying interior zero blocks: against
            // [1@0, Z@-100, Z@-150, Z@-200, 1@-1700] the 2^-1700 term was lost and the
            // product stopped at 2^-588, where the same VALUE written without the zero
            // blocks reached 2^-2088 (universal#1373). Newton's reciprocal walked
            // straight into it, because the cancellation in 2 - b*r emits a growing run
            // of zero blocks as it converges (1, 2, 6, 14, 31 ...), so past a certain
            // depth the correction term was discarded whole and the iteration
            // fixed-pointed -- capping dense division at ~513 digits on a double host
            // and ~62 on float, at any requested depth.
            //
            // Consume only `as`, and set the accumulator to the empty (zero) ZBCL,
            // which is what prevs + as actually is here.
            st.prevs  = ZBCL<FpType>{};                   // the sum is exactly zero
            st.inputs = rest1;                            // (bs:rest) -- do NOT drop bs
            continue;
        }
        const B high = s.head();
        if (isSafe(high, bs, bs, es, prev)) {
            st.prevs  = s.tail();
            st.inputs = rest1;                            // (bs:rest)
            st.bound  = high.exponent() - k;
            return high;
        }
        st.prevs  = s;                                    // (high:highs)
        st.inputs = rest1;                                // (bs:rest)
        // bound unchanged
    }
    throw std::runtime_error(
        "sw::universal::infsumRec_step: safety counter exhausted (non-convergence bug)");
}

// infsumRec_stream(st): pull one block and defer the rest.
//
// The thunk captures ONLY the state. It used to be a std::function holding a
// shared_ptr to the control block that owned it -- the function object owned itself,
// so releasing the returned ZBCL could never bring the count to zero and every stream
// stranded its state. Measured before the fix: 41 kB per mul_online() stream, so
// 80,000 streams created and dropped took RSS from 3.5 MB to 2.3 GB
// (universal#1378).
template <typename FpType>
inline ZBCL<FpType> infsumRec_stream(std::shared_ptr<infsumRec_state<FpType>> st) {
    auto nxt = infsumRec_step(*st);
    if (!nxt) return ZBCL<FpType>{};
    block<FpType> head = *nxt;
    return ZBCL<FpType>::cons(head, [st]() { return infsumRec_stream<FpType>(st); });
}

// infsum(s): streaming sum of the series `s`. Direct translation of FCL.hs infSum.
template <typename FpType>
inline ZBCL<FpType> infsum(series<FpType> s) {
    using Z = ZBCL<FpType>;

    if (s.is_empty()) return Z{};                         // infSum [] = []
    Z as = s.head();
    series<FpType> rest1 = s.tail();
    if (rest1.is_empty()) return as;                      // infSum (as:[]) = as

    Z bs = rest1.head();
    series<FpType> rest = rest1.tail();

    // nprevs = add as bs ; bound = getExp(head as) + getSize(head as) + 1.
    Z nprevs = add(as, bs);
    const typename block<FpType>::exp_t lead = as.is_empty()
        ? (bs.is_empty() ? typename block<FpType>::exp_t(0) : bs.head().exponent())
        : as.head().exponent();
    const typename block<FpType>::exp_t bound = lead + block<FpType>::k + 1;

    auto st = std::make_shared<infsumRec_state<FpType>>(
        infsumRec_state<FpType>{ std::move(rest), std::move(nprevs), bound });

    return infsumRec_stream<FpType>(std::move(st));
}

// drop_zeros(z): lazily drop every zero block from z. The EFTs leave exact (zero-residual)
// blocks in the stream -- e.g. block_two_mult of an exactly-representable product yields a
// zero low block at the SAME exponent as the high block. Those are value-neutral and pass
// the 0-overlap check (zero blocks are special-cased), but a consumer like singleDiv/infSum
// treats them as real blocks at that exponent and the long division goes wrong (#1061). Drop
// them so mul/div sub-results are minimal.
template <typename FpType>
inline ZBCL<FpType> drop_zeros(ZBCL<FpType> z) {
    while (!z.is_empty() && z.head().is_zero_block()) z = z.tail();
    if (z.is_empty()) return z;
    block<FpType> h = z.head();
    ZBCL<FpType> rest = z.tail();
    return ZBCL<FpType>::cons(h, [rest]() { return drop_zeros(rest); });
}

// infinitesum(s): infsum, with all zero blocks dropped. FCL.hs `infiniteSum` drops only a
// leading zero; we drop every zero (see drop_zeros) so the mul/div forms built on this are
// minimal and compose correctly.
template <typename FpType>
inline ZBCL<FpType> infinitesum(series<FpType> s) {
    return drop_zeros(infsum(std::move(s)));
}

}} // namespace sw::universal
