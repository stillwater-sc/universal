#pragma once
//  posit_traits.hpp : traits for posits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace unum {

	// define a trait for posit types
	template<typename _Ty>
	struct is_posit_trait
		: false_type
	{
	};
	template<size_t nbits, size_t es>
	struct is_posit_trait< sw::unum::posit<nbits, es> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_posit = is_posit_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_posit = std::enable_if_t<is_posit<_Ty>, Type>;

	template<typename _Ty>
	struct value_type_trait
	{
		using type = typename _Ty::value_type;
	};
	// with template alias we don't need the typename upfront anymore
	template<typename _Ty>
	using value_type = typename value_type_trait<_Ty>::type;

}} // namespace sw::unum
