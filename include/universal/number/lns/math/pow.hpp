#pragma once
// pow.hpp: pow functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> pow(lns<nbits, rbits, bt> x, lns<nbits, rbits, bt> y) {
	return lns<nbits, rbits, bt>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> pow(lns<nbits, rbits, bt> x, int y) {
	return lns<nbits, rbits, bt>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> pow(lns<nbits, rbits, bt> x, double y) {
	return lns<nbits, rbits, bt>(std::pow(double(x), y));
}

}} // namespace sw::universal
