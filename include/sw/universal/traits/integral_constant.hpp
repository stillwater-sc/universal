#pragma once
//  integral_constant.hpp : integral constant template type
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// integral_constant base type
	template<typename Ty, Ty val>
	struct integral_constant
	{
		static constexpr Ty value = val;
		using value_type = Ty;
		using type = integral_constant;

		constexpr operator value_type() const noexcept { return (value); }
		constexpr value_type operator()() const noexcept { return (value); }
	};

	// define boolean template types
	template<bool val>
	using bool_constant = integral_constant<bool, val>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	template<typename _Ty>
	struct value_type_trait
	{
		using type = typename _Ty::value_type;
	};
	// with template alias we don't need the typename upfront anymore
	template<typename _Ty>
	using value_type = typename value_type_trait<_Ty>::type;

}} // namespace sw::universal
