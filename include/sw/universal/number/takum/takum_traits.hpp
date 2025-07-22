#pragma once
// takum_traits.hpp : traits for takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for takum types
	template<typename _Ty>
	struct is_takum_trait
		: false_type
	{
	};

	template<unsigned nbits, typename BlockType>
	struct is_takum_trait< takum<nbits, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_takum = is_takum_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_takum = std::enable_if_t<is_takum<_Ty>, _Ty>;

}} // namespace sw::universal
