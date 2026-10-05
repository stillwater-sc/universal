// hyperbolic.hpp: hyperbolic functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/takum/math/wide_range.hpp>
#include <universal/number/takum/math/saturate.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> sinh(const takum<nbits, rbits, bt>& x) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::sinh(x)) return *far;
	}
	return saturate_from_double<takum<nbits, rbits, bt>>(std::sinh(double(x)), !x.iszero());
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> cosh(const takum<nbits, rbits, bt>& x) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::cosh(x)) return *far;
	}
	return saturate_from_double<takum<nbits, rbits, bt>>(std::cosh(double(x)), true);
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> tanh(const takum<nbits, rbits, bt>& x) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::identity_if_tiny(x)) return *far;
	}
	return takum<nbits, rbits, bt>(std::tanh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> asinh(const takum<nbits, rbits, bt>& x) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::asinh(x)) return *far;
	}
	return takum<nbits, rbits, bt>(std::asinh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> acosh(const takum<nbits, rbits, bt>& x) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::acosh(x)) return *far;
	}
	return takum<nbits, rbits, bt>(std::acosh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> atanh(const takum<nbits, rbits, bt>& x) {
	if constexpr (!takum<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::identity_if_tiny(x)) return *far;
	}
	return takum<nbits, rbits, bt>(std::atanh(double(x)));
}

// ---------------------------------------------------------------------------
// takum_log: hyperbolic functions are sums of exponentials, and the sum is the
// linear-domain operation this representation is worst at, so no shortcut.
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> sinh(const takum_log<nbits, rbits, bt>& x) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::sinh(x)) return *far;
	}
	return saturate_from_double<takum_log<nbits, rbits, bt>>(std::sinh(double(x)), !x.iszero());
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> cosh(const takum_log<nbits, rbits, bt>& x) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::cosh(x)) return *far;
	}
	return saturate_from_double<takum_log<nbits, rbits, bt>>(std::cosh(double(x)), true);
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> tanh(const takum_log<nbits, rbits, bt>& x) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::identity_if_tiny(x)) return *far;
	}
	return takum_log<nbits, rbits, bt>(std::tanh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> asinh(const takum_log<nbits, rbits, bt>& x) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::asinh(x)) return *far;
	}
	return takum_log<nbits, rbits, bt>(std::asinh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> acosh(const takum_log<nbits, rbits, bt>& x) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::acosh(x)) return *far;
	}
	return takum_log<nbits, rbits, bt>(std::acosh(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> atanh(const takum_log<nbits, rbits, bt>& x) {
	if constexpr (!takum_log<nbits, rbits, bt>::range_fits_double) {   // #1626
		if (auto far = takum_wide_range::identity_if_tiny(x)) return *far;
	}
	return takum_log<nbits, rbits, bt>(std::atanh(double(x)));
}

}} // namespace sw::universal
