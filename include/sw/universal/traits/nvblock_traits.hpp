#pragma once
// nvblock_traits.hpp : traits for the nvblock number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for nvblock types
	template<typename _Ty>
	struct is_nvblock_trait
		: false_type
	{
	};

	template<typename ElementType, size_t BlockSize, typename ScaleType>
	struct is_nvblock_trait< sw::universal::nvblock<ElementType, BlockSize, ScaleType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_nvblock = is_nvblock_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_nvblock = std::enable_if_t<is_nvblock<_Ty>, Type>;

}} // namespace sw::universal
