#pragma once
// fractional.hpp: fractional functions for classic floating-point cfloats
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> fmod(cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	return cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::fmod(double(x), double(y)));
}

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> remainder(cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	return cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::remainder(double(x), double(y)));
}

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> frac(cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> x) {
	return cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating>(double(x)-long(x));
}


}} // namespace sw::universal
