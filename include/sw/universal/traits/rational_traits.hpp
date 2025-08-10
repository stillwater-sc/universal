#pragma once
// rational_traits.hpp : traits for fixed-size arbitrary configuration binary rational number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for the erational number system type
	template<typename _Ty>
	struct is_rational_trait
		: false_type
	{
	};
	template<unsigned nbits, typename bt>
	struct is_rational_trait< sw::universal::rational<nbits,bt> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_rational = is_rational_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_rational = std::enable_if_t<is_rational<_Ty>, Type>;

}} // namespace sw::universal
