#pragma once
// error_gamma.hpp: error and gamma functions for cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es> erf(cfloat<nbits, es, bt> x) {
	return cfloat<nbits, es, bt>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es> erfc(cfloat<nbits, es, bt> x) {
	return cfloat<nbits, es, bt>(std::erfc(double(x)));
}

}  // namespace sw::universal
