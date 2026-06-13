// pow.hpp: power functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> pow(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::pow(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> pow(const takum<nbits, rbits, bt>& x, int y) {
	return takum<nbits, rbits, bt>(std::pow(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> pow(const takum<nbits, rbits, bt>& x, double y) {
	return takum<nbits, rbits, bt>(std::pow(double(x), y));
}

// Exact integer power via repeated squaring; no double round-trip.
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
