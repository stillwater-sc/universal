#pragma once
// manipulators.hpp: definition of manipulation functions for native types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>

namespace sw::universal {

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	std::string type_tag(const Real f) {
		std::stringstream s;
		s << typeid(Real).name();
		return s.str();
	}

	// Generate a type tag for this native floating-point type
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	std::string type_tag() {
//		constexpr size_t nbits = ieee754_parameter<Real>::nbits;
//		constexpr size_t es = ieee754_parameter<Real>::ebits;
		Real f(0);
		return type_tag(f);
	}

} // namespace sw::universal
