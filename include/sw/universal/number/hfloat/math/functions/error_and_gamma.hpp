#pragma once
// error_and_gamma.hpp: error and gamma functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> erf(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> erfc(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::erfc(double(x)));
}

// Compute the gamma function
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> tgamma(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::tgamma(double(x)));
}

// Compute the natural logarithm of the gamma function
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> lgamma(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::lgamma(double(x)));
}

}} // namespace sw::universal
