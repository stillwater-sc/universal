#pragma once
// error_gamma.hpp: error and gamma functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<unsigned nbits, unsigned es>
posito<nbits,es> erf(posito<nbits,es> x) {
	return posito<nbits,es>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<unsigned nbits, unsigned es>
posito<nbits,es> erfc(posito<nbits,es> x) {
	return posito<nbits,es>(std::erfc(double(x)));
}

}} // namespace sw::universal
