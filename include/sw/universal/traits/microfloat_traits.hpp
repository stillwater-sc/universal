#pragma once
// microfloat_traits.hpp : traits for the microfloat number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for microfloat types
	template<typename _Ty>
	struct is_microfloat_trait
		: false_type
	{
	};

	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	struct is_microfloat_trait< sw::universal::microfloat<nbits, es, hasInf, hasNaN, isSaturating> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_microfloat = is_microfloat_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_microfloat = std::enable_if_t<is_microfloat<_Ty>, Type>;

}} // namespace sw::universal
