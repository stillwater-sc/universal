#pragma once
// hfloat_traits.hpp : traits for hexadecimal floating-point number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
#include <universal/number/hfloat/hfloat_fwd.hpp>

namespace sw { namespace universal {

	// define a trait for hfloat types
	template<typename _Ty>
	struct is_hfloat_trait
		: false_type
	{
	};

	template<unsigned ndigits, unsigned es, typename BlockType>
	struct is_hfloat_trait< sw::universal::hfloat<ndigits, es, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_hfloat = is_hfloat_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_hfloat = std::enable_if_t<is_hfloat<_Ty>, _Ty>;

}} // namespace sw::universal
