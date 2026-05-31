// series.hpp: McCleeary LFPERA series<FpType> -- a lazy co-list of ZBCL terms.
//
// Phase 5 (#929) needs to sum a "stream of streams": a (possibly infinite)
// sequence of ZBCL<FpType> terms x_0, x_1, x_2, ... whose partial sums form a
// Cauchy sequence. This is the issue's `colist<ZBCL<FpType>>`. ZBCL<FpType> is
// itself a co-list of block<FpType>; series<FpType> is the next level up: a
// co-list whose elements are whole ZBCL streams.
//
// Representation (identical idiom to zbcl.hpp)
//   - Empty co-list = the empty series (its sum is 0).
//   - Non-empty co-list = (head ZBCL, tail thunk). The tail thunk is a
//     std::function<series()>, evaluated on demand by tail()/take(n) and
//     memoised on the node so a thunk fires at most once. Nodes are shared via
//     std::shared_ptr so series values are cheap to copy.
//   - End-of-stream sentinel is an empty thunk (default-constructed
//     std::function); a finite series terminates at its last materialised term.
//
// A series may be finite (built from a vector of terms) or infinite (built from
// an index generator). sum() in sum.hpp bounds an infinite series with a depth
// budget; see dissertation 4.2.3.
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

#include <universal/number/elreal/zbcl.hpp>

namespace sw { namespace universal {

template <typename FpType>
class series {
public:
    using value_type = ZBCL<FpType>;            // each element is a whole stream
    using thunk_type = std::function<series<FpType>()>;

private:
    struct Node {
        value_type            head;
        thunk_type            tail_thunk;
        mutable bool          tail_evaluated;
        mutable series<FpType> tail_value;

        Node(const value_type& h, thunk_type t)
            : head(h),
              tail_thunk(std::move(t)),
              tail_evaluated(false),
              tail_value() {}
    };

    std::shared_ptr<const Node> _node;

    explicit series(std::shared_ptr<const Node> n) noexcept : _node(std::move(n)) {}

public:
    // Default constructor: empty series (its sum is 0).
    series() noexcept : _node() {}

    // cons(head, tail): non-empty series. An empty `tail` function terminates
    // the series at `head`.
    static series<FpType> cons(const value_type& head_term, thunk_type tail) {
        auto node = std::make_shared<Node>(head_term, std::move(tail));
        return series<FpType>(std::move(node));
    }

    // Convenience: cons with an already-materialised tail.
    static series<FpType> cons(const value_type& head_term, series<FpType> tail) {
        return cons(head_term, [tail]() { return tail; });
    }

    // Convenience: single-term series (tail is empty).
    static series<FpType> singleton(const value_type& head_term) {
        return cons(head_term, thunk_type{});
    }

    bool is_empty() const noexcept { return !_node; }
    explicit operator bool() const noexcept { return !is_empty(); }

    // head(): undefined on the empty series (assertion-checked).
    const value_type& head() const noexcept {
        assert(_node && "head() called on empty series");
        return _node->head;
    }

    // tail(): forces the thunk on first call, memoises the result. Returns the
    // empty series for a node with an empty thunk.
    series<FpType> tail() const {
        assert(_node && "tail() called on empty series");
        if (!_node->tail_evaluated) {
            if (_node->tail_thunk) {
                _node->tail_value = _node->tail_thunk();
            } else {
                _node->tail_value = series<FpType>{};
            }
            _node->tail_evaluated = true;
        }
        return _node->tail_value;
    }

    // take(n): force the first n terms; returns fewer if the series terminates
    // earlier. Like ZBCL::take, it does not force the (n+1)-th thunk.
    std::vector<value_type> take(std::size_t n) const {
        std::vector<value_type> result;
        result.reserve(n);
        series<FpType> cur = *this;
        while (result.size() < n) {
            if (cur.is_empty()) break;
            result.push_back(cur.head());
            if (result.size() == n) break;
            cur = cur.tail();
        }
        return result;
    }
};

// -----------------------------------------------------------------------------
// Generators
// -----------------------------------------------------------------------------

// series_from_vector: a finite series from an already-materialised list of
// terms. The resulting series terminates after the last term, so sum() over it
// is an exact finite sum (no depth-budget truncation). Empty ZBCL terms (which
// represent 0) are retained -- a 0 term is a legitimate element, distinct from
// end-of-series.
template <typename FpType>
inline series<FpType> series_from_vector(const std::vector<ZBCL<FpType>>& terms) {
    series<FpType> tail{};
    for (std::size_t i = terms.size(); i-- > 0;) {
        tail = series<FpType>::cons(terms[i], tail);
    }
    return tail;
}

// series_from_generator: an infinite series whose i-th term is gen(i), i >= 0.
// Each call lazily produces the next node, so only the consumed prefix is
// realised. Callers must bound consumption (sum()'s max_depth does this).
template <typename FpType>
inline series<FpType> series_from_generator(std::function<ZBCL<FpType>(std::size_t)> gen,
                                            std::size_t index = 0) {
    ZBCL<FpType> head_term = gen(index);
    return series<FpType>::cons(head_term, [gen, index]() {
        return series_from_generator<FpType>(gen, index + 1);
    });
}

}} // namespace sw::universal
