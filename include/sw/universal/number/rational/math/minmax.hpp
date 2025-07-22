#pragma once
// minmax.hpp: min/max functions for rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// minimum of two values
template<unsigned nbits, typename bt>
rational<nbits, bt>
min(rational<nbits, bt> x, rational<nbits, bt> y) {
	return rational<nbits, bt>(std::min(double(x), double(y)));
}

// maximum of two values
template<unsigned nbits, typename bt>
rational<nbits, bt>
max(rational<nbits, bt> x, rational<nbits, bt> y) {
	return rational<nbits, bt>(std::max(double(x), double(y)));
}


}} // namespace sw::universal
