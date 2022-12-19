#pragma once
// attributes.hpp: definition of attribute functions for native real and integer types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

	/*
	* we are doing this through std::numeric_limits<Scalar> in arithmetic_traits.hpp
	

	template<typename IntegralType, 
		std::enable_if_t< std::is_integral<IntegralType>::value, bool> = true
	>
	std::string dynamic_range(IntegralType = {}) {
		return type_string;
	}

	template<typename RealType,
		std::enable_if_t< std::is_floating_point<RealType>::value, bool> = true
	>
	std::string dynamic_range(RealType = {}) {
		return type_string;
	}
	*/

}} // namespace sw::universal
