#pragma once
// e8m0_traits.hpp : traits for the e8m0 number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for e8m0 type
	template<typename _Ty>
	struct is_e8m0_trait
		: false_type
	{
	};

	template<>
	struct is_e8m0_trait< sw::universal::e8m0 >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_e8m0 = is_e8m0_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_e8m0 = std::enable_if_t<is_e8m0<_Ty>, Type>;

}} // namespace sw::universal
