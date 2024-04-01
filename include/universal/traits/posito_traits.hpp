#pragma once
//  posito_traits.hpp : traits for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for posito types
template<typename _Ty>
struct is_posito_trait
	: false_type
{
};
template<unsigned nbits, unsigned es>
struct is_posito_trait< sw::universal::posito<nbits, es> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_posito = is_posito_trait<_Ty>::value;

template<typename _Ty, typename Type = void>
using enable_if_posito = std::enable_if_t<is_posito<_Ty>, Type>;

}} // namespace sw::universal
