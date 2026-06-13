// exponent.hpp: exponent functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> exp(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::exp(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> exp2(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::exp2(double(x)));
}

// Base-10 exponential: 10^x
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> exp10(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::pow(10.0, double(x)));
}

// exp(x) - 1, more accurate near zero
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> expm1(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::expm1(double(x)));
}

}} // namespace sw::universal
