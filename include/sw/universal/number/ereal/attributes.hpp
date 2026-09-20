#pragma once 
// attributes.hpp: functions to query number system attributes 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>  
#include <type_traits>

#include <universal/number/ereal/core.hpp>

namespace sw { namespace universal {  

// functions to provide details about properties of an ereal configuration

	template<unsigned nlimbs, typename FpType>
	inline int sign(const ereal<nlimbs, FpType>& v) { return v.sign(); }

	template<unsigned nlimbs, typename FpType>
	inline int64_t scale(const ereal<nlimbs, FpType>& v) { return v.scale(); }
	
	// Real stays the second template argument, so significant<nlimbs, float>(v) keeps
	// working; the limb type after it is deduced from the argument
	template<unsigned nlimbs, typename Real, typename FpType,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	inline Real significant(const ereal<nlimbs, FpType>& v) { return static_cast<Real>(v.significant()); }

}}  // namespace sw::universal
