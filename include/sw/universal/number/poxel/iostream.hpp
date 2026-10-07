#pragma once
// iostream.hpp: stream insertion for the poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

namespace sw { namespace universal {

// an exact tile prints its value, an open tile its interval: "(lower, upper)"
template<unsigned nbits, unsigned es, typename bt>
std::ostream& operator<<(std::ostream& ostr, const poxel<nbits, es, bt>& v) {
	return ostr << to_interval(v, static_cast<int>(ostr.precision()));
}

}} // namespace sw::universal
