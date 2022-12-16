#pragma once
// manipulators.hpp: definition of manipulation functions for native types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

	template<typename IntegralType, 
		std::enable_if_t< std::is_integral<IntegralType>::value, bool> = true
	>
	std::string type_tag(IntegralType = {}) {
		// can't use a simple typeid(Real).name() because gcc and clang obfuscate the native types
		constexpr unsigned nbits = sizeof(IntegralType) * 8;
		std::string type_string;
		if constexpr (nbits == 8) {
			type_string = std::string("char");
		}
		else if constexpr (nbits == 16) {
			type_string = std::string("short");
		}
		else if constexpr (nbits == 32) {
			type_string = std::string("long");
		}
		else if constexpr (nbits == 64) {
			type_string = std::string("long long");
		}
		else {
			type_string = std::string("unknown");
		}
		return type_string;
	}

	template<typename RealType,
		std::enable_if_t< std::is_floating_point<RealType>::value, bool> = true
	>
	std::string type_tag(RealType = {}) {
		// can't use a simple typeid(Real).name() because gcc and clang obfuscate the native types
		constexpr unsigned nbits = sizeof(RealType) * 8;
		std::string type_string;
		if constexpr (nbits == 32) {
			type_string = std::string("float");
		}
		else if constexpr (nbits == 64) {
			type_string = std::string("double");
		}
		else if constexpr (nbits == 128) {
			type_string = std::string("long double");
		}
		else {
			type_string = std::string("unknown");
		}
		return type_string;
	}

}} // namespace sw::universal
