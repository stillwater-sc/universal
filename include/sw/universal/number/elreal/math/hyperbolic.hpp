// hyperbolic.hpp: McCleeary LFPERA sinh / cosh / tanh on ZBCL<FpType> (Phase 7.4, #931).
//
//   sinh(x) = (e^x - e^-x) / 2
//   cosh(x) = (e^x + e^-x) / 2
//   tanh(x) = (e^2x - 1) / (e^2x + 1)
//
// Direct identities on the Phase 7.3 exp(). depth carries the same modest default
// and ~O(depth^4) cost characteristics as exp/log (see exponent.hpp and #1040).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstddef>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/threeAdd.hpp>     // add
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>     // mul_scalar
#include <universal/number/elreal/divide.hpp>       // div
#include <universal/number/elreal/math/exponent.hpp>

namespace sw { namespace universal {

// sinh(x, depth) = (e^x - e^-x) / 2.
template <typename FpType>
inline ZBCL<FpType> sinh(ZBCL<FpType> x, std::size_t depth = 4) {
    ZBCL<FpType> ex  = exp(x, depth);
    ZBCL<FpType> emx = exp(negate(x), depth);
    return mul_scalar(block<FpType>{ static_cast<FpType>(0.5), 0 }, add(ex, negate(emx)), depth);
}

// cosh(x, depth) = (e^x + e^-x) / 2.
template <typename FpType>
inline ZBCL<FpType> cosh(ZBCL<FpType> x, std::size_t depth = 4) {
    ZBCL<FpType> ex  = exp(x, depth);
    ZBCL<FpType> emx = exp(negate(x), depth);
    return mul_scalar(block<FpType>{ static_cast<FpType>(0.5), 0 }, add(ex, emx), depth);
}

// tanh(x, depth) = (e^2x - 1) / (e^2x + 1). tanh(0) = 0.
template <typename FpType>
inline ZBCL<FpType> tanh(ZBCL<FpType> x, std::size_t depth = 4) {
    if (x.is_empty()) return ZBCL<FpType>{};                 // tanh(0) = 0
    ZBCL<FpType> e2 = exp(mul_scalar(block<FpType>{ static_cast<FpType>(2.0), 0 }, x, depth), depth);
    ZBCL<FpType> one = from_native<FpType>(1.0);
    return div(add(e2, negate(one)), add(e2, one), depth);
}

}} // namespace sw::universal
