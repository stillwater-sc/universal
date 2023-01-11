#pragma once
//  lns2b_traits.hpp : traits for 2-base logarithmic number system
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for lns types
template<typename _Ty>
struct is_lns2b_trait
	: false_type
{
};

template<unsigned nbits, unsigned fbbits, typename BlockType, auto... xtra>
struct is_lns2b_trait< lns2b<nbits, fbbits, BlockType, xtra...> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_lns2b = is_lns2b_trait<_Ty>::value;

template<typename _Ty>
using enable_if_lns2b = std::enable_if_t<is_lns2b<_Ty>, _Ty>;

}} // namespace sw::universal
