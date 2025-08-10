#pragma once
// integer_type_tag.hpp: definition of type_tag function for native integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

	template<typename IntegralType, 
		std::enable_if_t< ::std::is_integral<IntegralType>::value, bool> = true
	>
	std::string type_tag(IntegralType = {}) {
		// can't use a simple typeid(Real).name() because gcc and clang obfuscate the native types
		constexpr unsigned nbits = sizeof(IntegralType) * 8;
		std::string type_string;
		if constexpr (nbits == 8) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int8_t") : std::string("uint8_t");
		}
		else if constexpr (nbits == 16) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int16_t") : std::string("uint16_t");
		}
		else if constexpr (nbits == 32) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int32_t") : std::string("uint32_t");
		}
		else if constexpr (nbits == 64) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int64_t") : std::string("uint64_t");
		}
		else {
			type_string = std::string("unknown");
		}
		return type_string;
	}

}} // namespace sw::universal
