#pragma once
// manipulators.hpp: definition of manipulation functions for rational types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for rational type
	template<unsigned nbits, typename Base, typename bt>
	std::string type_tag(const rational<nbits,Base,bt>& = {}) {
		std::stringstream s;
		if constexpr (std::is_same_v<Base, base2>) {
			s << "rational<" << nbits << ", base2, " << type_tag(bt()) << '>';
		}
		else if constexpr (std::is_same_v<Base, base8>) {
			s << "rational<" << nbits << ", base8>";
		}
		else if constexpr (std::is_same_v<Base, base10>) {
			s << "rational<" << nbits << ", base10>";
		}
		else if constexpr (std::is_same_v<Base, base16>) {
			s << "rational<" << nbits << ", base16>";
		}
		else {
			s << "rational<" << nbits << ", unknown_base>";
		}
		return s.str();
	}

}} // namespace sw::universal
