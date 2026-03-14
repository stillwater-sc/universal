#pragma once
// posit_scale_helpers.hpp: scale-related helper functions for posit encoding
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// These functions compute regime/scale properties from template parameters only
// (no dependency on the posit class), so they can be included before posit_impl.hpp.

namespace sw { namespace universal {

template<unsigned es>
constexpr unsigned useed_scale() {
	return (1u << es);
}

// calculate exponential scale of maxpos
template<unsigned nbits, unsigned es>
constexpr int maxpos_scale() {
	return (nbits - 2) * (1 << es);
}

// calculate exponential scale of minpos
template<unsigned nbits, unsigned es>
constexpr int minpos_scale() {
	return static_cast<int>(2 - int(nbits)) * (1 << es);
}

// calculate the constrained k value (clamped to [minpos_scale, maxpos_scale])
template<unsigned nbits, unsigned es>
constexpr int calculate_k(int scale) {
	if (scale < 0) {
		scale = scale > minpos_scale<nbits, es>() ? scale : minpos_scale<nbits, es>();
	}
	else {
		scale = scale < maxpos_scale<nbits, es>() ? scale : maxpos_scale<nbits, es>();
	}
	int k = scale < 0 ? -(-scale >> es) : (scale >> es);
	if (k == 0 && scale < 0) {
		k = -1;
	}
	return k;
}

// calculate the unconstrained k value
template<unsigned nbits, unsigned es>
constexpr int calculate_unconstrained_k(int scale) {
	int k = scale < 0 ? -(-scale >> es) : (scale >> es);
	if (k == 0 && scale < 0) {
		k = -1;
	}
	return k;
}

}} // namespace sw::universal
