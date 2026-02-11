#pragma once
// mxfloat_traits.hpp : traits for the mxblock number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for mxblock types
	template<typename _Ty>
	struct is_mxblock_trait
		: false_type
	{
	};

	template<typename ElementType, size_t BlockSize>
	struct is_mxblock_trait< sw::universal::mxblock<ElementType, BlockSize> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_mxblock = is_mxblock_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_mxblock = std::enable_if_t<is_mxblock<_Ty>, Type>;

}} // namespace sw::universal
