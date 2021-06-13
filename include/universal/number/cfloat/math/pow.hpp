#pragma once
// pow.hpp: pow functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> pow(cfloat<nbits,es,bt> x, cfloat<nbits,es,bt> y) {
	return cfloat<nbits,es,bt>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> pow(cfloat<nbits,es,bt> x, int y) {
	return cfloat<nbits,es,bt>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> pow(cfloat<nbits,es,bt> x, double y) {
	return cfloat<nbits,es,bt>(std::pow(double(x), y));
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

}  // namespace sw::universal
