#pragma once
// interval_traits.hpp: type traits for the interval number type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>

namespace sw { namespace universal {

// type trait to check if a type is an interval
template<typename _Ty>
struct is_interval_trait : std::false_type {};

template<typename Scalar>
struct is_interval_trait<interval<Scalar>> : std::true_type {};

template<typename _Ty>
constexpr bool is_interval = is_interval_trait<_Ty>::value;

// enable_if helper for interval types
template<typename _Ty>
using enable_if_interval = std::enable_if_t<is_interval<_Ty>, _Ty>;

// extract the scalar type from an interval
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
