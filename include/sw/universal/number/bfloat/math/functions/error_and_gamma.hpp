#pragma once
// error_gamma.hpp: error and gamma functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
inline bfloat16 erf(bfloat16 x) {
	return bfloat16(std::erf(float(x)));
}

// Compute the complementary error function: 1 - erf(x)
inline bfloat16 erfc(bfloat16 x) {
	return bfloat16(std::erfc(float(x)));
}

// Compute the gamma function:
inline bfloat16 tgamma(bfloat16 x) {
	return bfloat16(std::tgamma(float(x)));
}

}} // namespace sw::universal
