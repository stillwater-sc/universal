#pragma once
// interval_traits.hpp: type traits for the interval number type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>

namespace sw { namespace universal {

/**
 * @brief Trait family for recognizing `interval<T>` in generic code.
 *
 * @details These helpers are intentionally lightweight: they do not describe interval arithmetic
 * semantics, only whether a type participates in the library's interval API surface and what its
 * underlying scalar is. They are mainly used to steer overload sets and adapters without forcing
 * those callers to include the full interval implementation.
 */
template<typename _Ty>
struct is_interval_trait : std::false_type {};

template<typename Scalar>
struct is_interval_trait<interval<Scalar>> : std::true_type {};

template<typename _Ty>
constexpr bool is_interval = is_interval_trait<_Ty>::value;

/// SFINAE helper for APIs that should accept only `interval<T>` specializations.
template<typename _Ty>
using enable_if_interval = std::enable_if_t<is_interval<_Ty>, _Ty>;

/// Extract the scalar bound type from an interval; yields `void` for non-interval inputs.
template<typename _Ty>
struct interval_scalar_type {
	using type = void;
};

template<typename Scalar>
struct interval_scalar_type<interval<Scalar>> {
	using type = Scalar;
};

template<typename _Ty>
using interval_scalar_t = typename interval_scalar_type<_Ty>::type;

}} // namespace sw::universal
