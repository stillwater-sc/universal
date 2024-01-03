#pragma once
// einteger_traits.hpp : traits for arbitrary size, elastic nteger number systems
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for integer types
	template<typename _Ty>
	struct is_einteger_trait
		: false_type
	{
	};
	template<typename _Ty>
	struct is_einteger_trait< sw::universal::einteger<_Ty> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_einteger = is_einteger_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_einteger = std::enable_if_t<is_einteger<_Ty>, Type>;

}} // namespace sw::universal
