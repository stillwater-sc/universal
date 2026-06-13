// hyperbolic.hpp: hyperbolic functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> sinh(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::sinh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> cosh(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::cosh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> tanh(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::tanh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> asinh(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::asinh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> acosh(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::acosh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> atanh(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::atanh(double(x)));
}

}} // namespace sw::universal
