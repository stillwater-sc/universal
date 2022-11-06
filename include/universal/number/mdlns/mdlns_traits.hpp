#pragma once
//  mdlns_traits.hpp : traits for logarithmic number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for lns types
template<typename _Ty>
struct is_mdlns_trait
	: false_type
{
};

template<unsigned nbits, unsigned rbits, typename BlockType, auto... xtra>
struct is_mdlns_trait< mdlns<nbits, rbits, BlockType, xtra...> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_mdlns = is_mdlns_trait<_Ty>::value;

template<typename _Ty>
using enable_if_mdlns = std::enable_if_t<is_mdlns<_Ty>, _Ty>;

}} // namespace sw::universal
