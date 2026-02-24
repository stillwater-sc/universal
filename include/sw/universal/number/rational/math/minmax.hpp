#pragma once
// minmax.hpp: min/max functions for rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// minimum of two values
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt>
min(rational<nbits, Base, bt> x, rational<nbits, Base, bt> y) {
	return rational<nbits, Base, bt>(std::min(double(x), double(y)));
}

// maximum of two values
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt>
max(rational<nbits, Base, bt> x, rational<nbits, Base, bt> y) {
	return rational<nbits, Base, bt>(std::max(double(x), double(y)));
}


}} // namespace sw::universal
