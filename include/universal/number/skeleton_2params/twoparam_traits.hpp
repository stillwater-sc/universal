#pragma once
// twoparam_traits.hpp : traits for two-param number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for twoparam types
	template<typename _Ty>
	struct is_twoparam_trait
		: false_type
	{
	};

	template<unsigned nbits, unsigned es, typename BlockType>
	struct is_twoparam_trait< twoparam<nbits, es, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_twoparam = is_twoparam_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_twoparam = std::enable_if_t<is_twoparam<_Ty>, _Ty>;

}} // namespace sw::universal
