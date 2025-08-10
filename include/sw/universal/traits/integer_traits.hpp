#pragma once
// integer_traits.hpp : traits for arbitrary precision, fixed-size integer number systems
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for integer types
	template<typename _Ty>
	struct is_integer_trait
		: false_type
	{
	};
	template<unsigned nbits, typename bt, IntegerNumberType nt>
	struct is_integer_trait< sw::universal::integer<nbits, bt, nt> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_integer = is_integer_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_integer = std::enable_if_t<is_integer<_Ty>, Type>;

}} // namespace sw::universal
