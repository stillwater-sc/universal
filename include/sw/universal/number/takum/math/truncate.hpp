// truncate.hpp: truncation functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Round toward zero
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> trunc(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::trunc(double(x)));
}

// Round to nearest, halfway cases away from zero
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> round(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::round(double(x)));
}

// Round downward (floor)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> floor(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::floor(double(x)));
}

// Round upward (ceil)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> ceil(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(std::ceil(double(x)));
}

}} // namespace sw::universal
