// zbcl_helpers.hpp: convenience constructors / destructors for ZBCL<FpType>
// used by Phase 2 tests and (later) by higher-level arithmetic.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>

namespace sw { namespace universal {

// empty<FpType>(): canonical empty ZBCL (represents the real number 0).
template <typename FpType>
inline ZBCL<FpType> empty() noexcept { return ZBCL<FpType>{}; }

// from_native<FpType>(double v): construct a finite ZBCL containing the value v.
//
// For Phase 2 the simplest construction is used: a single block whose value
// is `static_cast<FpType>(v)`, with `exp_offset = 0`. This is exact when v is
// representable in FpType. Lossy conversion is acceptable for Phase 2 -- this
// helper exists for tests, not as a production conversion path.
//
// Canonicalisation:
//   - v == 0.0 -> empty stream (zero is represented by the empty co-list).
//   - v that underflows to FpType{0} during the cast -> also empty stream.
//   - Subnormal results that do NOT underflow are preserved verbatim as the
//     block's stored value (note that such a block fails block::is_normalised
//     -- callers requiring normalised blocks must reject these inputs).
//   - NaN / inf are not supported; the caller must not pass them.
template <typename FpType>
inline ZBCL<FpType> from_native(double v) {
    if (v == 0.0) return ZBCL<FpType>{};
    FpType host_v = static_cast<FpType>(v);
    if (host_v == FpType{0}) return ZBCL<FpType>{}; // underflowed in cast
    block<FpType> b{host_v, 0};
    return ZBCL<FpType>::singleton(b);
}

// to_double_approx(stream, depth): sum the first `depth` blocks as doubles.
// The sum loses precision because each block converts to double via
// value_as<double>; this helper is for sanity-checking, not exact comparison.
template <typename FpType>
inline double to_double_approx(const ZBCL<FpType>& stream, std::size_t depth) {
    double acc = 0.0;
    auto blocks = stream.take(depth);
    for (const auto& b : blocks) {
        acc += b.template value_as<double>();
    }
    return acc;
}

}} // namespace sw::universal
