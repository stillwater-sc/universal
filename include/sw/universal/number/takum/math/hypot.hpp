// hypot.hpp: hypotenuse functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/takum/math/wide_range.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> hypot(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::hypot(x, y)) return *far;
	}
	return takum<nbits, rbits, bt>(std::hypot(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> hypotf(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::hypot(x, y)) return *far;
	}
	return takum<nbits, rbits, bt>(std::hypotf(float(x), float(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> hypotl(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::hypot(x, y)) return *far;
	}
	return takum<nbits, rbits, bt>(std::hypotl((long double)(x), (long double)(y)));
}

// ---------------------------------------------------------------------------
// takum_log
// ---------------------------------------------------------------------------

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> hypot(const takum_log<nbits, rbits, bt>& x,
                                         const takum_log<nbits, rbits, bt>& y) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::hypot(x, y)) return *far;
	}
	return takum_log<nbits, rbits, bt>(std::hypot(double(x), double(y)));
}

}} // namespace sw::universal
