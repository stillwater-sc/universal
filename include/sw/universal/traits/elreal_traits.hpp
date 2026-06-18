#pragma once
// elreal_traits.hpp : traits for the McCleeary LFPERA lazy exact-real number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>
#include <universal/number/elreal/elreal_fwd.hpp>

namespace sw { namespace universal {

	// trait to identify elreal types
	template<typename _Ty>
	struct is_elreal_trait
		: false_type
	{
	};

	template<typename FpType>
	struct is_elreal_trait< sw::universal::elreal<FpType> >
		: true_type
	{
	};

	template<typename _Ty>
	constexpr bool is_elreal = is_elreal_trait<_Ty>::value;

	template<typename _Ty, typename Type = void>
	using enable_if_elreal = std::enable_if_t<is_elreal<_Ty>, Type>;

}} // namespace sw::universal
