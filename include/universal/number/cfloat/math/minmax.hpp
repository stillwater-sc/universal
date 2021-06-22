#pragma once
// math_minmax.hpp: min/max functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> min(cfloat<nbits,es,bt> x, cfloat<nbits,es,bt> y) {
	return cfloat<nbits,es,bt>(std::pow(double(x), double(y)));
}

template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> max(cfloat<nbits,es,bt> x, cfloat<nbits,es,bt> y) {
	return cfloat<nbits,es,bt>(std::pow(double(x), double(y)));
}

}  // namespace sw::universal
