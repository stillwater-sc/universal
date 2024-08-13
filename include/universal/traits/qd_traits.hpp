#pragma once
// qd_traits.hpp : traits for quad-double (qd) arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for quad-double (qd) type
template<typename _Ty>
struct is_qd_trait
	: false_type
{
};

template<>
struct is_qd_trait< qd >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_qd = is_qd_trait<_Ty>::value;

template<typename _Ty>
using enable_if_qd = std::enable_if_t<is_qd<_Ty>, _Ty>;

}} // namespace sw::universal
