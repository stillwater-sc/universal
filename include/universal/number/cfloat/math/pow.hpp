#pragma once
// pow.hpp: pow functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> pow(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> pow(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> x, int y) {
	return cfloat<nbits,es,bt, hasSubnormals, hasSupernormals, isSaturating>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> pow(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> x, double y) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::pow(double(x), y));
}

}  // namespace sw::universal
