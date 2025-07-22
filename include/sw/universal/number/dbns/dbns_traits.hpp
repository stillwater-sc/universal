#pragma once
//  dbns_traits.hpp : traits for double base number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for dbns types
template<typename _Ty>
struct is_dbns_trait
	: false_type
{
};

template<unsigned nbits, unsigned fbbits, typename BlockType, auto... xtra>
struct is_dbns_trait< dbns<nbits, fbbits, BlockType, xtra...> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_dbns = is_dbns_trait<_Ty>::value;

template<typename _Ty>
using enable_if_dbns = std::enable_if_t<is_dbns<_Ty>, _Ty>;

}} // namespace sw::universal
