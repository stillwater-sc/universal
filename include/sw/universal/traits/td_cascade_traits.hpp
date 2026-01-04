#pragma once
// td_cascade_traits.hpp : traits for triple-double cascade (td_cascade) arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
#include <universal/number/td_cascade/td_cascade_fwd.hpp>

namespace sw { namespace universal {

// define a trait for triple-double cascade (td_cascade) type
template<typename _Ty>
struct is_td_cascade_trait
	: false_type
{
};

template<>
struct is_td_cascade_trait< td_cascade >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_td_cascade = is_td_cascade_trait<_Ty>::value;

template<typename _Ty>
using enable_if_td_cascade = std::enable_if_t<is_td_cascade<_Ty>, _Ty>;

}} // namespace sw::universal
