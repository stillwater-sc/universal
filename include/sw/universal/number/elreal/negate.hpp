// negate.hpp: McCleeary LFPERA unary negation on ZBCL<FpType> (dissertation 4.2.4).
//
// negate(x) flips the sign of every block. value(-b) = -value(b) exactly, and
// the combined exponent E(b) = scale_of_v(v) + exp is unchanged by a sign flip
// (the IEEE exponent field does not depend on the sign bit), so the 0-overlap
// invariant is preserved without any renormalisation. The empty co-list (which
// represents 0) negates to itself.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>

namespace sw { namespace universal {

// negate(x): lazy per-block sign flip. Forcing the i-th block of negate(x)
// forces the i-th block of x and returns its negation, so negate is as lazy as
// its operand.
template <typename FpType>
inline ZBCL<FpType> negate(ZBCL<FpType> x) {
    if (x.is_empty()) return ZBCL<FpType>{};
    const block<FpType>& h = x.head();
    block<FpType> nh{ -h.v, h.exp };
    return ZBCL<FpType>::cons(nh, [x]() { return negate(x.tail()); });
}

}} // namespace sw::universal
