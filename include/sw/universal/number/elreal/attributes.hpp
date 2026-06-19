#pragma once
// attributes.hpp: free functions to query elreal value attributes.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstdint>
#include <type_traits>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>

namespace sw { namespace universal {

// sign(v): -1 if v < 0, +1 otherwise (+1 for zero).
template <typename FpType>
inline int sign(const elreal<FpType>& v) { return v.sign(); }

// scale(v): the value's binary exponent E (the leading block's combined exponent);
// 0 for zero. value(v) ~ significand * 2^scale.
template <typename FpType>
inline int64_t scale(const elreal<FpType>& v) { return static_cast<int64_t>(v.scale()); }

// significand(v): the normalised significand in [1,2) (sign applied), so that
// v ~ significand * 2^scale. Defined as v / 2^scale, taken from the host
// approximation; returns 0 for zero.
template <typename FpType, typename Real = double,
          typename = std::enable_if_t<std::is_floating_point<Real>::value, Real>>
inline Real significand(const elreal<FpType>& v) {
    if (v.iszero()) return Real{0};
    return static_cast<Real>(std::ldexp(v.template approx<Real>(2), -v.scale()));
}

}} // namespace sw::universal
