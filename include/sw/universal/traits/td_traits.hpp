#pragma once
// td_traits.hpp : traits for triple-double (td) arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
#include <universal/number/td/td_fwd.hpp>

namespace sw { namespace universal {

// define a trait for triple-double (td) type
template<typename _Ty>
struct is_td_trait
	: false_type
{
};

template<>
struct is_td_trait< td >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_td = is_td_trait<_Ty>::value;

template<typename _Ty>
using enable_if_td = std::enable_if_t<is_td<_Ty>, _Ty>;

}} // namespace sw::universal
