#pragma once
// efloat_traits.hpp : traits for adaptive precision floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	// define a trait for cfloat types
	template<typename _Ty>
	struct is_efloat_trait
		: false_type
	{
	};

	template<>
	struct is_efloat_trait< sw::universal::efloat >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_efloat = is_efloat_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_efloat = std::enable_if_t<is_efloat<_Ty>, Type>;

}} // namespace sw::universal
