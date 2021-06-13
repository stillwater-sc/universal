#pragma once
// math_frac.hpp: fractional functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> fmod(cfloat<nbits,es,bt> x, cfloat<nbits,es,bt> y) {
	return cfloat<nbits,es,bt>(std::fmod(double(x), double(y)));
}

template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> remainder(cfloat<nbits,es,bt> x, cfloat<nbits,es,bt> y) {
	return cfloat<nbits,es,bt>(std::remainder(double(x), double(y)));
}

template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> frac(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(double(x)-long(x));
}


}  // namespace sw::universal
