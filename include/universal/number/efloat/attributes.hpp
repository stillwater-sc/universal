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

// functions to provide details about properties of an efloat configuration

	inline int sign(const efloat& v) { return v.sign(); }

	inline int64_t scale(const efloat& v) { return v.scale(); }
	
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	inline Real significant(const efloat& v) { return Real(v); }

}}  // namespace sw::universal
