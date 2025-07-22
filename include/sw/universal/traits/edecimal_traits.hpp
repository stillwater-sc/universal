#pragma once
// edecimal_traits.hpp : traits for adaptive precision decimal number systems
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for the edecimal number system type
	template<typename _Ty>
	struct is_edecimal_trait
		: false_type
	{
	};
	template<>
	struct is_edecimal_trait< sw::universal::edecimal>
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_edecimal = is_edecimalal_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_edecimal = std::enable_if_t<is_edecimal<_Ty>, Type>;

}} // namespace sw::universal
