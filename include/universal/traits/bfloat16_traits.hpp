#pragma once
// bfloat16_traits.hpp : traits for the bfloat16 number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for bfloat16 type
	template<typename _Ty>
	struct is_bfloat16_trait
		: false_type
	{
	};

	template<>
	struct is_bfloat16_trait< sw::universal::bfloat16 >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_bfloat16 = is_bfloat16_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_bfloat16 = std::enable_if_t<is_bfloat16<_Ty>, Type>;

}} // namespace sw::universal
