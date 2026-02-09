#pragma once
// zfpblock_traits.hpp : traits for the zfpblock number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

	///////////////////////////////////////////////////////////////////////////
	// define a trait for zfpblock types
	template<typename _Ty>
	struct is_zfpblock_trait
		: false_type
	{
	};

	template<typename Real, unsigned Dim>
	struct is_zfpblock_trait< sw::universal::zfpblock<Real, Dim> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_zfpblock = is_zfpblock_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_zfpblock = std::enable_if_t<is_zfpblock<_Ty>, Type>;

}} // namespace sw::universal
