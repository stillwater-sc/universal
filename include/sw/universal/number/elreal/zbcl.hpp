// zbcl.hpp: McCleeary LFPERA ZBCL<FpType> -- Zero-overlap Block Co-List.
//
// A ZBCL is a (possibly empty, possibly infinite) lazy stream of block<FpType>
// satisfying the 0-overlap invariant: for any two adjacent blocks (b_n, b_{n+1})
// in the stream, `zero_overlap(b_n, b_{n+1})` holds, i.e.
//
//     E(b_n) >= E(b_{n+1}) + k
//
// where E(.) is the combined exponent (per block::exponent()).
//
// Representation
//   - Empty co-list represents the real number 0.
//   - Non-empty co-list = (head block, tail thunk). The tail thunk is a
//     std::function<ZBCL()>; it is evaluated on demand by `tail()` / `take(n)`
//     and the result is memoised on the node so a thunk is invoked at most
//     once. Nodes are shared via std::shared_ptr so ZBCL values are cheap to
//     copy.
//   - Sentinel for end-of-stream is an empty thunk (default-constructed
//     std::function). The terminal block is the last one materialised; its
//     `tail()` returns an empty ZBCL.
//
// Cauchy-sequence proof obligation (paraphrasing dissertation 4.1.4)
// ------------------------------------------------------------------
// A ZBCL <b_0, b_1, b_2, ...> represents the real number sum_{i>=0} value(b_i)
// where value(b) = sign(b) * 2^E(b) * (significand bits of b). The 0-overlap
// property ensures the partial-sum sequence S_n = sum_{i<=n} value(b_i) is
// Cauchy: |S_m - S_n| <= 2^(E(b_n) + 1 - k) for m > n. Convergence to a
// well-defined real value follows. Operations on ZBCLs must preserve both the
// value and the 0-overlap property; the proofs live with each operation as it
// is introduced (Phase 3+).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#include <universal/number/elreal/block.hpp>

namespace sw { namespace universal {

template <typename FpType>
class ZBCL {
public:
    using value_type = block<FpType>;
    using thunk_type = std::function<ZBCL<FpType>()>;

private:
    struct Node {
        value_type            head;
        thunk_type            tail_thunk;
        mutable bool          tail_evaluated;
        mutable ZBCL<FpType>  tail_value;

        Node(const value_type& h, thunk_type t) noexcept
            : head(h),
              tail_thunk(std::move(t)),
              tail_evaluated(false),
              tail_value() {}
    };

    std::shared_ptr<const Node> _node;

    explicit ZBCL(std::shared_ptr<const Node> n) noexcept : _node(std::move(n)) {}

public:
    // Default constructor: empty co-list (represents 0).
    ZBCL() noexcept : _node() {}

    // cons(head, tail): non-empty co-list. If `tail` is an empty function, the
    // resulting stream terminates at `head`. The 0-overlap invariant is
    // asserted in debug builds.
    static ZBCL<FpType> cons(const value_type& head_block, thunk_type tail) {
        auto node = std::make_shared<Node>(head_block, std::move(tail));
        return ZBCL<FpType>(std::move(node));
    }

    // Convenience: cons with a finite (already-materialised) tail.
    static ZBCL<FpType> cons(const value_type& head_block, ZBCL<FpType> tail) {
        // Wrap the materialised tail in a thunk that just returns it.
        return cons(head_block, [tail]() noexcept { return tail; });
    }

    // Convenience: single-block co-list (tail is empty).
    static ZBCL<FpType> singleton(const value_type& head_block) {
        return cons(head_block, thunk_type{});
    }

    bool is_empty() const noexcept { return !_node; }
    explicit operator bool() const noexcept { return !is_empty(); }

    // head(): undefined on empty co-lists (assertion-checked).
    const value_type& head() const noexcept {
        assert(_node && "head() called on empty ZBCL");
        return _node->head;
    }

    // tail(): forces the thunk on first call, memoises the result.
    // Returns an empty ZBCL when called on a node with an empty thunk.
    ZBCL<FpType> tail() const {
        assert(_node && "tail() called on empty ZBCL");
        if (!_node->tail_evaluated) {
            if (_node->tail_thunk) {
                _node->tail_value = _node->tail_thunk();
                // 0-overlap invariant: if the tail is non-empty, head and
                // tail.head() must satisfy zero_overlap. Asserted only in
                // debug builds because forcing the thunk to check would
                // defeat laziness in release builds.
                assert((_node->tail_value.is_empty() ||
                        zero_overlap(_node->head, _node->tail_value.head()))
                       && "ZBCL 0-overlap invariant violated");
            } else {
                _node->tail_value = ZBCL<FpType>{};
            }
            _node->tail_evaluated = true;
        }
        return _node->tail_value;
    }

    // take(n): force the first n blocks; returns fewer if the stream terminates
    // earlier. Memoises forced tails on the way.
    //
    // Note on laziness: we deliberately do NOT call cur.tail() after consuming
    // the n-th block, because doing so would force the (n+1)-th thunk for no
    // benefit. take(1) on a multi-block stream therefore touches the first
    // block only and leaves the rest of the stream unforced.
    std::vector<value_type> take(std::size_t n) const {
        std::vector<value_type> result;
        result.reserve(n);
        ZBCL<FpType> cur = *this;
        while (result.size() < n) {
            if (cur.is_empty()) break;
            result.push_back(cur.head());
            if (result.size() == n) break;
            cur = cur.tail();
        }
        return result;
    }
};

}} // namespace sw::universal
