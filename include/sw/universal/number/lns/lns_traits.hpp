#pragma once
//  lns_traits.hpp : traits for logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for lns types
template<typename _Ty>
struct is_lns_trait
	: false_type
{
};

template<unsigned nbits, unsigned rbits, typename BlockType, auto... xtra>
struct is_lns_trait< lns<nbits, rbits, BlockType, xtra...> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_lns = is_lns_trait<_Ty>::value;

template<typename _Ty>
using enable_if_lns = std::enable_if_t<is_lns<_Ty>, _Ty>;

}} // namespace sw::universal
