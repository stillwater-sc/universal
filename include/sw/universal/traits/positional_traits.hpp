#pragma once
// positional_traits.hpp : traits for positional integer number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for the positional number system type
	template<typename _Ty>
	struct is_positional_trait
		: false_type
	{
	};
	template<unsigned ndigits, unsigned radix>
	struct is_positional_trait< sw::universal::positional<ndigits, radix> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_positional = is_positional_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_positional = std::enable_if_t<is_positional<_Ty>, Type>;

}} // namespace sw::universal
