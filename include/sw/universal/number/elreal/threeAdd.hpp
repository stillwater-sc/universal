// threeAdd.hpp: McCleeary LFPERA threeAdd helper + lazy ZBCL addition.
//
// Phase 4 (#928) of the McCleeary reimplementation (epic #923).
//
// threeAdd<FpType>(a, b, w) takes three blocks (one from each input ZBCL plus
// a workspace block) and produces three output blocks (out1, out2, out3) with:
//
//   - Value preservation: out1 + out2 + out3 = a + b + w  (exactly)
//   - Gap preservation:   zero_overlap(out1, out2)
//                         zero_overlap(out2, out3)
//   - Magnitude ordering: |out1| >= |out2| >= |out3|
//
// Algorithm (Priest-style 3-element addition + renormalisation):
//
//   (s,  e1) = block_two_sum(a, b)         // a + b = s + e1 (exact)
//   (t,  e2) = block_two_sum(s, w)         // s + w = t + e2 (exact)
//   (m0, l)  = block_two_sum(e1, e2)       // e1 + e2 = m0 + l (exact)
//   (o1, m1) = block_two_sum(t, m0)        // t + m0 = o1 + m1 (exact)
//   (o2, o3) = block_two_sum(m1, l)        // m1 + l = o2 + o3 (exact)
//
// Validity sketch:
//   - Total value: o1 + o2 + o3 = (o1 + m1) + (-m1 + (o2 + o3))
//                              = t + m0 + (m1 + l) - m1
//                              = t + (m0 + l)
//                              = t + e1 + e2
//                              = (s + w) + e1
//                              = (a + b) + w   QED
//   - Gap (o1, o2): block_two_sum(t, m0) guarantees |m1| <= ulp(o1) / 2, so
//     E(m1) <= E(o1) - k - 1.  block_two_sum(m1, l) gives E(o2) <= E(m1) so
//     E(o2) <= E(o1) - k - 1 < E(o1) - k, i.e. zero_overlap(o1, o2).
//   - Gap (o2, o3): direct from block_two_sum's contract.
//
// Note: this is approximately McCleeary's threeAdd, not bit-identical. The
// dissertation's variant produces blocks satisfying additional exponent
// bounds and a no-shrink guarantee that aren't enforced here. The simpler
// renormalisation suffices for value preservation + 0-overlap, which is what
// the Phase 4 acceptance test exercises.
//
// Precondition: all three inputs share the same exp_offset. Cross-exp_offset
// addition is a Phase 5+ concern (the threeAdd contract above assumes blocks
// in the same exponent range).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_eft.hpp>
#include <universal/number/elreal/zbcl.hpp>

namespace sw { namespace universal {

template <typename FpType>
struct threeAdd_result {
    block<FpType> out1;
    block<FpType> out2;
    block<FpType> out3;
};

template <typename FpType>
inline threeAdd_result<FpType>
threeAdd(const block<FpType>& a,
         const block<FpType>& b,
         const block<FpType>& w) {
    assert(a.exp_offset == b.exp_offset
        && b.exp_offset == w.exp_offset
        && "threeAdd precondition: all inputs must share exp_offset");

    auto [s,  e1] = block_two_sum(a, b);
    auto [t,  e2] = block_two_sum(s, w);
    auto [m0, l]  = block_two_sum(e1, e2);
    auto [o1, m1] = block_two_sum(t, m0);
    auto [o2, o3] = block_two_sum(m1, l);

    return { o1, o2, o3 };
}

// add(x, y) -- lazy ZBCL-level addition combinator.
//
// At each step the combinator:
//   1. Pulls one block from x (or a zero block if x is exhausted),
//   2. Pulls one block from y (or zero if exhausted),
//   3. Combines with a workspace block via threeAdd,
//   4. Emits the highest-magnitude output as the next block of the stream,
//   5. Folds the two smaller outputs back into the workspace via twoSum.
//
// Step 5 collapses 2 blocks into 1 by EFT, which preserves the leading
// significant bits but drops the residual. For an infinite-precision result,
// the dropped bits must reappear via further refinement -- Phase 5 (infinite
// summation) tackles that. For Phase 4 the contract is:
//   - The leading O(k) bits of each emitted block match the true sum (a + b).
//   - 0-overlap holds across every emitted prefix.
//   - The combinator is lazy: thunks fire only when forced.
//
// Workspace exp_offset: inherits from the first non-empty input's head; zero
// inputs imply a zero-block workspace.
template <typename FpType>
inline ZBCL<FpType> add(ZBCL<FpType> x, ZBCL<FpType> y) {
    using B = block<FpType>;
    using Z = ZBCL<FpType>;

    struct State {
        Z cur_x;
        Z cur_y;
        B  w;
        bool w_initialised;
    };

    auto state = std::make_shared<State>(State{std::move(x), std::move(y), B{}, false});

    // step(): produce the next block of the result, advancing the state.
    // Returns empty optional when both inputs and workspace are exhausted.
    auto step = [state]() -> std::optional<B> {
        bool x_empty = state->cur_x.is_empty();
        bool y_empty = state->cur_y.is_empty();
        bool w_zero  = !state->w_initialised || state->w.is_zero_block();
        if (x_empty && y_empty && w_zero) return std::nullopt;

        // Determine the shared exp_offset for this step. All blocks at this
        // step share the same offset (Phase 4 precondition). Use the offset
        // of whichever non-empty source provides it.
        std::int32_t off = 0;
        if (!x_empty)            off = state->cur_x.head().exp_offset;
        else if (!y_empty)       off = state->cur_y.head().exp_offset;
        else if (state->w_initialised) off = state->w.exp_offset;

        B a = x_empty ? B{FpType{0}, off} : state->cur_x.head();
        B b = y_empty ? B{FpType{0}, off} : state->cur_y.head();
        B w = state->w_initialised ? state->w : B{FpType{0}, off};

        auto [o1, o2, o3] = threeAdd(a, b, w);

        // Advance input cursors
        if (!x_empty) state->cur_x = state->cur_x.tail();
        if (!y_empty) state->cur_y = state->cur_y.tail();

        // Fold (o2, o3) into the new workspace via twoSum (lossy: drops o3's
        // residual contribution beyond the leading bits of o2 + o3).
        auto [new_w, _drop] = block_two_sum(o2, o3);
        state->w = new_w;
        state->w_initialised = true;

        return o1;
    };

    // Build the lazy stream. We need a recursive thunk: each "next block"
    // call produces a ZBCL whose tail is again "next block". Hold the
    // recursion via a shared_ptr<std::function> so the lambda can capture
    // and call itself across activations.
    auto loop = std::make_shared<std::function<Z()>>();
    *loop = [state, step, loop]() -> Z {
        auto next = step();
        if (!next) return Z{};
        return Z::cons(*next, [loop]() { return (*loop)(); });
    };

    // Kick: produce the first block (or return empty if the inputs are empty).
    auto first = step();
    if (!first) return Z{};
    return Z::cons(*first, [loop]() { return (*loop)(); });
}

}} // namespace sw::universal
