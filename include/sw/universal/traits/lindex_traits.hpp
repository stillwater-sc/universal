#pragma once
// lindex_traits.hpp : traits for the level-index number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>   // std::enable_if_t
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for lindex types
	template<typename _Ty>
	struct is_lindex_trait
		: false_type
	{
	};

	template<unsigned nbits, unsigned lbits, typename BlockType>
	struct is_lindex_trait< lindex<nbits, lbits, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_lindex = is_lindex_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_lindex = std::enable_if_t<is_lindex<_Ty>, _Ty>;

}} // namespace sw::universal
