// hypot.hpp: McCleeary LFPERA hypotenuse on ZBCL<FpType> (Phase 7.2, #931).
//
// hypot(x, y) = sqrt(x*x + y*y). The expansion arithmetic carries the squares
// exactly (no intermediate overflow of the represented value), so the naive
// formula is safe here -- the block exponents are int32_t and absorb the
// doubling of scale.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstddef>

#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/threeAdd.hpp>   // add
#include <universal/number/elreal/multiply.hpp>   // mul
#include <universal/number/elreal/math/sqrt.hpp>

namespace sw { namespace universal {

// hypot(x, y, depth): sqrt(x*x + y*y), refined to ~depth blocks.
template <typename FpType>
inline ZBCL<FpType> hypot(ZBCL<FpType> x, ZBCL<FpType> y, std::size_t depth = 64) {
    ZBCL<FpType> x2 = mul(x, x, depth);
    ZBCL<FpType> y2 = mul(y, y, depth);
    return sqrt(add(x2, y2), depth);
}

}} // namespace sw::universal
