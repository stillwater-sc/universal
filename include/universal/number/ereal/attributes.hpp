#pragma once 
// attributes.hpp: functions to query number system attributes 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>  
#include <string>
#include <sstream>

namespace sw { namespace universal {  

// functions to provide details about properties of an ereal configuration

	template<unsigned nlimbs>
	inline int sign(const ereal<nlimbs>& v) { return v.sign(); }

	template<unsigned nlimbs>
	inline int64_t scale(const ereal<nlimbs>& v) { return v.scale(); }
	
	template<unsigned nlimbs, typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	inline Real significant(const ereal<nlimbs>& v) { return static_cast<Real>(v.significant()); }

}}  // namespace sw::universal
