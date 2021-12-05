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
		constexpr size_t nbits = ieee754_parameter<Real>::nbits;
		std::string realType;
		switch (nbits) {
		case 16:
			realType = "half";
			break;
		case 32:
			realType = "float";
			break;
		case 64:
			realType = "double";
			break;
		case 80:
			realType = "extended precision";
			break;
		case 128:
			realType = "quad";
			break;
		default:
			realType = "unknown";
		}
		std::stringstream s;
		// s << typeid(Real).name();  gcc and clang obfuscate the native types
		s << realType;
		if (f == 0.0) s << ' '; // to get rid of unreferenced formal parameter warning
		return s.str();
	}

	// Generate a type tag for this native floating-point type
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	std::string type_tag() {
		Real f(0);
		return type_tag(f);
	}

} // namespace sw::universal
