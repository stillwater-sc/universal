#pragma once
// error_gamma.hpp: error and gamma functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
rational erf(rational x) {
	return rational(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
rational erfc(rational x) {
	return rational(std::erfc(double(x)));
}

}  // namespace sw::universal
