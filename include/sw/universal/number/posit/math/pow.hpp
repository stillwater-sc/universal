#pragma once
// pow.hpp: pow functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> pow(posit<nbits,es,bt> x, posit<nbits, es, bt> y) {
	return posit<nbits,es,bt>(std::pow(double(x), double(y)));
}
		
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> pow(posit<nbits,es,bt> x, int y) {
	return posit<nbits,es,bt>(std::pow(double(x), double(y)));
}
		
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> pow(posit<nbits,es,bt> x, double y) {
	return posit<nbits,es,bt>(std::pow(double(x), y));
}

// calculate an integer power function base^int
template<typename Scalar>
Scalar integer_power(Scalar base, int exponent) {
	if (exponent < 0) {
		base = Scalar(1) / base;
		exponent = -exponent;
	}
	if (exponent == 0) return Scalar(1);
	Scalar power = Scalar(1);
	while (exponent > 1) {
		if (exponent & 0x1) {
			power = base * power;
			base *= base;
			exponent = (exponent - 1) / 2;
		}
		else {
			base *= base;
			exponent /= 2;
		}
	}
	return base * power;
}

}} // namespace sw::universal
