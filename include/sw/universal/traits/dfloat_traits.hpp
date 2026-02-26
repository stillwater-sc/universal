#pragma once
// dfloat_traits.hpp : traits for decimal floating-point number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
#include <universal/number/dfloat/dfloat_fwd.hpp>

namespace sw { namespace universal {

	// define a trait for dfloat types
	template<typename _Ty>
	struct is_dfloat_trait
		: false_type
	{
	};

	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
	struct is_dfloat_trait< sw::universal::dfloat<ndigits, es, Encoding, BlockType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_dfloat = is_dfloat_trait<_Ty>::value;

	template<typename _Ty>
	using enable_if_dfloat = std::enable_if_t<is_dfloat<_Ty>, _Ty>;

}} // namespace sw::universal
