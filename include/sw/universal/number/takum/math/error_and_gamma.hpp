// error_and_gamma.hpp: error and gamma functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Error function: erf(x) = (2/sqrt(pi)) * integral_0^x e^(-t^2) dt
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> erf(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::erf(double(x)));
}

// Complementary error function: erfc(x) = 1 - erf(x)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> erfc(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::erfc(double(x)));
}

}} // namespace sw::universal
