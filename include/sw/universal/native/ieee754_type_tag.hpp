#pragma once
// ieee754_type_tag.hpp: definition of the type_tag for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
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

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>
	std::string type_field(RealType = {}) {
		std::string fields{};
		if constexpr (sizeof(RealType) == 4) {
			fields = "fields(s:1|e:8|f:23)";
		}
		else if constexpr (sizeof(RealType) == 8) {
			fields = "fields(s:1|e:11|f:52)";
		}
		if constexpr (sizeof(RealType) == 16) {
			fields = "fields(s:1|e:15|f:112)";
		}
		return fields;
	}

}} // namespace sw::universal
