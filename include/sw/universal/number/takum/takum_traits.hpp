#pragma once
// takum_traits.hpp : traits for takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for takum types
	template<typename _Ty>
	struct is_takum_trait
		: false_type
	{
	};

	template<unsigned nbits, unsigned rbits, typename BlockType>
	struct is_takum_trait< takum<nbits, rbits, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_takum = is_takum_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_takum = std::enable_if_t<is_takum<_Ty>, _Ty>;

	// define a trait for the logarithmic takum, which shares takum's codec but is a
	// distinct number system: same bit layout, base sqrt(e) instead of base 2.
	template<typename _Ty>
	struct is_takum_log_trait
		: false_type
	{
	};

	template<unsigned nbits, unsigned rbits, typename BlockType>
	struct is_takum_log_trait< takum_log<nbits, rbits, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_takum_log = is_takum_log_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_takum_log = std::enable_if_t<is_takum_log<_Ty>, _Ty>;

	// either variant: use for code that only touches the shared encoding, such as
	// the bit-level manipulators.  Value-level code should pick one.
	template<typename _Ty>
	constexpr bool is_any_takum = is_takum<_Ty> || is_takum_log<_Ty>;

	template<typename _Ty>
	using enable_if_any_takum = std::enable_if_t<is_any_takum<_Ty>, _Ty>;

}} // namespace sw::universal
