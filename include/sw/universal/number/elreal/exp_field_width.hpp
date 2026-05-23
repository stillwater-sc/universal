// exp_field_width.hpp: trait that yields the exponent-field width E (in bits)
//                       of a host floating-point type. Used by elreal block<>.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/bfloat16/bfloat16_fwd.hpp>

namespace sw { namespace universal {

// exp_field_width<T>::value -- bit width of T's biased exponent field.
// Primary template intentionally left undefined so unsupported types fail at compile time.
template <typename T>
struct exp_field_width;

template <> struct exp_field_width<float>       { static constexpr int value = 8;  };
template <> struct exp_field_width<double>      { static constexpr int value = 11; };
#if LONG_DOUBLE_SUPPORT
template <> struct exp_field_width<long double> { static constexpr int value = 15; };
#endif

template <unsigned nbits, unsigned es, typename bt,
          bool hasSubnormals, bool hasSupernormals, bool isSaturating>
struct exp_field_width<cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>> {
    static constexpr int value = static_cast<int>(es);
};

template <> struct exp_field_width<bfloat16> { static constexpr int value = 8; };

template <typename T>
inline constexpr int exp_field_width_v = exp_field_width<T>::value;

// exp_step<T>: 2^E_T, the multiplier in units of `exp_offset`.
template <typename T>
inline constexpr std::int64_t exp_step_v =
    static_cast<std::int64_t>(1) << exp_field_width_v<T>;

}} // namespace sw::universal
