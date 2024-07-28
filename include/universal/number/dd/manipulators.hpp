// manipulators.hpp: definitions of helper functions for doubledouble type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for a doubledouble
	std::string type_tag(const dd& = {}) {
		return std::string("doubledouble");
	}


}} // namespace sw::universal
