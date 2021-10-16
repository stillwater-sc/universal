#pragma once
// minmax.hpp: min/max functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
min(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::min(double(x), double(y)));
}

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
max(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::max(double(x), double(y)));
}

}  // namespace sw::universal
