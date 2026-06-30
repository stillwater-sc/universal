// error_and_gamma.hpp: error and gamma functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// erf: Error function
template<unsigned nlimbs>
constexpr efloat<nlimbs> erf(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	return efloat<nlimbs>(std::erf(double(x)));
}

// erfc: Complementary error function
template<unsigned nlimbs>
constexpr efloat<nlimbs> erfc(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	return efloat<nlimbs>(std::erfc(double(x)));
}

// tgamma: Gamma function
template<unsigned nlimbs>
constexpr efloat<nlimbs> tgamma(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	return efloat<nlimbs>(std::tgamma(double(x)));
}

// lgamma: Log-gamma function
template<unsigned nlimbs>
constexpr efloat<nlimbs> lgamma(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	return efloat<nlimbs>(std::lgamma(double(x)));
}

}} // namespace sw::universal
