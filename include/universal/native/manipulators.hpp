#pragma once
// manipulators.hpp: definition of manipulation functions for native types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

	template<typename Real, 
		std::enable_if_t< std::is_floating_point<Real>::value, bool> = true
	>
	std::string type_tag(Real f) {
		// can't use a simple typeid(Real).name() because gcc and clang obfuscate the native types
		constexpr unsigned nbits = sizeof(f) * 8;
		std::string real;
		if constexpr (nbits == 32) {
			real = std::string("float");
		}
		else if constexpr (nbits == 64) {
			real = std::string("double");
		}
		else if constexpr (nbits == 128) {
			real = std::string("long double");
		}
		else {
			real = std::string("unknown");
		}
		return real;
	}

	template<typename Real,
		std::enable_if_t< std::is_floating_point<Real>::value, bool > = true
	>
	std::string type_tag() {
		Real f{ 0.0 };
		return type_tag(f);
	}

}} // namespace sw::universal
