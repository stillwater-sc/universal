#pragma once
// manipulators.hpp: definition of manipulation functions for binary rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for rational type
	template<unsigned nbits, typename bt>
	std::string type_tag(const rational<nbits,bt>& = {}) {
		std::stringstream s;
		s << "rational<" << nbits << ", " << type_tag(bt()) << '>';
		return s.str();
	}

	template<unsigned nbits, typename bt>
	inline std::string components(const rational<nbits,bt>& v) {
		std::stringstream s;
		if (v.iszero()) {
			s << " zero b" << std::setw(nbits) << v.fraction();
			return s.str();
		}
		else if (v.isinf()) {
			s << " infinite b" << std::setw(nbits) << v.fraction();
			return s.str();
		}
		s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
		return s.str();
	}
}} // namespace sw::universal
