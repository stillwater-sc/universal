#pragma once
// dfixpnt_traits.hpp: traits for decimal fixed-point number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
#include <universal/number/dfixpnt/dfixpnt_fwd.hpp>

namespace sw { namespace universal {

	// define a trait for dfixpnt types
	template<typename _Ty>
	struct is_dfixpnt_trait
		: false_type
	{
	};

	template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
	struct is_dfixpnt_trait< sw::universal::dfixpnt<ndigits, radix, encoding, arithmetic, bt> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_dfixpnt = is_dfixpnt_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_dfixpnt = std::enable_if_t<is_dfixpnt<_Ty>, Type>;

}} // namespace sw::universal
