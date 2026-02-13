#pragma once
//  posit1_traits.hpp : traits for posit1 (old posit, 2-parameter template)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for posit1 types (old posit design, 2 template parameters)
template<typename _Ty>
struct is_posit1_trait
	: false_type
{
};
template<unsigned nbits, unsigned es>
struct is_posit1_trait< sw::universal::posit<nbits, es> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_posit1 = is_posit1_trait<_Ty>::value;

template<typename _Ty, typename Type = void>
using enable_if_posit1 = std::enable_if_t<is_posit1<_Ty>, Type>;

}} // namespace sw::universal
