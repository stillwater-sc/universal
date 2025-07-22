#pragma once
// bfloat8_traits.hpp : traits for the bfloat8 number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for bfloat8 type
	template<typename _Ty>
	struct is_bfloat8_trait
		: false_type
	{
	};

	template<>
	struct is_bfloat8_trait< sw::universal::bfloat8 >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_bfloat8 = is_bfloat8_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_bfloat8 = std::enable_if_t<is_bfloat8<_Ty>, Type>;

}} // namespace sw::universal
