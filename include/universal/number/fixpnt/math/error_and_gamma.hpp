#pragma once
// error_gamma.hpp: error and gamma functions for fixed-points
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, es, arithmetic, bt> erf(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, es, arithmetic, bt> erfc(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::erfc(double(x)));
}

}  // namespace sw::universal
