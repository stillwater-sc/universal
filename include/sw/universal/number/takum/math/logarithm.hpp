// logarithm.hpp: logarithm functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log2(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log2(double(x)));
}

// Decimal (base-10) logarithm
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log10(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log10(double(x)));
}

// log(1 + x), more accurate near zero
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log1p(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
