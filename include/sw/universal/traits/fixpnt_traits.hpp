#pragma once
//  fixpnt_traits.hpp : traits for fixed-point number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for fixed-point types
	template<typename _Ty>
	struct is_fixpnt_trait
		: false_type
	{
	};
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename  bt>
	struct is_fixpnt_trait< sw::universal::fixpnt<nbits, rbits, arithmetic, bt> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_fixpnt = is_fixpnt_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_fixpnt = std::enable_if_t<is_fixpnt<_Ty>, Type>;

}} // namespace sw::universal
