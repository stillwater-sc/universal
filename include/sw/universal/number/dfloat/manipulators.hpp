// manipulators.hpp: definitions of helper functions for decimal dfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for this dfloat
	template<unsigned ndigits, unsigned es, typename bt>
	std::string type_tag(const dfloat<ndigits, es, bt>& = {}) {
		std::stringstream s;
		s << "dfloat<"
			<< std::setw(3) << ndigits << ", "
			<< std::setw(3) << es << ", "
			<< typeid(bt).name() << ">";
		return s.str();
	}

}} // namespace sw::universal
