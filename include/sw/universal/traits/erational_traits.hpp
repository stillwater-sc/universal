#pragma once
// erational_traits.hpp : traits for adaptive precision decimal rational number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for the erational number system type
	template<typename _Ty>
	struct is_erational_trait
		: false_type
	{
	};
	template<>
	struct is_erational_trait< sw::universal::erational >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_erational = is_erational_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_erational = std::enable_if_t<is_erational<_Ty>, Type>;

}} // namespace sw::universal
