#pragma once
// minmax.hpp: min/max functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> 
min(lns<nbits, rbits, bt> x, lns<nbits, rbits, bt> y) {
	return lns<nbits, rbits, bt>(std::min(double(x), double(y)));
}

template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> 
max(lns<nbits, rbits, bt> x, lns<nbits, rbits, bt> y) {
	return lns<nbits, rbits, bt>(std::max(double(x), double(y)));
}

}} // namespace sw::universal
