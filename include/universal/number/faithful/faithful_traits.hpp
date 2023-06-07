#pragma once
//  faithful_traits.hpp : traits for faithful number system
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for faithful arithmetic types
	template<typename _Ty>
	struct is_faithful_trait
		: false_type
	{
	};

	template<typename FloatingPointType>
	struct is_faithful_trait< faithful< FloatingPointType > >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_faithful = is_faithful_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_faithful = std::enable_if_t<is_faithful<_Ty>, _Ty>;

}} // namespace sw::universal
