#pragma once
// dd_traits.hpp : traits for double-double (dd) arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for double-double (dd) type
template<typename _Ty>
struct is_dd_trait
	: false_type
{
};

template<>
struct is_dd_trait< dd >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_dd = is_dd_trait<_Ty>::value;

template<typename _Ty>
using enable_if_dd = std::enable_if_t<is_dd<_Ty>, _Ty>;

}} // namespace sw::universal
